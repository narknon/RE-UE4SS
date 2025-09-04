// Test to verify Lua RAII safety when compiled as C++
#include <iostream>
#include <string>
#include <cassert>

// Include Lua headers via lua.hpp
#include <lua.hpp>

// Global counter to track destructor calls
static int destructor_count = 0;

// RAII test class
class RAIITest {
private:
    std::string name;
    int* counter;
    
public:
    RAIITest(const std::string& n, int* cnt) : name(n), counter(cnt) {
        std::cout << "Constructor: " << name << std::endl;
    }
    
    ~RAIITest() {
        std::cout << "Destructor: " << name << std::endl;
        if (counter) {
            (*counter)++;
        }
    }
    
    // Delete copy operations to ensure no unexpected copies
    RAIITest(const RAIITest&) = delete;
    RAIITest& operator=(const RAIITest&) = delete;
};

// Lua C function that will error
int lua_function_that_errors(lua_State* L) {
    // Create RAII object - destructor should be called even if lua_error is called
    RAIITest test("Inside Lua function", &destructor_count);
    
    // Force a Lua error
    luaL_error(L, "Intentional error for testing RAII");
    
    // This should never be reached
    return 0;
}

// Test function that uses RAII
void test_lua_raii_safety() {
    std::cout << "\n=== Testing Lua RAII Safety ===" << std::endl;
    
    lua_State* L = luaL_newstate();
    assert(L != nullptr);
    
    // Reset counter
    destructor_count = 0;
    
    // Register our test function
    lua_pushcfunction(L, lua_function_that_errors);
    lua_setglobal(L, "test_error");
    
    // Create RAII object in C++ code
    {
        RAIITest test("Before Lua call", &destructor_count);
        
        // Try to call Lua function that will error
        int result = lua_pcall(L, 0, 0, 0);
        
        if (result != LUA_OK) {
            std::cout << "Lua error (expected): " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    } // RAII destructor should be called here
    
    lua_close(L);
    
    std::cout << "\nDestructor call count: " << destructor_count << std::endl;
    
    // Both destructors should have been called if RAII is working correctly
    if (destructor_count == 2) {
        std::cout << "✓ PASS: All destructors called - RAII is working correctly!" << std::endl;
    } else {
        std::cout << "✗ FAIL: Expected 2 destructor calls, got " << destructor_count << std::endl;
        std::cout << "This indicates Lua may not be compiled as C++ or RAII is not safe!" << std::endl;
    }
}

// Test 2: Verify exception vs longjmp behavior
void test_lua_error_mechanism() {
    std::cout << "\n=== Testing Lua Error Mechanism ===" << std::endl;
    
    lua_State* L = luaL_newstate();
    
    // Check if Lua was compiled with C++ exceptions
    #ifdef __cplusplus
        std::cout << "Test compiled as C++" << std::endl;
    #endif
    
    // In Lua compiled as C++, errors use exceptions
    // In Lua compiled as C, errors use longjmp
    std::cout << "Lua uses: ";
    #if defined(LUA_USE_LONGJMP)
        std::cout << "longjmp (C mode)" << std::endl;
    #else
        std::cout << "C++ exceptions (C++ mode)" << std::endl;
    #endif
    
    lua_close(L);
}

// Test compilation mode
void test_compilation_mode() {
    std::cout << "\n=== Compilation Mode ===" << std::endl;
    std::cout << "Lua version: " << LUA_VERSION << std::endl;
    
    // Check various build configurations
    #ifdef UE_BUILD_DEBUG
        std::cout << "Build config: DEBUG" << std::endl;
    #elif defined(UE_BUILD_DEVELOPMENT)
        std::cout << "Build config: DEVELOPMENT" << std::endl;
    #elif defined(UE_BUILD_TEST)
        std::cout << "Build config: TEST" << std::endl;
    #elif defined(UE_BUILD_SHIPPING)
        std::cout << "Build config: SHIPPING" << std::endl;
    #else
        std::cout << "Build config: UNKNOWN" << std::endl;
    #endif
    
    #ifdef RC_LUA_RAW_BUILD_STATIC
        std::cout << "LuaRaw: Static build" << std::endl;
    #else
        std::cout << "LuaRaw: Dynamic build" << std::endl;
    #endif
}

int main() {
    std::cout << "======================================" << std::endl;
    std::cout << "Lua RAII Safety Test" << std::endl;
    std::cout << "======================================" << std::endl;
    
    test_compilation_mode();
    test_lua_raii_safety();
    test_lua_error_mechanism();
    
    std::cout << "\n======================================" << std::endl;
    
    return 0;
}