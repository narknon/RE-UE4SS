#pragma once

#include <functional>
#include <shared_mutex>
#include <optional>
#include <expected>
#include <vector>
#include <variant>
#include <concepts>
#include <imgui.h>

namespace RC::ImDataControls {

// Forward declarations
enum class ValueSource {
    User,      // User changed via UI
    External,  // External system (e.g., game engine)
    Default,   // Initial/default value
    Config     // Loaded from configuration
};

// Policy 1: External Synchronization
template<typename T>
class ExternalSyncPolicy {
public:
    using value_type = T;
    using Getter = std::function<T()>;
    using Setter = std::function<void(const T&)>;
    
    void set_external_getter(Getter getter) { m_getter = std::move(getter); }
    void set_external_setter(Setter setter) { m_setter = std::move(setter); }
    
    void sync_from_external() {
        if (m_getter && !has_pending_user_changes()) {
            T external_value = m_getter();
            if (get_value() != external_value) {
                set_value(external_value);
                m_last_source = ValueSource::External;
            }
        }
    }
    
    void sync_to_external() {
        if (m_setter) {
            m_setter(get_value());
        }
    }
    
protected:
    // These methods must be provided by the composed class
    virtual T& get_value() = 0;
    virtual const T& get_value() const = 0;
    virtual void set_value(const T& value) = 0;
    virtual bool has_pending_user_changes() const { return false; }
    
    Getter m_getter;
    Setter m_setter;
    ValueSource m_last_source = ValueSource::Default;
};

// Policy 2: Thread Safety
template<typename T>
class ThreadSafetyPolicy {
public:
    using value_type = T;
    
    class ReadLock {
    public:
        explicit ReadLock(const ThreadSafetyPolicy& policy) 
            : m_lock(policy.m_mutex) {}
        
    private:
        std::shared_lock<std::shared_mutex> m_lock;
    };
    
    class WriteLock {
    public:
        explicit WriteLock(ThreadSafetyPolicy& policy)
            : m_lock(policy.m_mutex) {}
            
    private:
        std::unique_lock<std::shared_mutex> m_lock;
    };
    
    [[nodiscard]] ReadLock read_lock() const { return ReadLock(*this); }
    [[nodiscard]] WriteLock write_lock() { return WriteLock(*this); }
    
protected:
    mutable std::shared_mutex m_mutex;
};

// Policy 3: Value Source Tracking
template<typename T>
class ValueSourcePolicy {
public:
    using value_type = T;
    
    [[nodiscard]] ValueSource get_last_source() const { return m_last_source; }
    
protected:
    void track_source(ValueSource source) { m_last_source = source; }
    
private:
    ValueSource m_last_source = ValueSource::Default;
};

// Policy 4: Change Notification
template<typename T>
class ChangeNotificationPolicy {
public:
    using value_type = T;
    using ChangeCallback = std::function<void(const T& old_value, const T& new_value)>;
    using SimpleChangeCallback = std::function<void()>;
    
    void add_change_listener(ChangeCallback callback) {
        m_change_callbacks.push_back(std::move(callback));
    }
    
    void add_simple_change_listener(SimpleChangeCallback callback) {
        m_simple_callbacks.push_back(std::move(callback));
    }
    
    void clear_change_listeners() {
        m_change_callbacks.clear();
        m_simple_callbacks.clear();
    }
    
protected:
    void notify_change(const T& old_value, const T& new_value) {
        for (const auto& callback : m_change_callbacks) {
            callback(old_value, new_value);
        }
        for (const auto& callback : m_simple_callbacks) {
            callback();
        }
    }
    
private:
    std::vector<ChangeCallback> m_change_callbacks;
    std::vector<SimpleChangeCallback> m_simple_callbacks;
};

// Policy 5: Deferred Update
template<typename T>
class DeferredUpdatePolicy {
public:
    using value_type = T;
    
    [[nodiscard]] bool has_pending_changes() const {
        return m_pending_value.has_value() && 
               m_pending_value.value() != get_current_value();
    }
    
    [[nodiscard]] const T& get_pending_value() const {
        return m_pending_value.value_or(get_current_value());
    }
    
    void set_pending_value(const T& value) {
        m_pending_value = value;
        m_is_dirty = true;
    }
    
    void apply_changes() {
        if (has_pending_changes()) {
            set_current_value(m_pending_value.value());
            m_pending_value.reset();
            m_is_dirty = false;
        }
    }
    
    void revert_changes() {
        m_pending_value.reset();
        m_is_dirty = false;
    }
    
    [[nodiscard]] bool is_dirty() const { return m_is_dirty; }
    
protected:
    // These must be provided by the composed class
    virtual const T& get_current_value() const = 0;
    virtual void set_current_value(const T& value) = 0;
    
private:
    std::optional<T> m_pending_value;
    bool m_is_dirty = false;
};

// Policy 6: Default Value
template<typename T>
class DefaultValuePolicy {
public:
    using value_type = T;
    
    explicit DefaultValuePolicy(T default_value = T{})
        : m_default_value(std::move(default_value)) {}
    
    [[nodiscard]] const T& get_default_value() const { return m_default_value; }
    
    void reset_to_default() {
        reset_value(m_default_value);
    }
    
protected:
    // Must be provided by composed class
    virtual void reset_value(const T& value) = 0;
    
private:
    T m_default_value;
};

// Policy 7: Validation
template<typename T>
class ValidationPolicy {
public:
    using value_type = T;
    using Validator = std::function<std::expected<T, std::string>(const T&)>;
    
    void set_validator(Validator validator) {
        m_validator = std::move(validator);
    }
    
    [[nodiscard]] std::expected<T, std::string> validate(const T& value) const {
        if (m_validator) {
            return m_validator(value);
        }
        return value; // No validation, accept any value
    }
    
    [[nodiscard]] bool is_valid(const T& value) const {
        return validate(value).has_value();
    }
    
    [[nodiscard]] std::string get_validation_error(const T& value) const {
        auto result = validate(value);
        return result.has_value() ? "" : result.error();
    }
    
private:
    Validator m_validator;
};

// Policy 8: Text Representation Display
template<typename T>
class TextRepresentationPolicy {
public:
    using value_type = T;
    
    void set_show_text_representation(bool show) { m_show_text = show; }
    [[nodiscard]] bool should_show_text_representation() const { return m_show_text; }
    
    void set_text_format(const std::string& format) { m_text_format = format; }
    [[nodiscard]] const std::string& get_text_format() const { return m_text_format; }
    
    // Get text representation of current value
    [[nodiscard]] std::string get_text_representation() const {
        const T& value = get_value();
        
        if constexpr (std::is_same_v<T, bool>) {
            return value ? "true" : "false";
        }
        else if constexpr (std::is_integral_v<T>) {
            return std::to_string(value);
        }
        else if constexpr (std::is_floating_point_v<T>) {
            if (m_text_format.empty()) {
                // Default format for floating point
                char buffer[64];
                if constexpr (std::is_same_v<T, float>) {
                    snprintf(buffer, sizeof(buffer), "%.3f", value);
                } else {
                    snprintf(buffer, sizeof(buffer), "%.6f", value);
                }
                return std::string(buffer);
            } else {
                // Use custom format string
                char buffer[128];
                snprintf(buffer, sizeof(buffer), m_text_format.c_str(), value);
                return std::string(buffer);
            }
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return value;
        }
        else {
            return "<unsupported type>";
        }
    }
    
    // Draw text representation after the widget
    void draw_text_representation() {
        if (m_show_text && ImGui::GetCurrentContext()) {
            ImGui::SameLine();
            ImGui::TextDisabled("(%s)", get_text_representation().c_str());
        }
    }
    
protected:
    // Bridge to get current value from composed class
    [[nodiscard]] virtual const T& get_value() const = 0;
    
private:
    bool m_show_text = false;
    std::string m_text_format;  // Optional printf-style format string
};

// Policy 9: Value History (Undo/Redo)
template<typename T>
class ValueHistoryPolicy {
public:
    using value_type = T;
    
    void push_history(const T& value) {
        // Remove any redo history when pushing new value
        if (m_current_index < m_history.size() - 1) {
            m_history.erase(m_history.begin() + m_current_index + 1, m_history.end());
        }
        
        m_history.push_back(value);
        m_current_index = m_history.size() - 1;
        
        // Limit history size
        if (m_history.size() > m_max_history_size) {
            m_history.erase(m_history.begin());
            m_current_index--;
        }
    }
    
    [[nodiscard]] bool can_undo() const { return m_current_index > 0; }
    [[nodiscard]] bool can_redo() const { return m_current_index < m_history.size() - 1; }
    
    [[nodiscard]] std::optional<T> undo() {
        if (can_undo()) {
            m_current_index--;
            return m_history[m_current_index];
        }
        return std::nullopt;
    }
    
    [[nodiscard]] std::optional<T> redo() {
        if (can_redo()) {
            m_current_index++;
            return m_history[m_current_index];
        }
        return std::nullopt;
    }
    
    void clear_history() {
        m_history.clear();
        m_current_index = 0;
    }
    
    void set_max_history_size(size_t size) { m_max_history_size = size; }
    
private:
    std::vector<T> m_history;
    size_t m_current_index = 0;
    size_t m_max_history_size = 100;
};

// Concept to check if a type is a policy
template<typename P>
concept IsPolicy = requires {
    typename P::value_type;
};

// Helper to extract value type from policies
template<typename... Policies>
struct PolicyValueType {
    using type = typename std::tuple_element_t<0, std::tuple<Policies...>>::value_type;
};

} // namespace RC::ImDataControls