local projectName = "LuaRAIITest"

target(projectName)
    set_kind("binary")
    set_languages("cxx23")
    set_exceptions("cxx")
    add_rules("ue4ss.dependency")
    
    add_files("src/main.cpp")
    add_deps("LuaRaw")
    
    set_basename("lua_raii_test")
    
    -- Run test after build
    after_build(function (target)
        print("Built Lua RAII test: " .. target:targetfile())
    end)