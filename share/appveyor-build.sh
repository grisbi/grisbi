#!/bin/sh

source /appveyor.environment
export MSYSTEM

libofx_version="0.9.13"

wget -O /libofx.zip "https://github.com/xfred81/libofx/releases/download/0.9.13/libofx_$MSYSTEM.zip"
cd /
unzip libofx.zip

PATH=$PATH:/inst/bin
export PATH

cd /c/projects/grisbi-src
./autogen.sh

./configure --prefix /c/projects/grisbi-inst/ --with-ofx PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/inst/lib/pkgconfig

v=$(grep PACKAGE_VERSION config.h | cut -f2 -d '"')
v="$v-$(date +'%Y.%m.%d')"
powershell.exe -command "Update-AppveyorBuild -Version \"$v\""
# -B%APPVEYOR_BUILD_NUMBER%\""

make -j 2

make install

cd /nsis-3.03
./makensis.exe /c/projects/grisbi-src/share/grisbi.nsi
