#pragma once

#include "Core.hpp"
#include "Policies.hpp"
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
    using Base = BasicWidget<bool, SimpleImGuiValue>;
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

// Monitored Toggle - With external sync and text representation
class MonitoredToggle : public BasicWidget<bool, MonitoredImGuiValue>, public TextRepresentationPolicy<bool> {
public:
    using Base = BasicWidget<bool, MonitoredImGuiValue>;
    using Base::Base;
    
    static auto create(bool initial_value = false) {
        return std::make_unique<MonitoredToggle>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, bool default_value = false) {
        return std::make_unique<MonitoredToggle>(std::move(getter), std::move(setter), default_value);
    }
    
protected:
    // Bridge method for TextRepresentationPolicy
    [[nodiscard]] const bool& get_value() const override { return this->m_value; }
    
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
        
        // Draw text representation if enabled
        draw_text_representation();
        
        return changed && is_editable();
    }
};

// Config Toggle - With validation and deferred updates
class ConfigToggle : public BasicWidget<bool, ConfigImGuiValue> {
public:
    using Base = BasicWidget<bool, ConfigImGuiValue>;
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
    using Base = BasicWidget<float, SimpleImGuiValue>;
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
    using Base = BasicWidget<float, MonitoredImGuiValue>;
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

// Config Float - With validation and deferred updates
class ConfigFloat : public BasicWidget<float, ConfigImGuiValue> {
public:
    using Base = BasicWidget<float, ConfigImGuiValue>;
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
    using Base = BasicWidget<double, SimpleImGuiValue>;
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
    using Base = BasicWidget<double, MonitoredImGuiValue>;
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

// Config Double - With validation and deferred updates
class ConfigDouble : public BasicWidget<double, ConfigImGuiValue> {
public:
    using Base = BasicWidget<double, ConfigImGuiValue>;
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
    using Base = BasicWidget<int32_t, SimpleImGuiValue>;
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
    using Base = BasicWidget<int32_t, MonitoredImGuiValue>;
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

// Config Int32 - With validation and deferred updates
class ConfigInt32 : public BasicWidget<int32_t, ConfigImGuiValue> {
public:
    using Base = BasicWidget<int32_t, ConfigImGuiValue>;
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
    using Base = BasicWidget<std::string, SimpleImGuiValue>;
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
    using Base = BasicWidget<std::string, MonitoredImGuiValue>;
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
    using Base = BasicWidget<std::string, ConfigImGuiValue>;
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

// Type aliases for compatibility
using ImGuiToggle = SimpleToggle;
using ImGuiFloat = SimpleFloat;
using ImGuiDouble = SimpleDouble;
using ImGuiInt32 = SimpleInt32;
using ImGuiString = SimpleString;

} // namespace RC::ImDataControls