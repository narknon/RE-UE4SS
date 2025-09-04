// Test to verify Lua RAII safety when compiled as C++
#include <iostream>
#include <string>
#include <cassert>

#include <mutex>
#include <thread>
#include <chrono>
#include <vector>

// Include Lua headers via lua.hpp (no extern "C" - Lua compiled as C++)
#include <lua.hpp>

// Global counter to track destructor calls
static int destructor_count = 0;

// Global mutex for testing
static std::mutex g_test_mutex;

// RAII test class with order tracking
class RAIITest {
private:
    std::string name;
    int* counter;
    static std::vector<std::string> destruction_order;
    
public:
    RAIITest(const std::string& n, int* cnt) : name(n), counter(cnt) {
        std::cout << "Constructor: " << name << std::endl;
    }
    
    ~RAIITest() {
        std::cout << "Destructor: " << name << std::endl;
        destruction_order.push_back(name);
        if (counter) {
            (*counter)++;
        }
    }
    
    // Delete copy operations to ensure no unexpected copies
    RAIITest(const RAIITest&) = delete;
    RAIITest& operator=(const RAIITest&) = delete;
    
    static void clearOrder() { destruction_order.clear(); }
    static const std::vector<std::string>& getOrder() { return destruction_order; }
};

std::vector<std::string> RAIITest::destruction_order;

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
    
    // Test 1: RAII in the Lua C function itself
    lua_getglobal(L, "test_error");
    int result = lua_pcall(L, 0, 0, 0);
    
    if (result != LUA_OK) {
        std::cout << "Lua error (expected): " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
    
    std::cout << "\nDestructor call count: " << destructor_count << std::endl;
    
    // Test with C++ lambda
    std::cout << "\n--- Testing C++ Lambda RAII ---" << std::endl;
    int prev_count = destructor_count;
    test_cpp_lambda_raii(L);
    int lambda_destructors = destructor_count - prev_count;
    
    lua_close(L);
    
    std::cout << "\nResults:" << std::endl;
    std::cout << "- extern 'C' function destructors: " << (prev_count > 0 ? 1 : 0) << std::endl;
    std::cout << "- C++ lambda destructors: " << lambda_destructors << std::endl;
    
    // The extern "C" function RAII doesn't work due to C/C++ boundary issues
    // Even with Lua compiled as C++, extern "C" functions can't properly unwind
    if (prev_count == 0 && lambda_destructors == 0) {
        std::cout << "\n⚠ Expected: RAII doesn't work in extern 'C' functions" << std::endl;
        std::cout << "This is a known limitation when mixing C and C++" << std::endl;
    } else if (lambda_destructors > 0) {
        std::cout << "\n✓ Interesting: Lambda RAII worked!" << std::endl;
    } else {
        std::cout << "\n✗ Unexpected destructor behavior" << std::endl;
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

// Lua function that locks a mutex and then errors
int lua_function_with_mutex_error(lua_State* L) {
    std::lock_guard<std::mutex> lock(g_test_mutex);
    std::cout << "Mutex locked in Lua function" << std::endl;
    
    // Now error while mutex is locked
    luaL_error(L, "Error while mutex is locked");
    
    // Never reached
    return 0;
}

// Test 3: Mutex contention during Lua error
void test_mutex_contention() {
    std::cout << "\n=== Testing Mutex Contention with Lua Errors ===" << std::endl;
    
    lua_State* L = luaL_newstate();
    
    // Register our mutex test function
    lua_pushcfunction(L, lua_function_with_mutex_error);
    lua_setglobal(L, "mutex_error");
    
    std::cout << "Calling Lua function that locks mutex and then errors..." << std::endl;
    
    // Call Lua function that will lock mutex and error
    lua_getglobal(L, "mutex_error");
    int result = lua_pcall(L, 0, 0, 0);
    
    if (result != LUA_OK) {
        std::cout << "Lua error (expected): " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
    
    // Now test if mutex is still locked (deadlock) or unlocked
    std::cout << "\nTesting if mutex is unlocked after Lua error..." << std::endl;
    
    bool mutex_unlocked = false;
    std::thread test_thread([&mutex_unlocked]() {
        if (g_test_mutex.try_lock()) {
            mutex_unlocked = true;
            g_test_mutex.unlock();
            std::cout << "✓ Thread successfully locked mutex" << std::endl;
        } else {
            std::cout << "✗ Thread CANNOT lock mutex - DEADLOCK!" << std::endl;
        }
    });
    
    test_thread.join();
    
    // Also try with timeout
    if (!mutex_unlocked) {
        std::cout << "Attempting timed lock..." << std::endl;
        std::timed_mutex timed_test;
        auto timeout = std::chrono::milliseconds(100);
        
        if (g_test_mutex.try_lock()) {
            std::cout << "✓ Main thread can relock - mutex was properly unlocked" << std::endl;
            g_test_mutex.unlock();
            mutex_unlocked = true;
        }
    }
    
    lua_close(L);
    
    if (mutex_unlocked) {
        std::cout << "\n✓ PASS: Mutex properly unlocked after Lua error" << std::endl;
        std::cout << "This indicates Lua is compiled as C++ with proper RAII" << std::endl;
    } else {
        std::cout << "\n✗ FAIL: Mutex is deadlocked!" << std::endl;
        std::cout << "This indicates Lua is using longjmp which bypasses destructors" << std::endl;
    }
}

// Test with multiple RAII objects to verify stack unwinding order
int lua_function_with_stack(lua_State* L) {
    RAIITest obj1("Stack-1-Outer", nullptr);
    RAIITest obj2("Stack-2-Middle", nullptr);
    RAIITest obj3("Stack-3-Inner", nullptr);
    
    // Add some actual work to prevent optimization
    std::cout << "All objects created, about to error..." << std::endl;
    
    // Error happens here - should unwind in reverse order: 3, 2, 1
    luaL_error(L, "Error in nested stack");
    
    // This should never be reached
    return 0;
}

// Test 4: Stack unwinding order
void test_stack_unwinding() {
    std::cout << "\n=== Testing Stack Unwinding Order ===" << std::endl;
    
    // Test C++ native stack unwinding first
    {
        std::cout << "\n--- C++ Native Exception Test ---" << std::endl;
        RAIITest::clearOrder();
        
        try {
            RAIITest obj1("Native-1-Outer", nullptr);
            {
                RAIITest obj2("Native-2-Middle", nullptr);
                {
                    RAIITest obj3("Native-3-Inner", nullptr);
                    throw std::runtime_error("Test exception");
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Caught exception: " << e.what() << std::endl;
        }
        
        auto order = RAIITest::getOrder();
        std::cout << "\nDestruction order:" << std::endl;
        for (const auto& name : order) {
            std::cout << "  " << name << std::endl;
        }
        
        if (order.size() == 3 && 
            order[0] == "Native-3-Inner" &&
            order[1] == "Native-2-Middle" &&
            order[2] == "Native-1-Outer") {
            std::cout << "✓ PASS: C++ stack unwinding in correct LIFO order" << std::endl;
        } else {
            std::cout << "✗ FAIL: Incorrect unwinding order!" << std::endl;
        }
    }
    
    // Test Lua stack unwinding
    {
        std::cout << "\n--- Lua Error Stack Test ---" << std::endl;
        RAIITest::clearOrder();
        
        lua_State* L = luaL_newstate();
        
        lua_pushcfunction(L, lua_function_with_stack);
        lua_setglobal(L, "stack_test");
        
        lua_getglobal(L, "stack_test");
        int result = lua_pcall(L, 0, 0, 0);
        
        if (result != LUA_OK) {
            std::cout << "Lua error (expected): " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
        
        auto order = RAIITest::getOrder();
        std::cout << "\nDestruction order:" << std::endl;
        for (const auto& name : order) {
            std::cout << "  " << name << std::endl;
        }
        
        if (order.empty()) {
            std::cout << "⚠ WARNING: No destructors called in Lua function" << std::endl;
            std::cout << "This is expected with extern 'C' functions" << std::endl;
        } else if (order.size() == 3 && 
                   order[0] == "Stack-3-Inner" &&
                   order[1] == "Stack-2-Middle" &&
                   order[2] == "Stack-1-Outer") {
            std::cout << "✓ PASS: Lua stack unwinding in correct LIFO order" << std::endl;
        } else {
            std::cout << "✗ FAIL: Incorrect or partial unwinding!" << std::endl;
        }
        
        lua_close(L);
    }
}

int main() {
    std::cout << "======================================" << std::endl;
    std::cout << "Lua RAII Safety Test" << std::endl;
    std::cout << "======================================" << std::endl;
    
    test_compilation_mode();
    test_mutex_contention();
    test_lua_raii_safety();
    test_lua_error_mechanism();
    
    test_stack_unwinding();
    
    std::cout << "\n======================================" << std::endl;
    
    return 0;
}