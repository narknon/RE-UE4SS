Write-Host "========================================"
Write-Host "Testing Lua C vs C++ Compilation"
Write-Host "========================================"

# Get the script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# Default configuration
$Config = "Game__Shipping__Win64"

# Create output directory for results
$OutputDir = "lua_test_results"
if (!(Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Force $OutputDir | Out-Null
}

$Timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$ResultsFile = Join-Path $OutputDir "lua_test_comparison_$Timestamp.txt"

# Function to run tests and capture output
function Run-Tests {
    param(
        [string]$BuildDir,
        [string]$BuildSystem,
        [string]$CompileMode
    )
    
    $Output = @()
    $Output += "=== $BuildSystem - Lua compiled as $CompileMode ==="
    
    # Find and run all test executables
    $tests = @("lua_raii_test.exe", "test_exception.exe", "test_order.exe", "test_full_cpp.exe")
    
    foreach ($test in $tests) {
        $testExe = Get-ChildItem -Path $BuildDir -Recurse -Filter $test -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($testExe) {
            $Output += "`n--- Running $test ---"
            try {
                $result = & $testExe.FullName 2>&1
                $Output += $result
            } catch {
                $Output += "Error running test: $_"
            }
        } else {
            $Output += "`n--- $test not found ---"
        }
    }
    
    return $Output -join "`n"
}

# Initialize results
$AllResults = @()
$AllResults += "Lua RAII Test Comparison - $Timestamp"
$AllResults += "========================================"

# Test 1: CMake with Lua as C
Write-Host "`n=== CMake - Building Lua as C ==="
if (Test-Path "build_lua_c") {
    Remove-Item -Recurse -Force "build_lua_c"
}
New-Item -ItemType Directory -Force "build_lua_c" | Out-Null
Set-Location build_lua_c
cmake .. "-DCMAKE_BUILD_TYPE=$Config" "-DLUA_COMPILE_AS_CPP=OFF"
cmake --build . --config $Config

$CmakeCResults = Run-Tests -BuildDir "." -BuildSystem "CMake" -CompileMode "C"
$AllResults += "`n`n$CmakeCResults"

Set-Location ..

# Test 2: CMake with Lua as C++
Write-Host "`n=== CMake - Building Lua as C++ ==="
if (Test-Path "build_lua_cpp") {
    Remove-Item -Recurse -Force "build_lua_cpp"
}
New-Item -ItemType Directory -Force "build_lua_cpp" | Out-Null
Set-Location build_lua_cpp
cmake .. "-DCMAKE_BUILD_TYPE=$Config" "-DLUA_COMPILE_AS_CPP=ON"
cmake --build . --config $Config

$CmakeCppResults = Run-Tests -BuildDir "." -BuildSystem "CMake" -CompileMode "C++"
$AllResults += "`n`n$CmakeCppResults"

Set-Location ..

# Test 3: xmake with Lua as C
Write-Host "`n=== xmake - Building Lua as C ==="
xmake f -m $Config --lua_compile_as_cpp=false -c
xmake build LuaRAIITest test_exception test_order test_full_cpp

$XmakeCResults = @()
$XmakeCResults += "=== xmake - Lua compiled as C ==="
foreach ($test in @("LuaRAIITest", "test_exception", "test_order", "test_full_cpp")) {
    $XmakeCResults += "`n--- Running $test ---"
    try {
        $result = xmake run $test 2>&1
        $XmakeCResults += $result
    } catch {
        $XmakeCResults += "Error running test: $_"
    }
}

$AllResults += "`n`n$($XmakeCResults -join "`n")"

# Test 4: xmake with Lua as C++
Write-Host "`n=== xmake - Building Lua as C++ ==="
xmake f -m $Config --lua_compile_as_cpp=true -c
xmake build LuaRAIITest test_exception test_order test_full_cpp

$XmakeCppResults = @()
$XmakeCppResults += "=== xmake - Lua compiled as C++ ==="
foreach ($test in @("LuaRAIITest", "test_exception", "test_order", "test_full_cpp")) {
    $XmakeCppResults += "`n--- Running $test ---"
    try {
        $result = xmake run $test 2>&1
        $XmakeCppResults += $result
    } catch {
        $XmakeCppResults += "Error running test: $_"
    }
}

$AllResults += "`n`n$($XmakeCppResults -join "`n")"

# Create comparison summary
$AllResults += "`n`n========================================"
$AllResults += "COMPARISON SUMMARY"
$AllResults += "========================================"

# Function to extract key metrics
function Extract-Metrics {
    param([string]$Results)
    
    $metrics = @{}
    
    # Extract destructor count
    if ($Results -match "Destructor call count: (\d+)") {
        $metrics.DestructorCount = $Matches[1]
    }
    
    # Extract extern C destructors
    if ($Results -match "extern 'C' function destructors: (\d+)") {
        $metrics.ExternCDestructors = $Matches[1]
    }
    
    # Extract lambda destructors
    if ($Results -match "lambda destructors: (\d+)") {
        $metrics.LambdaDestructors = $Matches[1]
    }
    
    # Check if stack unwinding passed
    $metrics.StackUnwindingPass = $Results -contains "PASS: Lua stack unwinding in correct LIFO order"
    
    # Check if mutex test passed
    $metrics.MutexTestPass = $Results -contains "PASS: Mutex properly unlocked after Lua error"
    
    return $metrics
}

# Extract and compare metrics
$AllResults += "`n`nKey Metrics Comparison:"
$AllResults += "-----------------------"

$configs = @(
    @{Name="CMake C"; Results=$CmakeCResults},
    @{Name="CMake C++"; Results=$CmakeCppResults},
    @{Name="xmake C"; Results=($XmakeCResults -join "`n")},
    @{Name="xmake C++"; Results=($XmakeCppResults -join "`n")}
)

$AllResults += "`n{0,-15} {1,-20} {2,-20} {3,-20} {4,-15} {5,-15}" -f `
    "Build", "Destructor Count", "Extern C Destrs", "Lambda Destrs", "Stack Unwind", "Mutex Test"
$AllResults += "-" * 105

foreach ($config in $configs) {
    $metrics = Extract-Metrics -Results $config.Results
    $AllResults += "{0,-15} {1,-20} {2,-20} {3,-20} {4,-15} {5,-15}" -f `
        $config.Name, 
        $(if($metrics.DestructorCount){"$($metrics.DestructorCount)"}else{"N/A"}),
        $(if($metrics.ExternCDestructors){"$($metrics.ExternCDestructors)"}else{"N/A"}),
        $(if($metrics.LambdaDestructors){"$($metrics.LambdaDestructors)"}else{"N/A"}),
        $(if($metrics.StackUnwindingPass){"PASS"}else{"FAIL"}),
        $(if($metrics.MutexTestPass){"PASS"}else{"FAIL"})
}

# Save results to file
$AllResults | Out-File -FilePath $ResultsFile -Encoding UTF8

Write-Host "`n========================================"
Write-Host "Test complete. Results saved to:"
Write-Host $ResultsFile
Write-Host "========================================"

# Display summary on console
Write-Host "`nSummary:"
Get-Content $ResultsFile | Select-Object -Last 10