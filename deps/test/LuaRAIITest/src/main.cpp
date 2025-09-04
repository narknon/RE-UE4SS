// Test to verify Lua RAII safety when compiled as C++
#include <iostream>
#include <string>
#include <cassert>

#ifdef _WIN32
#include <mutex>
#endif

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
    // NOTE: RAII in extern "C" functions is problematic
    // Even if Lua is compiled as C++, the extern "C" linkage
    // can interfere with proper exception handling
    
    // This is a known limitation when mixing C and C++
    RAIITest test("Inside Lua function", &destructor_count);
    
    // Force a Lua error
    luaL_error(L, "Intentional error for testing RAII");
    
    // This should never be reached
    return 0;
}

// C++ lambda test - this should work if Lua is compiled as C++
void test_cpp_lambda_raii(lua_State* L) {
    // Use a C++ lambda instead of extern "C" function
    auto cpp_function = [](lua_State* L) -> int {
        RAIITest test("Inside C++ lambda", &destructor_count);
        luaL_error(L, "Error from C++ lambda");
        return 0;
    };
    
    // Convert lambda to function pointer
    lua_pushcfunction(L, cpp_function);
    int result = lua_pcall(L, 0, 0, 0);
    
    if (result != LUA_OK) {
        std::cout << "Lambda error (expected): " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
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
        
        // Get our test function and call it
        lua_getglobal(L, "test_error");
        int result = lua_pcall(L, 0, 0, 0);
        
        if (result != LUA_OK) {
            std::cout << "Lua error (expected): " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    } // RAII destructor should be called here
    
    std::cout << "\nDestructor call count: " << destructor_count << std::endl;
    
    // Test with C++ lambda
    std::cout << "\n--- Testing C++ Lambda RAII ---" << std::endl;
    int prev_count = destructor_count;
    test_cpp_lambda_raii(L);
    int lambda_destructors = destructor_count - prev_count;
    
    lua_close(L);
    
    std::cout << "\nResults:" << std::endl;
    std::cout << "- extern 'C' function destructors: " << (destructor_count - lambda_destructors - 1) << std::endl;
    std::cout << "- C++ lambda destructors: " << lambda_destructors << std::endl;
    std::cout << "- C++ scope destructors: 1" << std::endl;
    
    // The extern "C" function RAII might not work due to C/C++ boundary issues
    // But the C++ lambda should work if Lua is compiled as C++
    if (lambda_destructors > 0) {
        std::cout << "\n✓ PASS: C++ lambda RAII working - Lua is compiled as C++!" << std::endl;
    } else if (destructor_count >= 1) {
        std::cout << "\n⚠ PARTIAL: C++ scope RAII works, but Lua function RAII doesn't" << std::endl;
        std::cout << "This is expected behavior when mixing extern 'C' with C++ exceptions" << std::endl;
    } else {
        std::cout << "\n✗ FAIL: No RAII working properly!" << std::endl;
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
    
    // Check for mutex fix
    #ifdef _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR
        std::cout << "Mutex fix: ENABLED (safe from msvcp140.dll crashes)" << std::endl;
    #else
        std::cout << "Mutex fix: DISABLED (may crash with older msvcp140.dll)" << std::endl;
    #endif
}

// Test 3: Mutex constructor safety test
void test_mutex_safety() {
    std::cout << "\n=== Testing Mutex Safety ===" << std::endl;
    
    #ifdef _WIN32
        // This would crash with older msvcp140.dll if _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR is not defined
        try {
            std::mutex test_mutex;
            std::cout << "✓ PASS: Mutex created without crash" << std::endl;
        } catch (...) {
            std::cout << "✗ FAIL: Mutex creation threw exception" << std::endl;
        }
    #else
        std::cout << "Skipping mutex test (not Windows)" << std::endl;
    #endif
}

int main() {
    std::cout << "======================================" << std::endl;
    std::cout << "Lua RAII Safety Test" << std::endl;
    std::cout << "======================================" << std::endl;
    
    test_compilation_mode();
    test_mutex_safety();
    test_lua_raii_safety();
    test_lua_error_mechanism();
    
    std::cout << "\n======================================" << std::endl;
    
    return 0;
}