cmake_minimum_required(VERSION 3.22)

set(TARGET LuaRAIITest)
project(${TARGET})

message("Project: ${TARGET}")

# Create executable
add_executable(${TARGET} 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

# Add additional test executable
add_executable(test_lua_direct
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test_direct.cpp"  
)
target_compile_features(test_lua_direct PRIVATE cxx_std_23)
target_link_libraries(test_lua_direct PRIVATE LuaRaw)

add_executable(test_exception
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test_exception.cpp"  
)
target_compile_features(test_exception PRIVATE cxx_std_23)
target_link_libraries(test_exception PRIVATE LuaRaw)

add_executable(test_order
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test_order.cpp"  
)
target_compile_features(test_order PRIVATE cxx_std_23)
target_link_libraries(test_order PRIVATE LuaRaw)

# Set C++23 standard
target_compile_features(${TARGET} PRIVATE cxx_std_23)

# Link with LuaRaw and inherit UE4SS definitions
target_link_libraries(${TARGET} PRIVATE LuaRaw)

# If UE4SS target exists, inherit its compile definitions
if(TARGET UE4SS)
    # Get UE4SS's interface compile definitions
    get_target_property(UE4SS_INTERFACE_DEFS UE4SS INTERFACE_COMPILE_DEFINITIONS)
    if(UE4SS_INTERFACE_DEFS)
        target_compile_definitions(${TARGET} PRIVATE ${UE4SS_INTERFACE_DEFS})
    endif()
endif()

# Make headers visible in the IDE
# Uses make_headers_visible() from cmake/modules/IDEVisibility.cmake
make_headers_visible(${TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Set output name without lib prefix
set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME "lua_raii_test"
)

# Add as test
enable_testing()
add_test(NAME ${TARGET} COMMAND ${TARGET})