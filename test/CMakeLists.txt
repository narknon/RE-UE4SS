cmake_minimum_required(VERSION 3.22)
project(LuaRAIITest)

# Add test executable
add_executable(test_lua_raii ../test_lua_raii.cpp)

# Link with LuaRaw
target_link_libraries(test_lua_raii PRIVATE LuaRaw)

# Ensure C++23
target_compile_features(test_lua_raii PRIVATE cxx_std_23)

# Add to test suite
add_test(NAME LuaRAIITest COMMAND test_lua_raii)

# Print info about how Lua was built
get_target_property(LUA_SOURCES LuaRaw SOURCES)
message(STATUS "LuaRaw sources: ${LUA_SOURCES}")

# Check if Lua is being compiled as C++
get_source_file_property(LAPI_LANG ${CMAKE_CURRENT_SOURCE_DIR}/../deps/first/LuaRaw/src/lapi.c LANGUAGE)
if(LAPI_LANG)
    message(STATUS "Lua lapi.c is being compiled as: ${LAPI_LANG}")
endif()