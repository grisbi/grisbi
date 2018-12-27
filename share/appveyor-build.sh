#!/bin/sh

git_src="https://github.com/xfred81"
libofx_version="0.9.13"
libgoffice_version="2018.12.26"

compdir="/tmp"

if test -f /appveyor.environment; then
	grisbisrcdir=/c/projects/grisbi-src
else
	grisbisrcdir=`pwd`
fi

if test -f /appveyor.environment; then
	source /appveyor.environment
	export MSYSTEM	
fi

cd $compdir
if test ! -f $compdir/libofx.zip; then
	wget -m --no-verbose -O $compdir/libofx.zip "$git_src/libofx/releases/download/0.9.13/libofx_$MSYSTEM.zip"
	# libofx archive contains /tmp/inst/; we do want to get in / first
	# otherwise /tmp/inst will be extracted within /tmp ($compdir)
	cd /
	unzip $compdir/libofx.zip
	cd $compdir
fi

if test ! -f $compdir/goffice.zip; then
	wget -m --no-verbose -O $compdir/goffice.zip "$git_src/goffice/releases/download/v-$libgoffice_version/goffice-$MSYSTEM-$libgoffice_version-archive.zip"
	unzip $compdir/goffice.zip
fi

PATH=$PATH:$compdir/inst/bin
export PATH

cd $grisbisrcdir

./autogen.sh

./configure --prefix $compdir/grisbi-inst/ --with-ofx --with-goffice \
   PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$compdir/inst/lib/pkgconfig
   # \
   # CFLAGS="-I$compdir/inst/include -I$compdir/inst/include/libgoffice-0.10" \
   # LDFLAGS="-L/$compdir/inst/lib -L$compdir/inst"

if test -f /appveyor.environment; then
	v=$(grep PACKAGE_VERSION config.h | cut -f2 -d '"')
	v="$v-$(date +'%Y.%m.%d-%H')"
	powershell.exe -command "Update-AppveyorBuild -Version \"$v\""
	
	bits="32bit"
	if [ $MSYSTEM = "MINGW64" ]; then
		bits="64bit"
	fi
fi

make -j 2

make install

if test -f /appveyor.environment; then
	cd $compdir/nsis-3.03
	./makensis.exe $grisbisrcdir/share/grisbi.nsi

	cd $grisbisrcdir

	powershell.exe -command "Push-AppveyorArtifact \"share/Grisbi-$bits-$v-setup.exe\" -DeploymentName \"grisbi-compil\""
fi
