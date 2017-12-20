#!/bin/sh

source /appveyor.environment
export MSYSTEM
export BUILD_NUMBER

cd /c/projects/grisbi-src
./autogen.sh
./configure --prefix /c/projects/grisbi-inst/

v=$(grep PACKAGE_VERSION config.h | cut -f2 -d '"')
v="$v Build-$BUILD_NUMBER" 
powershell.exe -command "Update-AppveyorBuild -Version \"$v\""

make -j 2

make install

cd /nsis-3.02.1
./makensis.exe /c/projects/grisbi-src/share/grisbi.nsi
