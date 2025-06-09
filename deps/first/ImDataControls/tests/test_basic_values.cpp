#include <ImDataControls/ImDataControls.hpp>
#include <cassert>
#include <iostream>

using namespace RC::ImDataControls;

void test_simple_value() {
    // Test SimpleImGuiValue - zero overhead
    auto simple = SimpleImGuiValue<int>::create(42);
    assert(simple->value() == 42);
    
    simple->operator=(100);
    assert(simple->value() == 100);
    assert(simple->is_changed());
    
    simple->clear_changed();
    assert(!simple->is_changed());
    
    std::cout << "SimpleImGuiValue tests passed\n";
}

void test_monitored_value() {
    // Test MonitoredImGuiValue with external sync
    int external_value = 50;
    
    auto monitored = MonitoredImGuiValue<int>::create(
        [&external_value]() { return external_value; },  // Getter
        [&external_value](const int& v) { external_value = v; },  // Setter
        0  // Default value
    );
    
    // Should sync from external on creation
    assert(monitored->get() == 50);
    
    // Set value and sync to external
    monitored->set(75);
    assert(external_value == 75);
    assert(monitored->get_last_source() == ValueSource::User);
    
    // Change external value and refresh
    external_value = 100;
    monitored->refresh();
    assert(monitored->get() == 100);
    assert(monitored->get_last_source() == ValueSource::External);
    
    std::cout << "MonitoredImGuiValue tests passed\n";
}

void test_config_value() {
    // Test ConfigImGuiValue with validation
    auto config = ConfigImGuiValue<int>::create(10);
    
    // Set validator that only accepts positive values
    config->set_validator([](const int& v) -> std::expected<int, std::string> {
        if (v > 0) {
            return v;
        }
        return std::unexpected("Value must be positive");
    });
    
    // Valid value
    assert(config->try_set(20));
    assert(config->has_pending_changes());
    assert(config->get_error().empty());
    
    // Invalid value
    assert(!config->try_set(-5));
    assert(config->get_validation_error(-5) == "Value must be positive");
    
    // Apply changes
    config->apply_changes();
    assert(config->value() == 20);
    assert(!config->has_pending_changes());
    
    std::cout << "ConfigImGuiValue tests passed\n";
}

void test_full_value() {
    // Test FullImGuiValue with all features
    auto full = FullImGuiValue<std::string>::create("initial");
    
    // Set up external sync
    std::string external = "external";
    full->set_external_getter([&external]() { return external; });
    full->set_external_setter([&external](const std::string& v) { external = v; });
    
    // Set up validation
    full->set_validator([](const std::string& v) -> std::expected<std::string, std::string> {
        if (v.length() <= 10) {
            return v;
        }
        return std::unexpected("String too long (max 10 chars)");
    });
    
    // Set up change notification
    bool changed = false;
    full->add_simple_change_listener([&changed]() { changed = true; });
    
    // Test validation
    full->set_with_validation("short");
    assert(changed);
    assert(full->has_pending_changes());
    
    // Apply with history
    full->apply_with_history();
    assert(full->value() == "short");
    assert(external == "short");  // Should sync to external
    
    // Test undo
    assert(full->can_undo());
    auto undo_val = full->undo();
    assert(undo_val.has_value() && undo_val.value() == "initial");
    
    // Test redo
    assert(full->can_redo());
    auto redo_val = full->redo();
    assert(redo_val.has_value() && redo_val.value() == "short");
    
    std::cout << "FullImGuiValue tests passed\n";
}

int main() {
    test_simple_value();
    test_monitored_value();
    test_config_value();
    test_full_value();
    
    std::cout << "\nAll tests passed successfully!\n";
    return 0;
}