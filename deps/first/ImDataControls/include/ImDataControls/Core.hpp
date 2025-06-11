#pragma once

#include <imgui.h>
#include <type_traits>
#include <utility>
#include <string>
#include <memory>
#include <Helpers/String.hpp>  // For to_utf8_string
#include "Policies.hpp"

namespace RC::ImDataControls {

// Forward declarations
template<typename T> class SimpleImGuiValue;
template<typename T> class MonitoredImGuiValue;
template<typename T> class MonitoredImGuiValueWithText;
template<typename T> class ConfigImGuiValue;
template<typename T> class FullImGuiValue;

// Base interface with template string handling
class IImGuiDrawable {
public:
    virtual ~IImGuiDrawable() = default;
    
    // Template method accepts any string type (uses RC::to_utf8_string internally)
    template<typename T>
    [[nodiscard]] bool draw(T&& label) {
        // Use existing string helpers to convert to UTF-8
        m_label_cache = RC::to_utf8_string(std::forward<T>(label));
        return draw_impl(m_label_cache.c_str());
    }
    
    // Overload for nullptr
    [[nodiscard]] bool draw(std::nullptr_t) {
        return draw_impl(nullptr);
    }
    
    // Overload for const char* (no conversion needed)
    [[nodiscard]] bool draw(const char* label) {
        return draw_impl(label);
    }
    
    [[nodiscard]] virtual bool is_changed() const = 0;
    
protected:
    // Derived classes only implement const char* version
    virtual bool draw_impl(const char* label) = 0;
    
private:
    mutable std::string m_label_cache;  // Cache for converted strings
};

// Basic value storage without bells and whistles
template<typename T>
class BasicImGuiValue : public IImGuiDrawable {
public:
    using value_type = T;
    using pointer = std::unique_ptr<BasicImGuiValue<T>>;
    
    explicit BasicImGuiValue(T initial_value = T{})
        : m_value(std::move(initial_value))
        , m_changed(false)
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
    
protected:
    T m_value;
    bool m_changed;
};

// Policy-based composition helper
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
    // Bridge methods for policies to access base value
    value_type& get_value() { return Base::value(); }
    const value_type& get_value() const { return Base::value(); }
    void set_value(const value_type& value) { Base::operator=(value); }
    
    // For DeferredUpdatePolicy
    const value_type& get_current_value() const { return Base::value(); }
    void set_current_value(const value_type& value) { Base::operator=(value); }
    
    // For DefaultValuePolicy
    void reset_value(const value_type& value) { Base::operator=(value); }
};

// SimpleImGuiValue - Just BasicImGuiValue with no policies (zero overhead)
template<typename T>
class SimpleImGuiValue : public BasicImGuiValue<T> {
public:
    using Base = BasicImGuiValue<T>;
    using Base::Base;  // Inherit constructors
    
    static auto create(T initial_value = T{}) {
        return std::make_unique<SimpleImGuiValue<T>>(std::move(initial_value));
    }
};

// MonitoredImGuiValue - For external synchronization with thread safety
template<typename T>
class MonitoredImGuiValue : public ComposedValue<
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
    
    explicit MonitoredImGuiValue(T initial_value = T{})
        : Base(std::move(initial_value))
    {}
    
    MonitoredImGuiValue(Getter getter, Setter setter, T default_value = T{})
        : Base(std::move(default_value))
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(T initial_value = T{}) {
        return std::make_unique<MonitoredImGuiValue<T>>(std::move(initial_value));
    }
    
    static auto create(Getter getter, Setter setter, T default_value = T{}) {
        return std::make_unique<MonitoredImGuiValue<T>>(
            std::move(getter), std::move(setter), std::move(default_value)
        );
    }
    
    // Thread-safe value access
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
    
    // Refresh from external source
    void refresh() {
        this->sync_from_external();
    }
};

// MonitoredImGuiValueWithText - For external synchronization with text representation
template<typename T>
class MonitoredImGuiValueWithText : public ComposedValue<
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
    
    explicit MonitoredImGuiValueWithText(T initial_value = T{})
        : Base(std::move(initial_value))
    {}
    
    MonitoredImGuiValueWithText(Getter getter, Setter setter, T default_value = T{})
        : Base(std::move(default_value))
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(T initial_value = T{}) {
        return std::make_unique<MonitoredImGuiValueWithText<T>>(std::move(initial_value));
    }
    
    static auto create(Getter getter, Setter setter, T default_value = T{}) {
        return std::make_unique<MonitoredImGuiValueWithText<T>>(
            std::move(getter), std::move(setter), std::move(default_value)
        );
    }
    
    // Thread-safe value access
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
    
    // Refresh from external source
    void refresh() {
        this->sync_from_external();
    }
    
protected:
    // Bridge method for TextRepresentationPolicy
    [[nodiscard]] const T& get_value() const override { return this->value(); }
};

// ConfigImGuiValue - For configuration with validation and deferred updates
template<typename T>
class ConfigImGuiValue : public ComposedValue<
    BasicImGuiValue<T>,
    DeferredUpdatePolicy<T>,
    DefaultValuePolicy<T>,
    ValidationPolicy<T>,
    ValueSourcePolicy<T>
> {
public:
    using Base = ComposedValue<
        BasicImGuiValue<T>,
        DeferredUpdatePolicy<T>,
        DefaultValuePolicy<T>,
        ValidationPolicy<T>,
        ValueSourcePolicy<T>
    >;
    
    explicit ConfigImGuiValue(T default_value = T{})
        : Base(std::move(default_value))
    {
        this->set_default_value(default_value); 
    }
    
    static auto create(T default_value = T{}) {
        return std::make_unique<ConfigImGuiValue<T>>(std::move(default_value));
    }
    
    // Override to validate before setting
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
            return this->get_validation_error(this->get_pending_value());
        }
        return "";
    }
};

// FullImGuiValue - All policies for maximum functionality
template<typename T>
class FullImGuiValue : public ComposedValue<
    BasicImGuiValue<T>,
    ExternalSyncPolicy<T>,
    ThreadSafetyPolicy<T>,
    ValueSourcePolicy<T>,
    ChangeNotificationPolicy<T>,
    DeferredUpdatePolicy<T>,
    DefaultValuePolicy<T>,
    ValidationPolicy<T>,
    ValueHistoryPolicy<T>
> {
public:
    using Base = ComposedValue<
        BasicImGuiValue<T>,
        ExternalSyncPolicy<T>,
        ThreadSafetyPolicy<T>,
        ValueSourcePolicy<T>,
        ChangeNotificationPolicy<T>,
        DeferredUpdatePolicy<T>,
        DefaultValuePolicy<T>,
        ValidationPolicy<T>,
        ValueHistoryPolicy<T>
    >;
    
    using typename ExternalSyncPolicy<T>::Getter;
    using typename ExternalSyncPolicy<T>::Setter;
    
    explicit FullImGuiValue(T default_value = T{})
        : Base(std::move(default_value))
        , DefaultValuePolicy<T>(default_value)
    {
        this->push_history(default_value);
    }
    
    static auto create(T default_value = T{}) {
        return std::make_unique<FullImGuiValue<T>>(std::move(default_value));
    }
    
    // Thread-safe operations with all features
    void set_with_validation(const T& new_value) {
        auto lock = this->write_lock();
        auto result = this->validate(new_value);
        if (result.has_value()) {
            T old_value = this->value();
            this->set_pending_value(result.value());
            this->track_source(ValueSource::User);
            this->push_history(result.value());
            lock.unlock(); // Release lock before notifications
            this->notify_change(old_value, result.value());
        }
    }
    
    // Apply changes with history tracking
    void apply_with_history() {
        if (this->has_pending_changes()) {
            auto lock = this->write_lock();
            T old_value = this->value();
            this->apply_changes();
            this->push_history(this->value());
            lock.unlock();
            this->notify_change(old_value, this->value());
            this->sync_to_external();
        }
    }
};

} // namespace RC::ImDataControls