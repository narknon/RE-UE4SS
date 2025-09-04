-- Test for Lua RAII safety
target("test_lua_raii")
    set_kind("binary")
    set_languages("cxx23")
    set_exceptions("cxx")
    add_files("../test_lua_raii.cpp")
    add_deps("LuaRaw")
    
    -- Run test after build
    after_build(function (target)
        print("Running Lua RAII safety test...")
        os.exec(target:targetfile())
    end)