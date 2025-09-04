// Test Lua without extern "C" wrappers
#include <iostream>
#include <string>

// Include Lua headers WITHOUT extern "C"
// This only works if Lua is compiled as C++
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

class CppRAII {
    std::string name;
public:
    CppRAII(const std::string& n) : name(n) {
        std::cout << "CppRAII construct: " << name << std::endl;
    }
    ~CppRAII() {
        std::cout << "CppRAII destruct: " << name << std::endl;
    }
};

// Pure C++ function (no extern "C")
int cpp_function(lua_State* L) {
    CppRAII obj1("cpp-func-1");
    CppRAII obj2("cpp-func-2");
    CppRAII obj3("cpp-func-3");
    
    std::cout << "About to error from C++ function..." << std::endl;
    luaL_error(L, "Error from pure C++ function");
    
    return 0;
}

// Test with a C++ class method
class LuaWrapper {
public:
    static int method(lua_State* L) {
        CppRAII obj("method-raii");
        luaL_error(L, "Error from class method");
        return 0;
    }
};

int main() {
    std::cout << "=== Testing Lua as Pure C++ ===" << std::endl;
    
    #ifdef __cplusplus
    std::cout << "Compiled as C++" << std::endl;
    #endif
    
    lua_State* L = luaL_newstate();
    
    // Test 1: Pure C++ function
    std::cout << "\n### Test 1: C++ Function ###" << std::endl;
    lua_pushcfunction(L, cpp_function);
    if (lua_pcall(L, 0, 0, 0) != 0) {
        std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
    
    // Test 2: Class static method
    std::cout << "\n### Test 2: Class Method ###" << std::endl;
    lua_pushcfunction(L, LuaWrapper::method);
    if (lua_pcall(L, 0, 0, 0) != 0) {
        std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
    
    // Test 3: Lambda without extern C casting
    std::cout << "\n### Test 3: Direct Lambda ###" << std::endl;
    
    // This might not compile if lua_CFunction expects extern "C"
    auto lambda = [](lua_State* L) -> int {
        CppRAII obj("lambda-raii");
        luaL_error(L, "Error from lambda");
        return 0;
    };
    
    lua_pushcfunction(L, lambda);
    if (lua_pcall(L, 0, 0, 0) != 0) {
        std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
    
    lua_close(L);
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    return 0;
}