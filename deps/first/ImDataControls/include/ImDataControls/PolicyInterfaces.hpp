#pragma once
#include <string>
#include <expected>
#include <functional>
#include <type_traits>

namespace RC::ImDataControls {

// Base drawing interface - pure virtual
class IImGuiDrawable {
public:
    virtual ~IImGuiDrawable() = default;
    virtual bool draw(const char* label) = 0;
    virtual bool is_changed() const = 0;
};

// Value source tracking
enum class ValueSource {
    User,      // User changed via UI
    External,  // External system (e.g., game engine)
    Default,   // Initial/default value
    Config     // Loaded from configuration
};

// Forward declaration
struct Capabilities;

// Base interface for all value controls
class IValueControl : public virtual IImGuiDrawable {
public:
    virtual ~IValueControl() = default;
    
    // Capability detection
    virtual Capabilities get_capabilities() const = 0;
    
    // Common metadata operations
    virtual const std::string& get_name() const = 0;
    virtual void set_name(const std::string& name) = 0;
    virtual const std::string& get_tooltip() const = 0;
    virtual void set_tooltip(const std::string& tooltip) = 0;
    
    // Just declare the query interface methods
    template<typename T>
    T* query_interface();
    
    template<typename T>
    const T* query_interface() const;

private:
    template<typename T>
    bool get_capabilities_for() const;
};

// Capability: Deferred updates
class IDeferredUpdate {
public:
    virtual ~IDeferredUpdate() = default;
    virtual void apply_changes() = 0;
    virtual void revert_changes() = 0;
    virtual bool has_pending_changes() const = 0;
};

// Capability: External synchronization
class IExternalSync {
public:
    virtual ~IExternalSync() = default;
    virtual void sync_from_external() = 0;
    virtual void sync_to_external() = 0;
    virtual bool is_externally_synced() const = 0;
};

// Capability: Validation
class IValidatable {
public:
    virtual ~IValidatable() = default;
    virtual bool validate() = 0;
    virtual bool is_valid() const = 0;
    virtual std::string get_validation_error() const = 0;
};

// Capability: History (undo/redo)
class IHistorical {
public:
    virtual ~IHistorical() = default;
    virtual bool can_undo() const = 0;
    virtual bool can_redo() const = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual void clear_history() = 0;
    virtual size_t history_size() const = 0;
};

// Capability: String conversion
class IStringConvertible {
public:
    virtual ~IStringConvertible() = default;
    virtual std::string to_string() const = 0;
    virtual bool from_string(const std::string& str) = 0;
};

// Capability: Edit mode control
class IEditModeControl {
public:
    enum class EditMode {
        Editable,   // Normal user interaction
        ReadOnly,   // Display with disabled controls
        ViewOnly    // Text-only display
    };
    
    virtual ~IEditModeControl() = default;
    virtual EditMode get_edit_mode() const = 0;
    virtual void set_edit_mode(EditMode mode) = 0;
    virtual bool is_editable() const = 0;
};

// Capability: Visibility control
class IVisibilityControl {
public:
    virtual ~IVisibilityControl() = default;
    virtual bool is_visible() const = 0;
    virtual void set_visible(bool visible) = 0;
    virtual bool is_advanced() const = 0;
    virtual void set_advanced(bool advanced) = 0;
    virtual const std::string& get_group() const = 0;
    virtual void set_group(const std::string& group) = 0;
};

// Capability: Text representation
class ITextRepresentation {
public:
    virtual ~ITextRepresentation() = default;
    virtual bool should_show_text_representation() const = 0;
    virtual void set_show_text_representation(bool show) = 0;
    virtual std::string get_text_representation() const = 0;
};

// Capability: Custom callbacks
class ICustomCallbacks {
public:
    virtual ~ICustomCallbacks() = default;
    virtual void set_custom_tooltip_callback(std::function<void()> callback) = 0;
    virtual void set_custom_context_menu_callback(std::function<void()> callback) = 0;
};

// Capability: Immediate apply
class IImmediateApply {
public:
    virtual ~IImmediateApply() = default;
    virtual void set_immediate_apply(bool immediate) = 0;
    virtual bool is_immediate_apply() const = 0;
};

// Capabilities structure for runtime capability detection
struct Capabilities {
    bool has_deferred_update : 1;
    bool has_external_sync : 1;
    bool has_validation : 1;
    bool has_history : 1;
    bool has_visibility : 1;
    bool has_string_conversion : 1;
    bool has_text_representation : 1;
    bool has_custom_callbacks : 1;
    bool has_immediate_apply : 1;
    
    constexpr Capabilities() 
        : has_deferred_update(false)
        , has_external_sync(false)
        , has_validation(false)
        , has_history(false)
        , has_visibility(false)
        , has_string_conversion(false)
        , has_text_representation(false)
        , has_custom_callbacks(false)
        , has_immediate_apply(false)
    {}
};

    template<typename T>
T* IValueControl::query_interface() {
        if (get_capabilities_for<T>()) {
            return reinterpret_cast<T*>(this);
        }
        return nullptr;
    }

    template<typename T>
    const T* IValueControl::query_interface() const {
        if (get_capabilities_for<T>()) {
            return reinterpret_cast<const T*>(this);
        }
        return nullptr;
    }

    template<typename T>
    bool IValueControl::get_capabilities_for() const {
        auto caps = get_capabilities();
        if constexpr (std::is_same_v<T, IDeferredUpdate>) return caps.has_deferred_update;
        else if constexpr (std::is_same_v<T, IExternalSync>) return caps.has_external_sync;
        else if constexpr (std::is_same_v<T, IValidatable>) return caps.has_validation;
        else if constexpr (std::is_same_v<T, IHistorical>) return caps.has_history;
        else if constexpr (std::is_same_v<T, IVisibilityControl>) return caps.has_visibility;
        else if constexpr (std::is_same_v<T, IStringConvertible>) return caps.has_string_conversion;
        else if constexpr (std::is_same_v<T, ITextRepresentation>) return caps.has_text_representation;
        else if constexpr (std::is_same_v<T, ICustomCallbacks>) return caps.has_custom_callbacks;
        else if constexpr (std::is_same_v<T, IImmediateApply>) return caps.has_immediate_apply;
        else if constexpr (std::is_same_v<T, IEditModeControl>) return true; // Always available
        else return false;
    }

} // namespace RC::ImDataControls