project "GLFW"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"
    warnings "off"

    files
    {
        "GLFW/include/GLFW/glfw3.h",
        "GLFW/include/GLFW/glfw3native.h",
        "GLFW/src/glfw_config.h",
        "GLFW/src/context.c",
        "GLFW/src/init.c",
        "GLFW/src/input.c",
        "GLFW/src/monitor.c",
        "GLFW/src/null_init.c",
        "GLFW/src/null_joystick.c",
        "GLFW/src/null_monitor.c",
        "GLFW/src/null_window.c",
        "GLFW/src/platform.c",
        "GLFW/src/vulkan.c",
        "GLFW/src/window.c",
    }

    targetdir ("Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:linux"
        pic "On"
        systemversion "latest"

        files
        {
            "GLFW/src/x11_init.c",
            "GLFW/src/x11_monitor.c",
            "GLFW/src/x11_window.c",
            "GLFW/src/xkb_unicode.c",
            "GLFW/src/posix_module.c",
            "GLFW/src/posix_time.c",
            "GLFW/src/posix_thread.c",
            "GLFW/src/posix_module.c",
            "GLFW/src/posix_poll.c",
            "GLFW/src/glx_context.c",
            "GLFW/src/egl_context.c",
            "GLFW/src/osmesa_context.c",
            "GLFW/src/linux_joystick.c"
        }

                links { "GL" }
        defines { "_GLFW_X11" }

    filter "system:windows"
        systemversion "latest"

        files
        {
            "GLFW/src/win32_init.c",
            "GLFW/src/win32_joystick.c",
            "GLFW/src/win32_module.c",
            "GLFW/src/win32_monitor.c",
            "GLFW/src/win32_time.c",
            "GLFW/src/win32_thread.c",
            "GLFW/src/win32_window.c",
            "GLFW/src/wgl_context.c",
            "GLFW/src/egl_context.c",
            "GLFW/src/osmesa_context.c"
        }

                links { "OpenGL32" }
        defines { "_GLFW_WIN32", }
        
    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        runtime "Release"
        optimize "On"
        symbols "Off"
