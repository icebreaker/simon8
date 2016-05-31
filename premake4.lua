solution "smn8"
  local sdl2_config = "sdl2-config"

  if _ACTION == "clean" then
    os.rmdir("build")
  elseif _ACTION == "gmake" and os.is("windows") then
    local mingw32 = "i686-w64-mingw32"
    premake.gcc.cc = mingw32 .. "-gcc" -- set cross-compiler
    sdl2_config = "/usr/" .. mingw32  .. "/sys-root/mingw/bin/" .. sdl2_config
    if not os.isfile(sdl2_config) then
      printf("WARNING: '%s' cannot be found.", sdl2_config)
    end
  end

  newoption {
    trigger     = "with-audio",
    description = "Enables support for audio"
  }
  newoption {
    trigger     = "with-arrow-keys",
    description = "Enables support for arrow keys"
  }
  newoption {
    trigger     = "with-lime",
    description = "Enables 'lime' as foreground color"
  }

  location "build"
  objdir "build"
  targetdir "build"
  language "C"

  configurations { "debug", "release" }
  includedirs { "src" }  

  configuration "debug"
    defines { "SMN8_DEBUG" }

  configuration "release"
    defines { "SMN8_NDEBUG" }

  configuration "with-audio"
    defines { "HAVE_AUDIO" }

  configuration "with-arrow-keys"
    defines { "HAVE_ARROW_KEYS" }

  configuration "with-lime"
    defines { "HAVE_LIME" }

  configuration "gmake"
    buildoptions { "-O3", "-funroll-loops" }

  configuration { "linux", "debug", "gmake" }
    buildoptions { "-Wall", "-Wextra", "-Werror", "-ansi", "-pedantic" }
    defines { "_GNU_SOURCE" }

  project "disasm"
    kind "ConsoleApp"
    files { "src/disasm.c", "src/smn8/rom.c" }

  project "emulator"
    if os.is("macosx") then -- Avoid creating an '.app' bundle
      kind "ConsoleApp"
    else
      kind "WindowedApp"
    end

    configuration "gmake"
      buildoptions { "`" .. sdl2_config .. " --cflags`" }
      linkoptions { "`" .. sdl2_config .. " --libs`" }

    configuration { "linux", "gmake" }
      links { "m" }

    files { "src/emulator.c", "src/smn8/*.c" }
