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

target("test_exception")
    set_kind("binary")
    set_languages("cxx23")
    set_exceptions("cxx")
    add_rules("ue4ss.dependency")
    
    add_files("src/test_exception.cpp")
    add_deps("LuaRaw")
    
    set_basename("test_exception")

target("test_order")
    set_kind("binary")
    set_languages("cxx23")
    set_exceptions("cxx")
    add_rules("ue4ss.dependency")
    
    add_files("src/test_order.cpp")
    add_deps("LuaRaw")
    
    set_basename("test_order")

target("test_full_cpp")
    set_kind("binary")
    set_languages("cxx23")
    set_exceptions("cxx")
    add_rules("ue4ss.dependency")
    
    add_files("src/test_full_cpp.cpp")
    add_deps("LuaRaw")
    
    set_basename("test_full_cpp")