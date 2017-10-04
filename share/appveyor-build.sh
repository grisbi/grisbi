#!/bin/sh

source /appveyor.environment
export MSYSTEM

cd /c/projects/grisbi-src
./autogen.sh
./configure --prefix /c/projects/grisbi-inst/

v=$(grep PACKAGE_VERSION config.h | cut -f2 -d '"')
powershell.exe -command "Update-AppveyorBuild -Version \"$v\""
# -B%APPVEYOR_BUILD_NUMBER%\""

make -j 2

make install

cd /nsis-3.02.1
./makensis.exe /c/projects/grisbi-src/share/grisbi.nsi
