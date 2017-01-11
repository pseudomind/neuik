#!/usr/bin/env python2
# encoding: utf-8

from build_neuik_srcs import *
import os

PLATFORM_NAME = "PLATFORM_OSX"


def build_NEUIK(bld):
	#--------------------------------------------------------------------------#
	# Build the NEUIK library                                                  #
	#--------------------------------------------------------------------------#
	SOURCES.extend(SOURCES_OSX)

	bldCflags = [
		"-Wall",
	]

	bldDefines = [
		PLATFORM_NAME,
	]

	if bld.options.debug:
		# Debug build options
		print "Building NEUIK using debug build options"
		bldCflags.extend([
			"-O0",
			"-ggdb",
		])
		bldDefines.extend([
			"ENABLE_RUNTIME_TYPE_CHECKING",
		])
	else:
		# Optimized build options
		print "Building NEUIK using Optimized build options"
		bldCflags.extend([
			"-O2",
		])
		bldDefines.extend([
			"ENABLE_RUNTIME_TYPE_CHECKING",
		])

	if bld.options.stlib:
		bld.stlib(
			source = SOURCES,
			target = LIB_DIR + "neuik", 
			defines = bldDefines,
			cflags  = bldCflags,
			lib = [
				"SDL2",
				"SDL2_ttf",
				"SDL2_image",
			],
			linkflags = [
			],
			includes = [
			] + bld.env.INCLUDES_SDL2,
			libpath  = [
			] + bld.env.LIBPATH_SDL2,
		)

	bld.shlib(
		source = SOURCES,
		target = LIB_DIR + "neuik", 
		defines = bldDefines,
		cflags  = bldCflags,
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
		] + bld.env.INCLUDES_SDL2,
		
		libpath  = [
		] + bld.env.LIBPATH_SDL2,
	)


def build_standalone(bld):
	#--------------------------------------------------------------------------#
	# Build the standalone programs                                            #
	#--------------------------------------------------------------------------#

	bldCflags = [
		"-Wall",
	]

	if bld.options.debug:
		# Debug build options
		print "Building Examples using debug build options"
		bldCflags.extend([
			"-O0",
			"-ggdb",
		])
	else:
		# Optimized build options
		print "Building Examples using Optimized build options"
		bldCflags.extend([
			"-O2",
		])

	name = "standalone_label"
	bld.program(
		source = [
			"examples/src/" + name + "/main.c",
		],
		target = "../examples/bin/" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		lib = [
			"SDL2",
			"SDL2_ttf",
		],
		linkflags = [
		],
		includes = [
			"/usr/local/include/SDL2",
			"/usr/local/include/SDL2_ttf",
		],
		libpath  = [
			"/usr/local/lib",
		],
	)


def build_examples(bld):
	#--------------------------------------------------------------------------#
	# Build the resulting example programs                                     #
	#--------------------------------------------------------------------------#

	bldCflags = [
		"-Wall",
	]

	if bld.options.debug:
		# Debug build options
		print "Building Examples using debug build options"
		bldCflags.extend([
			"-O0",
			"-ggdb",
		])
	else:
		# Optimized build options
		print "Building Examples using Optimized build options"
		bldCflags.extend([
			"-O2",
		])


	number = "00"
	name   = "helloWorld"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"neuik",
		],
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		libpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "01"
	name   = "blank"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"neuik",
		],
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		libpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "02"
	name   = "mainMenu"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
			"neuik",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "03"
	name   = "hasButton"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
			"neuik",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "04"
	name   = "vGroup"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
			"neuik",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "05"
	name   = "hGroup"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
			"neuik",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "06"
	name   = "calculator"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
			"neuik",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "07"
	name   = "textEntry"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
			"neuik",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "08"
	name   = "wetbulbCalc"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
			"neuik",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "09"
	name   = "progressCountdown"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"neuik",
		],
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
			"../src",
		] + bld.env.INCLUDES_SDL2,
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		] + bld.env.LIBPATH_SDL2,
	)

	number = "10"
	name   = "timeSinceStarted"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"neuik",
		],
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
			"../src",
		] + bld.env.INCLUDES_SDL2,
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		] + bld.env.LIBPATH_SDL2,
	)

	number = "11"
	name   = "hideAndShow"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"neuik",
		],
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "12"
	name   = "windowConfigure"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"neuik",
		],
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
			"../src",
		],
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		],
	)

	number = "13"
	name   = "eventHandler"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"neuik",
		],
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
			"../src",
		] + bld.env.INCLUDES_SDL2,
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		] + bld.env.LIBPATH_SDL2,
	)

	number = "14"
	name   = "stack"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"neuik",
		],
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
			"../src",
		] + bld.env.INCLUDES_SDL2,
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		] + bld.env.LIBPATH_SDL2,
	)


	number = "15"
	name   = "comboBox"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"neuik",
		],
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
			"../src",
		] + bld.env.INCLUDES_SDL2,
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		] + bld.env.LIBPATH_SDL2,
	)

	number = "16"
	name   = "popupVisual"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = bldCflags,
		stlib = [
			"neuik",
		],
		lib = [
			"SDL2",
			"SDL2_ttf",
			"SDL2_image",
		],
		linkflags = [
		],
		includes = [
			"../src",
		] + bld.env.INCLUDES_SDL2,
		stlibpath  = [
			"/usr/local/lib",
			"../lib",
		] + bld.env.LIBPATH_SDL2,
	)
