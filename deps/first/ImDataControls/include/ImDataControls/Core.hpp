#pragma once

#include <imgui.h>
#include <type_traits>
#include <utility>
#include <string>
#include <memory>
#include <functional>
#include <Helpers/String.hpp>
#include <Helpers/StringCache.hpp>
#include <String/StringType.hpp>
#include "PolicyInterfaces.hpp"
#include "Policies.hpp"

namespace RC::ImDataControls {

// Update ImGuiDrawableBase implementation
class ImGuiDrawableBase : public virtual IImGuiDrawable {
public:
    virtual ~ImGuiDrawableBase() = default;
    
    // IImGuiDrawable implementation (required by interface)
    bool draw(const char* label) override { 
        return draw_impl(label); 
    }
    
    /**
     * Draw the widget with the given label.
     * 
     * @param label The label to display. Any string-like type is accepted.
     * 
     * Thread Safety: This method is thread-safe with respect to the widget's
     * internal state. However, if 'label' refers to data shared between threads,
     * the caller must ensure no other thread modifies it during this call.
     * 
     * Performance: Non-UTF8 strings are automatically cached for efficiency.
     * String literals (const char*) have zero overhead.
     */
    template<RC::StringLike T>
    [[nodiscard]] bool draw(T&& label) {
        if constexpr (std::is_same_v<std::decay_t<T>, const char*>) {
            return draw_impl(label);  // Zero overhead for string literals
        } else {
            // Use high-performance cache for all other string types
            const char* cached_label = RC::GetThreadLocalStringCache().convert(std::forward<T>(label));
            return draw_impl(cached_label);
        }
    }
    
    // Special handling for nullptr
    [[nodiscard]] bool draw(std::nullptr_t) { 
        return draw_impl(nullptr); 
    }
    
protected:
    virtual bool draw_impl(const char* label) = 0;
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
    Capabilities get_capabilities() const override {
        return Capabilities{}; // All false by default
    }
    
    const std::string& get_name() const override { return m_name; }
    void set_name(const std::string& name) override { m_name = name; }
    const std::string& get_tooltip() const override { return m_tooltip; }
    void set_tooltip(const std::string& tooltip) override { m_tooltip = tooltip; }
    
    // Add concept-based string setters alongside the virtual ones
    template<RC::StringLike U>
    void set_name(U&& name) { 
        m_name = RC::to_utf8_string(std::forward<U>(name)); 
    }
    
    template<RC::StringLike U>
    void set_tooltip(U&& tooltip) { 
        m_tooltip = RC::to_utf8_string(std::forward<U>(tooltip)); 
    }
    
    // IEditModeControl implementation
    EditMode get_edit_mode() const override { return m_edit_mode; }
    void set_edit_mode(EditMode mode) override { m_edit_mode = mode; }
    bool is_editable() const override { return m_edit_mode == EditMode::Editable; }

    bool draw(const char* label) override {
        // This is the final overrider for IImGuiDrawable::draw.
        return this->draw_impl(label);
    }
    
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
    
    // Automatically build capabilities from included policies at compile time
    Capabilities get_capabilities() const override {
        Capabilities caps;
        
        // Use fold expression to check each policy
        ([&]{
            if constexpr (std::is_base_of_v<IDeferredUpdate, Policies>) { 
                caps.has_deferred_update = true; 
            }
            if constexpr (std::is_base_of_v<IExternalSync, Policies>) { 
                caps.has_external_sync = true; 
            }
            if constexpr (std::is_base_of_v<IValidatable, Policies>) { 
                caps.has_validation = true; 
            }
            if constexpr (std::is_base_of_v<IHistorical, Policies>) { 
                caps.has_history = true; 
            }
            if constexpr (std::is_base_of_v<IVisibilityControl, Policies>) { 
                caps.has_visibility = true; 
            }
            if constexpr (std::is_base_of_v<IStringConvertible, Policies>) { 
                caps.has_string_conversion = true; 
            }
            if constexpr (std::is_base_of_v<ITextRepresentation, Policies>) { 
                caps.has_text_representation = true; 
            }
            if constexpr (std::is_base_of_v<ICustomCallbacks, Policies>) { 
                caps.has_custom_callbacks = true; 
            }
            if constexpr (std::is_base_of_v<IImmediateApply, Policies>) { 
                caps.has_immediate_apply = true; 
            }
        }(), ...);
        
        return caps;
    }
    
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

    using Base::operator=;
    
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
class ImDataMonitoredValueWithText : public ComposedValue<
    BasicImGuiValue<T>,
    ExternalSyncPolicy<T>,
    ThreadSafetyPolicy<T>,
    ValueSourcePolicy<T>,
    ChangeNotificationPolicy<T>,
    TextRepresentationPolicy<T>
> {
public:
    using Base = ComposedValue<
        BasicImGuiValue<T>,
        ExternalSyncPolicy<T>,
        ThreadSafetyPolicy<T>,
        ValueSourcePolicy<T>,
        ChangeNotificationPolicy<T>,
        TextRepresentationPolicy<T>
    >;

    using Base::operator=;

    using typename ExternalSyncPolicy<T>::Getter;
    using typename ExternalSyncPolicy<T>::Setter;

    explicit ImDataMonitoredValueWithText(T initial_value = T{})
        : Base(std::move(initial_value))
    {}

    ImDataMonitoredValueWithText(Getter getter, Setter setter, T default_value = T{})
        : Base(std::move(default_value))
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }

    static auto create(T initial_value = T{}) {
        return std::make_unique<ImDataMonitoredValueWithText<T>>(std::move(initial_value));
    }

    static auto create(Getter getter, Setter setter, T default_value = T{}) {
        return std::make_unique<ImDataMonitoredValueWithText<T>>(
            std::move(getter), std::move(setter), std::move(default_value)
        );
    }

    // Thread-safe operations (same as ImDataMonitoredValue)
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