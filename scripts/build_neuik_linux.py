#!/usr/bin/env python2
# encoding: utf-8

from build_neuik_srcs import *
import os

PLATFORM_NAME = "PLATFORM_LINUX"


def build_NEUIK(bld):
	#--------------------------------------------------------------------------#
	# Build the NEUIK library                                                  #
	#--------------------------------------------------------------------------#
	SOURCES.extend(SOURCES_LINUX)
	
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
		# bldDefines.extend([
		# 	"ENABLE_RUNTIME_TYPE_CHECKING",
		# ])
	else:
		# Optimized build options
		print "Building NEUIK using Optimized build options"
		bldCflags.extend([
			"-O2",
		])

	if bld.options.stlib:
		bld.stlib(
			source  = SOURCES,
			target  = LIB_DIR + "neuik", 
			defines = bldDefines,
			cflags  = bldCflags,
			stlib   = [
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
		source  = SOURCES,
		target  = LIB_DIR + "neuik", 
		defines = bldDefines,
		cflags  = bldCflags,
		lib     = [
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


def build_standalone_debug(bld):
	#--------------------------------------------------------------------------#
	# Build the resulting program (UN-optimized)                               #
	#--------------------------------------------------------------------------#
	name = "standalone_label"
	bld.program(
		source = [
			"examples/src/" + name + "/main.c",
		],
		target = "../examples/bin/" + name, 
		defines = [
			PLATFORM_NAME,
		],
		cflags  = [
			"-Wall",
			"-O0",
			"-ggdb",
		],
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
	bldDefines = [
		PLATFORM_NAME,
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

	lLIB        = []
	lLIB_PATH   = []
	lSTLIB      = []
	lSTLIB_PATH = []

	if bld.options.static:
		print "EXAMPLES WILL BE LINKED STATICALLY"
		# lSTLIB.extend([
		# 	"SDL2",
		# 	"SDL2_ttf",
		# 	"SDL2_image",
		# 	])
		lSTLIB.extend(["neuik"]) 
		lSTLIB_PATH.extend(["../lib"])

		#----------------------------------------------------------------------#
		# Add in dynamic links, don't add duplicate entries or any static libs #
		#----------------------------------------------------------------------#
		libSets = [
			bld.env.STLIB_SDL2, 
			bld.env.STLIB_SDL2_TTF, 
			bld.env.STLIB_SDL2_IMAGE,
			]

		lLIB.append("freetype")
		for libSet in libSets:
			for lib in libSet:
				if lib not in lSTLIB and lib not in lLIB:
					lLIB.append(lib)

		lLIB_PATH.extend(
			bld.env.STLIBPATH_SDL2 + \
			bld.env.STLIBPATH_SDL2_TTF + \
			bld.env.STLIBPATH_SDL_IMAGE
			)

		lSTLIB_PATH.extend(
			bld.env.STLIBPATH_SDL2 + \
			bld.env.STLIBPATH_SDL2_TTF + \
			bld.env.STLIBPATH_SDL_IMAGE
			)
	else:
		print "EXAMPLES WILL BE LINKED DYNAMICALLY"

		lLIB.extend(["neuik"]) 
		lLIB_PATH.extend([".", "../lib"])

		#----------------------------------------------------------------------#
		# Add in dynamic links, don't add duplicate entries                    #
		#----------------------------------------------------------------------#
		libSets = [
			bld.env.LIB_SDL2, 
			bld.env.LIB_SDL2_TTF, 
			bld.env.LIB_SDL2_IMAGE,
			]

		for libSet in libSets:
			for lib in libSet:
				if lib not in lLIB:
					lLIB.append(lib)

		lLIB_PATH.extend(
			bld.env.LIBPATH_SDL2 + \
			bld.env.LIBPATH_SDL2_TTF + \
			bld.env.LIBPATH_SDL_IMAGE
			)

	number = "00"
	name   = "helloWorld"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "01"
	name   = "blank"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	# number = "02"
	# name   = "mainMenu"
	# bld.program(
	# 	source = [
	# 		"examples/src/" + number + "-" + name + "/main-" + name + ".c",
	# 	],
	# 	target  = "../examples/bin/" + number + "-" + name, 
	# 	defines = bldDefines,
	# 	cflags  = bldCflags,
	# 	stlib   = lSTLIB,
	# 	lib     = lLIB,
	# 	linkflags = [],
	# 	includes = [
	# 		"../src",
	# 	],
	# 	stlibpath = lSTLIB_PATH,
	# 	libpath = lLIB_PATH,
	# )
	
	number = "03"
	name   = "hasButton"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)
	
	number = "04"
	name   = "vGroup"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "05"
	name   = "hGroup"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "06"
	name   = "calculator"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "07"
	name   = "textEntry"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "08"
	name   = "wetbulbCalc"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "09"
	name   = "progressCountdown"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB + ["SDL2"],
		linkflags = [],
		includes = [
			"../src",
		] + bld.env.INCLUDES_SDL2,
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH + bld.env.LIBPATH_SDL2,
	)

	number = "10"
	name   = "timeSinceStarted"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB + ["SDL2"],
		linkflags = [],
		includes = [
			"../src",
		] + bld.env.INCLUDES_SDL2,
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH + bld.env.LIBPATH_SDL2,
	)


	number = "11"
	name   = "hideAndShow"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "12"
	name   = "windowConfigure"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "13"
	name   = "eventHandler"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		] + bld.env.INCLUDES_SDL2,
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "14"
	name   = "stack"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "15"
	name   = "comboBox"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	# number = "16"
	# name   = "popupVisual"
	# bld.program(
	# 	source = [
	# 		"examples/src/" + number + "-" + name + "/main-" + name + ".c",
	# 	],
	# 	target  = "../examples/bin/" + number + "-" + name, 
	# 	defines = bldDefines,
	# 	cflags  = bldCflags,
	# 	stlib   = lSTLIB,
	# 	lib     = lLIB,
	# 	linkflags = [],
	# 	includes = [
	# 		"../src",
	# 	],
	# 	stlibpath = lSTLIB_PATH,
	# 	libpath = lLIB_PATH,
	# )

	number = "17"
	name   = "flowGroup"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "18"
	name   = "listGroup"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "19"
	name   = "errorReporter"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "20"
	name   = "hasImage"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)

	number = "21"
	name   = "textEdit"
	bld.program(
		source = [
			"examples/src/" + number + "-" + name + "/main-" + name + ".c",
		],
		target  = "../examples/bin/" + number + "-" + name, 
		defines = bldDefines,
		cflags  = bldCflags,
		stlib   = lSTLIB,
		lib     = lLIB,
		linkflags = [],
		includes = [
			"../src",
		],
		stlibpath = lSTLIB_PATH,
		libpath = lLIB_PATH,
	)
