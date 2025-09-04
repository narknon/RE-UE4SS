// Direct test to verify if Lua is really compiled as C++
#include <iostream>
#include <exception>

// Test if Lua headers declare functions with C or C++ linkage
#include <lua.hpp>

// RAII test class
class TestRAII {
public:
    TestRAII() { std::cout << "RAII Constructor\n"; }
    ~TestRAII() { std::cout << "RAII Destructor\n"; }
};

// This function tests if lua_error throws a C++ exception
void test_lua_throws_exception() {
    std::cout << "\n=== Direct Lua Exception Test ===" << std::endl;
    
    lua_State* L = luaL_newstate();
    
    // Push a C++ function that uses RAII
    lua_pushcfunction(L, [](lua_State* L) -> int {
        TestRAII raii_object;
        
        // If Lua is compiled as C++, this should throw an exception
        // and the RAII destructor should be called
        lua_pushstring(L, "Test error");
        lua_error(L);  // This is the key test
        
        return 0; // Never reached
    });
    
    // Try to call it
    try {
        lua_pcall(L, 0, 0, 0);
        std::cout << "lua_pcall returned normally (C-style error handling)\n";
    } catch (const std::exception& e) {
        std::cout << "Caught C++ exception: " << e.what() << "\n";
    } catch (...) {
        std::cout << "Caught unknown C++ exception\n";
    }
    
    lua_close(L);
}

int main() {
    std::cout << "Direct Lua Compilation Test\n";
    std::cout << "==========================\n";
    
    // Check if we can detect Lua's compilation mode
    #ifdef __cplusplus
        std::cout << "This test is compiled as C++\n";
    #endif
    
    // The real test
    test_lua_throws_exception();
    
    std::cout << "\nIf you see 'RAII Destructor' above, Lua is compiled as C++\n";
    std::cout << "If not, Lua is compiled as C\n";
    
    return 0;
}