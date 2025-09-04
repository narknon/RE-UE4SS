local projectName = "LuaRAIITest"

target(projectName)
    set_kind("binary")
    set_languages("cxx23")
    set_exceptions("cxx")
    add_rules("ue4ss.dependency")
    
    add_files("src/main.cpp")
    add_deps("LuaRaw")
    
    set_basename("lua_raii_test")
    
    -- Inherit definitions from UE4SS if available
    after_load(function (target)
        -- Don't try to inherit from UE4SS since it's not a dependency
        -- The test only depends on LuaRaw
        -- Build definitions should come from the build system automatically
    end)
    
    -- Run test after build
    after_build(function (target)
        print("Built Lua RAII test: " .. target:targetfile())
    end)