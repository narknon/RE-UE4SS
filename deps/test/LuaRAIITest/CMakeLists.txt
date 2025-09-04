cmake_minimum_required(VERSION 3.22)

set(TARGET LuaRAIITest)
project(${TARGET})

message("Project: ${TARGET}")

# Create executable
add_executable(${TARGET} 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

# Set C++23 standard
target_compile_features(${TARGET} PRIVATE cxx_std_23)

# Link with LuaRaw
target_link_libraries(${TARGET} PRIVATE LuaRaw)

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