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

// ============================================================================
// Combo Box Widget
// ============================================================================

template<template<typename> class ComposedType>
class ImDataComboWidget : public BasicWidget<int32_t, ComposedType> {
public:
    using Base = BasicWidget<int32_t, ComposedType>;
    
    ImDataComboWidget(const std::vector<std::string>& options, int32_t initial_value = 0)
        : Base(initial_value), m_options(options) {
        update_option_pointers();
    }
    
    void set_options(const std::vector<std::string>& options) {
        m_options = options;
        update_option_pointers();
        this->m_value = std::clamp(this->m_value, 0, 
                                   static_cast<int32_t>(m_options.size() - 1));
    }
    
    [[nodiscard]] const std::vector<std::string>& get_options() const { 
        return m_options; 
    }
    
    [[nodiscard]] std::string get_selected_text() const {
        if (this->m_value >= 0 && this->m_value < static_cast<int32_t>(m_options.size())) {
            return m_options[this->m_value];
        }
        return "";
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ViewOnly) {
            ImGui::Text("%s: %s", this->get_display_label(label), 
                       get_selected_text().c_str());
            return false;
        }
        
        this->begin_disabled_if_readonly();
        
        int32_t temp_value = this->m_value;
        bool changed = ImGui::Combo(this->get_display_label(label), &temp_value,
                                   m_option_pointers.data(),
                                   static_cast<int32_t>(m_option_pointers.size()));
        
        if (changed && this->is_editable()) {
            if constexpr (requires { this->set(temp_value); }) {
                this->set(temp_value);
            } else {
                this->m_value = temp_value;
                this->m_changed = true;
            }
        }
        
        this->end_disabled_if_readonly();
        
        return changed && this->is_editable();
    }
    
private:
    void update_option_pointers() {
        m_option_pointers.clear();
        for (const auto& option : m_options) {
            m_option_pointers.push_back(option.c_str());
        }
    }
    
    std::vector<std::string> m_options;
    std::vector<const char*> m_option_pointers;
};

// Type aliases
using ImDataSimpleCombo = ImDataComboWidget<ImDataSimpleValue>;
using ImDataMonitoredCombo = ImDataComboWidget<ImDataMonitoredValue>;
using ImDataConfigCombo = ImDataComboWidget<ImDataConfigValue>;

// ============================================================================
// Drag Float Widget
// ============================================================================

template<template<typename> class ComposedType>
class ImDataDragFloatWidget : public BasicWidget<float, ComposedType> {
public:
    using Base = BasicWidget<float, ComposedType>;
    
    ImDataDragFloatWidget(float initial_value = 0.0f, float speed = 1.0f, 
                          float min = 0.0f, float max = 0.0f)
        : Base(initial_value), m_speed(speed), m_min(min), m_max(max) {}
    
    void set_speed(float speed) { m_speed = speed; }
    void set_range(float min, float max) { m_min = min; m_max = max; }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ViewOnly) {
            ImGui::Text("%s: %.3f", this->get_display_label(label), this->m_value);
            return false;
        }
        
        this->begin_disabled_if_readonly();
        
        float temp_value = this->m_value;
        bool changed = ImGui::DragFloat(this->get_display_label(label), 
                                       &temp_value, m_speed, m_min, m_max);
        
        if (changed && this->is_editable()) {
            if constexpr (requires { this->set(temp_value); }) {
                this->set(temp_value);
            } else {
                this->m_value = temp_value;
                this->m_changed = true;
            }
        }
        
        this->end_disabled_if_readonly();
        
        return changed && this->is_editable();
    }
    
private:
    float m_speed;
    float m_min;
    float m_max;
};

using ImDataSimpleDragFloat = ImDataDragFloatWidget<ImDataSimpleValue>;
using ImDataMonitoredDragFloat = ImDataDragFloatWidget<ImDataMonitoredValue>;

// Similar implementations for DragInt, DragDouble...

// ============================================================================
// Color3 Widget (RGB)
// ============================================================================

template<template<typename> class ComposedType>
class ImDataColor3Widget : public BasicWidget<std::array<float, 3>, ComposedType> {
public:
    using Base = BasicWidget<std::array<float, 3>, ComposedType>;
    
    ImDataColor3Widget(float r = 1.0f, float g = 1.0f, float b = 1.0f)
        : Base({r, g, b}) {}
    
    void set_rgb(float r, float g, float b) {
        this->m_value = {r, g, b};
    }
    
    [[nodiscard]] float r() const { return this->m_value[0]; }
    [[nodiscard]] float g() const { return this->m_value[1]; }
    [[nodiscard]] float b() const { return this->m_value[2]; }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ViewOnly) {
            ImGui::Text("%s: (%.2f, %.2f, %.2f)", this->get_display_label(label),
                       this->m_value[0], this->m_value[1], this->m_value[2]);
            return false;
        }
        
        this->begin_disabled_if_readonly();
        
        std::array<float, 3> temp_value = this->m_value;
        bool changed = ImGui::ColorEdit3(this->get_display_label(label), 
                                        temp_value.data());
        
        if (changed && this->is_editable()) {
            if constexpr (requires { this->set(temp_value); }) {
                this->set(temp_value);
            } else {
                this->m_value = temp_value;
                this->m_changed = true;
            }
        }
        
        this->end_disabled_if_readonly();
        
        return changed && this->is_editable();
    }
};

using ImDataSimpleColor3 = ImDataColor3Widget<ImDataSimpleValue>;
using ImDataMonitoredColor3 = ImDataColor3Widget<ImDataMonitoredValue>;

// ============================================================================
// Vector2 Widget
// ============================================================================

template<template<typename> class ComposedType>
class ImDataVector2Widget : public BasicWidget<std::array<float, 2>, ComposedType> {
public:
    using Base = BasicWidget<std::array<float, 2>, ComposedType>;
    
    ImDataVector2Widget(float x = 0.0f, float y = 0.0f)
        : Base({x, y}) {}
    
    void set_xy(float x, float y) {
        this->m_value = {x, y};
    }
    
    [[nodiscard]] float x() const { return this->m_value[0]; }
    [[nodiscard]] float y() const { return this->m_value[1]; }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ViewOnly) {
            ImGui::Text("%s: (%.3f, %.3f)", this->get_display_label(label),
                       this->m_value[0], this->m_value[1]);
            return false;
        }
        
        this->begin_disabled_if_readonly();
        
        std::array<float, 2> temp_value = this->m_value;
        bool changed = ImGui::InputFloat2(this->get_display_label(label), 
                                         temp_value.data());
        
        if (changed && this->is_editable()) {
            if constexpr (requires { this->set(temp_value); }) {
                this->set(temp_value);
            } else {
                this->m_value = temp_value;
                this->m_changed = true;
            }
        }
        
        this->end_disabled_if_readonly();
        
        return changed && this->is_editable();
    }
};

using ImDataSimpleVector2 = ImDataVector2Widget<ImDataSimpleValue>;
using ImDataMonitoredVector2 = ImDataVector2Widget<ImDataMonitoredValue>;

// ============================================================================
// Vector3 Widget
// ============================================================================

template<template<typename> class ComposedType>
class ImDataVector3Widget : public BasicWidget<std::array<float, 3>, ComposedType> {
public:
    using Base = BasicWidget<std::array<float, 3>, ComposedType>;
    
    ImDataVector3Widget(float x = 0.0f, float y = 0.0f, float z = 0.0f)
        : Base({x, y, z}) {}
    
    void set_xyz(float x, float y, float z) {
        this->m_value = {x, y, z};
    }
    
    [[nodiscard]] float x() const { return this->m_value[0]; }
    [[nodiscard]] float y() const { return this->m_value[1]; }
    [[nodiscard]] float z() const { return this->m_value[2]; }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ViewOnly) {
            ImGui::Text("%s: (%.3f, %.3f, %.3f)", this->get_display_label(label),
                       this->m_value[0], this->m_value[1], this->m_value[2]);
            return false;
        }
        
        this->begin_disabled_if_readonly();
        
        std::array<float, 3> temp_value = this->m_value;
        bool changed = ImGui::InputFloat3(this->get_display_label(label), 
                                         temp_value.data());
        
        if (changed && this->is_editable()) {
            if constexpr (requires { this->set(temp_value); }) {
                this->set(temp_value);
            } else {
                this->m_value = temp_value;
                this->m_changed = true;
            }
        }
        
        this->end_disabled_if_readonly();
        
        return changed && this->is_editable();
    }
};

using ImDataSimpleVector3 = ImDataVector3Widget<ImDataSimpleValue>;
using ImDataMonitoredVector3 = ImDataVector3Widget<ImDataMonitoredValue>;

// ============================================================================
// Color4 Widget (RGBA)
// ============================================================================

template<template<typename> class ComposedType>
class ImDataColor4Widget : public BasicWidget<std::array<float, 4>, ComposedType> {
public:
    using Base = BasicWidget<std::array<float, 4>, ComposedType>;
    
    ImDataColor4Widget(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
        : Base({r, g, b, a}) {}
    
    void set_rgba(float r, float g, float b, float a) {
        this->m_value = {r, g, b, a};
    }
    
    [[nodiscard]] float r() const { return this->m_value[0]; }
    [[nodiscard]] float g() const { return this->m_value[1]; }
    [[nodiscard]] float b() const { return this->m_value[2]; }
    [[nodiscard]] float a() const { return this->m_value[3]; }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ViewOnly) {
            ImGui::Text("%s: (%.2f, %.2f, %.2f, %.2f)", this->get_display_label(label),
                       this->m_value[0], this->m_value[1], this->m_value[2], this->m_value[3]);
            return false;
        }
        
        this->begin_disabled_if_readonly();
        
        std::array<float, 4> temp_value = this->m_value;
        bool changed = ImGui::ColorEdit4(this->get_display_label(label), 
                                        temp_value.data());
        
        if (changed && this->is_editable()) {
            if constexpr (requires { this->set(temp_value); }) {
                this->set(temp_value);
            } else {
                this->m_value = temp_value;
                this->m_changed = true;
            }
        }
        
        this->end_disabled_if_readonly();
        
        return changed && this->is_editable();
    }
};

using ImDataSimpleColor4 = ImDataColor4Widget<ImDataSimpleValue>;
using ImDataMonitoredColor4 = ImDataColor4Widget<ImDataMonitoredValue>;

// ============================================================================
// Drag Int Widget
// ============================================================================

template<template<typename> class ComposedType>
class ImDataDragIntWidget : public BasicWidget<int32_t, ComposedType> {
public:
    using Base = BasicWidget<int32_t, ComposedType>;
    
    ImDataDragIntWidget(int32_t initial_value = 0, float speed = 1.0f, 
                        int32_t min = 0, int32_t max = 0)
        : Base(initial_value), m_speed(speed), m_min(min), m_max(max) {}
    
    void set_speed(float speed) { m_speed = speed; }
    void set_range(int32_t min, int32_t max) { m_min = min; m_max = max; }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ViewOnly) {
            ImGui::Text("%s: %d", this->get_display_label(label), this->m_value);
            return false;
        }
        
        this->begin_disabled_if_readonly();
        
        int32_t temp_value = this->m_value;
        bool changed = ImGui::DragInt(this->get_display_label(label), 
                                     &temp_value, m_speed, m_min, m_max);
        
        if (changed && this->is_editable()) {
            if constexpr (requires { this->set(temp_value); }) {
                this->set(temp_value);
            } else {
                this->m_value = temp_value;
                this->m_changed = true;
            }
        }
        
        this->end_disabled_if_readonly();
        
        return changed && this->is_editable();
    }
    
private:
    float m_speed;
    int32_t m_min;
    int32_t m_max;
};

using ImDataSimpleDragInt = ImDataDragIntWidget<ImDataSimpleValue>;
using ImDataMonitoredDragInt = ImDataDragIntWidget<ImDataMonitoredValue>;

// ============================================================================
// Drag Double Widget
// ============================================================================

template<template<typename> class ComposedType>
class ImDataDragDoubleWidget : public BasicWidget<double, ComposedType> {
public:
    using Base = BasicWidget<double, ComposedType>;
    
    ImDataDragDoubleWidget(double initial_value = 0.0, float speed = 1.0f, 
                           double min = 0.0, double max = 0.0)
        : Base(initial_value), m_speed(speed), m_min(min), m_max(max) {}
    
    void set_speed(float speed) { m_speed = speed; }
    void set_range(double min, double max) { m_min = min; m_max = max; }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ViewOnly) {
            ImGui::Text("%s: %.6f", this->get_display_label(label), this->m_value);
            return false;
        }
        
        this->begin_disabled_if_readonly();
        
        double temp_value = this->m_value;
        float temp_float = static_cast<float>(temp_value);
        bool changed = ImGui::DragFloat(this->get_display_label(label), 
                                       &temp_float, m_speed, 
                                       static_cast<float>(m_min), 
                                       static_cast<float>(m_max));
        
        if (changed) {
            temp_value = static_cast<double>(temp_float);
            if (this->is_editable()) {
                if constexpr (requires { this->set(temp_value); }) {
                    this->set(temp_value);
                } else {
                    this->m_value = temp_value;
                    this->m_changed = true;
                }
            }
        }
        
        this->end_disabled_if_readonly();
        
        return changed && this->is_editable();
    }
    
private:
    float m_speed;
    double m_min;
    double m_max;
};

using ImDataSimpleDragDouble = ImDataDragDoubleWidget<ImDataSimpleValue>;
using ImDataMonitoredDragDouble = ImDataDragDoubleWidget<ImDataMonitoredValue>;

// ============================================================================
// Radio Button Widget
// ============================================================================

template<template<typename> class ComposedType>
class ImDataRadioButtonWidget : public BasicWidget<int32_t, ComposedType> {
public:
    using Base = BasicWidget<int32_t, ComposedType>;
    
    ImDataRadioButtonWidget(const std::vector<std::string>& options, int32_t initial_value = 0)
        : Base(initial_value), m_options(options) {}
    
    void set_options(const std::vector<std::string>& options) {
        m_options = options;
        this->m_value = std::clamp(this->m_value, 0, 
                                   static_cast<int32_t>(m_options.size() - 1));
    }
    
    [[nodiscard]] const std::vector<std::string>& get_options() const { 
        return m_options; 
    }
    
protected:
    bool draw_impl(const char* label) override {
        if (this->get_edit_mode() == IEditModeControl::EditMode::ViewOnly) {
            ImGui::Text("%s: %s", this->get_display_label(label), 
                       (this->m_value >= 0 && this->m_value < static_cast<int32_t>(m_options.size())) 
                       ? m_options[this->m_value].c_str() : "");
            return false;
        }
        
        this->begin_disabled_if_readonly();
        
        ImGui::Text("%s", this->get_display_label(label));
        
        bool changed = false;
        for (int32_t i = 0; i < static_cast<int32_t>(m_options.size()); ++i) {
            if (ImGui::RadioButton(m_options[i].c_str(), &this->m_value, i)) {
                changed = true;
                if (this->is_editable()) {
                    if constexpr (requires { this->set(this->m_value); }) {
                        this->set(this->m_value);
                    } else {
                        this->m_changed = true;
                    }
                }
            }
            if (i < static_cast<int32_t>(m_options.size()) - 1) {
                ImGui::SameLine();
            }
        }
        
        this->end_disabled_if_readonly();
        
        return changed && this->is_editable();
    }
    
private:
    std::vector<std::string> m_options;
};

using ImDataSimpleRadioButton = ImDataRadioButtonWidget<ImDataSimpleValue>;
using ImDataMonitoredRadioButton = ImDataRadioButtonWidget<ImDataMonitoredValue>;

} // namespace RC::ImDataControls