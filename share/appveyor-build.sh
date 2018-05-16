#!/bin/sh

source /appveyor.environment
export MSYSTEM

cd /c/projects/grisbi-src
./autogen.sh
./configure --prefix /c/projects/grisbi-inst/

v=$(grep PACKAGE_VERSION config.h | cut -f2 -d '"')
v="$v-$(date +'%Y.%m.%d-%H.%M')"
powershell.exe -command "Update-AppveyorBuild -Version \"$v\""
# -B%APPVEYOR_BUILD_NUMBER%\""

echo "Version = $v"

make -j 2

make install

cd /nsis-3.03
./makensis.exe /c/projects/grisbi-src/share/grisbi.nsi

if [ MSYSTEM = "MINGW32" ]; then
  powershell.exe -command "Push-AppveyorArtifact \"C:\projects\grisbi-src\share\Grisbi-32bit-$v-setup.exe\" -DeploymentName \"grisbi-compil\""
else
  powershell.exe -command "Push-AppveyorArtifact \"C:\projects\grisbi-src\share\Grisbi-64bit-$v-setup.exe\" -DeploymentName \"grisbi-compil\""
fi
exit
