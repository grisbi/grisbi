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

# Warnings enabled
CFLAGS=""
CFLAGS="-Wall"
CFLAGS+=" -Wextra"

#CFLAGS+=" -Wbad-function-cast"
#CFLAGS+=" -Wcast-align"
CFLAGS+=" -Wchar-subscripts"
CFLAGS+=" -Wempty-body"
CFLAGS+=" -Wformat"
CFLAGS+=" -Wformat-security"
CFLAGS+=" -Winit-self"
CFLAGS+=" -Winline"
CFLAGS+=" -Wmissing-declarations"
CFLAGS+=" -Wmissing-include-dirs"
CFLAGS+=" -Wmissing-prototypes"
CFLAGS+=" -Wnested-externs"
CFLAGS+=" -Wold-style-definition"
CFLAGS+=" -Wpointer-arith"
CFLAGS+=" -Wredundant-decls"
#CFLAGS+=" -Wshadow"
CFLAGS+=" -Wstrict-prototypes"
CFLAGS+=" -Wswitch-enum"
CFLAGS+=" -Wundef"
CFLAGS+=" -Wuninitialized"
CFLAGS+=" -Wunused"
#CFLAGS+=" -Wwrite-strings"

# warnings disabled on purpose
CFLAGS+=" -Wno-unused-parameter"

export CFLAGS
echo "CFLAGS: $CFLAGS"

PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/inst/lib/pkgconfig
export PKG_CONFIG_PATH

./configure \
	--prefix /c/projects/grisbi-inst/ \
	--with-ofx \
	--with-goffice

v=$(grep PACKAGE_VERSION config.h | cut -f2 -d '"')
v="$v-$(git rev-parse --short HEAD)"
powershell.exe -command "Update-AppveyorBuild -Version \"$v\""
# -B%APPVEYOR_BUILD_NUMBER%\""
echo "Version = $v"

make -j 2

make install

cd /nsis-3.03
./makensis.exe /c/projects/grisbi-src/share/grisbi.nsi

cd /c/projects/grisbi-src

bits="32bit"
if [ $MSYSTEM = "MINGW64" ]; then
	bits="64bit"
fi

powershell.exe -command "Push-AppveyorArtifact \"share/Grisbi-$bits-$v-setup.exe\" -DeploymentName \"grisbi-compil\""
