project "asmjit"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    systemversion "latest"
    staticruntime "off"

    defines { "ASMJIT_STATIC" }

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("obj/" .. outputdir .. "/%{prj.name}")

    files {
        "asmjit/src/**.cpp",
        "asmjit/src/**.h"
    }

    includedirs {
        "asmjit/src"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
  
    filter "configurations:Release"
        runtime "Release"
        optimize "On"