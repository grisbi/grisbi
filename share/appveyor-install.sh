#!/bin/sh

if test "$MSYSTEM"x == "MINGW64"; then
	i_pkg_postfix = "w64-x86_64"
fi

pacman -Syu --noconfirm
pacman -S --noconfirm mingw-$i_pkg_postfix-cairo
pacman -S --noconfirm mingw-$i_pkg_postfix-gtk3
pacman -S --noconfirm mingw-$i_pkg_postfix-freetype
pacman -S --noconfirm mingw-$i_pkg_postfix-libgsf  
pacman -S unzip --noconfirm
