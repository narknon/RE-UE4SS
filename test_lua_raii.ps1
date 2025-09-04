Write-Host "========================================"
Write-Host "Testing Lua RAII Safety on Windows"
Write-Host "========================================"

# Get the script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# Default configuration
$Config = "Game__Shipping__Win64"

# Test with CMake
Write-Host "`n=== Building with CMake ==="
if (-not (Test-Path "build_lua_test")) {
    New-Item -ItemType Directory -Force "build_lua_test" | Out-Null
    Set-Location build_lua_test
    cmake .. "-DCMAKE_BUILD_TYPE=$Config"
    Set-Location ..
}

Set-Location build_lua_test
cmake --build . --target LuaRAIITest --config $Config

Write-Host "`n--- CMake Build Test Output ---"
# Find and run the test executable
$testExe = Get-ChildItem -Recurse -Filter "lua_raii_test.exe" | Select-Object -First 1
if ($testExe) {
    & $testExe.FullName
} else {
    # Try common locations
    $paths = @(
        ".\deps\test\LuaRAIITest\lua_raii_test.exe",
        ".\$Config\bin\lua_raii_test.exe",
        ".\bin\lua_raii_test.exe",
        ".\Debug\lua_raii_test.exe",
        ".\Release\lua_raii_test.exe"
    )
    foreach ($path in $paths) {
        if (Test-Path $path) {
            & $path
            break
        }
    }
}
Set-Location ..

# Test with xmake
Write-Host "`n`n=== Building with xmake ==="
xmake f -m $Config
xmake build LuaRAIITest
Write-Host "`n--- xmake Build Test Output ---"
xmake run LuaRAIITest

Write-Host "`n========================================"
Write-Host "Test complete. Both builds should show:"
Write-Host "1. Build config: SHIPPING"
Write-Host "2. Destructor count: 2"
Write-Host "3. PASS: RAII is working correctly"
Write-Host "========================================"