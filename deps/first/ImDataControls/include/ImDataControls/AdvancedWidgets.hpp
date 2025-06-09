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
class SimpleSliderFloat : public SliderWidget<float, SimpleImGuiValue> {
public:
    using Base = SliderWidget<float, SimpleImGuiValue>;
    
    SimpleSliderFloat(float min_val, float max_val, float initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    static auto create(float min_val, float max_val, float initial_value = 0.0f) {
        return std::make_unique<SimpleSliderFloat>(min_val, max_val, initial_value);
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
class MonitoredSliderFloat : public SliderWidget<float, MonitoredImGuiValue> {
public:
    using Base = SliderWidget<float, MonitoredImGuiValue>;
    using typename Base::Getter;
    using typename Base::Setter;
    
    MonitoredSliderFloat(float min_val, float max_val, float initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    MonitoredSliderFloat(Getter getter, Setter setter, float min_val, float max_val, float default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(float min_val, float max_val, float initial_value = 0.0f) {
        return std::make_unique<MonitoredSliderFloat>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, float min_val, float max_val, float default_value = 0.0f) {
        return std::make_unique<MonitoredSliderFloat>(std::move(getter), std::move(setter), min_val, max_val, default_value);
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
class MonitoredSliderFloatWithText : public SliderWidget<float, MonitoredImGuiValueWithText> {
public:
    using Base = SliderWidget<float, MonitoredImGuiValueWithText>;
    using typename Base::Getter;
    using typename Base::Setter;
    
    MonitoredSliderFloatWithText(float min_val, float max_val, float initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    MonitoredSliderFloatWithText(Getter getter, Setter setter, float min_val, float max_val, float default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(float min_val, float max_val, float initial_value = 0.0f) {
        return std::make_unique<MonitoredSliderFloatWithText>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, float min_val, float max_val, float default_value = 0.0f) {
        return std::make_unique<MonitoredSliderFloatWithText>(std::move(getter), std::move(setter), min_val, max_val, default_value);
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
class ConfigSliderFloat : public SliderWidget<float, ConfigImGuiValue> {
public:
    using Base = SliderWidget<float, ConfigImGuiValue>;
    
    ConfigSliderFloat(float min_val, float max_val, float default_value)
        : Base(min_val, max_val, default_value)
    {}
    
    static auto create(float min_val, float max_val, float default_value = 0.0f) {
        return std::make_unique<ConfigSliderFloat>(min_val, max_val, default_value);
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
class SimpleSliderInt32 : public SliderWidget<int32_t, SimpleImGuiValue> {
public:
    using Base = SliderWidget<int32_t, SimpleImGuiValue>;
    
    SimpleSliderInt32(int32_t min_val, int32_t max_val, int32_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    static auto create(int32_t min_val, int32_t max_val, int32_t initial_value = 0) {
        return std::make_unique<SimpleSliderInt32>(min_val, max_val, initial_value);
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
class MonitoredSliderInt32 : public SliderWidget<int32_t, MonitoredImGuiValue> {
public:
    using Base = SliderWidget<int32_t, MonitoredImGuiValue>;
    using typename Base::Getter;
    using typename Base::Setter;
    
    MonitoredSliderInt32(int32_t min_val, int32_t max_val, int32_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    MonitoredSliderInt32(Getter getter, Setter setter, int32_t min_val, int32_t max_val, int32_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(int32_t min_val, int32_t max_val, int32_t initial_value = 0) {
        return std::make_unique<MonitoredSliderInt32>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int32_t min_val, int32_t max_val, int32_t default_value = 0) {
        return std::make_unique<MonitoredSliderInt32>(std::move(getter), std::move(setter), min_val, max_val, default_value);
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
class MonitoredSliderInt32WithText : public SliderWidget<int32_t, MonitoredImGuiValueWithText> {
public:
    using Base = SliderWidget<int32_t, MonitoredImGuiValueWithText>;
    using typename Base::Getter;
    using typename Base::Setter;
    
    MonitoredSliderInt32WithText(int32_t min_val, int32_t max_val, int32_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    MonitoredSliderInt32WithText(Getter getter, Setter setter, int32_t min_val, int32_t max_val, int32_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(int32_t min_val, int32_t max_val, int32_t initial_value = 0) {
        return std::make_unique<MonitoredSliderInt32WithText>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, int32_t min_val, int32_t max_val, int32_t default_value = 0) {
        return std::make_unique<MonitoredSliderInt32WithText>(std::move(getter), std::move(setter), min_val, max_val, default_value);
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
class ConfigSliderInt32 : public SliderWidget<int32_t, ConfigImGuiValue> {
public:
    using Base = SliderWidget<int32_t, ConfigImGuiValue>;
    
    ConfigSliderInt32(int32_t min_val, int32_t max_val, int32_t default_value)
        : Base(min_val, max_val, default_value)
    {}
    
    static auto create(int32_t min_val, int32_t max_val, int32_t default_value = 0) {
        return std::make_unique<ConfigSliderInt32>(min_val, max_val, default_value);
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
class SimpleSliderDouble : public SliderWidget<double, SimpleImGuiValue> {
public:
    using Base = SliderWidget<double, SimpleImGuiValue>;
    
    SimpleSliderDouble(double min_val, double max_val, double initial_value, bool show_precision_input = false)
        : Base(min_val, max_val, initial_value)
        , m_show_precision_input(show_precision_input)
    {}
    
    static auto create(double min_val, double max_val, double initial_value = 0.0, bool show_precision_input = false) {
        return std::make_unique<SimpleSliderDouble>(min_val, max_val, initial_value, show_precision_input);
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
class MonitoredSliderDouble : public SliderWidget<double, MonitoredImGuiValue> {
public:
    using Base = SliderWidget<double, MonitoredImGuiValue>;
    using typename Base::Getter;
    using typename Base::Setter;
    
    MonitoredSliderDouble(double min_val, double max_val, double initial_value, bool show_precision_input = false)
        : Base(min_val, max_val, initial_value)
        , m_show_precision_input(show_precision_input)
    {}
    
    MonitoredSliderDouble(Getter getter, Setter setter, double min_val, double max_val, double default_value, bool show_precision_input = false)
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
        return std::make_unique<MonitoredSliderDouble>(min_val, max_val, initial_value, show_precision_input);
    }
    
    static auto create(Getter getter, Setter setter, double min_val, double max_val, double default_value = 0.0, bool show_precision_input = false) {
        return std::make_unique<MonitoredSliderDouble>(std::move(getter), std::move(setter), min_val, max_val, default_value, show_precision_input);
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
class MonitoredSliderDoubleWithText : public SliderWidget<double, MonitoredImGuiValueWithText> {
public:
    using Base = SliderWidget<double, MonitoredImGuiValueWithText>;
    using typename Base::Getter;
    using typename Base::Setter;
    
    MonitoredSliderDoubleWithText(double min_val, double max_val, double initial_value, bool show_precision_input = false)
        : Base(min_val, max_val, initial_value)
        , m_show_precision_input(show_precision_input)
    {}
    
    MonitoredSliderDoubleWithText(Getter getter, Setter setter, double min_val, double max_val, double default_value, bool show_precision_input = false)
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
        return std::make_unique<MonitoredSliderDoubleWithText>(min_val, max_val, initial_value, show_precision_input);
    }
    
    static auto create(Getter getter, Setter setter, double min_val, double max_val, double default_value = 0.0, bool show_precision_input = false) {
        return std::make_unique<MonitoredSliderDoubleWithText>(std::move(getter), std::move(setter), min_val, max_val, default_value, show_precision_input);
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
class SimpleSliderUInt8 : public SliderWidget<uint8_t, SimpleImGuiValue> {
public:
    using Base = SliderWidget<uint8_t, SimpleImGuiValue>;
    
    SimpleSliderUInt8(uint8_t min_val, uint8_t max_val, uint8_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    static auto create(uint8_t min_val, uint8_t max_val, uint8_t initial_value = 0) {
        return std::make_unique<SimpleSliderUInt8>(min_val, max_val, initial_value);
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
class MonitoredSliderUInt8 : public SliderWidget<uint8_t, MonitoredImGuiValue> {
public:
    using Base = SliderWidget<uint8_t, MonitoredImGuiValue>;
    using typename Base::Getter;
    using typename Base::Setter;
    
    MonitoredSliderUInt8(uint8_t min_val, uint8_t max_val, uint8_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    MonitoredSliderUInt8(Getter getter, Setter setter, uint8_t min_val, uint8_t max_val, uint8_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(uint8_t min_val, uint8_t max_val, uint8_t initial_value = 0) {
        return std::make_unique<MonitoredSliderUInt8>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint8_t min_val, uint8_t max_val, uint8_t default_value = 0) {
        return std::make_unique<MonitoredSliderUInt8>(std::move(getter), std::move(setter), min_val, max_val, default_value);
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
class MonitoredSliderUInt8WithText : public SliderWidget<uint8_t, MonitoredImGuiValueWithText> {
public:
    using Base = SliderWidget<uint8_t, MonitoredImGuiValueWithText>;
    using typename Base::Getter;
    using typename Base::Setter;
    
    MonitoredSliderUInt8WithText(uint8_t min_val, uint8_t max_val, uint8_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    MonitoredSliderUInt8WithText(Getter getter, Setter setter, uint8_t min_val, uint8_t max_val, uint8_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(uint8_t min_val, uint8_t max_val, uint8_t initial_value = 0) {
        return std::make_unique<MonitoredSliderUInt8WithText>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint8_t min_val, uint8_t max_val, uint8_t default_value = 0) {
        return std::make_unique<MonitoredSliderUInt8WithText>(std::move(getter), std::move(setter), min_val, max_val, default_value);
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
class SimpleSliderUInt16 : public SliderWidget<uint16_t, SimpleImGuiValue> {
public:
    using Base = SliderWidget<uint16_t, SimpleImGuiValue>;
    
    SimpleSliderUInt16(uint16_t min_val, uint16_t max_val, uint16_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    static auto create(uint16_t min_val, uint16_t max_val, uint16_t initial_value = 0) {
        return std::make_unique<SimpleSliderUInt16>(min_val, max_val, initial_value);
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
class MonitoredSliderUInt16 : public SliderWidget<uint16_t, MonitoredImGuiValue> {
public:
    using Base = SliderWidget<uint16_t, MonitoredImGuiValue>;
    using typename Base::Getter;
    using typename Base::Setter;
    
    MonitoredSliderUInt16(uint16_t min_val, uint16_t max_val, uint16_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    MonitoredSliderUInt16(Getter getter, Setter setter, uint16_t min_val, uint16_t max_val, uint16_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(uint16_t min_val, uint16_t max_val, uint16_t initial_value = 0) {
        return std::make_unique<MonitoredSliderUInt16>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint16_t min_val, uint16_t max_val, uint16_t default_value = 0) {
        return std::make_unique<MonitoredSliderUInt16>(std::move(getter), std::move(setter), min_val, max_val, default_value);
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
class MonitoredSliderUInt16WithText : public SliderWidget<uint16_t, MonitoredImGuiValueWithText> {
public:
    using Base = SliderWidget<uint16_t, MonitoredImGuiValueWithText>;
    using typename Base::Getter;
    using typename Base::Setter;
    
    MonitoredSliderUInt16WithText(uint16_t min_val, uint16_t max_val, uint16_t initial_value)
        : Base(min_val, max_val, initial_value)
    {}
    
    MonitoredSliderUInt16WithText(Getter getter, Setter setter, uint16_t min_val, uint16_t max_val, uint16_t default_value)
        : Base(min_val, max_val, default_value)
    {
        this->set_external_getter(std::move(getter));
        this->set_external_setter(std::move(setter));
        if (this->m_getter) {
            this->sync_from_external();
        }
    }
    
    static auto create(uint16_t min_val, uint16_t max_val, uint16_t initial_value = 0) {
        return std::make_unique<MonitoredSliderUInt16WithText>(min_val, max_val, initial_value);
    }
    
    static auto create(Getter getter, Setter setter, uint16_t min_val, uint16_t max_val, uint16_t default_value = 0) {
        return std::make_unique<MonitoredSliderUInt16WithText>(std::move(getter), std::move(setter), min_val, max_val, default_value);
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
using ImGuiSlider = SimpleSliderFloat;
using ImGuiSliderDouble = SimpleSliderDouble;
using ImGuiSliderInt32 = SimpleSliderInt32;
using ImGuiSliderUInt8 = SimpleSliderUInt8;
using ImGuiSliderUInt16 = SimpleSliderUInt16;

} // namespace RC::ImDataControls