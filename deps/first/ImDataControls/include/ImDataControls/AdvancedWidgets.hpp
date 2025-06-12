#pragma once

#include "BasicWidgets.hpp"
#include <imgui.h>
#include <limits>
#include <cstdint>

namespace RC::ImDataControls {

// ============================================================================
// Slider Widgets (with min/max parameters)
// ============================================================================

// Base template for slider widgets
template<typename ValueType, template<typename> class ComposedType>
class SliderWidget : public BasicWidget<ValueType, ComposedType> {
public:
    using Base = BasicWidget<ValueType, ComposedType>;
    
    SliderWidget(ValueType min_val, ValueType max_val, ValueType initial_value)
        : Base(initial_value)
        , m_min(min_val)
        , m_max(max_val)
    {}
    
    void set_range(ValueType min_val, ValueType max_val) {
        m_min = min_val;
        m_max = max_val;
    }
    
    [[nodiscard]] ValueType get_min() const { return m_min; }
    [[nodiscard]] ValueType get_max() const { return m_max; }
    
protected:
    ValueType m_min;
    ValueType m_max;
};

// ============================================================================
// Float Slider
// ============================================================================

// Simple Float Slider
class ImDataSimpleSliderFloat : public SliderWidget<float, ImDataSimpleValue> {
public:
    using Base = SliderWidget;
    
    ImDataSimpleSliderFloat(float min_val, float max_val, float initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    static auto create(float min_val, float max_val, float initial_value = 0.0f) {
        return std::make_unique<ImDataSimpleSliderFloat>(min_val, max_val, initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.3f", label ? label : "", this->m_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool changed = ImGui::SliderFloat(label ? label : "##slider", &this->m_value, m_min, m_max);
        if (changed) {
            this->m_changed = true;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
};

// Monitored Float Slider
class ImDataMonitoredSliderFloat : public SliderWidget<float, ImDataMonitoredValue> {
public:
    using Base = SliderWidget;
    using typename Base::Getter;
    using typename Base::Setter;
    
    ImDataMonitoredSliderFloat(float min_val, float max_val, float initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    ImDataMonitoredSliderFloat(Getter getter, Setter setter, float min_val, float max_val, float default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(float min_val, float max_val, float initial_value = 0.0f) {
        return std::make_unique<ImDataMonitoredSliderFloat>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, float min_val, float max_val, float default_value = 0.0f) {
        return std::make_unique<ImDataMonitoredSliderFloat>(std::move(getter), std::move(setter), min_val, max_val, default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.3f", label ? label : "", this->m_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        float temp_value = this->m_value;
        bool changed = ImGui::SliderFloat(label ? label : "##slider", &temp_value, m_min, m_max);
        
        if (changed && this->is_editable()) {
            this->set(temp_value);
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && this->is_editable();
    }
};

// Monitored Float Slider With Text
class ImDataMonitoredSliderFloatWithText : public SliderWidget<float, ImDataMonitoredValueWithText> {
public:
    using Base = SliderWidget;
    using typename Base::Getter;
    using typename Base::Setter;
    
    ImDataMonitoredSliderFloatWithText(float min_val, float max_val, float initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    ImDataMonitoredSliderFloatWithText(Getter getter, Setter setter, float min_val, float max_val, float default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(float min_val, float max_val, float initial_value = 0.0f) {
        return std::make_unique<ImDataMonitoredSliderFloatWithText>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, float min_val, float max_val, float default_value = 0.0f) {
        return std::make_unique<ImDataMonitoredSliderFloatWithText>(std::move(getter), std::move(setter), min_val, max_val, default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.3f", label ? label : "", this->m_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        float temp_value = this->m_value;
        bool changed = ImGui::SliderFloat(label ? label : "##slider", &temp_value, m_min, m_max);
        
        if (changed && this->is_editable()) {
            this->set(temp_value);
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation (automatically enabled by default)
        this->draw_text_representation();
        
        return changed && this->is_editable();
    }
};

// Config Float Slider
class ImDataConfigSliderFloat : public SliderWidget<float, ImDataConfigValue> {
public:
    using Base = SliderWidget;
    
    ImDataConfigSliderFloat(float min_val, float max_val, float default_value)
        : Base(min_val, max_val, default_value)
    {}
    
    static auto create(float min_val, float max_val, float default_value = 0.0f) {
        return std::make_unique<ImDataConfigSliderFloat>(min_val, max_val, default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            float display_value = this->has_pending_changes() ? this->get_pending_value() : this->m_value;
            ImGui::Text("%s: %.3f", label ? label : "", display_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        float temp_value = this->has_pending_changes() ? this->get_pending_value() : this->m_value;
        bool changed = ImGui::SliderFloat(label ? label : "##slider", &temp_value, m_min, m_max);
        
        if (changed && this->is_editable()) {
            this->try_set(temp_value);
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Show pending indicator
        if (this->has_pending_changes() && this->is_editable()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Value has pending changes");
            }
        }
        
        return changed && this->is_editable();
    }
};

// ============================================================================
// Int32 Slider
// ============================================================================

// Simple Int32 Slider
class ImDataSimpleSliderInt32 : public SliderWidget<int32_t, ImDataSimpleValue> {
public:
    using Base = SliderWidget;
    
    ImDataSimpleSliderInt32(int32_t min_val, int32_t max_val, int32_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    static auto create(int32_t min_val, int32_t max_val, int32_t initial_value = 0) {
        return std::make_unique<ImDataSimpleSliderInt32>(min_val, max_val, initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %d", label ? label : "", this->m_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool changed = ImGui::SliderInt(label ? label : "##slider", &this->m_value, m_min, m_max);
        if (changed) {
            this->m_changed = true;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
};

// Monitored Int32 Slider
class ImDataMonitoredSliderInt32 : public SliderWidget<int32_t, ImDataMonitoredValue> {
public:
    using Base = SliderWidget;
    using typename Base::Getter;
    using typename Base::Setter;
    
    ImDataMonitoredSliderInt32(int32_t min_val, int32_t max_val, int32_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    ImDataMonitoredSliderInt32(Getter getter, Setter setter, int32_t min_val, int32_t max_val, int32_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(int32_t min_val, int32_t max_val, int32_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredSliderInt32>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int32_t min_val, int32_t max_val, int32_t default_value = 0) {
        return std::make_unique<ImDataMonitoredSliderInt32>(std::move(getter), std::move(setter), min_val, max_val, default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %d", label ? label : "", this->m_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int32_t temp_value = this->m_value;
        bool changed = ImGui::SliderInt(label ? label : "##slider", &temp_value, m_min, m_max);
        
        if (changed && this->is_editable()) {
            this->set(temp_value);
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && this->is_editable();
    }
};

// Monitored Int32 Slider With Text
class ImDataMonitoredSliderInt32WithText : public SliderWidget<int32_t, ImDataMonitoredValueWithText> {
public:
    using Base = SliderWidget;
    using typename Base::Getter;
    using typename Base::Setter;
    
    ImDataMonitoredSliderInt32WithText(int32_t min_val, int32_t max_val, int32_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    ImDataMonitoredSliderInt32WithText(Getter getter, Setter setter, int32_t min_val, int32_t max_val, int32_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(int32_t min_val, int32_t max_val, int32_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredSliderInt32WithText>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int32_t min_val, int32_t max_val, int32_t default_value = 0) {
        return std::make_unique<ImDataMonitoredSliderInt32WithText>(std::move(getter), std::move(setter), min_val, max_val, default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %d", label ? label : "", this->m_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int32_t temp_value = this->m_value;
        bool changed = ImGui::SliderInt(label ? label : "##slider", &temp_value, m_min, m_max);
        
        if (changed && this->is_editable()) {
            this->set(temp_value);
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation (automatically enabled by default)
        this->draw_text_representation();
        
        return changed && this->is_editable();
    }
};

// Config Int32 Slider
class ImDataConfigSliderInt32 : public SliderWidget<int32_t, ImDataConfigValue> {
public:
    using Base = SliderWidget;
    
    ImDataConfigSliderInt32(int32_t min_val, int32_t max_val, int32_t default_value)
        : Base(min_val, max_val, default_value)
    {}
    
    static auto create(int32_t min_val, int32_t max_val, int32_t default_value = 0) {
        return std::make_unique<ImDataConfigSliderInt32>(min_val, max_val, default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            int32_t display_value = this->has_pending_changes() ? this->get_pending_value() : this->m_value;
            ImGui::Text("%s: %d", label ? label : "", display_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int32_t temp_value = this->has_pending_changes() ? this->get_pending_value() : this->m_value;
        bool changed = ImGui::SliderInt(label ? label : "##slider", &temp_value, m_min, m_max);
        
        if (changed && this->is_editable()) {
            this->try_set(temp_value);
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Show pending indicator
        if (this->has_pending_changes() && this->is_editable()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Value has pending changes");
            }
        }
        
        return changed && this->is_editable();
    }
};

// ============================================================================
// Double Slider (uses float UI but stores double precision)
// ============================================================================

// Simple Double Slider
class ImDataSimpleSliderDouble : public SliderWidget<double, ImDataSimpleValue> {
public:
    using Base = SliderWidget;
    
    ImDataSimpleSliderDouble(double min_val, double max_val, double initial_value, bool show_precision_input = false)
        : Base(min_val, max_val, initial_value)
        , m_show_precision_input(show_precision_input)
    {}
    
    static auto create(double min_val, double max_val, double initial_value = 0.0, bool show_precision_input = false) {
        return std::make_unique<ImDataSimpleSliderDouble>(min_val, max_val, initial_value, show_precision_input);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.6f", label ? label : "", this->m_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool changed = false;
        
        if (m_show_precision_input) {
            // Draw slider with limited precision
            float temp_float = static_cast<float>(this->m_value);
            if (ImGui::SliderFloat(label ? label : "##slider", &temp_float, 
                                   static_cast<float>(m_min), static_cast<float>(m_max))) {
                this->m_value = static_cast<double>(temp_float);
                this->m_changed = true;
                changed = true;
            }
            
            // Draw precision input next to slider
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f);
            if (ImGui::InputDouble("##precision", &this->m_value)) {
                this->m_value = std::clamp(this->m_value, m_min, m_max);
                this->m_changed = true;
                changed = true;
            }
        } else {
            // Just use float slider
            float temp_float = static_cast<float>(this->m_value);
            if (ImGui::SliderFloat(label ? label : "##slider", &temp_float, 
                                   static_cast<float>(m_min), static_cast<float>(m_max))) {
                this->m_value = static_cast<double>(temp_float);
                this->m_changed = true;
                changed = true;
            }
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
    
private:
    bool m_show_precision_input;
};

// Monitored Double Slider
class ImDataMonitoredSliderDouble : public SliderWidget<double, ImDataMonitoredValue> {
public:
    using Base = SliderWidget;
    using typename Base::Getter;
    using typename Base::Setter;
    
    ImDataMonitoredSliderDouble(double min_val, double max_val, double initial_value, bool show_precision_input = false)
        : Base(min_val, max_val, initial_value)
        , m_show_precision_input(show_precision_input)
    {}
    
    ImDataMonitoredSliderDouble(Getter getter, Setter setter, double min_val, double max_val, double default_value, bool show_precision_input = false)
        : Base(min_val, max_val, default_value)
        , m_show_precision_input(show_precision_input)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(double min_val, double max_val, double initial_value = 0.0, bool show_precision_input = false) {
        return std::make_unique<ImDataMonitoredSliderDouble>(min_val, max_val, initial_value, show_precision_input);
    }
    
    static auto create(Getter getter, Setter setter, double min_val, double max_val, double default_value = 0.0, bool show_precision_input = false) {
        return std::make_unique<ImDataMonitoredSliderDouble>(std::move(getter), std::move(setter), min_val, max_val, default_value, show_precision_input);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.6f", label ? label : "", this->m_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool changed = false;
        double temp_value = this->m_value;
        
        if (m_show_precision_input) {
            // Draw slider with limited precision
            float temp_float = static_cast<float>(temp_value);
            if (ImGui::SliderFloat(label ? label : "##slider", &temp_float, 
                                   static_cast<float>(m_min), static_cast<float>(m_max))) {
                temp_value = static_cast<double>(temp_float);
                changed = true;
            }
            
            // Draw precision input next to slider
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f);
            if (ImGui::InputDouble("##precision", &temp_value)) {
                temp_value = std::clamp(temp_value, m_min, m_max);
                changed = true;
            }
        } else {
            // Just use float slider
            float temp_float = static_cast<float>(temp_value);
            if (ImGui::SliderFloat(label ? label : "##slider", &temp_float, 
                                   static_cast<float>(m_min), static_cast<float>(m_max))) {
                temp_value = static_cast<double>(temp_float);
                changed = true;
            }
        }
        
        if (changed && this->is_editable()) {
            this->set(temp_value);
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && this->is_editable();
    }
    
private:
    bool m_show_precision_input;
};

// Monitored Double Slider With Text
class ImDataMonitoredSliderDoubleWithText : public SliderWidget<double, ImDataMonitoredValueWithText> {
public:
    using Base = SliderWidget;
    using typename Base::Getter;
    using typename Base::Setter;
    
    ImDataMonitoredSliderDoubleWithText(double min_val, double max_val, double initial_value, bool show_precision_input = false)
        : Base(min_val, max_val, initial_value)
        , m_show_precision_input(show_precision_input)
    {}
    
    ImDataMonitoredSliderDoubleWithText(Getter getter, Setter setter, double min_val, double max_val, double default_value, bool show_precision_input = false)
        : Base(min_val, max_val, default_value)
        , m_show_precision_input(show_precision_input)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(double min_val, double max_val, double initial_value = 0.0, bool show_precision_input = false) {
        return std::make_unique<ImDataMonitoredSliderDoubleWithText>(min_val, max_val, initial_value, show_precision_input);
    }
    
    static auto create(Getter getter, Setter setter, double min_val, double max_val, double default_value = 0.0, bool show_precision_input = false) {
        return std::make_unique<ImDataMonitoredSliderDoubleWithText>(std::move(getter), std::move(setter), min_val, max_val, default_value, show_precision_input);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %.6f", label ? label : "", this->m_value);
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        bool changed = false;
        double temp_value = this->m_value;
        
        if (m_show_precision_input) {
            // Draw slider with limited precision
            float temp_float = static_cast<float>(temp_value);
            if (ImGui::SliderFloat(label ? label : "##slider", &temp_float, 
                                   static_cast<float>(m_min), static_cast<float>(m_max))) {
                temp_value = static_cast<double>(temp_float);
                changed = true;
            }
            
            // Draw precision input next to slider
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f);
            if (ImGui::InputDouble("##precision", &temp_value)) {
                temp_value = std::clamp(temp_value, m_min, m_max);
                changed = true;
            }
        } else {
            // Just use float slider
            float temp_float = static_cast<float>(temp_value);
            if (ImGui::SliderFloat(label ? label : "##slider", &temp_float, 
                                   static_cast<float>(m_min), static_cast<float>(m_max))) {
                temp_value = static_cast<double>(temp_float);
                changed = true;
            }
        }
        
        if (changed && this->is_editable()) {
            this->set(temp_value);
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation (automatically enabled by default)
        this->draw_text_representation();
        
        return changed && this->is_editable();
    }
    
private:
    bool m_show_precision_input;
};

// ============================================================================
// UInt8 Slider
// ============================================================================

// Simple UInt8 Slider
class ImDataSimpleSliderUInt8 : public SliderWidget<uint8_t, ImDataSimpleValue> {
public:
    using Base = SliderWidget;
    
    ImDataSimpleSliderUInt8(uint8_t min_val, uint8_t max_val, uint8_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    static auto create(uint8_t min_val, uint8_t max_val, uint8_t initial_value = 0) {
        return std::make_unique<ImDataSimpleSliderUInt8>(min_val, max_val, initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(this->m_value));
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Use int for ImGui
        int temp_value = static_cast<int>(this->m_value);
        bool changed = ImGui::SliderInt(label ? label : "##slider", &temp_value, 
                                        static_cast<int>(m_min), static_cast<int>(m_max));
        if (changed) {
            this->m_value = static_cast<uint8_t>(temp_value);
            this->m_changed = true;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
};

// Monitored UInt8 Slider
class ImDataMonitoredSliderUInt8 : public SliderWidget<uint8_t, ImDataMonitoredValue> {
public:
    using Base = SliderWidget;
    using typename Base::Getter;
    using typename Base::Setter;
    
    ImDataMonitoredSliderUInt8(uint8_t min_val, uint8_t max_val, uint8_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    ImDataMonitoredSliderUInt8(Getter getter, Setter setter, uint8_t min_val, uint8_t max_val, uint8_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(uint8_t min_val, uint8_t max_val, uint8_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredSliderUInt8>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint8_t min_val, uint8_t max_val, uint8_t default_value = 0) {
        return std::make_unique<ImDataMonitoredSliderUInt8>(std::move(getter), std::move(setter), min_val, max_val, default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(this->m_value));
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int temp_value = static_cast<int>(this->m_value);
        bool changed = ImGui::SliderInt(label ? label : "##slider", &temp_value, 
                                        static_cast<int>(m_min), static_cast<int>(m_max));
        
        if (changed && this->is_editable()) {
            this->set(static_cast<uint8_t>(temp_value));
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && this->is_editable();
    }
};

// Monitored UInt8 Slider With Text
class ImDataMonitoredSliderUInt8WithText : public SliderWidget<uint8_t, ImDataMonitoredValueWithText> {
public:
    using Base = SliderWidget;
    
    ImDataMonitoredSliderUInt8WithText(uint8_t min_val, uint8_t max_val, uint8_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    ImDataMonitoredSliderUInt8WithText(Getter getter, Setter setter, uint8_t min_val, uint8_t max_val, uint8_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(uint8_t min_val, uint8_t max_val, uint8_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredSliderUInt8WithText>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint8_t min_val, uint8_t max_val, uint8_t default_value = 0) {
        return std::make_unique<ImDataMonitoredSliderUInt8WithText>(std::move(getter), std::move(setter), min_val, max_val, default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(this->m_value));
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int temp_value = static_cast<int>(this->m_value);
        bool changed = ImGui::SliderInt(label ? label : "##slider", &temp_value, 
                                        static_cast<int>(m_min), static_cast<int>(m_max));
        
        if (changed && this->is_editable()) {
            this->set(static_cast<uint8_t>(temp_value));
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation
        this->draw_text_representation();
        
        return changed && this->is_editable();
    }
};

// ============================================================================
// UInt16 Slider
// ============================================================================

// Simple UInt16 Slider
class ImDataSimpleSliderUInt16 : public SliderWidget<uint16_t, ImDataSimpleValue> {
public:
    using Base = SliderWidget;
    
    ImDataSimpleSliderUInt16(uint16_t min_val, uint16_t max_val, uint16_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    static auto create(uint16_t min_val, uint16_t max_val, uint16_t initial_value = 0) {
        return std::make_unique<ImDataSimpleSliderUInt16>(min_val, max_val, initial_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(this->m_value));
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        // Use int for ImGui
        int temp_value = static_cast<int>(this->m_value);
        bool changed = ImGui::SliderInt(label ? label : "##slider", &temp_value, 
                                        static_cast<int>(m_min), static_cast<int>(m_max));
        if (changed) {
            this->m_value = static_cast<uint16_t>(temp_value);
            this->m_changed = true;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed;
    }
};

// Monitored UInt16 Slider
class ImDataMonitoredSliderUInt16 : public SliderWidget<uint16_t, ImDataMonitoredValue> {
public:
    using Base = SliderWidget;
    using typename Base::Getter;
    using typename Base::Setter;
    
    ImDataMonitoredSliderUInt16(uint16_t min_val, uint16_t max_val, uint16_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    ImDataMonitoredSliderUInt16(Getter getter, Setter setter, uint16_t min_val, uint16_t max_val, uint16_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(uint16_t min_val, uint16_t max_val, uint16_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredSliderUInt16>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint16_t min_val, uint16_t max_val, uint16_t default_value = 0) {
        return std::make_unique<ImDataMonitoredSliderUInt16>(std::move(getter), std::move(setter), min_val, max_val, default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(this->m_value));
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int temp_value = static_cast<int>(this->m_value);
        bool changed = ImGui::SliderInt(label ? label : "##slider", &temp_value, 
                                        static_cast<int>(m_min), static_cast<int>(m_max));
        
        if (changed && this->is_editable()) {
            this->set(static_cast<uint16_t>(temp_value));
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        return changed && this->is_editable();
    }
};

// Monitored UInt16 Slider With Text
class ImDataMonitoredSliderUInt16WithText : public SliderWidget<uint16_t, ImDataMonitoredValueWithText> {
public:
    using Base = SliderWidget;
    using typename Base::Getter;
    using typename Base::Setter;
    
    ImDataMonitoredSliderUInt16WithText(uint16_t min_val, uint16_t max_val, uint16_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    ImDataMonitoredSliderUInt16WithText(Getter getter, Setter setter, uint16_t min_val, uint16_t max_val, uint16_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(uint16_t min_val, uint16_t max_val, uint16_t initial_value = 0) {
        return std::make_unique<ImDataMonitoredSliderUInt16WithText>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint16_t min_val, uint16_t max_val, uint16_t default_value = 0) {
        return std::make_unique<ImDataMonitoredSliderUInt16WithText>(std::move(getter), std::move(setter), min_val, max_val, default_value);
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->m_edit_mode == EditMode::ViewOnly) {
            ImGui::Text("%s: %u", label ? label : "", static_cast<unsigned>(this->m_value));
            return false;
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::BeginDisabled();
        }
        
        int temp_value = static_cast<int>(this->m_value);
        bool changed = ImGui::SliderInt(label ? label : "##slider", &temp_value, 
                                        static_cast<int>(m_min), static_cast<int>(m_max));
        
        if (changed && this->is_editable()) {
            this->set(static_cast<uint16_t>(temp_value));
        }
        
        if (this->m_edit_mode == EditMode::ReadOnly) {
            ImGui::EndDisabled();
        }
        
        // Draw text representation
        this->draw_text_representation();
        
        return changed && this->is_editable();
    }
};

// Type aliases for compatibility
using ImGuiSlider = ImDataSimpleSliderFloat;
using ImGuiSliderDouble = ImDataSimpleSliderDouble;
using ImGuiSliderInt32 = ImDataSimpleSliderInt32;
using ImGuiSliderUInt8 = ImDataSimpleSliderUInt8;
using ImGuiSliderUInt16 = ImDataSimpleSliderUInt16;

} // namespace RC::ImDataControls