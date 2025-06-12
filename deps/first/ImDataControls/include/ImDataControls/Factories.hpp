#pragma once

#include "BasicWidgets.hpp"
#include "AdvancedWidgets.hpp"
#include <memory>
#include <functional>

namespace RC::ImDataControls {

// Factory functions for monitored values (without text representation)

[[nodiscard]] inline auto make_imdata_monitored_toggle(
    std::function<bool()> getter,
    std::function<void(bool)> setter,
    bool default_value = false)
{
    return ImDataMonitoredToggle::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_float(
    std::function<float()> getter,
    std::function<void(float)> setter,
    float default_value = 0.0f)
{
    return ImDataMonitoredFloat::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_double(
    std::function<double()> getter,
    std::function<void(double)> setter,
    double default_value = 0.0)
{
    return ImDataMonitoredDouble::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_int32(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    int32_t default_value = 0)
{
    return ImDataMonitoredInt32::create(std::move(getter), std::move(setter), default_value);
}

// Factory functions for monitored values with text representation

[[nodiscard]] inline auto make_imdata_monitored_toggle_with_text(
    std::function<bool()> getter,
    std::function<void(bool)> setter,
    bool default_value = false)
{
    return ImDataMonitoredToggleWithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_float_with_text(
    std::function<float()> getter,
    std::function<void(float)> setter,
    float default_value = 0.0f)
{
    return ImDataMonitoredFloatWithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_double_with_text(
    std::function<double()> getter,
    std::function<void(double)> setter,
    double default_value = 0.0)
{
    return ImDataMonitoredDoubleWithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_int32_with_text(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    int32_t default_value = 0)
{
    return ImDataMonitoredInt32WithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_string(
    std::function<std::string()> getter,
    std::function<void(const std::string&)> setter,
    const std::string& default_value = "")
{
    return ImDataMonitoredString::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_int64(
    std::function<int64_t()> getter,
    std::function<void(int64_t)> setter,
    int64_t default_value = 0)
{
    return ImDataMonitoredInt64::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_uint8(
    std::function<uint8_t()> getter,
    std::function<void(uint8_t)> setter,
    uint8_t default_value = 0)
{
    return ImDataMonitoredUInt8::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_uint16(
    std::function<uint16_t()> getter,
    std::function<void(uint16_t)> setter,
    uint16_t default_value = 0)
{
    return ImDataMonitoredUInt16::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_uint32(
    std::function<uint32_t()> getter,
    std::function<void(uint32_t)> setter,
    uint32_t default_value = 0)
{
    return ImDataMonitoredUInt32::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_uint64(
    std::function<uint64_t()> getter,
    std::function<void(uint64_t)> setter,
    uint64_t default_value = 0)
{
    return ImDataMonitoredUInt64::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_text_multiline(
    std::function<std::string()> getter,
    std::function<void(const std::string&)> setter,
    const std::string& default_value = "",
    const ImVec2& size = ImVec2(-1, 0))
{
    return ImDataMonitoredTextMultiline::create(std::move(getter), std::move(setter), default_value, size);
}

// Factory functions for monitored sliders (without text representation)

[[nodiscard]] inline auto make_imdata_monitored_slider(
    std::function<float()> getter,
    std::function<void(float)> setter,
    float min_val,
    float max_val,
    float default_value = 0.0f)
{
    return ImDataMonitoredSliderFloat::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_slider_int(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    int32_t min_val,
    int32_t max_val,
    int32_t default_value = 0)
{
    return ImDataMonitoredSliderInt32::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_slider_double(
    std::function<double()> getter,
    std::function<void(double)> setter,
    double min_val,
    double max_val,
    double default_value = 0.0,
    bool show_precision_input = false)
{
    return ImDataMonitoredSliderDouble::create(std::move(getter), std::move(setter), min_val, max_val, default_value, show_precision_input);
}

// Factory functions for monitored values with text representation (missing ones)

[[nodiscard]] inline auto make_imdata_monitored_int64_with_text(
    std::function<int64_t()> getter,
    std::function<void(int64_t)> setter,
    int64_t default_value = 0)
{
    return ImDataMonitoredInt64WithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_uint8_with_text(
    std::function<uint8_t()> getter,
    std::function<void(uint8_t)> setter,
    uint8_t default_value = 0)
{
    return ImDataMonitoredUInt8WithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_uint16_with_text(
    std::function<uint16_t()> getter,
    std::function<void(uint16_t)> setter,
    uint16_t default_value = 0)
{
    return ImDataMonitoredUInt16WithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_uint32_with_text(
    std::function<uint32_t()> getter,
    std::function<void(uint32_t)> setter,
    uint32_t default_value = 0)
{
    return ImDataMonitoredUInt32WithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_uint64_with_text(
    std::function<uint64_t()> getter,
    std::function<void(uint64_t)> setter,
    uint64_t default_value = 0)
{
    return ImDataMonitoredUInt64WithText::create(std::move(getter), std::move(setter), default_value);
}

// Factory functions for monitored sliders with text representation

[[nodiscard]] inline auto make_imdata_monitored_slider_with_text(
    std::function<float()> getter,
    std::function<void(float)> setter,
    float min_val,
    float max_val,
    float default_value = 0.0f)
{
    return ImDataMonitoredSliderFloatWithText::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_slider_int_with_text(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    int32_t min_val,
    int32_t max_val,
    int32_t default_value = 0)
{
    return ImDataMonitoredSliderInt32WithText::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_slider_double_with_text(
    std::function<double()> getter,
    std::function<void(double)> setter,
    double min_val,
    double max_val,
    double default_value = 0.0,
    bool show_precision_input = false)
{
    return ImDataMonitoredSliderDoubleWithText::create(std::move(getter), std::move(setter), min_val, max_val, default_value, show_precision_input);
}

[[nodiscard]] inline auto make_imdata_monitored_slider_uint8(
    std::function<uint8_t()> getter,
    std::function<void(uint8_t)> setter,
    uint8_t min_val,
    uint8_t max_val,
    uint8_t default_value = 0)
{
    return ImDataMonitoredSliderUInt8::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_slider_uint16(
    std::function<uint16_t()> getter,
    std::function<void(uint16_t)> setter,
    uint16_t min_val,
    uint16_t max_val,
    uint16_t default_value = 0)
{
    return ImDataMonitoredSliderUInt16::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_slider_uint8_with_text(
    std::function<uint8_t()> getter,
    std::function<void(uint8_t)> setter,
    uint8_t min_val,
    uint8_t max_val,
    uint8_t default_value = 0)
{
    return ImDataMonitoredSliderUInt8WithText::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_imdata_monitored_slider_uint16_with_text(
    std::function<uint16_t()> getter,
    std::function<void(uint16_t)> setter,
    uint16_t min_val,
    uint16_t max_val,
    uint16_t default_value = 0)
{
    return ImDataMonitoredSliderUInt16WithText::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

// Factory functions for advanced monitored widgets

[[nodiscard]] inline auto make_imdata_monitored_combo(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    const std::vector<std::string>& options,
    int32_t default_value = 0)
{
    auto combo = ImDataMonitoredCombo::create(getter, setter, default_value);
    combo->set_options(options);
    return combo;
}

[[nodiscard]] inline auto make_imdata_monitored_drag_float(
    std::function<float()> getter,
    std::function<void(float)> setter,
    float default_value = 0.0f,
    float speed = 1.0f,
    float min = 0.0f,
    float max = 0.0f)
{
    auto drag = ImDataMonitoredDragFloat::create(getter, setter, default_value);
    drag->set_speed(speed);
    drag->set_range(min, max);
    return drag;
}

[[nodiscard]] inline auto make_imdata_monitored_drag_int(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    int32_t default_value = 0,
    float speed = 1.0f,
    int32_t min = 0,
    int32_t max = 0)
{
    auto drag = ImDataMonitoredDragInt::create(getter, setter, default_value);
    drag->set_speed(speed);
    drag->set_range(min, max);
    return drag;
}

[[nodiscard]] inline auto make_imdata_monitored_drag_double(
    std::function<double()> getter,
    std::function<void(double)> setter,
    double default_value = 0.0,
    float speed = 1.0f,
    double min = 0.0,
    double max = 0.0)
{
    auto drag = ImDataMonitoredDragDouble::create(getter, setter, default_value);
    drag->set_speed(speed);
    drag->set_range(min, max);
    return drag;
}

[[nodiscard]] inline auto make_imdata_monitored_color3(
    std::function<std::array<float, 3>()> getter,
    std::function<void(const std::array<float, 3>&)> setter,
    float r = 1.0f,
    float g = 1.0f,
    float b = 1.0f)
{
    return ImDataMonitoredColor3::create(std::move(getter), std::move(setter), {r, g, b});
}

[[nodiscard]] inline auto make_imdata_monitored_color4(
    std::function<std::array<float, 4>()> getter,
    std::function<void(const std::array<float, 4>&)> setter,
    float r = 1.0f,
    float g = 1.0f,
    float b = 1.0f,
    float a = 1.0f)
{
    return ImDataMonitoredColor4::create(std::move(getter), std::move(setter), {r, g, b, a});
}

[[nodiscard]] inline auto make_imdata_monitored_vector2(
    std::function<std::array<float, 2>()> getter,
    std::function<void(const std::array<float, 2>&)> setter,
    float x = 0.0f,
    float y = 0.0f)
{
    return ImDataMonitoredVector2::create(std::move(getter), std::move(setter), {x, y});
}

[[nodiscard]] inline auto make_imdata_monitored_vector3(
    std::function<std::array<float, 3>()> getter,
    std::function<void(const std::array<float, 3>&)> setter,
    float x = 0.0f,
    float y = 0.0f,
    float z = 0.0f)
{
    return ImDataMonitoredVector3::create(std::move(getter), std::move(setter), {x, y, z});
}

[[nodiscard]] inline auto make_imdata_monitored_radio_button(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    const std::vector<std::string>& options,
    int32_t default_value = 0)
{
    auto radio = ImDataMonitoredRadioButton::create(getter, setter, default_value);
    radio->set_options(options);
    return radio;
}

// Convenience template for creating monitored values from member pointers
template<typename ObjectType, typename ValueType>
[[nodiscard]] auto make_imdata_monitored_member(
    ObjectType* object,
    ValueType ObjectType::*member,
    ValueType default_value = ValueType{})
{
    auto getter = [object, member]() -> ValueType {
        return object->*member;
    };
    
    auto setter = [object, member](const ValueType& value) {
        object->*member = value;
    };
    
    if constexpr (std::is_same_v<ValueType, bool>) {
        return make_imdata_monitored_toggle(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, float>) {
        return make_imdata_monitored_float(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, double>) {
        return make_imdata_monitored_double(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, int32_t>) {
        return make_imdata_monitored_int32(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, int64_t>) {
        return make_imdata_monitored_int64(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, uint8_t>) {
        return make_imdata_monitored_uint8(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, uint16_t>) {
        return make_imdata_monitored_uint16(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, uint32_t>) {
        return make_imdata_monitored_uint32(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, uint64_t>) {
        return make_imdata_monitored_uint64(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, std::string>) {
        return make_imdata_monitored_string(getter, setter, default_value);
    }
    else {
        static_assert(sizeof(ValueType) == 0, "Unsupported type for make_imdata_monitored_member");
    }
}

} // namespace RC::ImDataControls