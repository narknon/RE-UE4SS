#pragma once

#include <imgui.h>
#include <type_traits>
#include <utility>
#include <string>
#include <memory>
#include <functional>
#include <Helpers/String.hpp>
#include <String/StringType.hpp>
#include "PolicyInterfaces.hpp"
#include "Policies.hpp"

namespace RC::ImDataControls {

// Concrete implementation with convenience overloads
class ImGuiDrawableBase : public IImGuiDrawable {
public:
    virtual ~ImGuiDrawableBase() = default;
    
    // IImGuiDrawable implementation
    bool draw(const char* label) override { 
        return draw_impl(label); 
    }
    
    // Additional overloads for convenience
    [[nodiscard]] bool draw(const std::string& label) { 
        return draw_impl(label.c_str()); 
    }
    
    // StringType overload (only if different from std::string)
    template<typename T = StringType>
    [[nodiscard]] std::enable_if_t<!std::is_same_v<T, std::string>, bool>
    draw(const StringType& label) {
        m_label_cache = RC::to_utf8_string(label);
        return draw_impl(m_label_cache.c_str());
    }
    
    [[nodiscard]] bool draw(std::nullptr_t) { 
        return draw_impl(nullptr); 
    }
    
protected:
    virtual bool draw_impl(const char* label) = 0;
    
private:
    mutable std::string m_label_cache;
};

// Base value storage - now inherits from ImGuiDrawableBase
template<typename T>
class BasicImGuiValue : public ImGuiDrawableBase, public IValueControl, public IEditModeControl {
public:
    using value_type = T;
    using pointer = std::unique_ptr<BasicImGuiValue<T>>;
    
    explicit BasicImGuiValue(T initial_value = T{})
        : m_value(std::move(initial_value))
        , m_changed(false)
        , m_edit_mode(EditMode::Editable)
    {}
    
    // Value access
    [[nodiscard]] T& value() { return m_value; }
    [[nodiscard]] const T& value() const { return m_value; }
    [[nodiscard]] operator T&() { return m_value; }
    [[nodiscard]] operator const T&() const { return m_value; }
    
    // Assignment
    BasicImGuiValue& operator=(const T& new_value) {
        if (m_value != new_value) {
            m_value = new_value;
            m_changed = true;
        }
        return *this;
    }
    
    [[nodiscard]] bool is_changed() const override { return m_changed; }
    void clear_changed() { m_changed = false; }
    
    // IValueControl implementation
    const std::string& get_name() const override { return m_name; }
    void set_name(const std::string& name) override { m_name = name; }
    const std::string& get_tooltip() const override { return m_tooltip; }
    void set_tooltip(const std::string& tooltip) override { m_tooltip = tooltip; }
    
    // IEditModeControl implementation
    EditMode get_edit_mode() const override { return m_edit_mode; }
    void set_edit_mode(EditMode mode) override { m_edit_mode = mode; }
    bool is_editable() const override { return m_edit_mode == EditMode::Editable; }
    
    // Default draw implementation (will be overridden by widgets)
protected:
    bool draw_impl(const char* label) override {
        return false; // Base implementation does nothing
    }
    
protected:
    T m_value;
    bool m_changed;
    std::string m_name;
    std::string m_tooltip;
    EditMode m_edit_mode;
};

// Enhanced composition helper with proper bridge methods
template<typename Base, typename... Policies>
class ComposedValue : public Base, public Policies... {
public:
    using value_type = typename Base::value_type;
    using Base::operator=;
    
    template<typename... Args>
    explicit ComposedValue(Args&&... args)
        : Base(std::forward<Args>(args)...)
        , Policies()...
    {}
    
protected:
    // Bridge methods for policies
    value_type& get_value() { return Base::value(); }
    const value_type& get_value() const { return Base::value(); }
    void set_value(const value_type& value) { 
        Base::operator=(value); 
    }
    
    // For DeferredUpdatePolicy
    const value_type& get_current_value() const { return Base::value(); }
    void set_current_value(const value_type& value) { Base::operator=(value); }
    
    // For DefaultValuePolicy
    void reset_value(const value_type& value) { Base::operator=(value); }
};

// Simple value with no additional policies
template<typename T>
using ImDataSimpleValue = ComposedValue<BasicImGuiValue<T>>;

// Enhanced composition with specialized methods for monitored values
template<typename T>
class ImDataMonitoredValue : public ComposedValue<
    BasicImGuiValue<T>,
    ExternalSyncPolicy<T>,
    ThreadSafetyPolicy<T>,
    ValueSourcePolicy<T>,
    ChangeNotificationPolicy<T>
> {
public:
    using Base = ComposedValue<
        BasicImGuiValue<T>,
        ExternalSyncPolicy<T>,
        ThreadSafetyPolicy<T>,
        ValueSourcePolicy<T>,
        ChangeNotificationPolicy<T>
    >;
    
    using typename ExternalSyncPolicy<T>::Getter;
    using typename ExternalSyncPolicy<T>::Setter;
    
    explicit ImDataMonitoredValue(T initial_value = T{})
        : Base(std::move(initial_value))
    {}
    
    ImDataMonitoredValue(Getter getter, Setter setter, T default_value = T{})
        : Base(std::move(default_value))
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(T initial_value = T{}) {
        return std::make_unique<ImDataMonitoredValue<T>>(std::move(initial_value));
    }
    
    static auto create(Getter getter, Setter setter, T default_value = T{}) {
        return std::make_unique<ImDataMonitoredValue<T>>(
            std::move(getter), std::move(setter), std::move(default_value)
        );
    }
    
    // Thread-safe operations
    [[nodiscard]] T get() const {
        auto lock = this->read_lock();
        return this->value();
    }
    
    void set(const T& new_value) {
        T old_value;
        {
            auto lock = this->write_lock();
            old_value = this->value();
            if (old_value != new_value) {
                this->operator=(new_value);
                this->track_source(ValueSource::User);
            }
        }
        if (old_value != new_value) {
            this->notify_change(old_value, new_value);
            this->sync_to_external();
        }
    }
};

// Monitored value with text representation
template<typename T>
using ImDataMonitoredValueWithText = ComposedValue<BasicImGuiValue<T>, 
    ExternalSyncPolicy<T>, 
    ThreadSafetyPolicy<T>, 
    ValueSourcePolicy<T>, 
    ChangeNotificationPolicy<T>,
    TextRepresentationPolicy<T>>;

// Similar pattern for ConfigValue
template<typename T>
class ImDataConfigValue : public ComposedValue<
    BasicImGuiValue<T>,
    DeferredUpdatePolicy<T>,
    DefaultValuePolicy<T>,
    ValidationPolicy<T>,
    ValueSourcePolicy<T>,
    ChangeNotificationPolicy<T>
> {
public:
    using Base = ComposedValue<
        BasicImGuiValue<T>,
        DeferredUpdatePolicy<T>,
        DefaultValuePolicy<T>,
        ValidationPolicy<T>,
        ValueSourcePolicy<T>,
        ChangeNotificationPolicy<T>
    >;
    
    explicit ImDataConfigValue(T default_value = T{})
        : Base(std::move(default_value))
        , DefaultValuePolicy<T>(default_value)
    {
        this->set_default_value(default_value);
    }
    
    static auto create(T default_value = T{}) {
        return std::make_unique<ImDataConfigValue<T>>(std::move(default_value));
    }
    
    // Try to set with validation
    bool try_set(const T& new_value) {
        auto result = this->validate(new_value);
        if (result.has_value()) {
            this->set_pending_value(result.value());
            this->track_source(ValueSource::User);
            return true;
        }
        return false;
    }
    
    // Get validation error for current pending value
    [[nodiscard]] std::string get_error() const {
        if (this->has_pending_changes()) {
            auto result = this->validate(this->get_pending_value());
            return result.has_value() ? "" : result.error();
        }
        return "";
    }
};

// Full value with all policies
template<typename T>
using ImDataFullValue = ComposedValue<BasicImGuiValue<T>, 
    ExternalSyncPolicy<T>, 
    ThreadSafetyPolicy<T>, 
    ValueSourcePolicy<T>, 
    ChangeNotificationPolicy<T>, 
    DeferredUpdatePolicy<T>, 
    DefaultValuePolicy<T>, 
    ValidationPolicy<T>, 
    ValueHistoryPolicy<T>,
    TextRepresentationPolicy<T>>;

} // namespace RC::ImDataControls