#pragma once
#include "BasicWidgets.hpp"
#include "AdvancedWidgets.hpp"
#include <memory>
#include <string>

namespace RC::ImDataControls {

template<typename WidgetType>
class WidgetBuilder {
private:
    std::unique_ptr<WidgetType> m_widget;
    
public:
    WidgetBuilder() = default;
    
    explicit WidgetBuilder(std::unique_ptr<WidgetType> widget) 
        : m_widget(std::move(widget)) {}
    
    // Basic setters available for all widgets
    WidgetBuilder& with_name(std::string name) {
        if (m_widget) {
            m_widget->set_name(std::move(name));
        }
        return *this;
    }
    
    WidgetBuilder& with_tooltip(std::string tooltip) {
        if (m_widget) {
            m_widget->set_tooltip(std::move(tooltip));
        }
        return *this;
    }
    
    WidgetBuilder& with_edit_mode(IEditModeControl::EditMode mode) {
        if (m_widget) {
            m_widget->set_edit_mode(mode);
        }
        return *this;
    }
    
    WidgetBuilder& read_only() {
        return with_edit_mode(IEditModeControl::EditMode::ReadOnly);
    }
    
    WidgetBuilder& view_only() {
        return with_edit_mode(IEditModeControl::EditMode::ViewOnly);
    }
    
    // Conditional setters using if constexpr
    template<typename Validator>
    WidgetBuilder& with_validator(Validator&& validator) {
        if (m_widget) {
            if constexpr (requires { m_widget->set_validator(validator); }) {
                m_widget->set_validator(std::forward<Validator>(validator));
            }
        }
        return *this;
    }
    
    WidgetBuilder& with_default_value(const typename WidgetType::value_type& value) {
        if (m_widget) {
            if constexpr (requires { m_widget->set_default_value(value); }) {
                m_widget->set_default_value(value);
            }
        }
        return *this;
    }
    
    WidgetBuilder& with_text_format(const std::string& format) {
        if (m_widget) {
            if constexpr (requires { m_widget->set_text_format(format); }) {
                m_widget->set_text_format(format);
            }
        }
        return *this;
    }
    
    WidgetBuilder& show_text(bool show = true) {
        if (m_widget) {
            if constexpr (requires { m_widget->set_show_text_representation(show); }) {
                m_widget->set_show_text_representation(show);
            }
        }
        return *this;
    }
    
    WidgetBuilder& advanced() {
        if (m_widget) {
            if constexpr (requires { m_widget->set_advanced(true); }) {
                m_widget->set_advanced(true);
            }
        }
        return *this;
    }
    
    WidgetBuilder& in_group(const std::string& group) {
        if (m_widget) {
            if constexpr (requires { m_widget->set_group(group); }) {
                m_widget->set_group(group);
            }
        }
        return *this;
    }
    
    // Widget-specific methods
    WidgetBuilder& with_range(auto min, auto max) {
        if (m_widget) {
            if constexpr (requires { m_widget->set_range(min, max); }) {
                m_widget->set_range(min, max);
            }
        }
        return *this;
    }
    
    WidgetBuilder& with_speed(float speed) {
        if (m_widget) {
            if constexpr (requires { m_widget->set_speed(speed); }) {
                m_widget->set_speed(speed);
            }
        }
        return *this;
    }
    
    WidgetBuilder& with_options(const std::vector<std::string>& options) {
        if (m_widget) {
            if constexpr (requires { m_widget->set_options(options); }) {
                m_widget->set_options(options);
            }
        }
        return *this;
    }
    
    template<typename Callback>
    WidgetBuilder& on_change(Callback&& callback) {
        if (m_widget) {
            if constexpr (requires { m_widget->set_on_change_callback(callback); }) {
                m_widget->set_on_change_callback(std::forward<Callback>(callback));
            }
            else if constexpr (requires { m_widget->set_change_callback(callback); }) {
                m_widget->set_change_callback(std::forward<Callback>(callback));
            }
        }
        return *this;
    }
    
    [[nodiscard]] std::unique_ptr<WidgetType> build() {
        return std::move(m_widget);
    }
    
    [[nodiscard]] WidgetType* get() {
        return m_widget.get();
    }
};

// Helper factory
template<typename WidgetType>
auto make_builder(std::unique_ptr<WidgetType> widget) {
    return WidgetBuilder<WidgetType>(std::move(widget));
}

// Usage examples:
// 
// Simple toggle:
// auto toggle = make_builder(std::make_unique<ImDataSimpleToggle>(true))
//     .with_name("Enable Feature")
//     .with_tooltip("Turn this on to enable the special feature")
//     .build();
//
// Monitored float with callbacks:
// auto speed = make_builder(std::make_unique<ImDataMonitoredFloat>(
//         []() { return GetGameSpeed(); },
//         [](float v) { SetGameSpeed(v); },
//         1.0f))
//     .with_name("Game Speed")
//     .with_range(0.1f, 10.0f)
//     .on_change([]() { LogInfo("Speed changed!"); })
//     .build();
//
// Slider with validation:
// auto slider = make_builder(std::make_unique<ImDataConfigSliderFloat>(0.0f, 100.0f, 50.0f))
//     .with_name("Volume")
//     .with_validator([](float v) { return v >= 0.0f && v <= 100.0f; })
//     .with_default_value(75.0f)
//     .build();
//
// Read-only text:
// auto text = make_builder(std::make_unique<ImDataSimpleString>("Status"))
//     .with_name("Current Status")
//     .read_only()
//     .build();
//
// Advanced combo box:
// auto combo = make_builder(std::make_unique<ImDataMonitoredCombo>(
//         options_vector,
//         []() { return GetCurrentMode(); },
//         [](int v) { SetCurrentMode(v); },
//         0))
//     .with_name("Render Mode")
//     .advanced()
//     .in_group("Graphics")
//     .build();

} // namespace RC::ImDataControls