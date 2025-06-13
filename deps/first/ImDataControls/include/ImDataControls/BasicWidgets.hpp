#pragma once

#include "Core.hpp"
#include <imgui.h>
#include <string>
#include <algorithm>
#include <Helpers/String.hpp>
#include <String/StringType.hpp>

namespace RC::ImDataControls {
    
// Base template for widget types
template<typename ValueType, template<typename> class ComposedType>
class BasicWidget : public ComposedType<ValueType> {
public:
    using Base = ComposedType<ValueType>;
    using Base::Base;
    
    // Perfect forwarding constructor
    template<typename... Args>
    explicit BasicWidget(Args&&... args) 
        : Base(std::forward<Args>(args)...)
    {}
    
    template<RC::StringLike T>
    void set_name(T&& name) { 
        this->m_name = RC::to_utf8_string(std::forward<T>(name)); 
    }
    
    template<RC::StringLike T>
    void set_tooltip(T&& tooltip) { 
        this->m_tooltip = RC::to_utf8_string(std::forward<T>(tooltip)); 
    }
    
protected:
    const char* get_display_label(const char* override_label) const {
        return override_label ? override_label : 
               (!this->m_name.empty() ? this->m_name.c_str() : "##unnamed");
    }
    
    void begin_disabled_if_readonly() {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
    }
    
    void end_disabled_if_readonly() {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
    }
};

// ============================================================================
// Toggle (Checkbox) Widget
// ============================================================================

// Simple Toggle - No policies, just a checkbox
class ImDataSimpleToggle : public BasicWidget<bool, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(bool initial_value = false) {
        return std::make_unique<ImDataSimpleToggle>(initial_value);
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
class ImDataMonitoredToggle : public BasicWidget<bool, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(bool initial_value = false) {
        return std::make_unique<ImDataMonitoredToggle>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, bool default_value = false) {
        return std::make_unique<ImDataMonitoredToggle>(std::move(getter), std::move(setter), default_value);
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
            this->set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored Toggle With Text - With external sync and text representation
class ImDataMonitoredToggleWithText : public BasicWidget<bool, ImDataMonitoredValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(bool initial_value = false) {
        return std::make_unique<ImDataMonitoredToggleWithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, bool default_value = false) {
        return std::make_unique<ImDataMonitoredToggleWithText>(std::move(getter), std::move(setter), default_value);
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
            this->set(temp_value);
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
class ImDataConfigToggle : public BasicWidget<bool, ImDataConfigValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(bool default_value = false) {
        return std::make_unique<ImDataConfigToggle>(default_value);
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
class ImDataSimpleFloat : public BasicWidget<float, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(float initial_value = 0.0f) {
        return std::make_unique<ImDataSimpleFloat>(initial_value);
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
class ImDataMonitoredFloat : public BasicWidget<float, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(float initial_value = 0.0f) {
        return std::make_unique<ImDataMonitoredFloat>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, float default_value = 0.0f) {
        return std::make_unique<ImDataMonitoredFloat>(std::move(getter), std::move(setter), default_value);
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
            this->set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored Float With Text - With external sync and text representation
class ImDataMonitoredFloatWithText : public BasicWidget<float, ImDataMonitoredValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(float initial_value = 0.0f) {
        return std::make_unique<ImDataMonitoredFloatWithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, float default_value = 0.0f) {
        return std::make_unique<ImDataMonitoredFloatWithText>(std::move(getter), std::move(setter), default_value);
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
            this->set(temp_value);
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
class ImDataConfigFloat : public BasicWidget<float, ImDataConfigValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(float default_value = 0.0f) {
        return std::make_unique<ImDataConfigFloat>(default_value);
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
class ImDataSimpleDouble : public BasicWidget<double, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(double initial_value = 0.0) {
        return std::make_unique<ImDataSimpleDouble>(initial_value);
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
class ImDataMonitoredDouble : public BasicWidget<double, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(double initial_value = 0.0) {
        return std::make_unique<ImDataMonitoredDouble>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, double default_value = 0.0) {
        return std::make_unique<ImDataMonitoredDouble>(std::move(getter), std::move(setter), default_value);
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
            this->set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored Double With Text - With external sync and text representation
class ImDataMonitoredDoubleWithText : public BasicWidget<double, ImDataMonitoredValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(double initial_value = 0.0) {
        return std::make_unique<ImDataMonitoredDoubleWithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, double default_value = 0.0) {
        return std::make_unique<ImDataMonitoredDoubleWithText>(std::move(getter), std::move(setter), default_value);
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
            this->set(temp_value);
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
class ImDataConfigDouble : public BasicWidget<double, ImDataConfigValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(double default_value = 0.0) {
        return std::make_unique<ImDataConfigDouble>(default_value);
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
class ImDataSimpleInt32 : public BasicWidget<int32_t, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int32_t initial_value = 0) {
        return std::make_unique<ImDataSimpleInt32>(initial_value);
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
class ImDataMonitoredInt32 : public BasicWidget<int32_t, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int32_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredInt32>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int32_t default_value = 0) {
        return std::make_unique<ImDataMonitoredInt32>(std::move(getter), std::move(setter), default_value);
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
            this->set(temp_value);
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored Int32 With Text - With external sync and text representation
class ImDataMonitoredInt32WithText : public BasicWidget<int32_t, ImDataMonitoredValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int32_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredInt32WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int32_t default_value = 0) {
        return std::make_unique<ImDataMonitoredInt32WithText>(std::move(getter), std::move(setter), default_value);
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
            this->set(temp_value);
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
class ImDataConfigInt32 : public BasicWidget<int32_t, ImDataConfigValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int32_t default_value = 0) {
        return std::make_unique<ImDataConfigInt32>(default_value);
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
class ImDataSimpleString : public BasicWidget<std::string, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(const std::string& initial_value = "") {
        return std::make_unique<ImDataSimpleString>(initial_value);
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
        if (s_buffer.size() < m_value.size() + 256) {
            s_buffer.resize(m_value.size() + 256);
        }
        std::copy(m_value.begin(), m_value.end(), s_buffer.begin());
        s_buffer[m_value.size()] = '\0';
        
        bool changed = ImGui::InputText(label ? label : "##string", s_buffer.data(), s_buffer.size());
        if (changed) {
            m_value = std::string(s_buffer.data());
            m_changed = true;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
    
private:
    inline static thread_local std::vector<char> s_buffer;
};

// Monitored String - With external sync
class ImDataMonitoredString : public BasicWidget<std::string, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(const std::string& initial_value = "") {
        return std::make_unique<ImDataMonitoredString>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, const std::string& default_value = "") {
        return std::make_unique<ImDataMonitoredString>(std::move(getter), std::move(setter), default_value);
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
        if (s_buffer.size() < m_value.size() + 256) {
            s_buffer.resize(m_value.size() + 256);
        }
        std::copy(m_value.begin(), m_value.end(), s_buffer.begin());
        s_buffer[m_value.size()] = '\0';
        
        bool changed = ImGui::InputText(label ? label : "##string", s_buffer.data(), s_buffer.size());
        
        if (changed && is_editable()) {
            this->set(std::string(s_buffer.data()));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
    
private:
    inline static thread_local std::vector<char> s_buffer;
};

// Config String - With validation and deferred updates
class ImDataConfigString : public BasicWidget<std::string, ImDataConfigValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(const std::string& default_value = "") {
        return std::make_unique<ImDataConfigString>(default_value);
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
        if (s_buffer.size() < current_value.size() + 256) {
            s_buffer.resize(current_value.size() + 256);
        }
        std::copy(current_value.begin(), current_value.end(), s_buffer.begin());
        s_buffer[current_value.size()] = '\0';
        
        bool changed = ImGui::InputText(label ? label : "##string", s_buffer.data(), s_buffer.size());
        
        if (changed && is_editable()) {
            try_set(std::string(s_buffer.data()));
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
    inline static thread_local std::vector<char> s_buffer;
};

// ============================================================================
// Int64 Widget (string-based for full range)
// ============================================================================

// Simple Int64 - No policies
class ImDataSimpleInt64 : public BasicWidget<int64_t, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int64_t initial_value = 0) {
        return std::make_unique<ImDataSimpleInt64>(initial_value);
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
        RC::String::safe_string_copy(buffer, m_buffer.c_str());
        
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
class ImDataMonitoredInt64 : public BasicWidget<int64_t, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int64_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredInt64>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int64_t default_value = 0) {
        return std::make_unique<ImDataMonitoredInt64>(std::move(getter), std::move(setter), default_value);
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
        RC::String::safe_string_copy(buffer, m_buffer.c_str());
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##int64", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                int64_t new_value = std::stoll(buffer);
                if (new_value != m_value && is_editable()) {
                    this->set(new_value);
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
class ImDataMonitoredInt64WithText : public BasicWidget<int64_t, ImDataMonitoredValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(int64_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredInt64WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int64_t default_value = 0) {
        return std::make_unique<ImDataMonitoredInt64WithText>(std::move(getter), std::move(setter), default_value);
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
        RC::String::safe_string_copy(buffer, m_buffer.c_str());
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##int64", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                int64_t new_value = std::stoll(buffer);
                if (new_value != m_value && is_editable()) {
                    this->set(new_value);
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
class ImDataSimpleUInt8 : public BasicWidget<uint8_t, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint8_t initial_value = 0) {
        return std::make_unique<ImDataSimpleUInt8>(initial_value);
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
class ImDataMonitoredUInt8 : public BasicWidget<uint8_t, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint8_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredUInt8>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint8_t default_value = 0) {
        return std::make_unique<ImDataMonitoredUInt8>(std::move(getter), std::move(setter), default_value);
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
            this->set(static_cast<uint8_t>(temp_value));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored UInt8 With Text
class ImDataMonitoredUInt8WithText : public BasicWidget<uint8_t, ImDataMonitoredValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint8_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredUInt8WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint8_t default_value = 0) {
        return std::make_unique<ImDataMonitoredUInt8WithText>(std::move(getter), std::move(setter), default_value);
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
            this->set(static_cast<uint8_t>(temp_value));
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
class ImDataSimpleUInt16 : public BasicWidget<uint16_t, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint16_t initial_value = 0) {
        return std::make_unique<ImDataSimpleUInt16>(initial_value);
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
class ImDataMonitoredUInt16 : public BasicWidget<uint16_t, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint16_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredUInt16>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint16_t default_value = 0) {
        return std::make_unique<ImDataMonitoredUInt16>(std::move(getter), std::move(setter), default_value);
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
            this->set(static_cast<uint16_t>(temp_value));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
};

// Monitored UInt16 With Text
class ImDataMonitoredUInt16WithText : public BasicWidget<uint16_t, ImDataMonitoredValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint16_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredUInt16WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint16_t default_value = 0) {
        return std::make_unique<ImDataMonitoredUInt16WithText>(std::move(getter), std::move(setter), default_value);
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
            this->set(static_cast<uint16_t>(temp_value));
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
class ImDataSimpleUInt32 : public BasicWidget<uint32_t, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint32_t initial_value = 0) {
        return std::make_unique<ImDataSimpleUInt32>(initial_value);
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
        RC::String::safe_string_copy(buffer, m_buffer.c_str());
        
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
class ImDataMonitoredUInt32 : public BasicWidget<uint32_t, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint32_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredUInt32>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint32_t default_value = 0) {
        return std::make_unique<ImDataMonitoredUInt32>(std::move(getter), std::move(setter), default_value);
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
        RC::String::safe_string_copy(buffer, m_buffer.c_str());
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##uint32", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                uint64_t new_value = std::stoull(buffer);
                if (new_value <= UINT32_MAX && new_value != m_value && is_editable()) {
                    this->set(static_cast<uint32_t>(new_value));
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
class ImDataMonitoredUInt32WithText : public BasicWidget<uint32_t, ImDataMonitoredValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint32_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredUInt32WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint32_t default_value = 0) {
        return std::make_unique<ImDataMonitoredUInt32WithText>(std::move(getter), std::move(setter), default_value);
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
        RC::String::safe_string_copy(buffer, m_buffer.c_str());
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##uint32", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                uint64_t new_value = std::stoull(buffer);
                if (new_value <= UINT32_MAX && new_value != m_value && is_editable()) {
                    this->set(static_cast<uint32_t>(new_value));
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
class ImDataSimpleUInt64 : public BasicWidget<uint64_t, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint64_t initial_value = 0) {
        return std::make_unique<ImDataSimpleUInt64>(initial_value);
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
        RC::String::safe_string_copy(buffer, m_buffer.c_str());
        
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
class ImDataMonitoredUInt64 : public BasicWidget<uint64_t, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint64_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredUInt64>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint64_t default_value = 0) {
        return std::make_unique<ImDataMonitoredUInt64>(std::move(getter), std::move(setter), default_value);
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
        RC::String::safe_string_copy(buffer, m_buffer.c_str());
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##uint64", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                uint64_t new_value = std::stoull(buffer);
                if (new_value != m_value && is_editable()) {
                    this->set(new_value);
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
class ImDataMonitoredUInt64WithText : public BasicWidget<uint64_t, ImDataMonitoredValueWithText> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(uint64_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredUInt64WithText>(initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint64_t default_value = 0) {
        return std::make_unique<ImDataMonitoredUInt64WithText>(std::move(getter), std::move(setter), default_value);
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
        RC::String::safe_string_copy(buffer, m_buffer.c_str());
        
        bool changed = false;
        if (ImGui::InputText(label ? label : "##uint64", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsDecimal)) {
            try {
                uint64_t new_value = std::stoull(buffer);
                if (new_value != m_value && is_editable()) {
                    this->set(new_value);
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
class ImDataSimpleTextMultiline : public BasicWidget<std::string, ImDataSimpleValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(const std::string& initial_value = "", const ImVec2& size = ImVec2(-1, 0)) {
        auto widget = std::make_unique<ImDataSimpleTextMultiline>(initial_value);
        widget->m_size = size;
        return widget;
    }
    
    void set_size(const ImVec2& size) { m_size = size; }
    [[nodiscard]] const ImVec2& get_size() const { return m_size; }
    
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
        if (s_buffer.size() < m_value.size() + 1024) {
            s_buffer.resize(m_value.size() + 1024);
        }
        std::copy(m_value.begin(), m_value.end(), s_buffer.begin());
        s_buffer[m_value.size()] = '\0';
        
        bool changed = ImGui::InputTextMultiline(label ? label : "##textmultiline", 
                                                 s_buffer.data(), s_buffer.size(), m_size);
        if (changed) {
            m_value = std::string(s_buffer.data());
            m_changed = true;
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
    
private:
    inline static thread_local std::vector<char> s_buffer;
    ImVec2 m_size;
};

// Monitored TextMultiline - With external sync
class ImDataMonitoredTextMultiline : public BasicWidget<std::string, ImDataMonitoredValue> {
public:
    using Base = BasicWidget;
    using Base::Base;
    
    static auto create(const std::string& initial_value = "", const ImVec2& size = ImVec2(-1, 0)) {
        auto widget = std::make_unique<ImDataMonitoredTextMultiline>(initial_value);
        widget->m_size = size;
        return widget;
    }
    
    static auto create(Getter getter, Setter setter, const std::string& default_value = "", const ImVec2& size = ImVec2(-1, 0)) {
        auto widget = std::make_unique<ImDataMonitoredTextMultiline>(std::move(getter), std::move(setter), default_value);
        widget->m_size = size;
        return widget;
    }
    
    void set_size(const ImVec2& size) { m_size = size; }
    [[nodiscard]] const ImVec2& get_size() const { return m_size; }
    
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
        if (s_buffer.size() < m_value.size() + 1024) {
            s_buffer.resize(m_value.size() + 1024);
        }
        std::copy(m_value.begin(), m_value.end(), s_buffer.begin());
        s_buffer[m_value.size()] = '\0';
        
        bool changed = ImGui::InputTextMultiline(label ? label : "##textmultiline", 
                                                 s_buffer.data(), s_buffer.size(), m_size);
        
        if (changed && is_editable()) {
            this->set(std::string(s_buffer.data()));
        }
        
        if (m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && is_editable();
    }
    
private:
    inline static thread_local std::vector<char> s_buffer;
    ImVec2 m_size;
};

} // namespace RC::ImDataControls