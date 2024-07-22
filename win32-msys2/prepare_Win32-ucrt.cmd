@echo off

echo "Mise à jour de Msys2
pacman -Suuu

echo "installation de autoconf automake make"
pacman --noconfirm -S base-devel
pacman --noconfirm -S autoconf
pacman --noconfirm -S automake
pacman --noconfirm -S mingw-w64-ucrt-x86_64-libtool
pacman --noconfirm -S unzip

echo "installation de cairo gcc, gettext-tools, gtk3, libgsf librsvg, libxslt, ntldd pour UCRT system"
pacman --noconfirm -S mingw-w64-ucrt-x86_64-cairo
pacman --noconfirm -S mingw-w64-ucrt-x86_64-gcc
pacman --noconfirm -S mingw-w64-ucrt-x86_64-gettext-tools
pacman --noconfirm -S mingw-w64-ucrt-x86_64-gtk3
pacman --noconfirm -S mingw-w64-ucrt-x86_64-libgsf
pacman --noconfirm -S mingw-w64-ucrt-x86_64-librsvg
pacman --noconfirm -S mingw-w64-ucrt-x86_64-libxslt
pacman --noconfirm -S mingw-w64-ucrt-x86_64-ntldd

echo "installation de git
pacman --noconfirm -S git

echo "creation repertoire dist"
mkdir dist
cd dist
