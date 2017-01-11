#!/usr/bin/env bash

BuildCLEAN=
BuildDBG=
BuildEXAMPLES=
BuildLIB=1
BuildSTATIC=
BuildOPTS=
BuildINSTALL=

function showHelp {
	echo "usage: build_osx    (Builds NEUIK library by default)"
	echo "      -h, --help      :  Print usage info."
	echo "      -v, --verbose   :  Use verbose build option."
	echo "          --all       :  Build library and examples."
	echo "          --clean     :  Clean the build directory."
	echo "          --debug     :  Build targets with debug options."
	echo "          --examples  :  Build only NEUIK examples."
	echo "          --stlib     :  Build NEUIK as a static library."
	echo "          --static    :  Statically link examples."
	echo "          --install   :  Install compiled libraries."
}

#------------------------------------------------------------------------------#
# Parse command line options                                                   #
#------------------------------------------------------------------------------#
while [[ $1 == -* ]]; do
	case "$1" in
		-h|--help) 
			showHelp
			exit
			;;
		-v|--verbose) 
			shift
			BuildOPTS="$BuildOPTS -v"
			;;
		--all)
			shift
			BuildEXAMPLES=1
			;;
		--clean)
			shift
			BuildCLEAN=1
			;;
		--install)
			shift
			BuildINSTALL=1
			;;
		--debug)
			shift
			BuildDBG=1
			BuildOPTS="$BuildOPTS --debug"
			;;
		--examples)  
			shift
			echo "building examples" 
			BuildLIB=
			BuildEXAMPLES=1
			;;
		--stlib)
			shift
			BuildOPTS="$BuildOPTS --stlib"
			;;
		--static)
			shift
			BuildSTATIC=1
			BuildOPTS="$BuildOPTS --stlib --static"
			;;
		--) shift; break;;
		-*) echo "invalid option $1" 1>&2
			showHelp
			exit
			;; 
	esac
done

#------------------------------------------------------------------------------#
# Run waf configure before trying to build anything                            #
#------------------------------------------------------------------------------#
echo "-------------------------------------------------------------------------------"
echo "|  Configuring Build  :  Running \`./waf configure\`                            |"
echo "-------------------------------------------------------------------------------"
if [ "$BuildSTATIC" ] 
then
	./scripts/waf configure --static
else
	./scripts/waf configure
fi

#------------------------------------------------------------------------------#
# Build the NEUIK library                                                      #
#------------------------------------------------------------------------------#
if [ "$BuildCLEAN" ] 
then
	echo "-------------------------------------------------------------------------------"
	echo "|  Cleaning the build directory  :  Running \`./waf clean\`                     |"
	echo "-------------------------------------------------------------------------------"
	./scripts/waf clean
fi

#------------------------------------------------------------------------------#
# Build the NEUIK library                                                      #
#------------------------------------------------------------------------------#
if [ "$BuildLIB" ] 
then
	echo "-------------------------------------------------------------------------------"
	echo "|  Building the NEUIK Library  :  Running \`./waf\`                             |"
	echo "-------------------------------------------------------------------------------"
	./scripts/waf $BuildOPTS
fi

#------------------------------------------------------------------------------#
# Build the NEUIK examples                                                     #
#------------------------------------------------------------------------------#
if [ "$BuildEXAMPLES" ] 
then
	echo "-------------------------------------------------------------------------------"
	echo "|  Building the NEUIK examples  :  Running \`./waf --examples\`                 |"
	echo "-------------------------------------------------------------------------------"
	./scripts/waf --examples $BuildOPTS
else
	echo "-------------------------------------------------------------------------------"
	echo "|  Examples were NOT Built.  |  Call \`./build_osx.sh --examples\`              |"
	echo "|                            |  to build the NEUIK examples.                  |"
	echo "-------------------------------------------------------------------------------"
fi

#------------------------------------------------------------------------------#
# Install the NEUIK library                                                    #
#------------------------------------------------------------------------------#
if [ "$BuildINSTALL" ] 
then
	echo "-------------------------------------------------------------------------------"
	echo "|  Installing the NEUIK Library  :  Running \`sudo ./waf install\`              |"
	echo "-------------------------------------------------------------------------------"
	sudo ./scripts/waf install
fi
