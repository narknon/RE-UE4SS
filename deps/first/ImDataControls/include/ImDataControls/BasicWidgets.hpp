#pragma once

#include "Core.hpp"
#include <imgui.h>
#include <string>
#include <algorithm>

namespace RC::ImDataControls {

// Edit modes for controlling widget interaction
enum class EditMode {
    Editable,   // Normal user-editable control
    ReadOnly,   // Displays value with disabled controls (grayed out)
    ViewOnly    // Minimal text-only display
};

// Base template for widget types
template<typename ValueType, template<typename> class ComposedType>
class BasicWidget : public ComposedType<ValueType> {
public:
    using Base = ComposedType<ValueType>;
    using Base::Base;
    
    // Edit mode control
    void set_edit_mode(EditMode mode) { m_edit_mode = mode; }
    [[nodiscard]] EditMode get_edit_mode() const { return m_edit_mode; }
    [[nodiscard]] bool is_editable() const { return m_edit_mode == EditMode::Editable; }
    
protected:
    EditMode m_edit_mode = EditMode::Editable;
};

// ============================================================================
// Toggle (Checkbox) Widget
// ============================================================================

// Simple Toggle - No policies, just a checkbox
class SimpleToggle : public BasicWidget<bool, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(bool initial_value = false) {
        return std::make_unique<SimpleToggle>(initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %s", label ? label : "", m_value ? "true" : "false");
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool changed = ImGui::Checkbox(label ? label : "##toggle", &m_value);
        if (changed) {
            m_changed = true;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
};

// Monitored Toggle - With external sync
class MonitoredToggle : public BasicWidget<bool, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(bool initial_value = false) {
        return std::make_unique<MonitoredToggle>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, bool default_value = false) {
        return std::make_unique<MonitoredToggle>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %s", label ? label : "", m_value ? "true" : "false");
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool temp_value = m_value;
        bool changed = ImGui::Checkbox(label ? label : "##toggle", &temp_value);
        
        if (changed && is_editable()) {
            set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored Toggle With Text - With external sync and text representation
class MonitoredToggleWithText : public BasicWidget<bool, MonitoredImGuiValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(bool initial_value = false) {
        return std::make_unique<MonitoredToggleWithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, bool default_value = false) {
        return std::make_unique<MonitoredToggleWithText>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %s", label ? label : "", m_value ? "true" : "false");
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool temp_value = m_value;
        bool changed = ImGui::Checkbox(label ? label : "##toggle", &temp_value);
        
        if (changed && is_editable()) {
            set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation (automatically enabled by default)
        this->draw_text_representation();
        
        return changed && is_editable();
    }
};

// Config Toggle - With validation and deferred updates
class ConfigToggle : public BasicWidget<bool, ConfigImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(bool default_value = false) {
        return std::make_unique<ConfigToggle>(default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            bool display_value = has_pending_changes() ? get_pending_value() : m_value;
            ImGui::Text("%s: %s", label ? label : "", display_value ? "true" : "false");
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool temp_value = has_pending_changes() ? get_pending_value() : m_value;
        bool changed = ImGui::Checkbox(label ? label : "##toggle", &temp_value);
        
        if (changed && is_editable()) {
            try_set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Show pending indicator
        if (has_pending_changes() && is_editable()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Value has pending changes");
            }
        }
        
        return changed && is_editable();
    }
};

// ============================================================================
// Float Widget
// ============================================================================

// Simple Float - No policies
class SimpleFloat : public BasicWidget<float, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(float initial_value = 0.0f) {
        return std::make_unique<SimpleFloat>(initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.3f", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool changed = ImGui::InputFloat(label ? label : "##float", &m_value);
        if (changed) {
            m_changed = true;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
};

// Monitored Float - With external sync
class MonitoredFloat : public BasicWidget<float, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(float initial_value = 0.0f) {
        return std::make_unique<MonitoredFloat>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, float default_value = 0.0f) {
        return std::make_unique<MonitoredFloat>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.3f", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        float temp_value = m_value;
        bool changed = ImGui::InputFloat(label ? label : "##float", &temp_value);
        
        if (changed && is_editable()) {
            set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored Float With Text - With external sync and text representation
class MonitoredFloatWithText : public BasicWidget<float, MonitoredImGuiValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(float initial_value = 0.0f) {
        return std::make_unique<MonitoredFloatWithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, float default_value = 0.0f) {
        return std::make_unique<MonitoredFloatWithText>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.3f", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        float temp_value = m_value;
        bool changed = ImGui::InputFloat(label ? label : "##float", &temp_value);
        
        if (changed && is_editable()) {
            set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation (automatically enabled by default)
        this->draw_text_representation();
        
        return changed && is_editable();
    }
};

// Config Float - With validation and deferred updates
class ConfigFloat : public BasicWidget<float, ConfigImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(float default_value = 0.0f) {
        return std::make_unique<ConfigFloat>(default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            float display_value = has_pending_changes() ? get_pending_value() : m_value;
            ImGui::Text("%s: %.3f", label ? label : "", display_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        float temp_value = has_pending_changes() ? get_pending_value() : m_value;
        bool changed = ImGui::InputFloat(label ? label : "##float", &temp_value);
        
        if (changed && is_editable()) {
            try_set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Show pending indicator
        if (has_pending_changes() && is_editable()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Value has pending changes");
            }
        }
        
        // Show validation error if any
        auto error = get_error();
        if (!error.empty()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(!)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", error.c_str());
            }
        }
        
        return changed && is_editable();
    }
};

// ============================================================================
// Double Widget
// ============================================================================

// Simple Double - No policies
class SimpleDouble : public BasicWidget<double, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(double initial_value = 0.0) {
        return std::make_unique<SimpleDouble>(initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.6f", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // ImGui doesn't have InputDouble, so we use InputScalar
        bool changed = ImGui::InputDouble(label ? label : "##double", &m_value);
        if (changed) {
            m_changed = true;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
};

// Monitored Double - With external sync
class MonitoredDouble : public BasicWidget<double, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(double initial_value = 0.0) {
        return std::make_unique<MonitoredDouble>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, double default_value = 0.0) {
        return std::make_unique<MonitoredDouble>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.6f", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        double temp_value = m_value;
        bool changed = ImGui::InputDouble(label ? label : "##double", &temp_value);
        
        if (changed && is_editable()) {
            set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored Double With Text - With external sync and text representation
class MonitoredDoubleWithText : public BasicWidget<double, MonitoredImGuiValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(double initial_value = 0.0) {
        return std::make_unique<MonitoredDoubleWithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, double default_value = 0.0) {
        return std::make_unique<MonitoredDoubleWithText>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.6f", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        double temp_value = m_value;
        bool changed = ImGui::InputDouble(label ? label : "##double", &temp_value);
        
        if (changed && is_editable()) {
            set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation (automatically enabled by default)
        this->draw_text_representation();
        
        return changed && is_editable();
    }
};

// Config Double - With validation and deferred updates
class ConfigDouble : public BasicWidget<double, ConfigImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(double default_value = 0.0) {
        return std::make_unique<ConfigDouble>(default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            double display_value = has_pending_changes() ? get_pending_value() : m_value;
            ImGui::Text("%s: %.6f", label ? label : "", display_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        double temp_value = has_pending_changes() ? get_pending_value() : m_value;
        bool changed = ImGui::InputDouble(label ? label : "##double", &temp_value);
        
        if (changed && is_editable()) {
            try_set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Show pending indicator
        if (has_pending_changes() && is_editable()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Value has pending changes");
            }
        }
        
        return changed && is_editable();
    }
};

// ============================================================================
// Int32 Widget
// ============================================================================

// Simple Int32 - No policies
class SimpleInt32 : public BasicWidget<int32_t, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int32_t initial_value = 0) {
        return std::make_unique<SimpleInt32>(initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %d", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool changed = ImGui::InputInt(label ? label : "##int32", &m_value);
        if (changed) {
            m_changed = true;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
};

// Monitored Int32 - With external sync
class MonitoredInt32 : public BasicWidget<int32_t, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int32_t initial_value = 0) {
        return std::make_unique<MonitoredInt32>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int32_t default_value = 0) {
        return std::make_unique<MonitoredInt32>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %d", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int32_t temp_value = m_value;
        bool changed = ImGui::InputInt(label ? label : "##int32", &temp_value);
        
        if (changed && is_editable()) {
            set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored Int32 With Text - With external sync and text representation
class MonitoredInt32WithText : public BasicWidget<int32_t, MonitoredImGuiValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int32_t initial_value = 0) {
        return std::make_unique<MonitoredInt32WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int32_t default_value = 0) {
        return std::make_unique<MonitoredInt32WithText>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %d", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int32_t temp_value = m_value;
        bool changed = ImGui::InputInt(label ? label : "##int32", &temp_value);
        
        if (changed && is_editable()) {
            set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation (automatically enabled by default)
        this->draw_text_representation();
        
        return changed && is_editable();
    }
};

// Config Int32 - With validation and deferred updates
class ConfigInt32 : public BasicWidget<int32_t, ConfigImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int32_t default_value = 0) {
        return std::make_unique<ConfigInt32>(default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            int32_t display_value = has_pending_changes() ? get_pending_value() : m_value;
            ImGui::Text("%s: %d", label ? label : "", display_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int32_t temp_value = has_pending_changes() ? get_pending_value() : m_value;
        bool changed = ImGui::InputInt(label ? label : "##int32", &temp_value);
        
        if (changed && is_editable()) {
            try_set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Show pending indicator
        if (has_pending_changes() && is_editable()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Value has pending changes");
            }
        }
        
        return changed && is_editable();
    }
};

// ============================================================================
// String Widget
// ============================================================================

// Simple String - No policies
class SimpleString : public BasicWidget<std::string, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(const std::string& initial_value = "") {
        return std::make_unique<SimpleString>(initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %s", label ? label : "", m_value.c_str());
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Ensure buffer is large enough
        if (m_buffer.size() < m_value.size() + 256) {
            m_buffer.resize(m_value.size() + 256);
        }
        std::copy(m_value.begin(), m_value.end(), m_buffer.begin());
        m_buffer[m_value.size()] = '\0';
        
        bool changed = ImGui::InputText(label ? label : "##string", m_buffer.data(), m_buffer.size());
        if (changed) {
            m_value = std::string(m_buffer.data());
            m_changed = true;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
    
private:
    mutable std::vector<char> m_buffer;
};

// Monitored String - With external sync
class MonitoredString : public BasicWidget<std::string, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(const std::string& initial_value = "") {
        return std::make_unique<MonitoredString>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, const std::string& default_value = "") {
        return std::make_unique<MonitoredString>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %s", label ? label : "", m_value.c_str());
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Ensure buffer is large enough
        if (m_buffer.size() < m_value.size() + 256) {
            m_buffer.resize(m_value.size() + 256);
        }
        std::copy(m_value.begin(), m_value.end(), m_buffer.begin());
        m_buffer[m_value.size()] = '\0';
        
        bool changed = ImGui::InputText(label ? label : "##string", m_buffer.data(), m_buffer.size());
        
        if (changed && is_editable()) {
            set(std::string(m_buffer.data()));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
    
private:
    mutable std::vector<char> m_buffer;
};

// Config String - With validation and deferred updates
class ConfigString : public BasicWidget<std::string, ConfigImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(const std::string& default_value = "") {
        return std::make_unique<ConfigString>(default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            const std::string& display_value = has_pending_changes() ? get_pending_value() : m_value;
            ImGui::Text("%s: %s", label ? label : "", display_value.c_str());
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        const std::string& current_value = has_pending_changes() ? get_pending_value() : m_value;
        
        // Ensure buffer is large enough
        if (m_buffer.size() < current_value.size() + 256) {
            m_buffer.resize(current_value.size() + 256);
        }
        std::copy(current_value.begin(), current_value.end(), m_buffer.begin());
        m_buffer[current_value.size()] = '\0';
        
        bool changed = ImGui::InputText(label ? label : "##string", m_buffer.data(), m_buffer.size());
        
        if (changed && is_editable()) {
            try_set(std::string(m_buffer.data()));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Show pending indicator
        if (has_pending_changes() && is_editable()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Value has pending changes");
            }
        }
        
        return changed && is_editable();
    }
    
private:
    mutable std::vector<char> m_buffer;
};

// ============================================================================
// Int64 Widget (string-based for full range)
// ============================================================================

// Simple Int64 - No policies
class SimpleInt64 : public BasicWidget<int64_t, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int64_t initial_value = 0) {
        return std::make_unique<SimpleInt64>(initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %lld", label ? label : "", static_cast<long long>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Convert to string for editing
        if (m_buffer.empty()) {
            m_buffer = std::to_string(m_value);
        }
        
        char buffer[32];
#ifdef _WIN32
    strncpy_s(buffer, sizeof(buffer), m_buffer.c_str(), sizeof(buffer) - 1);
#else
    strncpy(buffer, m_buffer.c_str(), sizeof(buffer) - 1);
#endif
        buffer[sizeof(buffer) - 1] = '\0';
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##int64", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                int64_t new_value = std::stoll(buffer);
                if (new_value != m_value) {
                    m_value = new_value;
                    m_changed = true;
                    changed = true;
                }
                m_buffer = buffer;
            } catch (...) {
                // Invalid input, keep old value
            }
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
    
private:
    mutable std::string m_buffer;
};

// Monitored Int64 - With external sync
class MonitoredInt64 : public BasicWidget<int64_t, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int64_t initial_value = 0) {
        return std::make_unique<MonitoredInt64>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int64_t default_value = 0) {
        return std::make_unique<MonitoredInt64>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %lld", label ? label : "", static_cast<long long>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Convert to string for editing
        if (m_buffer.empty() || m_external_updated) {
            m_buffer = std::to_string(m_value);
            m_external_updated = false;
        }
        
        char buffer[32];
#ifdef _WIN32
    strncpy_s(buffer, sizeof(buffer), m_buffer.c_str(), sizeof(buffer) - 1);
#else
    strncpy(buffer, m_buffer.c_str(), sizeof(buffer) - 1);
#endif
        buffer[sizeof(buffer) - 1] = '\0';
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##int64", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                int64_t new_value = std::stoll(buffer);
                if (new_value != m_value && is_editable()) {
                    set(new_value);
                    changed = true;
                }
                m_buffer = buffer;
            } catch (...) {
                // Invalid input, keep old value
            }
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
    
    void sync_from_external() override {
        Base::sync_from_external();
        m_external_updated = true;
    }
    
private:
    mutable std::string m_buffer;
    mutable bool m_external_updated = false;
};

// Monitored Int64 With Text
class MonitoredInt64WithText : public BasicWidget<int64_t, MonitoredImGuiValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int64_t initial_value = 0) {
        return std::make_unique<MonitoredInt64WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int64_t default_value = 0) {
        return std::make_unique<MonitoredInt64WithText>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %lld", label ? label : "", static_cast<long long>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Convert to string for editing
        if (m_buffer.empty() || m_external_updated) {
            m_buffer = std::to_string(m_value);
            m_external_updated = false;
        }
        
        char buffer[32];
#ifdef _WIN32
    strncpy_s(buffer, sizeof(buffer), m_buffer.c_str(), sizeof(buffer) - 1);
#else
    strncpy(buffer, m_buffer.c_str(), sizeof(buffer) - 1);
#endif
        buffer[sizeof(buffer) - 1] = '\0';
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##int64", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                int64_t new_value = std::stoll(buffer);
                if (new_value != m_value && is_editable()) {
                    set(new_value);
                    changed = true;
                }
                m_buffer = buffer;
            } catch (...) {
                // Invalid input, keep old value
            }
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation
        this->draw_text_representation();
        
        return changed && is_editable();
    }
    
    void sync_from_external() override {
        Base::sync_from_external();
        m_external_updated = true;
    }
    
private:
    mutable std::string m_buffer;
    mutable bool m_external_updated = false;
};

// ============================================================================
// UInt8 Widget
// ============================================================================

// Simple UInt8 - No policies
class SimpleUInt8 : public BasicWidget<uint8_t, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint8_t initial_value = 0) {
        return std::make_unique<SimpleUInt8>(initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Use int for ImGui, constrain to uint8 range
        int temp_value = static_cast<int>(m_value);
        bool changed = ImGui::InputInt(label ? label : "##uint8", &temp_value);
        
        if (changed) {
            temp_value = std::clamp(temp_value, 0, 255);
            m_value = static_cast<uint8_t>(temp_value);
            m_changed = true;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
};

// Monitored UInt8 - With external sync
class MonitoredUInt8 : public BasicWidget<uint8_t, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint8_t initial_value = 0) {
        return std::make_unique<MonitoredUInt8>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint8_t default_value = 0) {
        return std::make_unique<MonitoredUInt8>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int temp_value = static_cast<int>(m_value);
        bool changed = ImGui::InputInt(label ? label : "##uint8", &temp_value);
        
        if (changed && is_editable()) {
            temp_value = std::clamp(temp_value, 0, 255);
            set(static_cast<uint8_t>(temp_value));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored UInt8 With Text
class MonitoredUInt8WithText : public BasicWidget<uint8_t, MonitoredImGuiValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint8_t initial_value = 0) {
        return std::make_unique<MonitoredUInt8WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint8_t default_value = 0) {
        return std::make_unique<MonitoredUInt8WithText>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int temp_value = static_cast<int>(m_value);
        bool changed = ImGui::InputInt(label ? label : "##uint8", &temp_value);
        
        if (changed && is_editable()) {
            temp_value = std::clamp(temp_value, 0, 255);
            set(static_cast<uint8_t>(temp_value));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation
        this->draw_text_representation();
        
        return changed && is_editable();
    }
};

// ============================================================================
// UInt16 Widget
// ============================================================================

// Simple UInt16 - No policies
class SimpleUInt16 : public BasicWidget<uint16_t, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint16_t initial_value = 0) {
        return std::make_unique<SimpleUInt16>(initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Use int for ImGui, constrain to uint16 range
        int temp_value = static_cast<int>(m_value);
        bool changed = ImGui::InputInt(label ? label : "##uint16", &temp_value);
        
        if (changed) {
            temp_value = std::clamp(temp_value, 0, 65535);
            m_value = static_cast<uint16_t>(temp_value);
            m_changed = true;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
};

// Monitored UInt16 - With external sync
class MonitoredUInt16 : public BasicWidget<uint16_t, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint16_t initial_value = 0) {
        return std::make_unique<MonitoredUInt16>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint16_t default_value = 0) {
        return std::make_unique<MonitoredUInt16>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int temp_value = static_cast<int>(m_value);
        bool changed = ImGui::InputInt(label ? label : "##uint16", &temp_value);
        
        if (changed && is_editable()) {
            temp_value = std::clamp(temp_value, 0, 65535);
            set(static_cast<uint16_t>(temp_value));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored UInt16 With Text
class MonitoredUInt16WithText : public BasicWidget<uint16_t, MonitoredImGuiValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint16_t initial_value = 0) {
        return std::make_unique<MonitoredUInt16WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint16_t default_value = 0) {
        return std::make_unique<MonitoredUInt16WithText>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int temp_value = static_cast<int>(m_value);
        bool changed = ImGui::InputInt(label ? label : "##uint16", &temp_value);
        
        if (changed && is_editable()) {
            temp_value = std::clamp(temp_value, 0, 65535);
            set(static_cast<uint16_t>(temp_value));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation
        this->draw_text_representation();
        
        return changed && is_editable();
    }
};

// ============================================================================
// UInt32 Widget (string-based for full range)
// ============================================================================

// Simple UInt32 - No policies
class SimpleUInt32 : public BasicWidget<uint32_t, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint32_t initial_value = 0) {
        return std::make_unique<SimpleUInt32>(initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Convert to string for editing
        if (m_buffer.empty()) {
            m_buffer = std::to_string(m_value);
        }
        
        char buffer[16];
#ifdef _WIN32
    strncpy_s(buffer, sizeof(buffer), m_buffer.c_str(), sizeof(buffer) - 1);
#else
    strncpy(buffer, m_buffer.c_str(), sizeof(buffer) - 1);
#endif
        buffer[sizeof(buffer) - 1] = '\0';
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##uint32", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                uint64_t new_value = std::stoull(buffer);
                if (new_value <= UINT32_MAX && new_value != m_value) {
                    m_value = static_cast<uint32_t>(new_value);
                    m_changed = true;
                    changed = true;
                }
                m_buffer = buffer;
            } catch (...) {
                // Invalid input, keep old value
            }
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
    
private:
    mutable std::string m_buffer;
};

// Monitored UInt32 - With external sync
class MonitoredUInt32 : public BasicWidget<uint32_t, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint32_t initial_value = 0) {
        return std::make_unique<MonitoredUInt32>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint32_t default_value = 0) {
        return std::make_unique<MonitoredUInt32>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Convert to string for editing
        if (m_buffer.empty() || m_external_updated) {
            m_buffer = std::to_string(m_value);
            m_external_updated = false;
        }
        
        char buffer[16];
#ifdef _WIN32
    strncpy_s(buffer, sizeof(buffer), m_buffer.c_str(), sizeof(buffer) - 1);
#else
    strncpy(buffer, m_buffer.c_str(), sizeof(buffer) - 1);
#endif
        buffer[sizeof(buffer) - 1] = '\0';
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##uint32", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                uint64_t new_value = std::stoull(buffer);
                if (new_value <= UINT32_MAX && new_value != m_value && is_editable()) {
                    set(static_cast<uint32_t>(new_value));
                    changed = true;
                }
                m_buffer = buffer;
            } catch (...) {
                // Invalid input, keep old value
            }
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
    
    void sync_from_external() override {
        Base::sync_from_external();
        m_external_updated = true;
    }
    
private:
    mutable std::string m_buffer;
    mutable bool m_external_updated = false;
};

// Monitored UInt32 With Text
class MonitoredUInt32WithText : public BasicWidget<uint32_t, MonitoredImGuiValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint32_t initial_value = 0) {
        return std::make_unique<MonitoredUInt32WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint32_t default_value = 0) {
        return std::make_unique<MonitoredUInt32WithText>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", m_value);
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Convert to string for editing
        if (m_buffer.empty() || m_external_updated) {
            m_buffer = std::to_string(m_value);
            m_external_updated = false;
        }
        
        char buffer[16];
#ifdef _WIN32
    strncpy_s(buffer, sizeof(buffer), m_buffer.c_str(), sizeof(buffer) - 1);
#else
    strncpy(buffer, m_buffer.c_str(), sizeof(buffer) - 1);
#endif
        buffer[sizeof(buffer) - 1] = '\0';
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##uint32", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                uint64_t new_value = std::stoull(buffer);
                if (new_value <= UINT32_MAX && new_value != m_value && is_editable()) {
                    set(static_cast<uint32_t>(new_value));
                    changed = true;
                }
                m_buffer = buffer;
            } catch (...) {
                // Invalid input, keep old value
            }
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation
        this->draw_text_representation();
        
        return changed && is_editable();
    }
    
    void sync_from_external() override {
        Base::sync_from_external();
        m_external_updated = true;
    }
    
private:
    mutable std::string m_buffer;
    mutable bool m_external_updated = false;
};

// ============================================================================
// UInt64 Widget (string-based for full range)
// ============================================================================

// Simple UInt64 - No policies
class SimpleUInt64 : public BasicWidget<uint64_t, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint64_t initial_value = 0) {
        return std::make_unique<SimpleUInt64>(initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %llu", label ? label : "", static_cast<unsigned long long>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Convert to string for editing
        if (m_buffer.empty()) {
            m_buffer = std::to_string(m_value);
        }
        
        char buffer[32];
#ifdef _WIN32
    strncpy_s(buffer, sizeof(buffer), m_buffer.c_str(), sizeof(buffer) - 1);
#else
    strncpy(buffer, m_buffer.c_str(), sizeof(buffer) - 1);
#endif
        buffer[sizeof(buffer) - 1] = '\0';
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##uint64", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                uint64_t new_value = std::stoull(buffer);
                if (new_value != m_value) {
                    m_value = new_value;
                    m_changed = true;
                    changed = true;
                }
                m_buffer = buffer;
            } catch (...) {
                // Invalid input, keep old value
            }
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
    
private:
    mutable std::string m_buffer;
};

// Monitored UInt64 - With external sync
class MonitoredUInt64 : public BasicWidget<uint64_t, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint64_t initial_value = 0) {
        return std::make_unique<MonitoredUInt64>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint64_t default_value = 0) {
        return std::make_unique<MonitoredUInt64>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %llu", label ? label : "", static_cast<unsigned long long>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Convert to string for editing
        if (m_buffer.empty() || m_external_updated) {
            m_buffer = std::to_string(m_value);
            m_external_updated = false;
        }
        
        char buffer[32];
#ifdef _WIN32
    strncpy_s(buffer, sizeof(buffer), m_buffer.c_str(), sizeof(buffer) - 1);
#else
    strncpy(buffer, m_buffer.c_str(), sizeof(buffer) - 1);
#endif
        buffer[sizeof(buffer) - 1] = '\0';
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##uint64", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                uint64_t new_value = std::stoull(buffer);
                if (new_value != m_value && is_editable()) {
                    set(new_value);
                    changed = true;
                }
                m_buffer = buffer;
            } catch (...) {
                // Invalid input, keep old value
            }
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
    
    void sync_from_external() override {
        Base::sync_from_external();
        m_external_updated = true;
    }
    
private:
    mutable std::string m_buffer;
    mutable bool m_external_updated = false;
};

// Monitored UInt64 With Text
class MonitoredUInt64WithText : public BasicWidget<uint64_t, MonitoredImGuiValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint64_t initial_value = 0) {
        return std::make_unique<MonitoredUInt64WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint64_t default_value = 0) {
        return std::make_unique<MonitoredUInt64WithText>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %llu", label ? label : "", static_cast<unsigned long long>(m_value));
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Convert to string for editing
        if (m_buffer.empty() || m_external_updated) {
            m_buffer = std::to_string(m_value);
            m_external_updated = false;
        }
        
        char buffer[32];
#ifdef _WIN32
    strncpy_s(buffer, sizeof(buffer), m_buffer.c_str(), sizeof(buffer) - 1);
#else
    strncpy(buffer, m_buffer.c_str(), sizeof(buffer) - 1);
#endif
        buffer[sizeof(buffer) - 1] = '\0';
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##uint64", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                uint64_t new_value = std::stoull(buffer);
                if (new_value != m_value && is_editable()) {
                    set(new_value);
                    changed = true;
                }
                m_buffer = buffer;
            } catch (...) {
                // Invalid input, keep old value
            }
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation
        this->draw_text_representation();
        
        return changed && is_editable();
    }
    
    void sync_from_external() override {
        Base::sync_from_external();
        m_external_updated = true;
    }
    
private:
    mutable std::string m_buffer;
    mutable bool m_external_updated = false;
};

// ============================================================================
// TextMultiline Widget
// ============================================================================

// Simple TextMultiline - No policies
class SimpleTextMultiline : public BasicWidget<std::string, SimpleImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(const std::string& initial_value = "", const ImVec2& size = ImVec2(-1, 0)) {
        auto widget = std::make_unique<SimpleTextMultiline>(initial_value);
        widget->m_size = size;
        return widget;
    }
    
    void set_size(const ImVec2& size) { m_size = size; }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s:", label ? label : "");
            ImGui::TextWrapped("%s", m_value.c_str());
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Ensure buffer is large enough
        if (m_buffer.size() < m_value.size() + 1024) {
            m_buffer.resize(m_value.size() + 1024);
        }
        std::copy(m_value.begin(), m_value.end(), m_buffer.begin());
        m_buffer[m_value.size()] = '\0';
        
        bool changed = ImGui::InputTextMultiline(label ? label : "##textmultiline", 
                                                 m_buffer.data(), m_buffer.size(), m_size);
        if (changed) {
            m_value = std::string(m_buffer.data());
            m_changed = true;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
    
private:
    mutable std::vector<char> m_buffer;
    ImVec2 m_size;
};

// Monitored TextMultiline - With external sync
class MonitoredTextMultiline : public BasicWidget<std::string, MonitoredImGuiValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(const std::string& initial_value = "", const ImVec2& size = ImVec2(-1, 0)) {
        auto widget = std::make_unique<MonitoredTextMultiline>(initial_value);
        widget->m_size = size;
        return widget;
    }
    
    static auto create(Getter getter, Setter setter, const std::string& default_value = "", const ImVec2& size = ImVec2(-1, 0)) {
        auto widget = std::make_unique<MonitoredTextMultiline>(std::move(getter), std::move(setter), default_value);
        widget->m_size = size;
        return widget;
    }
    
    void set_size(const ImVec2& size) { m_size = size; }
    
protected:
    bool draw_impl(const char* label) override {
        if (m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s:", label ? label : "");
            ImGui::TextWrapped("%s", m_value.c_str());
            return false;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Ensure buffer is large enough
        if (m_buffer.size() < m_value.size() + 1024) {
            m_buffer.resize(m_value.size() + 1024);
        }
        std::copy(m_value.begin(), m_value.end(), m_buffer.begin());
        m_buffer[m_value.size()] = '\0';
        
        bool changed = ImGui::InputTextMultiline(label ? label : "##textmultiline", 
                                                 m_buffer.data(), m_buffer.size(), m_size);
        
        if (changed && is_editable()) {
            set(std::string(m_buffer.data()));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
    
private:
    mutable std::vector<char> m_buffer;
    ImVec2 m_size;
};

// Type aliases for compatibility
using ImGuiToggle = SimpleToggle;
using ImGuiFloat = SimpleFloat;
using ImGuiDouble = SimpleDouble;
using ImGuiInt32 = SimpleInt32;
using ImGuiInt64 = SimpleInt64;
using ImGuiUInt8 = SimpleUInt8;
using ImGuiUInt16 = SimpleUInt16;
using ImGuiUInt32 = SimpleUInt32;
using ImGuiUInt64 = SimpleUInt64;
using ImGuiString = SimpleString;
using ImGuiTextMultiline = SimpleTextMultiline;

} // namespace RC::ImDataControls