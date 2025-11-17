@echo off

echo "Mise à jour de Msys2"
pacman -Suuu

echo "installation de autoconf automake make"
pacman --noconfirm -S base-devel
pacman --noconfirm -S autoconf
pacman --noconfirm -S automake
pacboy --noconfirm -S libtool:p
pacman --noconfirm -S unzip

echo "installation de cairo gcc, gettext-tools, gtk3, libgsf librsvg, libxslt, ntldd pour UCRT system"
pacboy --noconfirm -S cairo:p
pacboy --noconfirm -S gcc:p
pacboy --noconfirm -S gettext-tools:p
pacboy --noconfirm -S gtk3:p
pacboy --noconfirm -S libgsf:p
pacboy --noconfirm -S librsvg:p
pacboy --noconfirm -S libxslt:p
pacboy --noconfirm -S ntldd:p

echo "installation de git"
pacman --noconfirm -S git

echo "creation repertoire dist"
mkdir dist
cd dist
