export PKG_CONFIG_PATH=/usr/local/Cellar/libxml2/2.9.4/lib/pkgconfig
export PATH="$PATH:/usr/local/Cellar/gettext/0.19.8.1/bin"

#CFLAGS="$CFLAGS -Wno-unused-parameter"
CFLAGS="-Wno-deprecated-declarations -Wno-unused-parameter"

./configure --with-ofx --without-openssl --enable-maintainer-mode
