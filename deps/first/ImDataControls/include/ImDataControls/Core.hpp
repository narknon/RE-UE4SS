#pragma once

#include <imgui.h>
#include <type_traits>
#include <utility>
#include <string>
#include <memory>
#include <functional>
#include <Helpers/String.hpp>
#include <String/StringType.hpp>
#include "Policies.hpp"
#include "PolicyInterfaces.hpp"

namespace RC::ImDataControls {

// Base drawing interface with proper StringType support
class IImGuiDrawable {
public:
    virtual ~IImGuiDrawable() = default;
    
    // std::string overload
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
    
    // Also support raw pointers
    [[nodiscard]] bool draw(const char* label) { 
        return draw_impl(label); 
    }
    
    [[nodiscard]] bool draw(std::nullptr_t) { 
        return draw_impl(nullptr); 
    }
    
    [[nodiscard]] virtual bool is_changed() const = 0;
    
protected:
    virtual bool draw_impl(const char* label) = 0;
    
private:
    mutable std::string m_label_cache;
};

// Base value storage with metadata support
template<typename T>
class BasicImGuiValue : public IValueControl, public IEditModeControl {
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

// Simple value with no policies
template<typename T>
using ImDataSimpleValue = ComposedValue<BasicImGuiValue<T>>;

// Monitored value with external sync
template<typename T>
using ImDataMonitoredValue = ComposedValue<BasicImGuiValue<T>, 
    ExternalSyncPolicy<T>, 
    ThreadSafetyPolicy<T>, 
    ValueSourcePolicy<T>, 
    ChangeNotificationPolicy<T>>;

// Monitored value with text representation
template<typename T>
using ImDataMonitoredValueWithText = ComposedValue<BasicImGuiValue<T>, 
    ExternalSyncPolicy<T>, 
    ThreadSafetyPolicy<T>, 
    ValueSourcePolicy<T>, 
    ChangeNotificationPolicy<T>,
    TextRepresentationPolicy<T>>;

// Config value with deferred updates
template<typename T>
using ImDataConfigValue = ComposedValue<BasicImGuiValue<T>, 
    DeferredUpdatePolicy<T>, 
    DefaultValuePolicy<T>, 
    ValidationPolicy<T>, 
    ValueSourcePolicy<T>, 
    ChangeNotificationPolicy<T>>;

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