#pragma once

#include "BasicWidgets.hpp"
#include "AdvancedWidgets.hpp"
#include <memory>
#include <functional>

namespace RC::ImDataControls {

// Factory functions for monitored values (without text representation)

[[nodiscard]] inline auto make_monitored_toggle(
    std::function<bool()> getter,
    std::function<void(bool)> setter,
    bool default_value = false)
{
    return MonitoredToggle::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_float(
    std::function<float()> getter,
    std::function<void(float)> setter,
    float default_value = 0.0f)
{
    return MonitoredFloat::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_double(
    std::function<double()> getter,
    std::function<void(double)> setter,
    double default_value = 0.0)
{
    return MonitoredDouble::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_int32(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    int32_t default_value = 0)
{
    return MonitoredInt32::create(std::move(getter), std::move(setter), default_value);
}

// Factory functions for monitored values with text representation

[[nodiscard]] inline auto make_monitored_toggle_with_text(
    std::function<bool()> getter,
    std::function<void(bool)> setter,
    bool default_value = false)
{
    return MonitoredToggleWithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_float_with_text(
    std::function<float()> getter,
    std::function<void(float)> setter,
    float default_value = 0.0f)
{
    return MonitoredFloatWithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_double_with_text(
    std::function<double()> getter,
    std::function<void(double)> setter,
    double default_value = 0.0)
{
    return MonitoredDoubleWithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_int32_with_text(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    int32_t default_value = 0)
{
    return MonitoredInt32WithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_string(
    std::function<std::string()> getter,
    std::function<void(const std::string&)> setter,
    const std::string& default_value = "")
{
    return MonitoredString::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_int64(
    std::function<int64_t()> getter,
    std::function<void(int64_t)> setter,
    int64_t default_value = 0)
{
    return MonitoredInt64::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_uint8(
    std::function<uint8_t()> getter,
    std::function<void(uint8_t)> setter,
    uint8_t default_value = 0)
{
    return MonitoredUInt8::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_uint16(
    std::function<uint16_t()> getter,
    std::function<void(uint16_t)> setter,
    uint16_t default_value = 0)
{
    return MonitoredUInt16::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_uint32(
    std::function<uint32_t()> getter,
    std::function<void(uint32_t)> setter,
    uint32_t default_value = 0)
{
    return MonitoredUInt32::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_uint64(
    std::function<uint64_t()> getter,
    std::function<void(uint64_t)> setter,
    uint64_t default_value = 0)
{
    return MonitoredUInt64::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_text_multiline(
    std::function<std::string()> getter,
    std::function<void(const std::string&)> setter,
    const std::string& default_value = "",
    const ImVec2& size = ImVec2(-1, 0))
{
    return MonitoredTextMultiline::create(std::move(getter), std::move(setter), default_value, size);
}

// Factory functions for monitored sliders (without text representation)

[[nodiscard]] inline auto make_monitored_slider(
    std::function<float()> getter,
    std::function<void(float)> setter,
    float min_val,
    float max_val,
    float default_value = 0.0f)
{
    return MonitoredSliderFloat::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_monitored_slider_int(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    int32_t min_val,
    int32_t max_val,
    int32_t default_value = 0)
{
    return MonitoredSliderInt32::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_monitored_slider_double(
    std::function<double()> getter,
    std::function<void(double)> setter,
    double min_val,
    double max_val,
    double default_value = 0.0,
    bool show_precision_input = false)
{
    return MonitoredSliderDouble::create(std::move(getter), std::move(setter), min_val, max_val, default_value, show_precision_input);
}

// Factory functions for monitored values with text representation (missing ones)

[[nodiscard]] inline auto make_monitored_int64_with_text(
    std::function<int64_t()> getter,
    std::function<void(int64_t)> setter,
    int64_t default_value = 0)
{
    return MonitoredInt64WithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_uint8_with_text(
    std::function<uint8_t()> getter,
    std::function<void(uint8_t)> setter,
    uint8_t default_value = 0)
{
    return MonitoredUInt8WithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_uint16_with_text(
    std::function<uint16_t()> getter,
    std::function<void(uint16_t)> setter,
    uint16_t default_value = 0)
{
    return MonitoredUInt16WithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_uint32_with_text(
    std::function<uint32_t()> getter,
    std::function<void(uint32_t)> setter,
    uint32_t default_value = 0)
{
    return MonitoredUInt32WithText::create(std::move(getter), std::move(setter), default_value);
}

[[nodiscard]] inline auto make_monitored_uint64_with_text(
    std::function<uint64_t()> getter,
    std::function<void(uint64_t)> setter,
    uint64_t default_value = 0)
{
    return MonitoredUInt64WithText::create(std::move(getter), std::move(setter), default_value);
}

// Factory functions for monitored sliders with text representation

[[nodiscard]] inline auto make_monitored_slider_with_text(
    std::function<float()> getter,
    std::function<void(float)> setter,
    float min_val,
    float max_val,
    float default_value = 0.0f)
{
    return MonitoredSliderFloatWithText::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_monitored_slider_int_with_text(
    std::function<int32_t()> getter,
    std::function<void(int32_t)> setter,
    int32_t min_val,
    int32_t max_val,
    int32_t default_value = 0)
{
    return MonitoredSliderInt32WithText::create(std::move(getter), std::move(setter), min_val, max_val, default_value);
}

[[nodiscard]] inline auto make_monitored_slider_double_with_text(
    std::function<double()> getter,
    std::function<void(double)> setter,
    double min_val,
    double max_val,
    double default_value = 0.0,
    bool show_precision_input = false)
{
    return MonitoredSliderDoubleWithText::create(std::move(getter), std::move(setter), min_val, max_val, default_value, show_precision_input);
}

// Convenience template for creating monitored values from member pointers
template<typename ObjectType, typename ValueType>
[[nodiscard]] auto make_monitored_member(
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
        return make_monitored_toggle(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, float>) {
        return make_monitored_float(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, double>) {
        return make_monitored_double(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, int32_t>) {
        return make_monitored_int32(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, int64_t>) {
        return make_monitored_int64(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, uint8_t>) {
        return make_monitored_uint8(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, uint16_t>) {
        return make_monitored_uint16(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, uint32_t>) {
        return make_monitored_uint32(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, uint64_t>) {
        return make_monitored_uint64(getter, setter, default_value);
    }
    else if constexpr (std::is_same_v<ValueType, std::string>) {
        return make_monitored_string(getter, setter, default_value);
    }
    else {
        static_assert(sizeof(ValueType) == 0, "Unsupported type for make_monitored_member");
    }
}

} // namespace RC::ImDataControls