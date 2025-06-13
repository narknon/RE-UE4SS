#pragma once

// Convenience header that includes all ImDataControls components

#include "Core.hpp"
#include "BasicWidgets.hpp"
#include "AdvancedWidgets.hpp"
#include "Container.hpp"
#include "Factories.hpp"
#include "WidgetBuilder.hpp"

namespace RC::ImDataControls {
    // ============================================================================
    // Minimal Convenience Aliases
    // Using IDC prefix (ImDataControls) to avoid namespace pollution
    // ============================================================================
    
    // Most commonly used widgets only
    using IDCToggle = ImDataSimpleToggle;
    using IDCFloatInput = ImDataSimpleFloat;
    using IDCIntInput = ImDataSimpleInt32;
    using IDCStringInput = ImDataSimpleString;
    using IDCTextBox = ImDataSimpleTextMultiline;
    using IDCFloatSlider = ImDataSimpleSliderFloat;
    using IDCIntSlider = ImDataSimpleSliderInt32;
    using IDCComboBox = ImDataSimpleCombo;
    using IDCColorPicker = ImDataSimpleColor4;  // RGBA is more common than RGB
    using IDCVector3 = ImDataSimpleVector3;
    
    // Container alias
    using IDCContainer = ImDataValueContainer;
    
    // ============================================================================
    // Namespace-scoped aliases for different widget types
    // This avoids top-level namespace pollution while keeping names short
    // ============================================================================
    
    namespace Simple {
        // Users can do: using namespace ImDataControls::Simple;
        // if they want these shorter names in their scope
        using Toggle = ImDataSimpleToggle;
        using FloatInput = ImDataSimpleFloat;
        using IntInput = ImDataSimpleInt32;
        using StringInput = ImDataSimpleString;
        using TextBox = ImDataSimpleTextMultiline;
        using FloatSlider = ImDataSimpleSliderFloat;
        using IntSlider = ImDataSimpleSliderInt32;
        using ComboBox = ImDataSimpleCombo;
    }
    
    namespace Monitored {
        // For widgets that sync with external data
        using Toggle = ImDataMonitoredToggle;
        using FloatInput = ImDataMonitoredFloat;
        using IntInput = ImDataMonitoredInt32;
        using StringInput = ImDataMonitoredString;
        using FloatSlider = ImDataMonitoredSliderFloat;
        using IntSlider = ImDataMonitoredSliderInt32;
        using ComboBox = ImDataMonitoredCombo;
    }
    
    namespace Config {
        // For settings with validation and deferred updates
        using Toggle = ImDataConfigToggle;
        using FloatInput = ImDataConfigFloat;
        using IntInput = ImDataConfigInt32;
        using StringInput = ImDataConfigString;
        using FloatSlider = ImDataConfigSliderFloat;
        using IntSlider = ImDataConfigSliderInt32;
    }
    
    // ============================================================================
    // Edit Mode Constants - Easier to use than the enum
    // ============================================================================
    namespace Edit {
        inline constexpr auto Editable = IEditModeControl::EditMode::Editable;
        inline constexpr auto ReadOnly = IEditModeControl::EditMode::ReadOnly;
        inline constexpr auto ViewOnly = IEditModeControl::EditMode::ViewOnly;
    }
    
    // Or just bring the enum into scope for convenience
    using IDCEditMode = IEditModeControl::EditMode;
    
    // ============================================================================
    // Note: Legacy ImGui* aliases are already defined in BasicWidgets.hpp and
    // AdvancedWidgets.hpp for backward compatibility
    // ============================================================================
    
    // ============================================================================
    // Builder Convenience Functions
    // These combine widget creation and builder creation for common cases
    // ============================================================================
    
    // ============================================================================
    // Simple widget builders
    // ============================================================================
    inline auto build_toggle(bool initial = false) {
        return make_builder(ImDataSimpleToggle::create(initial));
    }
    
    inline auto build_float(float initial = 0.0f) {
        return make_builder(ImDataSimpleFloat::create(initial));
    }
    
    inline auto build_double(double initial = 0.0) {
        return make_builder(ImDataSimpleDouble::create(initial));
    }
    
    inline auto build_int(int32_t initial = 0) {
        return make_builder(ImDataSimpleInt32::create(initial));
    }
    
    inline auto build_int64(int64_t initial = 0) {
        return make_builder(ImDataSimpleInt64::create(initial));
    }
    
    inline auto build_uint8(uint8_t initial = 0) {
        return make_builder(ImDataSimpleUInt8::create(initial));
    }
    
    inline auto build_uint16(uint16_t initial = 0) {
        return make_builder(ImDataSimpleUInt16::create(initial));
    }
    
    inline auto build_uint32(uint32_t initial = 0) {
        return make_builder(ImDataSimpleUInt32::create(initial));
    }
    
    inline auto build_uint64(uint64_t initial = 0) {
        return make_builder(ImDataSimpleUInt64::create(initial));
    }
    
    inline auto build_string(const std::string& initial = "") {
        return make_builder(ImDataSimpleString::create(initial));
    }
    
    inline auto build_text_multiline(const std::string& initial = "", const ImVec2& size = ImVec2(-1, 0)) {
        return make_builder(ImDataSimpleTextMultiline::create(initial, size));
    }
    
    // ============================================================================
    // Simple slider builders
    // ============================================================================
    inline auto build_slider(float min, float max, float initial = 0.0f) {
        return make_builder(ImDataSimpleSliderFloat::create(min, max, initial));
    }
    
    inline auto build_slider_int(int32_t min, int32_t max, int32_t initial = 0) {
        return make_builder(ImDataSimpleSliderInt32::create(min, max, initial));
    }
    
    inline auto build_slider_double(double min, double max, double initial = 0.0, bool show_precision = false) {
        return make_builder(ImDataSimpleSliderDouble::create(min, max, initial, show_precision));
    }
    
    inline auto build_slider_uint8(uint8_t min, uint8_t max, uint8_t initial = 0) {
        return make_builder(ImDataSimpleSliderUInt8::create(min, max, initial));
    }
    
    inline auto build_slider_uint16(uint16_t min, uint16_t max, uint16_t initial = 0) {
        return make_builder(ImDataSimpleSliderUInt16::create(min, max, initial));
    }
    
    // ============================================================================
    // Simple advanced widget builders
    // ============================================================================
    inline auto build_combo(const std::vector<std::string>& options, int32_t initial = 0) {
        return make_builder(std::make_unique<ImDataSimpleCombo>(options, initial));
    }
    
    inline auto build_drag_float(float initial = 0.0f, float speed = 1.0f, float min = 0.0f, float max = 0.0f) {
        return make_builder(std::make_unique<ImDataSimpleDragFloat>(initial, speed, min, max));
    }
    
    inline auto build_drag_int(int32_t initial = 0, float speed = 1.0f, int32_t min = 0, int32_t max = 0) {
        return make_builder(std::make_unique<ImDataSimpleDragInt>(initial, speed, min, max));
    }
    
    inline auto build_drag_double(double initial = 0.0, float speed = 1.0f, double min = 0.0, double max = 0.0) {
        return make_builder(std::make_unique<ImDataSimpleDragDouble>(initial, speed, min, max));
    }
    
    inline auto build_color3(float r = 1.0f, float g = 1.0f, float b = 1.0f) {
        return make_builder(std::make_unique<ImDataSimpleColor3>(r, g, b));
    }
    
    inline auto build_color4(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) {
        return make_builder(std::make_unique<ImDataSimpleColor4>(r, g, b, a));
    }
  
    
    inline auto build_vector2(float x = 0.0f, float y = 0.0f) {
        return make_builder(std::make_unique<ImDataSimpleVector2>(x, y));
    }
    
    inline auto build_vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) {
        return make_builder(std::make_unique<ImDataSimpleVector3>(x, y, z));
    }
    
    inline auto build_radio_button(const std::vector<std::string>& options, int32_t initial = 0) {
        return make_builder(std::make_unique<ImDataSimpleRadioButton>(options, initial));
    }
    
    // ============================================================================
    // Monitored widget builders (with getter/setter)
    // ============================================================================
    inline auto build_monitored_toggle(std::function<bool()> getter, 
                                      std::function<void(bool)> setter, 
                                      bool default_value = false) {
        return make_builder(ImDataMonitoredToggle::create(std::move(getter), 
                                                         std::move(setter), 
                                                         default_value));
    }
    
    inline auto build_monitored_float(std::function<float()> getter,
                                     std::function<void(float)> setter,
                                     float default_value = 0.0f) {
        return make_builder(ImDataMonitoredFloat::create(std::move(getter),
                                                        std::move(setter),
                                                        default_value));
    }
    
    inline auto build_monitored_double(std::function<double()> getter,
                                      std::function<void(double)> setter,
                                      double default_value = 0.0) {
        return make_builder(ImDataMonitoredDouble::create(std::move(getter),
                                                         std::move(setter),
                                                         default_value));
    }
    
    inline auto build_monitored_int32(std::function<int32_t()> getter,
                                     std::function<void(int32_t)> setter,
                                     int32_t default_value = 0) {
        return make_builder(ImDataMonitoredInt32::create(std::move(getter),
                                                        std::move(setter),
                                                        default_value));
    }
    
    inline auto build_monitored_int64(std::function<int64_t()> getter,
                                     std::function<void(int64_t)> setter,
                                     int64_t default_value = 0) {
        return make_builder(ImDataMonitoredInt64::create(std::move(getter),
                                                        std::move(setter),
                                                        default_value));
    }
    
    inline auto build_monitored_uint8(std::function<uint8_t()> getter,
                                     std::function<void(uint8_t)> setter,
                                     uint8_t default_value = 0) {
        return make_builder(ImDataMonitoredUInt8::create(std::move(getter),
                                                        std::move(setter),
                                                        default_value));
    }
    
    inline auto build_monitored_uint16(std::function<uint16_t()> getter,
                                      std::function<void(uint16_t)> setter,
                                      uint16_t default_value = 0) {
        return make_builder(ImDataMonitoredUInt16::create(std::move(getter),
                                                         std::move(setter),
                                                         default_value));
    }
    
    inline auto build_monitored_uint32(std::function<uint32_t()> getter,
                                      std::function<void(uint32_t)> setter,
                                      uint32_t default_value = 0) {
        return make_builder(ImDataMonitoredUInt32::create(std::move(getter),
                                                         std::move(setter),
                                                         default_value));
    }
    
    inline auto build_monitored_uint64(std::function<uint64_t()> getter,
                                      std::function<void(uint64_t)> setter,
                                      uint64_t default_value = 0) {
        return make_builder(ImDataMonitoredUInt64::create(std::move(getter),
                                                         std::move(setter),
                                                         default_value));
    }
    
    inline auto build_monitored_string(std::function<std::string()> getter,
                                      std::function<void(const std::string&)> setter,
                                      const std::string& default_value = "") {
        return make_builder(ImDataMonitoredString::create(std::move(getter),
                                                         std::move(setter),
                                                         default_value));
    }
    
    inline auto build_monitored_text_multiline(std::function<std::string()> getter,
                                              std::function<void(const std::string&)> setter,
                                              const std::string& default_value = "",
                                              const ImVec2& size = ImVec2(-1, 0)) {
        return make_builder(ImDataMonitoredTextMultiline::create(std::move(getter),
                                                                std::move(setter),
                                                                default_value,
                                                                size));
    }
    
    // ============================================================================
    // Monitored widget builders with text representation
    // ============================================================================
    inline auto build_monitored_toggle_with_text(std::function<bool()> getter,
                                                 std::function<void(bool)> setter,
                                                 bool default_value = false) {
        return make_builder(ImDataMonitoredToggleWithText::create(std::move(getter),
                                                                 std::move(setter),
                                                                 default_value));
    }
    
    inline auto build_monitored_float_with_text(std::function<float()> getter,
                                               std::function<void(float)> setter,
                                               float default_value = 0.0f) {
        return make_builder(ImDataMonitoredFloatWithText::create(std::move(getter),
                                                                std::move(setter),
                                                                default_value));
    }
    
    inline auto build_monitored_double_with_text(std::function<double()> getter,
                                                std::function<void(double)> setter,
                                                double default_value = 0.0) {
        return make_builder(ImDataMonitoredDoubleWithText::create(std::move(getter),
                                                                 std::move(setter),
                                                                 default_value));
    }
    
    inline auto build_monitored_int32_with_text(std::function<int32_t()> getter,
                                               std::function<void(int32_t)> setter,
                                               int32_t default_value = 0) {
        return make_builder(ImDataMonitoredInt32WithText::create(std::move(getter),
                                                                std::move(setter),
                                                                default_value));
    }
    
    inline auto build_monitored_int64_with_text(std::function<int64_t()> getter,
                                               std::function<void(int64_t)> setter,
                                               int64_t default_value = 0) {
        return make_builder(ImDataMonitoredInt64WithText::create(std::move(getter),
                                                                std::move(setter),
                                                                default_value));
    }
    
    inline auto build_monitored_uint8_with_text(std::function<uint8_t()> getter,
                                               std::function<void(uint8_t)> setter,
                                               uint8_t default_value = 0) {
        return make_builder(ImDataMonitoredUInt8WithText::create(std::move(getter),
                                                                std::move(setter),
                                                                default_value));
    }
    
    inline auto build_monitored_uint16_with_text(std::function<uint16_t()> getter,
                                                std::function<void(uint16_t)> setter,
                                                uint16_t default_value = 0) {
        return make_builder(ImDataMonitoredUInt16WithText::create(std::move(getter),
                                                                 std::move(setter),
                                                                 default_value));
    }
    
    inline auto build_monitored_uint32_with_text(std::function<uint32_t()> getter,
                                                std::function<void(uint32_t)> setter,
                                                uint32_t default_value = 0) {
        return make_builder(ImDataMonitoredUInt32WithText::create(std::move(getter),
                                                                 std::move(setter),
                                                                 default_value));
    }
    
    inline auto build_monitored_uint64_with_text(std::function<uint64_t()> getter,
                                                std::function<void(uint64_t)> setter,
                                                uint64_t default_value = 0) {
        return make_builder(ImDataMonitoredUInt64WithText::create(std::move(getter),
                                                                 std::move(setter),
                                                                 default_value));
    }
    
    // ============================================================================
    // Monitored slider builders
    // ============================================================================
    inline auto build_monitored_slider(std::function<float()> getter,
                                      std::function<void(float)> setter,
                                      float min, float max,
                                      float default_value = 0.0f) {
        return make_builder(ImDataMonitoredSliderFloat::create(std::move(getter),
                                                              std::move(setter),
                                                              min, max,
                                                              default_value));
    }
    
    inline auto build_monitored_slider_int(std::function<int32_t()> getter,
                                          std::function<void(int32_t)> setter,
                                          int32_t min, int32_t max,
                                          int32_t default_value = 0) {
        return make_builder(ImDataMonitoredSliderInt32::create(std::move(getter),
                                                               std::move(setter),
                                                               min, max,
                                                               default_value));
    }
    
    inline auto build_monitored_slider_double(std::function<double()> getter,
                                             std::function<void(double)> setter,
                                             double min, double max,
                                             double default_value = 0.0,
                                             bool show_precision = false) {
        return make_builder(ImDataMonitoredSliderDouble::create(std::move(getter),
                                                               std::move(setter),
                                                               min, max,
                                                               default_value,
                                                               show_precision));
    }
    
    inline auto build_monitored_slider_uint8(std::function<uint8_t()> getter,
                                            std::function<void(uint8_t)> setter,
                                            uint8_t min, uint8_t max,
                                            uint8_t default_value = 0) {
        return make_builder(ImDataMonitoredSliderUInt8::create(std::move(getter),
                                                               std::move(setter),
                                                               min, max,
                                                               default_value));
    }
    
    inline auto build_monitored_slider_uint16(std::function<uint16_t()> getter,
                                             std::function<void(uint16_t)> setter,
                                             uint16_t min, uint16_t max,
                                             uint16_t default_value = 0) {
        return make_builder(ImDataMonitoredSliderUInt16::create(std::move(getter),
                                                                std::move(setter),
                                                                min, max,
                                                                default_value));
    }
    
    // ============================================================================
    // Monitored slider builders with text representation
    // ============================================================================
    inline auto build_monitored_slider_with_text(std::function<float()> getter,
                                                std::function<void(float)> setter,
                                                float min, float max,
                                                float default_value = 0.0f) {
        return make_builder(ImDataMonitoredSliderFloatWithText::create(std::move(getter),
                                                                       std::move(setter),
                                                                       min, max,
                                                                       default_value));
    }
    
    inline auto build_monitored_slider_int_with_text(std::function<int32_t()> getter,
                                                    std::function<void(int32_t)> setter,
                                                    int32_t min, int32_t max,
                                                    int32_t default_value = 0) {
        return make_builder(ImDataMonitoredSliderInt32WithText::create(std::move(getter),
                                                                       std::move(setter),
                                                                       min, max,
                                                                       default_value));
    }
    
    inline auto build_monitored_slider_double_with_text(std::function<double()> getter,
                                                       std::function<void(double)> setter,
                                                       double min, double max,
                                                       double default_value = 0.0,
                                                       bool show_precision = false) {
        return make_builder(ImDataMonitoredSliderDoubleWithText::create(std::move(getter),
                                                                        std::move(setter),
                                                                        min, max,
                                                                        default_value,
                                                                        show_precision));
    }
    
    inline auto build_monitored_slider_uint8_with_text(std::function<uint8_t()> getter,
                                                      std::function<void(uint8_t)> setter,
                                                      uint8_t min, uint8_t max,
                                                      uint8_t default_value = 0) {
        return make_builder(ImDataMonitoredSliderUInt8WithText::create(std::move(getter),
                                                                       std::move(setter),
                                                                       min, max,
                                                                       default_value));
    }
    
    inline auto build_monitored_slider_uint16_with_text(std::function<uint16_t()> getter,
                                                       std::function<void(uint16_t)> setter,
                                                       uint16_t min, uint16_t max,
                                                       uint16_t default_value = 0) {
        return make_builder(ImDataMonitoredSliderUInt16WithText::create(std::move(getter),
                                                                        std::move(setter),
                                                                        min, max,
                                                                        default_value));
    }
    
    // ============================================================================
    // Monitored advanced widget builders
    // ============================================================================
    inline auto build_monitored_combo(std::function<int32_t()> getter,
                                     std::function<void(int32_t)> setter,
                                     const std::vector<std::string>& options,
                                     int32_t default_value = 0) {
        return make_builder(std::make_unique<ImDataMonitoredCombo>(options,
                                                                  std::move(getter),
                                                                  std::move(setter),
                                                                  default_value));
    }
    
    inline auto build_monitored_drag_float(std::function<float()> getter,
                                          std::function<void(float)> setter,
                                          float default_value = 0.0f,
                                          float speed = 1.0f,
                                          float min = 0.0f,
                                          float max = 0.0f) {
        return make_builder(std::make_unique<ImDataMonitoredDragFloat>(speed, min, max,
                                                                       std::move(getter),
                                                                       std::move(setter),
                                                                       default_value));
    }
    
    inline auto build_monitored_drag_int(std::function<int32_t()> getter,
                                        std::function<void(int32_t)> setter,
                                        int32_t default_value = 0,
                                        float speed = 1.0f,
                                        int32_t min = 0,
                                        int32_t max = 0) {
        return make_builder(std::make_unique<ImDataMonitoredDragInt>(speed, min, max,
                                                                     std::move(getter),
                                                                     std::move(setter),
                                                                     default_value));
    }
    
    inline auto build_monitored_drag_double(std::function<double()> getter,
                                           std::function<void(double)> setter,
                                           double default_value = 0.0,
                                           float speed = 1.0f,
                                           double min = 0.0,
                                           double max = 0.0) {
        return make_builder(std::make_unique<ImDataMonitoredDragDouble>(speed, min, max,
                                                                        std::move(getter),
                                                                        std::move(setter),
                                                                        default_value));
    }
    
    inline auto build_monitored_color3(std::function<std::array<float, 3>()> getter,
                                      std::function<void(const std::array<float, 3>&)> setter,
                                      float r = 1.0f, float g = 1.0f, float b = 1.0f) {
        return make_builder(std::make_unique<ImDataMonitoredColor3>(std::move(getter),
                                                                    std::move(setter),
                                                                    r, g, b));
    }
    
    inline auto build_monitored_color4(std::function<std::array<float, 4>()> getter,
                                      std::function<void(const std::array<float, 4>&)> setter,
                                      float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) {
        return make_builder(std::make_unique<ImDataMonitoredColor4>(std::move(getter),
                                                                    std::move(setter),
                                                                    r, g, b, a));
    }
    
    inline auto build_monitored_vector2(std::function<std::array<float, 2>()> getter,
                                       std::function<void(const std::array<float, 2>&)> setter,
                                       float x = 0.0f, float y = 0.0f) {
        return make_builder(std::make_unique<ImDataMonitoredVector2>(std::move(getter),
                                                                     std::move(setter),
                                                                     x, y));
    }
    
    inline auto build_monitored_vector3(std::function<std::array<float, 3>()> getter,
                                       std::function<void(const std::array<float, 3>&)> setter,
                                       float x = 0.0f, float y = 0.0f, float z = 0.0f) {
        return make_builder(std::make_unique<ImDataMonitoredVector3>(std::move(getter),
                                                                     std::move(setter),
                                                                     x, y, z));
    }
    
    inline auto build_monitored_radio_button(std::function<int32_t()> getter,
                                            std::function<void(int32_t)> setter,
                                            const std::vector<std::string>& options,
                                            int32_t default_value = 0) {
        return make_builder(std::make_unique<ImDataMonitoredRadioButton>(options,
                                                                         std::move(getter),
                                                                         std::move(setter),
                                                                         default_value));
    }
    
    // ============================================================================
    // Config widget builders
    // ============================================================================
    inline auto build_config_toggle(bool default_value = false) {
        return make_builder(ImDataConfigToggle::create(default_value));
    }
    
    inline auto build_config_float(float default_value = 0.0f) {
        return make_builder(ImDataConfigFloat::create(default_value));
    }
    
    inline auto build_config_double(double default_value = 0.0) {
        return make_builder(ImDataConfigDouble::create(default_value));
    }
    
    inline auto build_config_int32(int32_t default_value = 0) {
        return make_builder(ImDataConfigInt32::create(default_value));
    }
    
    inline auto build_config_string(const std::string& default_value = "") {
        return make_builder(ImDataConfigString::create(default_value));
    }
    
    inline auto build_config_slider(float min, float max, float default_value = 0.0f) {
        return make_builder(ImDataConfigSliderFloat::create(min, max, default_value));
    }
    
    inline auto build_config_slider_int(int32_t min, int32_t max, int32_t default_value = 0) {
        return make_builder(ImDataConfigSliderInt32::create(min, max, default_value));
    }
    
    inline auto build_config_combo(const std::vector<std::string>& options, int32_t default_value = 0) {
        return make_builder(std::make_unique<ImDataConfigCombo>(options, default_value));
    }
    
    // Usage examples:
    // auto toggle = build_toggle(true)
    //     .with_name("Enable Feature")
    //     .with_tooltip("Turn this on to enable the special feature")
    //     .build();
    //
    // auto slider = build_slider(0.0f, 100.0f, 50.0f)
    //     .with_name("Volume")
    //     .read_only()
    //     .build();
}