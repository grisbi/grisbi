#!/bin/sh

# enable log
set -v

source /appveyor.environment
export MSYSTEM

libofx_version="0.9.15"
libgoffice_version="2019.10.16-19"

bits="32bit"
if [ $MSYSTEM = "MINGW64" ]; then
	bits="64bit"
fi

git_src="https://github.com/xfred81"

pwd
cd /
wget -m --no-verbose -O /libofx.zip "$git_src/libofx/releases/download/$libofx_version/libofx_$MSYSTEM.zip"
unzip /libofx.zip

git_src="https://github.com/xfred81"

wget -m --no-verbose -O /goffice.zip "$git_src/goffice/releases/download/v-$libgoffice_version/goffice-$bits-$libgoffice_version-archive.zip"
unzip /goffice.zip

PATH=$PATH:/tmp/inst/bin
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

PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/tmp/inst/lib/pkgconfig
export PKG_CONFIG_PATH

configure_args=""
configure_args+="--prefix /c/projects/grisbi-inst/ "
configure_args+="--with-ofx "
configure_args+="--with-goffice "
configure_args+="--enable-config-file "
echo "configure_args: $configure_args $@"
./configure $configure_args "$@"

v=$(grep PACKAGE_VERSION config.h | cut -f2 -d '"')
minor=$(echo $v|cut -f2 -d.)
unstable=$((minor % 2))
if [ $unstable = "1" ]
then
	# append the date for unstable versions
	v="$v-$(date +'%Y.%m.%d-%H')"
fi
powershell.exe -command "Update-AppveyorBuild -Version \"$v\""
# -B%APPVEYOR_BUILD_NUMBER%\""
echo "Version = $v"

make -j 2

make install

cd /nsis-3.04
./makensis.exe /c/projects/grisbi-src/share/grisbi.nsi

cd /c/projects/grisbi-src

powershell.exe -command "Push-AppveyorArtifact \"share/Grisbi-$bits-$v-setup.exe\" -DeploymentName \"grisbi-compil\""
