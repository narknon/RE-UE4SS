#!/bin/bash

echo "========================================"
echo "Testing Lua RAII Safety"
echo "========================================"

# Get the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Default configuration
CONFIG="Game__Shipping__Win64"

# Test with CMake
echo -e "\n=== Building with CMake ==="
if [ ! -d "build_lua_test" ]; then
    mkdir build_lua_test
    cd build_lua_test
    cmake .. -DCMAKE_BUILD_TYPE=$CONFIG
    cd ..
fi

cd build_lua_test
cmake --build . --target LuaRAIITest --config $CONFIG
echo -e "\n--- CMake Build Test Output ---"
# Find and run the test executable
find . -name "lua_raii_test" -type f -executable -exec {} \; || \
    ./deps/test/LuaRAIITest/lua_raii_test || \
    ./$CONFIG/bin/lua_raii_test || \
    ./bin/lua_raii_test
cd ..

# Test with xmake  
echo -e "\n\n=== Building with xmake ==="
xmake f -m $CONFIG
xmake build LuaRAIITest
echo -e "\n--- xmake Build Test Output ---"
xmake run LuaRAIITest

echo -e "\n========================================"
echo "Test complete. Both builds should show:"
echo "1. Build config: SHIPPING"
echo "2. Destructor count: 2"
echo "3. PASS: RAII is working correctly"
echo "========================================"