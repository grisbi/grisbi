#!/bin/sh

source /appveyor.environment
export MSYSTEM

git_src="https://github.com/xfred81"
libofx_version="0.9.13"
libgoffice_version="2018.05.28-16"

cd /
wget -m --no-verbose -O /libofx.zip "$git_src/libofx/releases/download/0.9.13/libofx_$MSYSTEM.zip"
unzip libofx.zip

wget -m --no-verbose -O /goffice.zip "$git_src/goffice/releases/download/v-2018.05.28-16/goffice-$MSYSTEM-$libgoffice_version-archive.zip"
unzip /goffice.zip

PATH=$PATH:/inst/bin
export PATH

cd /c/projects/grisbi-src
./autogen.sh

./configure --prefix /c/projects/grisbi-inst/ --with-ofx --with-goffice PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/inst/lib/pkgconfig

v=$(grep PACKAGE_VERSION config.h | cut -f2 -d '"')
v="$v-$(date +'%Y.%m.%d-%H')"
powershell.exe -command "Update-AppveyorBuild -Version \"$v\""
# -B%APPVEYOR_BUILD_NUMBER%\""
echo "Version = $v"

make -j 2

make install

cd /nsis-3.03
./makensis.exe /c/projects/grisbi-src/share/grisbi.nsi

cd /c/projects/grisbi-src

if [ $MSYSTEM = "MINGW32" ]; then
  powershell.exe -command "Push-AppveyorArtifact \"share/Grisbi-32bit-$v-setup.exe\" -DeploymentName \"grisbi-compil\""
else
  powershell.exe -command "Push-AppveyorArtifact \"share/Grisbi-64bit-$v-setup.exe\" -DeploymentName \"grisbi-compil\""
fi
exit
