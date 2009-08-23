@echo off
wget http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.16/gtk+-bundle_2.16.5-20090731_win32.zip
unzip gtk+-bundle_2.16.5-20090731_win32.zip -d gtk+
cmake -G "Visual Studio 9 2008" .