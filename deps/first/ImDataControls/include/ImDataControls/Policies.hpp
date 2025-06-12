#pragma once

#include <functional>
#include <shared_mutex>
#include <optional>
#include <expected>
#include <vector>
#include <variant>
#include <concepts>
#include <imgui.h>
#include "PolicyInterfaces.hpp"

namespace RC::ImDataControls {

// Update each policy to implement its interface

template<typename T>
class DeferredUpdatePolicy : public IDeferredUpdate {
public:
    virtual ~DeferredUpdatePolicy() = default;
    // IDeferredUpdate implementation
    void apply_changes() override {
        if (has_pending_changes()) {
            set_current_value(m_pending_value.value());
            m_pending_value.reset();
            m_is_dirty = false;
        }
    }
    
    void revert_changes() override {
        m_pending_value.reset();
        m_is_dirty = false;
    }
    
    bool has_pending_changes() const override {
        return m_pending_value.has_value() && 
               m_pending_value.value() != get_current_value();
    }
    
    // Existing template methods remain...
    [[nodiscard]] const T& get_pending_value() const {
        return m_pending_value.value_or(get_current_value());
    }
    
    void set_pending_value(const T& value) {
        m_pending_value = value;
        m_is_dirty = true;
    }
    
    [[nodiscard]] bool is_dirty() const { return m_is_dirty; }
    
protected:
    virtual const T& get_current_value() const = 0;
    virtual void set_current_value(const T& value) = 0;
    
private:
    std::optional<T> m_pending_value;
    bool m_is_dirty = false;
};

template<typename T>
class ExternalSyncPolicy : public IExternalSync {
public:
    virtual ~ExternalSyncPolicy() = default;
    using Getter = std::function<T()>;
    using Setter = std::function<void(const T&)>;
    
    // IExternalSync implementation
    void sync_from_external() override {
        if (m_getter && !has_pending_user_changes()) {
            T external_value = m_getter();
            if (get_value() != external_value) {
                set_value(external_value);
                m_last_source = ValueSource::External;
            }
        }
    }
    
    void sync_to_external() override {
        if (m_setter) {
            m_setter(get_value());
        }
    }
    
    bool is_externally_synced() const override {
        return m_getter || m_setter;
    }
    
    void set_external_getter(Getter getter) { m_getter = std::move(getter); }
    void set_external_setter(Setter setter) { m_setter = std::move(setter); }
    
protected:
    virtual T& get_value() = 0;
    virtual const T& get_value() const = 0;
    virtual void set_value(const T& value) = 0;
    virtual bool has_pending_user_changes() const { return false; }
    
    Getter m_getter;
    Setter m_setter;
    ValueSource m_last_source = ValueSource::Default;
};

template<typename T>
class ValidationPolicy : public IValidatable {
public:
    virtual ~ValidationPolicy() = default;
    using Validator = std::function<std::expected<T, std::string>(const T&)>;
    
    bool validate() override {
        if constexpr (requires { get_value(); }) {
            auto result = validate(get_value());
            m_last_validation_error = result.has_value() ? "" : result.error();
            return result.has_value();
        }
        return true;
    }
    
    bool is_valid() const override {
        return m_last_validation_error.empty();
    }
    
    std::string get_validation_error() const override {
        return m_last_validation_error;
    }
    
    // Template methods
    [[nodiscard]] std::expected<T, std::string> validate(const T& value) const {
        if (m_validator) {
            return m_validator(value);
        }
        return value;
    }
    
    void set_validator(Validator validator) {
        m_validator = std::move(validator);
    }
    
protected:
    virtual const T& get_value() const = 0;
    
private:
    Validator m_validator;
    mutable std::string m_last_validation_error;
};

template<typename T>
class ValueHistoryPolicy : public IHistorical {
public:
    virtual ~ValueHistoryPolicy() = default;
    bool can_undo() const override { 
        return m_current_index > 0; 
    }
    
    bool can_redo() const override { 
        return m_current_index < m_history.size() - 1; 
    }
    
    void undo() override {
        if (can_undo()) {
            m_current_index--;
            if constexpr (requires { set_value(T{}); }) {
                set_value(m_history[m_current_index]);
            }
        }
    }
    
    void redo() override {
        if (can_redo()) {
            m_current_index++;
            if constexpr (requires { set_value(T{}); }) {
                set_value(m_history[m_current_index]);
            }
        }
    }
    
    void clear_history() override {
        m_history.clear();
        m_current_index = 0;
    }
    
    size_t history_size() const override {
        return m_history.size();
    }
    
    void push_history(const T& value) {
        if (m_current_index < m_history.size() - 1) {
            m_history.erase(m_history.begin() + m_current_index + 1, m_history.end());
        }
        m_history.push_back(value);
        m_current_index = m_history.size() - 1;
        
        if (m_history.size() > m_max_history_size) {
            m_history.erase(m_history.begin());
            m_current_index--;
        }
    }
    
protected:
    virtual void set_value(const T& value) = 0;
    
private:
    std::vector<T> m_history;
    size_t m_current_index = 0;
    size_t m_max_history_size = 100;
};

// Add new policies

template<typename T>
class TextRepresentationPolicy : public ITextRepresentation {
public:
    virtual ~TextRepresentationPolicy() = default;
    bool should_show_text_representation() const override { return m_show_text; }
    void set_show_text_representation(bool show) override { m_show_text = show; }
    
    std::string get_text_representation() const override {
        const T& value = get_value();
        
        if constexpr (std::is_same_v<T, bool>) {
            return value ? "true" : "false";
        }
        else if constexpr (std::is_integral_v<T>) {
            return std::to_string(value);
        }
        else if constexpr (std::is_floating_point_v<T>) {
            if (m_text_format.empty()) {
                char buffer[64];
                if constexpr (std::is_same_v<T, float>) {
                    snprintf(buffer, sizeof(buffer), "%.3f", value);
                } else {
                    snprintf(buffer, sizeof(buffer), "%.6f", value);
                }
                return std::string(buffer);
            } else {
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
    
    void set_text_format(const std::string& format) { m_text_format = format; }
    
    void draw_text_representation() {
        if (m_show_text && ImGui::GetCurrentContext()) {
            ImGui::SameLine();
            ImGui::TextDisabled("(%s)", get_text_representation().c_str());
        }
    }
    
protected:
    virtual const T& get_value() const = 0;
    
private:
    bool m_show_text = true;
    std::string m_text_format;
};

template<typename T>
class CallbackPolicy : public ICustomCallbacks {
public:
    virtual ~CallbackPolicy() = default;
    using TooltipCallback = std::function<void()>;
    using ContextMenuCallback = std::function<void()>;
    using ChangeCallback = std::function<void(const T&, const T&)>;
    
    void set_custom_tooltip_callback(TooltipCallback cb) override { 
        m_custom_tooltip = std::move(cb); 
    }
    
    void set_custom_context_menu_callback(ContextMenuCallback cb) override { 
        m_custom_context_menu = std::move(cb); 
    }
    
    void set_change_callback(ChangeCallback cb) {
        m_change_callback = std::move(cb);
    }
    
protected:
    void render_tooltip() {
        if (m_custom_tooltip && ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            m_custom_tooltip();
            ImGui::EndTooltip();
        } else if (!m_tooltip_text.empty() && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", m_tooltip_text.c_str());
        }
    }
    
    void render_context_menu(const char* id = nullptr) {
        if (ImGui::BeginPopupContextItem(id)) {
            if (m_custom_context_menu) {
                m_custom_context_menu();
            } else {
                if (ImGui::MenuItem("Reset to Default")) {
                    if constexpr (requires { reset_to_default(); }) {
                        reset_to_default();
                    }
                }
            }
            ImGui::EndPopup();
        }
    }
    
    void notify_change(const T& old_value, const T& new_value) {
        if (m_change_callback) {
            m_change_callback(old_value, new_value);
        }
    }
    
    void set_tooltip_text(const std::string& tooltip) { m_tooltip_text = tooltip; }
    
private:
    TooltipCallback m_custom_tooltip;
    ContextMenuCallback m_custom_context_menu;
    ChangeCallback m_change_callback;
    std::string m_tooltip_text;
};

template<typename T>
class VisibilityPolicy : public IVisibilityControl {
public:
    virtual ~VisibilityPolicy() = default;
    bool is_visible() const override { return m_visible; }
    void set_visible(bool visible) override { m_visible = visible; }
    bool is_advanced() const override { return m_is_advanced; }
    void set_advanced(bool advanced) override { m_is_advanced = advanced; }
    const std::string& get_group() const override { return m_group; }
    void set_group(const std::string& group) override { m_group = group; }
    
private:
    bool m_visible = true;
    bool m_is_advanced = false;
    std::string m_group;
};

template<typename T>
class StringConversionPolicy : public IStringConvertible {
public:
    virtual ~StringConversionPolicy() = default;
    std::string to_string() const override {
        if constexpr (std::is_same_v<T, std::string>) {
            return get_value();
        } else if constexpr (std::is_same_v<T, bool>) {
            return get_value() ? "true" : "false";
        } else if constexpr (std::is_arithmetic_v<T>) {
            return std::to_string(get_value());
        } else {
            return "<unsupported>";
        }
    }
    
    bool from_string(const std::string& str) override {
        try {
            if constexpr (std::is_same_v<T, std::string>) {
                set_value(str);
                return true;
            } else if constexpr (std::is_same_v<T, bool>) {
                set_value(str == "true" || str == "1");
                return true;
            } else if constexpr (std::is_integral_v<T>) {
                if constexpr (std::is_unsigned_v<T>) {
                    set_value(static_cast<T>(std::stoull(str)));
                } else {
                    set_value(static_cast<T>(std::stoll(str)));
                }
                return true;
            } else if constexpr (std::is_floating_point_v<T>) {
                if constexpr (std::is_same_v<T, float>) {
                    set_value(std::stof(str));
                } else {
                    set_value(std::stod(str));
                }
                return true;
            }
        } catch (...) {
            return false;
        }
        return false;
    }
    
protected:
    virtual const T& get_value() const = 0;
    virtual void set_value(const T& value) = 0;
};

template<typename T>
class ImmediateApplyPolicy : public IImmediateApply {
public:
    virtual ~ImmediateApplyPolicy() = default;
    void set_immediate_apply(bool immediate) override { m_immediate_apply = immediate; }
    bool is_immediate_apply() const override { return m_immediate_apply; }
    
protected:
    bool handle_change() {
        if (m_immediate_apply && has_pending_changes()) {
            apply_changes();
            return true;
        }
        return false;
    }
    
    virtual bool has_pending_changes() const { return false; }
    virtual void apply_changes() {}
    
private:
    bool m_immediate_apply = false;
};

// Keep existing policies unchanged
template<typename T>
class ThreadSafetyPolicy {
public:
    using SharedLock = std::shared_lock<std::shared_mutex>;
    using UniqueLock = std::unique_lock<std::shared_mutex>;
    
    [[nodiscard]] SharedLock read_lock() const {
        return SharedLock(m_mutex);
    }
    
    [[nodiscard]] UniqueLock write_lock() const {
        return UniqueLock(m_mutex);
    }
    
private:
    mutable std::shared_mutex m_mutex;
};

template<typename T>
class ValueSourcePolicy {
public:
    [[nodiscard]] ValueSource get_last_source() const { return m_last_source; }
    
protected:
    void track_source(ValueSource source) {
        m_last_source = source;
    }
    
private:
    ValueSource m_last_source = ValueSource::Default;
};

template<typename T>
class ChangeNotificationPolicy {
public:
    using ChangeCallback = std::function<void(const T& old_value, const T& new_value)>;
    
    void set_on_change_callback(ChangeCallback callback) {
        m_on_change = std::move(callback);
    }
    
protected:
    void notify_change(const T& old_value, const T& new_value) {
        if (m_on_change) {
            m_on_change(old_value, new_value);
        }
    }
    
private:
    ChangeCallback m_on_change;
};

template<typename T>
class DefaultValuePolicy {
public:
    virtual ~DefaultValuePolicy() = default;
    DefaultValuePolicy() = default;
    explicit DefaultValuePolicy(T default_value) 
        : m_default_value(std::move(default_value)) {}
    
    void set_default_value(const T& value) {
        m_default_value = value;
    }
    
    [[nodiscard]] const T& get_default_value() const {
        return m_default_value;
    }
    
    void reset_to_default() {
        reset_value(m_default_value);
    }
    
protected:
    virtual void reset_value(const T& value) = 0;
    
private:
    T m_default_value{};
};

} // namespace RC::ImDataControls