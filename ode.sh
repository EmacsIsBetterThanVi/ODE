#!/bin/bash
function ode(){
if [[ $1 == "-v" || $1 == "--version" ]]; then
echo "ODE Binary 0.85.625"
echo "INTERFACE VERSIONS"
echo "VARIABLE INTERFACE 1.0.0"
echo "PEN INTERFACE 0.0.0"
echo "STACK INTERFACE 1.0.0"
echo "DIRECTORY INTERFACE 1.0.0"
echo "FILE INTERFACE 1.0.0"
echo "THREAD INTERFACE 1.0.0"
echo "ODE INTERFACE 0.9.0"
echo "SYNTAX VERSION 0.9.5"
echo "CLASS VERSIONS"
cat $ODE_DIR/ode/.versions
elif [[ $1 == "-s" || $1 == "--startup" ]]; then
$ODE_DIR/ode.bin $ODE_DIR ode/startup
echo ""
elif [[ $1 == "-c" || $1 == "--command" ]]; then
$ODE_DIR/ode.bin $ODE_DIR "--command" "$2" ${@: 3}
echo "\n$?"
elif [[ $1 == "-h" || $1 == "--help" ]]; then
echo "ODE HELP"
echo "Usage: ode [options] [file|command]"
echo "Options:"
echo "-v, --version         prints the Ode version data and exits"
echo "-s, --startup         runs the preinstalled system for working with Ode"
echo "-h, --help            displays this help message"
echo "-c, --command         executs a command without invoking the repl or the Ode system"
echo "-b, --build           converts Ode to another programing language"
echo "-bh, --build-help     displays help for the --build option"
elif [[ $1 == "-bh" || $1 == "--build-help" ]]; then
echo "ODE BUILD HELP"
echo "Usage: ode --build language [-n, --no-compile] [-k, --keep] [-v, --verbose] file [options]"
echo "Note: do not include file extention the system will construct file extensions"
echo "Files will be automaticaly named"
echo "Options:"
echo "-k, --keep            keeps translated files"
echo "-n, --no-compile      tells the ode build system not to compile the resulting file"
echo "-v, --verbose         outputs verbose data during compilation"
echo "See the intended compiler for it's command line options"
echo "Languages:"
ls $ODE_DIR/ode/Build
elif [[ $1 == "-b" || $1 == "--build" ]]; then
$ODE_DIR/ode/Build/$2 ${@: 3}
else
if [[ $1 == "" ]]; then
tmp=0
while [[ $tmp != 255 ]]; do
"$ODE_DIR"ode.bin $ODE_DIR
tmp=$?
if [[ $tmp != 255 ]]; then
echo -e "Restarting(Press ^C to Quit)"
fi
sleep 1
done
else
"$ODE_DIR"/ode.bin $ODE_DIR $1 ${@: 2}
echo "\n$?"
fi
fi
}
function ode_config(){
    bash config
    ODE_DIR=$PWD
}