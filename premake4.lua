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

	function newos(opts)
		table.insert(premake.option.get("os").allowed, opts)
	end

	function newcc(opts)
		premake[opts.option[1]] = table.merge(premake.gcc, opts)
		table.insert(premake.action.get("gmake").valid_tools.cc, opts.option[1])
		table.insert(premake.option.get("cc").allowed, opts.option)
	end

	newos { "web", "Emscripten" }

	newcc {
		option    = { "emcc", "Emscripten (emcc/em++)" },
		cc        = "emcc",
		cxx       = "em++",
		ar        = "emar"
	}

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

		configuration "web"
			buildoptions { "-s USE_SDL=2", "-O2" }
			linkoptions  { "--preload-file ../roms", "--shell-file ../src/shell.html", "-s USE_SDL=2", "-O2" }
			links { "GL", "SDL" }
			targetextension ".html"

	project "emulator"
		if os.is("macosx") then -- Avoid creating an '.app' bundle
			kind "ConsoleApp"
		else
			kind "WindowedApp"
		end
		
		files { "src/emulator.c", "src/smn8/*.c" }

		configuration { "not web", "gmake" }
			buildoptions { "`" .. sdl2_config .. " --cflags`" }
			linkoptions { "`" .. sdl2_config .. " --libs`" }

		configuration { "linux", "gmake" }
			links { "m" }

		configuration "web"
			buildoptions { "-s USE_SDL=2", "-O2" }
			linkoptions  { "--preload-file ../roms", "--shell-file ../src/shell.html", "-s USE_SDL=2", "-O2" }
			links { "GL", "SDL" }
			targetextension ".html"
