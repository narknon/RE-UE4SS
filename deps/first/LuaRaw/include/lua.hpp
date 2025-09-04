// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

// When Lua is compiled as C, we need extern "C" wrappers
// When Lua is compiled as C++, we don't want them
#ifdef LUA_COMPILED_AS_CPP
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
#else
    extern "C" {
        #include "lua.h"
        #include "lualib.h"
        #include "lauxlib.h"
    }
#endif
