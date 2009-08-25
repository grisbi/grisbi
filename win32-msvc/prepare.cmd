@echo off
mkdir target
wget http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.16/gtk+-bundle_2.16.5-20090731_win32.zip -O target\gtk-dev.zip
unzip target\gtk-dev.zip -d target\gtk-dev
cmake -G "Visual Studio 9 2008" .