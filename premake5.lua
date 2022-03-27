workspace "LearnOpenGL"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

-- outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir = "%{cfg.buildcfg}"

project "GraphIt"
	location "src"
	kind "ConsoleApp"
	language "C++"
    cppdialect "C++17"
    debugdir "./"

	-- targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	-- objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	targetdir ("bin/" .. outputdir .. "/")
	objdir ("bin-int/" .. outputdir .. "/")

    -- printf("Configuration: %s %s", os.host(), _ACTION )

	files
	{
		"src/**.h",
		"src/**.cpp",

        "vendor/stb_image/stb_image.cpp",
        -- "vendor/imgui/imgui.cpp",
        -- "vendor/imgui/imgui_demo.cpp",
        -- "vendor/imgui/imgui_draw.cpp",
        -- "vendor/imgui/imgui_tables.cpp",
        -- "vendor/imgui/imgui_widgets.cpp",
        -- "vendor/imgui/backends/imgui_impl_opengl3.cpp",
        -- "vendor/imgui/backends/imgui_impl_glfw.cpp",
	}

	includedirs
	{
		"src",
        "vendor/GLFW/include",
        "vendor/GLEW/include",
        
        "vendor/glm",
        "vendor/imgui",
        "vendor/imgui/backends",
        "vendor/spdlog/include",
        "vendor/stb_image",

        "vendor/Freetype",
        "vendor/Freetype/Freetype"
	}

    filter "system:windows"
        staticruntime "On"
		systemversion "latest"
        defines
        {
            "RM_WIN=1",
        }

        links
        {
            "DbgHelp"
        }

    filter { "system:windows", "action:gmake*" }
        defines
        {
            "CATCH_SIGINT=1",
            "RM_WIN_MAKE=1"
        }

        includedirs {
            "C:/dev/external/include",
        }

        libdirs { 
            "C:/dev/external/lib",
        }

        links
        {
            "glew32",
            "glfw3",
            "freetype",

            "opengl32",
            "glu32",
            "gdi32",

            -- These are for freetype
            "bz2",
            "z",
        }
    
    filter { "system:windows", "action:vs*" }
        defines
        {
            "CATCH_SIGINT=0",
            "GLEW_STATIC",
            "RM_WIN_VS=1",
            "_CRT_SECURE_NO_WARNINGS"
        }

        libdirs {
            "vendor/GLEW/lib-win-vs/",
            "vendor/GLFW/lib-win-vs/", 
            "vendor/Freetype/lib-win-vs/",
        }

        links
        {
            "glew32s",
            "glfw3_mt",
            "freetype",

            "opengl32",
            "glu32",
            "gdi32",

            -- These are for freetype
            -- "bz2",
            -- "z",
        }
        

    filter "system:macosx"
        -- systemversion "latest"
        defines
		{
			"RM_MAC=1",
		}
        
        libdirs {
            "vendor/GLEW/lib-mac/",
            "vendor/GLFW/lib-mac/", 
            "vendor/Freetype/lib-mac/",
            "/usr/lib/",
        }

        links
        {
            "glew",
            "glfw3",
            "freetype",

            "Cocoa.framework",
            "IOKit.framework",
            "OpenGL.framework",
            "bz2.1.0",
            "z.1"

        }


	filter "configurations:Debug"
		defines "RM_DEBUG=1"
		symbols "On"

	filter "configurations:Release"
		defines "RM_RELEASE=1"
		optimize "On"

	filter "configurations:Dist"
		defines "RM_DIST=1"
		optimize "On"