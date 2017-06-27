@echo off

echo "Mise à jour de Msys2
pacman -Suuu

echo "installation de git, base-devel, gcc, glib2, gtk3, libgsf pour architecture 32 bits"
pacman -S git
pacman -S base-devel
pacman -S mingw32/mingw-w64-i686-gcc
pacman -S mingw32/mingw-w64-i686-glib2
pacman -S msys/glib2-devel
pacman -S mingw32/mingw-w64-i686-gtk3
pacman -S mingw32/mingw-w64-i686-libgsf

echo "installation devhelp"
pacman -S mingw64/mingw-w64-x86_64-devhelp

echo "Création du répertoire de download"
IF NOT EXIST downloads MKDIR downloads
SET DOWNLOADS_DIR=%CURRENT_DIR%\downloads

echo "installer libofx"
cd %DOWNLOADS_DIR%
chcp 1252 && wget -nc https://sourceforge.net/projects/libofx/files/latest/download?source=files

echo "Instaler et compiler Grisbi"
git clone https://github.com/pierre-biava/grisbi.git
cd grisbi
echo "compiler Grisbi"
./autogen.sh
./configure
make -j 15
make install


