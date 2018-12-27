#!/bin/sh

# Source environment file and set up few variables if we
# are using AppVeyor
if test -f /appveyor.environment; then
	source /appveyor.environment
	export MSYSTEM
fi

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
