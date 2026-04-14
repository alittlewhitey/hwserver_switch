set_project("hwserver_switch")
set_version("1.0.0")

set_languages("c++17")
add_requires("openssl")


target("StartServer")
    set_kind("binary")
    add_includedirs("thirdparty/nlohmann_json", "thirdparty/yhirose_cpp-httplib")
    add_files("StartServer.cpp")
    add_packages("openssl")
    if not is_plat("windows", "macosx") then 
        add_ldflags("-static", "-static-libgcc", "-static-libstdc++", {force = true})
    end
    if is_plat("macosx") then
        add_frameworks("CoreFoundation", "Security")
    end

target("StopServer")
    set_kind("binary")
    add_includedirs("thirdparty/nlohmann_json", "thirdparty/yhirose_cpp-httplib")
    add_files("StopServer.cpp")
    add_packages("openssl")
    if not is_plat("windows", "macosx") then 
        add_ldflags("-static", "-static-libgcc", "-static-libstdc++", {force = true})
    end
    if is_plat("macosx") then
        add_frameworks("CoreFoundation", "Security")
    end

target("Configure")
    set_kind("binary")
    add_includedirs("thirdparty/nlohmann_json", "thirdparty/yhirose_cpp-httplib")
    add_files("Configure.cpp")
    add_packages("openssl")
    if not is_plat("windows", "macosx") then 
        add_ldflags("-static", "-static-libgcc", "-static-libstdc++", {force = true})
    end
    if is_plat("macosx") then
        add_frameworks("CoreFoundation", "Security")
    end