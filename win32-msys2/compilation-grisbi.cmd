cd dist

echo "recupere Grisbi"
git clone https://github.com/pierre-biava/grisbi.git

echo "recupere libofx"
wget -nc https://github.com/xfred81/libofx/releases/download/0.9.15/libofx_MINGW64.zip
unzip libofx_MINGW64.zip -d /

echo "recupere liboffice"
wget -nc https://github.com/xfred81/goffice/releases/download/v-2019.10.16-19/goffice-64bit-2019.10.16-19-archive.zip
unzip goffice-64bit-2019.10.16-19-archive.zip -d /

echo "recupere nsys"
wget -O /nsis.zip 'https://downloads.sourceforge.net/project/nsis/NSIS 3/3.10/nsis-3.10.zip'
unzip /nsis.zip

rem exporte librairies pour lib ofx et goffice
echo "export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/tmp/inst/lib/pkgconfig"
echo "echo "$PKG_CONFIG_PATH"


cd grisbi

