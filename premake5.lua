function GetArchitecture()
    return iif(os.is("windows"), "Win64", "Unknown")
end

local BasePath 	        = os.getcwd()
local SourcePath 	    = path.join(BasePath, "Source")
local ThirdPartyPath    = path.join(BasePath, "ThirdParty")
local TargetPath 	    = path.join(BasePath, "Binaries", GetArchitecture(), "%{cfg.buildcfg}", "%{prj.name}")
local IntermediatePath 	= path.join(BasePath, "Intermediate", GetArchitecture(), "%{cfg.buildcfg}", "%{prj.name}")

local SDL2Path 	        = path.join(ThirdPartyPath, "SDL2-2.0.14")

workspace "Chip8-Emulator"
    architecture    "x86_64"
    startproject    "Emulator"
    language        "C++"
    cppdialect      "C++11"
    staticruntime   "On"
    flags {
        "MultiProcessorCompile"
    }
    configurations {
        "Debug",
        "Release"
    }
    debugdir("ROMs")
    targetdir(TargetPath)
    objdir(IntermediatePath)

project "Emulator"
    kind "ConsoleApp"
    location(SourcePath)
    includedirs {
        SourcePath,
        path.join(SDL2Path, "include")
    }
    files {
        path.join(SourcePath, "**.h"),
        path.join(SourcePath, "**.cpp")
    }
    libdirs {
        path.join(SDL2Path, "lib", "x64")
    }
    links {
        "SDL2",
        "SDL2main"
    }
    postbuildcommands {
        "{copy} "..path.join(SDL2Path, "lib", "x64", "SDL2.dll").." %{cfg.targetdir}"
    }