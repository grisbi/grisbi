#!/bin/sh

source /appveyor.environment
export MSYSTEM

git_src="https://github.com/xfred81"
libofx_version="0.9.13"
libgoffice_version="2018.05.28-16"

fileContent="-----BEGIN RSA PRIVATE KEY-----\n"
for i in `perl -e '$a=$ENV{priv_key}; $a =~ s/ /\n/g; print $a'`; do
fileContent="$fileContent\n$i"
done
fileContent="$fileContent\n-----END RSA PRIVATE KEY-----\n"
mkdir ~/.ssh
echo -e "$fileContent" > ~/.ssh/id_rsa

cd /c/projects
git clone git@github.com:xfred81/grisbi-cert.git
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

bits="32bit"
if [ $MSYSTEM = "MINGW64" ]; then
	bits="64bit"
fi

powershell.exe -command "Push-AppveyorArtifact \"share/Grisbi-$bits-$v-setup.exe\" -DeploymentName \"grisbi-compil\""
