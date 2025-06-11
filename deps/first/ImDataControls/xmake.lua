local projectName = "ImDataControls"

target(projectName)
    set_kind("headeronly")
    set_languages("cxx23")
    set_exceptions("cxx")
    add_rules("ue4ss.dependency")

    add_includedirs("include", { public = true })
    add_headerfiles("include/**.hpp")
    
    -- Dependencies
    add_deps("String", "Helpers")
    
    -- ImGui is added privately - not exported to dependent targets
    -- This allows parent projects to configure ImGui with their own settings
    -- Parent projects must provide their own ImGui
    add_packages("imgui")