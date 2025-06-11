#pragma once

#include "Core.hpp"
#include "BasicWidgets.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <typeindex>
#include <functional>

namespace RC::ImDataControls {

// Container for managing multiple ImGui values
class ImGuiValueContainer {
public:
    using ValuePtr = std::shared_ptr<IImGuiDrawable>;
    using ChangeCallback = std::function<void()>;
    
    ImGuiValueContainer(const std::string& name = "")
        : m_name(name)
        , m_visible(true)
        , m_immediate_apply(false)
    {}
    
    // Add a value to the container
    template<typename T>
    auto add_value(const std::string& id, std::unique_ptr<T> value) -> std::weak_ptr<T> {
        static_assert(std::is_base_of_v<IImGuiDrawable, T>, "T must derive from IImGuiDrawable");
        
        auto shared = std::shared_ptr<T>(std::move(value));

        // Use insert_or_assign to correctly handle insertion or update.
        // This is necessary for std::type_index, which is not default-constructible,
        // and fixes the compilation error caused by operator[].
        m_values.insert_or_assign(id, shared);
        m_value_types.insert_or_assign(id, std::type_index(typeid(T)));
        
        // Also fix the logic for the ordered list to avoid creating duplicates on update.
        auto it = std::find_if(m_ordered_values.begin(), m_ordered_values.end(),
            [&id](const auto& pair) { return pair.first == id; });

        if (it != m_ordered_values.end())
        {
            // If the ID already exists, update the pointer in-place.
            it->second = shared;
        }
        else
        {
            // Otherwise, add the new value to the list.
            m_ordered_values.push_back({ id, shared });
        }
        
        return shared;
    }
    
    // Get a value by ID with type checking
    template<typename T>
    [[nodiscard]] auto get_value(const std::string& id) const -> std::shared_ptr<T> {
        auto it = m_values.find(id);
        if (it != m_values.end()) {
            // Check if the stored type matches requested type
            auto type_it = m_value_types.find(id);
            if (type_it != m_value_types.end() && type_it->second == std::type_index(typeid(T))) {
                return std::dynamic_pointer_cast<T>(it->second);
            }
        }
        return nullptr;
    }
    
    // Get all values of a specific type
    template<typename T>
    [[nodiscard]] auto get_all_values_of_type() const -> std::vector<std::pair<std::string, std::shared_ptr<T>>> {
        std::vector<std::pair<std::string, std::shared_ptr<T>>> result;
        
        for (const auto& [id, value] : m_ordered_values) {
            auto type_it = m_value_types.find(id);
            if (type_it != m_value_types.end() && type_it->second == std::type_index(typeid(T))) {
                if (auto typed_value = std::dynamic_pointer_cast<T>(value)) {
                    result.emplace_back(id, typed_value);
                }
            }
        }
        
        return result;
    }
    
    // Remove a value
    void remove_value(const std::string& id) {
        m_values.erase(id);
        m_value_types.erase(id);
        m_ordered_values.erase(
            std::remove_if(m_ordered_values.begin(), m_ordered_values.end(),
                          [&id](const auto& pair) { return pair.first == id; }),
            m_ordered_values.end()
        );
    }
    
    // Clear all values
    void clear() {
        m_values.clear();
        m_value_types.clear();
        m_ordered_values.clear();
    }
    
    // Draw all values
    void draw(bool show_apply_button = true) {
        if (!m_visible) return;
        
        bool any_changed = false;
        
        for (const auto& [id, value] : m_ordered_values) {
            if (value) {
                bool changed = value->draw(id.c_str());
                if (changed) {
                    any_changed = true;
                    if (m_immediate_apply) {
                        // In immediate mode, notify right away
                        if (m_on_value_changed) {
                            m_on_value_changed(id);
                        }
                    }
                }
            }
        }
        
        // Show apply/revert buttons if not in immediate mode and requested
        if (!m_immediate_apply && show_apply_button && has_pending_changes()) {
            ImGui::Separator();
            
            if (ImGui::Button("Apply")) {
                apply_all();
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Revert")) {
                revert_all();
            }
        }
        
        // Notify about changes in deferred mode
        if (any_changed && !m_immediate_apply && m_on_value_changed) {
            for (const auto& [id, value] : m_ordered_values) {
                if (value && value->is_changed()) {
                    m_on_value_changed(id);
                }
            }
        }
    }
    
    // Draw without apply/revert buttons
    void draw_without_apply_button() {
        draw(false);
    }
    
    // Draw custom apply button
    [[nodiscard]] bool draw_apply_button(const char* label = "Apply") {
        if (!has_pending_changes()) {
            ImGui::BeginDisabled();
        }
        
        bool clicked = ImGui::Button(label);
        
        if (!has_pending_changes()) {
            ImGui::EndDisabled();
        }
        
        if (clicked && has_pending_changes()) {
            apply_all();
        }
        
        return clicked;
    }
    
    // Draw custom revert button  
    [[nodiscard]] bool draw_revert_button(const char* label = "Revert") {
        if (!has_pending_changes()) {
            ImGui::BeginDisabled();
        }
        
        bool clicked = ImGui::Button(label);
        
        if (!has_pending_changes()) {
            ImGui::EndDisabled();
        }
        
        if (clicked && has_pending_changes()) {
            revert_all();
        }
        
        return clicked;
    }
    
    // Check if any value has pending changes
    [[nodiscard]] bool has_pending_changes() const {
        // Check ConfigImGuiValue types for pending changes
        for (const auto& [id, value] : m_ordered_values) {
            auto type_it = m_value_types.find(id);
            if (type_it != m_value_types.end()) {
                // Check all config types
                if (type_it->second == std::type_index(typeid(ConfigToggle))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigToggle>(value)) {
                        if (config->has_pending_changes()) return true;
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigFloat))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigFloat>(value)) {
                        if (config->has_pending_changes()) return true;
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigDouble))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigDouble>(value)) {
                        if (config->has_pending_changes()) return true;
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigInt32))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigInt32>(value)) {
                        if (config->has_pending_changes()) return true;
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigString))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigString>(value)) {
                        if (config->has_pending_changes()) return true;
                    }
                }
            }
        }
        return false;
    }
    
    // Apply all pending changes
    void apply_all() {
        for (const auto& [id, value] : m_ordered_values) {
            auto type_it = m_value_types.find(id);
            if (type_it != m_value_types.end()) {
                // Apply changes for config types
                if (type_it->second == std::type_index(typeid(ConfigToggle))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigToggle>(value)) {
                        config->apply_changes();
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigFloat))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigFloat>(value)) {
                        config->apply_changes();
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigDouble))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigDouble>(value)) {
                        config->apply_changes();
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigInt32))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigInt32>(value)) {
                        config->apply_changes();
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigString))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigString>(value)) {
                        config->apply_changes();
                    }
                }
            }
        }
        
        if (m_on_applied) {
            m_on_applied();
        }
    }
    
    // Revert all pending changes
    void revert_all() {
        for (const auto& [id, value] : m_ordered_values) {
            auto type_it = m_value_types.find(id);
            if (type_it != m_value_types.end()) {
                // Revert changes for config types
                if (type_it->second == std::type_index(typeid(ConfigToggle))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigToggle>(value)) {
                        config->revert_changes();
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigFloat))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigFloat>(value)) {
                        config->revert_changes();
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigDouble))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigDouble>(value)) {
                        config->revert_changes();
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigInt32))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigInt32>(value)) {
                        config->revert_changes();
                    }
                }
                else if (type_it->second == std::type_index(typeid(ConfigString))) {
                    if (auto config = std::dynamic_pointer_cast<ConfigString>(value)) {
                        config->revert_changes();
                    }
                }
            }
        }
    }
    
    // Container settings
    void set_visible(bool visible) { m_visible = visible; }
    [[nodiscard]] bool is_visible() const { return m_visible; }
    
    void set_immediate_apply(bool immediate) { m_immediate_apply = immediate; }
    [[nodiscard]] bool is_immediate_apply() const { return m_immediate_apply; }
    
    // Callbacks
    void set_on_value_changed(std::function<void(const std::string&)> callback) {
        m_on_value_changed = std::move(callback);
    }
    
    void set_on_applied(ChangeCallback callback) {
        m_on_applied = std::move(callback);
    }
    
    // Convenience methods for adding common types
    auto add_toggle(const std::string& id, bool initial_value) -> std::weak_ptr<SimpleToggle> {
        return add_value(id, SimpleToggle::create(initial_value));
    }
    
    auto add_float(const std::string& id, float initial_value) -> std::weak_ptr<SimpleFloat> {
        return add_value(id, SimpleFloat::create(initial_value));
    }
    
    auto add_double(const std::string& id, double initial_value) -> std::weak_ptr<SimpleDouble> {
        return add_value(id, SimpleDouble::create(initial_value));
    }
    
    auto add_int(const std::string& id, int32_t initial_value) -> std::weak_ptr<SimpleInt32> {
        return add_value(id, SimpleInt32::create(initial_value));
    }
    
    auto add_string(const std::string& id, const std::string& initial_value) -> std::weak_ptr<SimpleString> {
        return add_value(id, SimpleString::create(initial_value));
    }
    
    auto add_slider(const std::string& id, float min_val, float max_val, float initial_value) -> std::weak_ptr<SimpleSliderFloat> {
        return add_value(id, SimpleSliderFloat::create(min_val, max_val, initial_value));
    }
    
    auto add_slider_int(const std::string& id, int32_t min_val, int32_t max_val, int32_t initial_value) -> std::weak_ptr<SimpleSliderInt32> {
        return add_value(id, SimpleSliderInt32::create(min_val, max_val, initial_value));
    }
    
private:
    std::string m_name;
    std::unordered_map<std::string, ValuePtr> m_values;
    std::unordered_map<std::string, std::type_index> m_value_types;
    std::vector<std::pair<std::string, ValuePtr>> m_ordered_values;  // Maintain insertion order
    bool m_visible;
    bool m_immediate_apply;
    std::function<void(const std::string&)> m_on_value_changed;
    ChangeCallback m_on_applied;
};

} // namespace RC::ImDataControls