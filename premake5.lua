workspace "superhotmcc"
   architecture "x64"
   configurations { "Debug", "Release" }
   startproject "superhotmcc"

   defines { 
      "ZYDIS_STATIC_BUILD",
      "ASMJIT_STATIC",
   }

   platforms { "Win64" }

   filter "platforms:Win64"
       system "Windows"
       architecture "x86_64"

outputdir = "%{cfg.buildcfg}-%{cfg.platform}"

group "Dependencies"
   include "vendor/minhook"
   include "vendor/zydis"
group ""

include "superhotmcc/premake5.lua"
include "superhotmcc-launcher/premake5.lua"