#pragma once

#include "Core.hpp"
#include "BasicWidgets.hpp"
#include "AdvancedWidgets.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <typeindex>
#include <functional>

namespace RC::ImDataControls {

// Container for managing multiple ImGui values
class ImDataValueContainer {
public:
    template<RC::StringLike T = const char*>
    explicit ImDataValueContainer(T&& name = "") 
        : m_name(RC::to_utf8_string(std::forward<T>(name))) {}
    
    template<typename T>
    T* add_value(const std::string& id, std::unique_ptr<T> value) {
        static_assert(std::is_base_of_v<IValueControl, T>, 
                      "T must derive from IValueControl");
        T* ptr = value.get();
        m_values[id] = std::move(value);
        m_ordered_values.emplace_back(id, ptr);
        return ptr;
    }
    
    template<typename T>
    [[nodiscard]] T* get_value(const std::string& id) {
        auto it = m_values.find(id);
        if (it != m_values.end()) {
            return dynamic_cast<T*>(it->second.get());
        }
        return nullptr;
    }
    
    void remove_value(const std::string& id) {
        m_values.erase(id);
        m_ordered_values.erase(
            std::remove_if(m_ordered_values.begin(), m_ordered_values.end(),
                          [&id](const auto& pair) { return pair.first == id; }),
            m_ordered_values.end()
        );
    }
    
    void clear() {
        m_values.clear();
        m_ordered_values.clear();
    }
    
    void draw(bool show_apply_button = true) {
        if (!m_visible) return;
        
        // Title
        if (!m_name.empty()) {
            ImGui::Text("%s", m_name.c_str());
            ImGui::Separator();
        }
        
        // Edit mode control
        if (m_show_edit_mode_control) {
            bool is_editable = m_global_edit_mode == IEditModeControl::EditMode::Editable;
            if (ImGui::Checkbox("Allow Editing", &is_editable)) {
                set_global_edit_mode(is_editable ? IEditModeControl::EditMode::Editable 
                                                 : IEditModeControl::EditMode::ReadOnly);
            }
            ImGui::Spacing();
        }
        
        // Draw values
        for (const auto& [id, ptr] : m_ordered_values) {
            auto caps = ptr->get_capabilities();
            
            // Check visibility
            if (caps.has_visibility) {
                auto visibility = static_cast<IVisibilityControl*>(ptr);
                if (!visibility->is_visible()) continue;
                if (visibility->is_advanced() && !m_show_advanced) continue;
            }
            
            // Apply global edit mode if enabled
            if (m_apply_global_edit_mode) {
                auto edit_control = static_cast<IEditModeControl*>(ptr);
                edit_control->set_edit_mode(m_global_edit_mode);
            }
            
            bool changed = ptr->draw(id.c_str());
            
            if (changed && m_on_value_changed) {
                m_on_value_changed(id);
            }
        }
        
        // Show advanced toggle if needed
        if (has_advanced_values()) {
            ImGui::Separator();
            if (ImGui::Checkbox("Show Advanced Settings", &m_show_advanced)) {
                // State changed
            }
        }
        
        // Apply/Revert buttons using proper polymorphism
        if (show_apply_button && has_pending_changes()) {
            ImGui::Separator();
            
            if (ImGui::Button("Apply")) {
                apply_all();
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Revert")) {
                revert_all();
            }
        }
    }
    
    
    [[nodiscard]] bool has_pending_changes() const {
        for (const auto& [id, value] : m_values) {
            auto caps = value->get_capabilities();
            if (caps.has_deferred_update) {
                auto deferred = static_cast<IDeferredUpdate*>(value.get());
                if (deferred->has_pending_changes()) return true;
            }
        }
        return false;
    }
    
    void apply_all() {
        for (const auto& [id, value] : m_values) {
            auto caps = value->get_capabilities();
            if (caps.has_deferred_update) {
                auto deferred = static_cast<IDeferredUpdate*>(value.get());
                deferred->apply_changes();
            }
        }
        if (m_on_applied) m_on_applied();
    }
    
    void revert_all() {
        for (const auto& [id, value] : m_values) {
            auto caps = value->get_capabilities();
            if (caps.has_deferred_update) {
                auto deferred = static_cast<IDeferredUpdate*>(value.get());
                deferred->revert_changes();
            }
        }
    }
    
    void sync_all_from_external() {
        for (const auto& [id, value] : m_values) {
            auto caps = value->get_capabilities();
            if (caps.has_external_sync) {
                auto external = static_cast<IExternalSync*>(value.get());
                external->sync_from_external();
            }
        }
    }
    
    void sync_all_to_external() {
        for (const auto& [id, value] : m_values) {
            auto caps = value->get_capabilities();
            if (caps.has_external_sync) {
                auto external = static_cast<IExternalSync*>(value.get());
                external->sync_to_external();
            }
        }
    }
    
    [[nodiscard]] bool has_advanced_values() const {
        for (const auto& [id, value] : m_values) {
            auto caps = value->get_capabilities();
            if (caps.has_visibility) {
                auto visibility = static_cast<IVisibilityControl*>(value.get());
                if (visibility->is_advanced()) return true;
            }
        }
        return false;
    }
    
    // Global edit mode
    void set_global_edit_mode(IEditModeControl::EditMode mode) {
        m_global_edit_mode = mode;
        m_apply_global_edit_mode = true;
    }
    
    void show_edit_mode_control(bool show) { m_show_edit_mode_control = show; }
    
    // Visibility
    void set_visible(bool visible) { m_visible = visible; }
    [[nodiscard]] bool is_visible() const { return m_visible; }
    
    // Callbacks
    void set_on_value_changed(std::function<void(const std::string&)> callback) {
        m_on_value_changed = std::move(callback);
    }
    
    void set_on_applied(std::function<void()> callback) {
        m_on_applied = std::move(callback);
    }
    
    // Convenience methods
    template<RC::StringLike IdT, 
             RC::StringLike LabelT = const char*, 
             RC::StringLike TooltipT = const char*>
    auto add_toggle(IdT&& id, bool initial_value, 
                    LabelT&& label = "", 
                    TooltipT&& tooltip = "") {
        auto toggle = ImDataSimpleToggle::create(initial_value);
        
        // Handle empty strings efficiently
        if constexpr (std::is_same_v<std::decay_t<LabelT>, const char*>) {
            if (label && *label) toggle->set_name(label);
        } else {
            auto label_str = RC::to_utf8_string(std::forward<LabelT>(label));
            if (!label_str.empty()) toggle->set_name(std::move(label_str));
        }
        
        if constexpr (std::is_same_v<std::decay_t<TooltipT>, const char*>) {
            if (tooltip && *tooltip) toggle->set_tooltip(tooltip);
        } else {
            auto tooltip_str = RC::to_utf8_string(std::forward<TooltipT>(tooltip));
            if (!tooltip_str.empty()) toggle->set_tooltip(std::move(tooltip_str));
        }
        
        return add_value(RC::to_utf8_string(std::forward<IdT>(id)), 
                         std::move(toggle));
    }
    
    template<RC::StringLike IdT>
    auto add_float(IdT&& id, float initial_value) {
        return add_value(RC::to_utf8_string(std::forward<IdT>(id)), 
                         ImDataSimpleFloat::create(initial_value));
    }
    
    template<RC::StringLike IdT>
    auto add_slider(IdT&& id, float min, float max, float initial_value) {
        return add_value(RC::to_utf8_string(std::forward<IdT>(id)), 
                         ImDataSimpleSliderFloat::create(min, max, initial_value));
    }
    
    template<RC::StringLike IdT>
    auto add_combo(IdT&& id, const std::vector<std::string>& options, 
                   int32_t initial_value = 0) {
        return add_value(RC::to_utf8_string(std::forward<IdT>(id)), 
                         std::make_unique<ImDataSimpleCombo>(options, initial_value));
    }
    
private:
    std::string m_name;
    std::unordered_map<std::string, std::unique_ptr<IValueControl>> m_values;
    std::vector<std::pair<std::string, IValueControl*>> m_ordered_values;
    bool m_visible = true;
    bool m_show_advanced = false;
    bool m_show_edit_mode_control = false;
    bool m_apply_global_edit_mode = false;
    IEditModeControl::EditMode m_global_edit_mode = IEditModeControl::EditMode::Editable;
    std::function<void(const std::string&)> m_on_value_changed;
    std::function<void()> m_on_applied;
};

} // namespace RC::ImDataControls