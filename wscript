#!/usr/bin/env python2
# encoding: utf-8

#==============================================================================#
# Arguments/Flags Unique to this Script:
#------------------------------------------------------------------------------#
#
#------------------------------------------------------------------------------#
# Getting/installing WAF:
#------------------------------------------------------------------------------#
#    Waf is a software build system written in python. It can be downloaded from
#    the following location:
#
#        waf.googlecode.com
#
#    * Simply download the latest copy of the program (e.g. "waf 1.7.16") from the
#      "Links" section of the webpage.
#    * Rename this python script to "waf"
#    * Move this script into a directory which is both in your $PATH, and is 
#      writable by you (this script will inflate itself in the directory which
#      contains it, the first time it is run)
#    * Try running "waf --version" from the command line and verify that you see
#      something like the following:
#
#        waf 1.7.16 (73c1705078f8c9c51a33e20f221a309d5a94b5e1)
#
#    If you have followed these steps and are correctly getting a version string
#    from the final step, then you have waf correctly installed.
#
#------------------------------------------------------------------------------#
# Basic usage of WAF:
#------------------------------------------------------------------------------#
#  Building a Project:
# ---------------------
#    Building a project with waf generally consists of two different steps:
#
#      1. Running `waf configure` in a directory which contains a waf script 
#         file (wscript).
#
#      2. Subsequently running `waf` or `waf build` in that same directory. 
#         NOTE: Calling waf without any arguments is equivalent to calling 
#         `waf build` at that location.
#
#  Verbose Mode:
# ---------------------
#    Addtional information about a build can be gained by adding the verbose 
#    flag to either command (e.g. `waf -v` or `waf -v build` will show the 
#	 command line arguments used for compiling each file of a project).
#
#  Clean & Distclean Procedures:
# ---------------------
#    The object files and resulting programs/libraries can be quickly deleted by
#    using the command `waf clean`. Note that an even more extensive clean 
#    procedure `waf distclean` exits. This procedure will completely remove the 
#    build directory and all of the temporary files generated by waf. After 
#    using distclean, you will be required to run `waf configure` again before
#    the project can be built
#
#==============================================================================#

import sys
import os

# used for capturing unsupported platforms
platform = None

if sys.platform in ["darwin"]:
	import scripts.build_neuik_osx as platform
if sys.platform in ["linux", "linux2", "linux3"]:
	import scripts.build_neuik_linux as platform
if sys.platform in ["win32"]:
	import scripts.build_neuik_windows as platform


APPNAME = "neuik"
VERSION = "0.0.2"
out     = "build"

def options(opt):
	opt.load("compiler_c")
	#----------------------------------------------------------------#
	# Here is where you can add additional flags to the build script #
	#----------------------------------------------------------------#
	opt.add_option(
		'--debug', 
		action  = "store_true", 
		default = False, 
		help    = "Build RELEASE version instead of DEBUG version",
	)

	opt.add_option(
		'--examples', 
		action  = "store_true", 
		default = False, 
		help    = "Build the associated example programs",
	)

	opt.add_option(
		'--all', 
		action  = "store_true", 
		default = False, 
		help    = "Build the library and the associated example programs",
	)

	opt.add_option(
		'--standalone', 
		action  = "store_true", 
		default = False, 
		help    = "Build associated standalone example programs",
	)

	opt.add_option(
		'--stlib', 
		action  = "store_true", 
		default = False, 
		help    = "Build NEUIK as a static library",
	)

	opt.add_option(
		'--static', 
		action  = "store_true", 
		default = False, 
		help    = "Statically link when creating binaries",
	)


def configure(conf):
	conf.load("compiler_c")

	if conf.options.static:
		# check that the SDL libraries are available
		print "[[[ USING STATIC LINKING ]]]"
		conf.check_cfg(package='sdl2', uselib_store = "SDL2",
			args=['--cflags', '--libs', '--static'])
		conf.check_cfg(package='SDL2_ttf', uselib_store = "SDL2_TTF",
			args=['--cflags', '--libs', '--static'])
		conf.check_cfg(package='SDL2_image', uselib_store = "SDL2_IMAGE",
			args=['--cflags', '--libs', '--static'])

	else:
		# check that the SDL libraries are available
		print "[[[ USING DYNAMIC LINKING ]]]"
		conf.check_cfg(package='sdl2', uselib_store = "SDL2",
			args=['--cflags', '--libs',])
		conf.check_cfg(package='SDL2_ttf', uselib_store = "SDL2_TTF",
			args=['--cflags', '--libs'])
		conf.check_cfg(package='SDL2_image', uselib_store = "SDL2_IMAGE", 
			args=['--cflags', '--libs'])


def build(bld):
	if platform == None:
		print "ERROR: Unsupported build platform `%s`!!!" % (sys.platform)
		return
		
	if bld.options.all or not bld.options.examples: 
		platform.build_NEUIK(bld)

	if bld.options.examples or bld.options.all:
		platform.build_examples(bld)

	if bld.options.standalone:
		platform.build_standalone(bld)


