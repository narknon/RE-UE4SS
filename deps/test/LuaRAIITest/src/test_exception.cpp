#include <iostream>
#include <mutex>
#include <string>
#include <vector>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

// Simple RAII tracker
class Tracker {
    std::string name;
public:
    Tracker(const std::string& n) : name(n) {
        std::cout << "CREATE: " << name << " at " << this << std::endl;
    }
    ~Tracker() {
        std::cout << "DESTROY: " << name << " at " << this << std::endl;
    }
};

// Test 1: What happens to std::lock_guard?
int test_lockguard(lua_State* L) {
    static std::mutex mtx;
    std::cout << "\n--- In test_lockguard ---" << std::endl;
    
    Tracker t1("before-lock");
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Mutex locked" << std::endl;
    Tracker t2("after-lock");
    
    luaL_error(L, "Error with mutex locked");
    return 0;
}

// Test 2: Order of different RAII types
int test_mixed_raii(lua_State* L) {
    static std::mutex mtx;
    std::cout << "\n--- In test_mixed_raii ---" << std::endl;
    
    Tracker t1("tracker-1");
    std::lock_guard<std::mutex> lock(mtx);
    Tracker t2("tracker-2");
    std::string str("test-string");
    Tracker t3("tracker-3");
    
    std::cout << "About to error..." << std::endl;
    luaL_error(L, "Mixed RAII error");
    return 0;
}

// Test 3: What if we catch and rethrow?
int test_catch_rethrow(lua_State* L) {
    std::cout << "\n--- In test_catch_rethrow ---" << std::endl;
    
    Tracker t1("catch-1");
    try {
        Tracker t2("catch-2");
        luaL_error(L, "Caught error");
    } catch (...) {
        std::cout << "Caught exception in C function" << std::endl;
        throw;
    }
    return 0;
}

int main() {
    std::cout << "=== Exception Diagnostic Test ===" << std::endl;
    
    lua_State* L = luaL_newstate();
    
    // Test 1
    {
        std::cout << "\n### Test 1: Lock Guard ###" << std::endl;
        lua_pushcfunction(L, test_lockguard);
        if (lua_pcall(L, 0, 0, 0) != 0) {
            std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
        
        // Try to lock the mutex
        static std::mutex mtx;
        if (mtx.try_lock()) {
            std::cout << "✓ Mutex is unlocked" << std::endl;
            mtx.unlock();
        } else {
            std::cout << "✗ Mutex is still locked!" << std::endl;
        }
    }
    
    // Test 2
    {
        std::cout << "\n### Test 2: Mixed RAII ###" << std::endl;
        lua_pushcfunction(L, test_mixed_raii);
        if (lua_pcall(L, 0, 0, 0) != 0) {
            std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    }
    
    // Test 3
    {
        std::cout << "\n### Test 3: Catch and Rethrow ###" << std::endl;
        lua_pushcfunction(L, test_catch_rethrow);
        if (lua_pcall(L, 0, 0, 0) != 0) {
            std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    }
    
    lua_close(L);
    
    // Test if luaL_error throws a C++ exception we can catch
    {
        std::cout << "\n### Test 4: Direct Exception Type ###" << std::endl;
        lua_State* L2 = luaL_newstate();
        
        auto test_func = [](lua_State* L) -> int {
            try {
                luaL_error(L, "Test exception type");
            } catch (const std::exception& e) {
                std::cout << "Caught std::exception: " << e.what() << std::endl;
                return 0;
            } catch (...) {
                std::cout << "Caught unknown exception type" << std::endl;
                return 0;
            }
            std::cout << "No exception caught!" << std::endl;
            return 0;
        };
        
        lua_pushcfunction(L2, test_func);
        lua_pcall(L2, 0, 0, 0);
        
        lua_close(L2);
    }
    
    return 0;
}