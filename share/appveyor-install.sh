#!/bin/sh

source /appveyor.environment
export MSYSTEM

if test "$MSYSTEM"x == "MINGW64"x; then
	i_pkg_postfix="w64-x86_64"
else
	i_pkg_postfix="w64-i686"
fi

pacman -S --noconfirm mingw-$i_pkg_postfix-cairo
pacman -S --noconfirm mingw-$i_pkg_postfix-gtk3
pacman -S --noconfirm mingw-$i_pkg_postfix-freetype
pacman -S --noconfirm mingw-$i_pkg_postfix-libgsf 
pacman -S --noconfirm mingw-$i_pkg_postfix-ntldd-git
pacman -S --noconfirm mingw-$i_pkg_postfix-libxslt
pacman -S --noconfirm autoconf
pacman -S --noconfirm automake
