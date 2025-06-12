#pragma once
#include <string>
#include <expected>
#include <functional>

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

// Base interface for all value controls
class IValueControl : public IImGuiDrawable {
public:
    virtual ~IValueControl() = default;
    
    // Common metadata operations
    virtual const std::string& get_name() const = 0;
    virtual void set_name(const std::string& name) = 0;
    virtual const std::string& get_tooltip() const = 0;
    virtual void set_tooltip(const std::string& tooltip) = 0;
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

} // namespace RC::ImDataControls