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
    on_load(function (target)
        local ue4ss = target:dep("UE4SS")
        if ue4ss then
            -- Copy compile definitions from UE4SS
            local defs = ue4ss:get("defines")
            if defs then
                target:add("defines", defs)
            end
        end
    end)
    
    -- Run test after build
    after_build(function (target)
        print("Built Lua RAII test: " .. target:targetfile())
    end)