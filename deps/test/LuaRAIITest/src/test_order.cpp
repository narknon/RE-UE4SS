#include <iostream>
#include <mutex>
#include <string>

#include <lua.hpp>

class OrderTest {
    std::string name;
    static int counter;
    int id;
public:
    OrderTest(const std::string& n) : name(n), id(++counter) {
        std::cout << "Create #" << id << ": " << name << std::endl;
    }
    ~OrderTest() {
        std::cout << "Destroy #" << id << ": " << name << std::endl;
    }
};
int OrderTest::counter = 0;

// Test what exactly gets destroyed
int test_destruction(lua_State* L) {
    std::cout << "\n--- Entering test_destruction ---" << std::endl;
    
    OrderTest obj1("First");
    OrderTest obj2("Second");
    OrderTest obj3("Third");
    
    std::cout << "About to call luaL_error..." << std::endl;
    
    // Let's also test if the problem is specific to luaL_error vs lua_error
    #if 1
        luaL_error(L, "Test error");
    #else
        lua_pushstring(L, "Test error");
        lua_error(L);
    #endif
    
    std::cout << "This line should never execute" << std::endl;
    return 0;
}

// Compare with lock_guard behavior
int test_with_mutex(lua_State* L) {
    static std::mutex m;
    std::cout << "\n--- Entering test_with_mutex ---" << std::endl;
    
    OrderTest obj1("Before-mutex");
    std::lock_guard<std::mutex> lock(m);
    OrderTest obj2("After-mutex");
    
    std::cout << "About to error with mutex locked..." << std::endl;
    luaL_error(L, "Error with mutex");
    
    return 0;
}

int main() {
    std::cout << "=== Destruction Order Test ===" << std::endl;
    
    // First, verify C++ exception behavior
    std::cout << "\n### C++ Exception Test ###" << std::endl;
    try {
        OrderTest obj1("Cpp-1");
        OrderTest obj2("Cpp-2");
        OrderTest obj3("Cpp-3");
        throw std::runtime_error("C++ exception");
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << std::endl;
    }
    
    // Test Lua error behavior
    lua_State* L = luaL_newstate();
    
    std::cout << "\n### Lua Error Test ###" << std::endl;
    lua_pushcfunction(L, test_destruction);
    if (lua_pcall(L, 0, 0, 0) != 0) {
        std::cout << "Lua error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
    
    std::cout << "\n### Lua Mutex Test ###" << std::endl;
    lua_pushcfunction(L, test_with_mutex);
    if (lua_pcall(L, 0, 0, 0) != 0) {
        std::cout << "Lua error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
    
    lua_close(L);
    
    // Check if we can detect the exception type
    std::cout << "\n### Exception Type Test ###" << std::endl;
    
    #ifdef __cpp_exceptions
        std::cout << "C++ exceptions are ENABLED" << std::endl;
    #else
        std::cout << "C++ exceptions are DISABLED" << std::endl;
    #endif
    
    #ifdef LUA_USE_LONGJMP
        std::cout << "Lua configured with LUA_USE_LONGJMP" << std::endl;
    #endif
    
    return 0;
}