#!/bin/bash

# Warnings enabled
CFLAGS=""
CFLAGS="-Wall"
CFLAGS+=" -Wextra"

#CFLAGS+=" -Wbad-function-cast"
#CFLAGS+=" -Wcast-align"
#CFLAGS+=" -Wchar-subscripts"
#CFLAGS+=" -Wempty-body"
#CFLAGS+=" -Wformat"
#CFLAGS+=" -Wformat-security"
#CFLAGS+=" -Winit-self"
#CFLAGS+=" -Winline"
#CFLAGS+=" -Wmissing-declarations"
#CFLAGS+=" -Wmissing-include-dirs"
#CFLAGS+=" -Wmissing-prototypes"
#CFLAGS+=" -Wnested-externs"
#CFLAGS+=" -Wold-style-definition"
#CFLAGS+=" -Wpointer-arith"
#CFLAGS+=" -Wredundant-decls"
#CFLAGS+=" -Wshadow"
#CFLAGS+=" -Wstrict-prototypes"
#CFLAGS+=" -Wswitch-enum"
#CFLAGS+=" -Wundef"
#CFLAGS+=" -Wuninitialized"
#CFLAGS+=" -Wunused"
#CFLAGS+=" -Wwrite-strings"

# warnings disabled on purpose
#CFLAGS+=" -Wno-unused-function"
CFLAGS+=" -Wno-deprecated-declarations"

export CFLAGS
echo "CFLAGS: $CFLAGS"

configure_args=""

# fail on warning
configure_args+=" --enable-werror"

if [ "$TRAVIS_OS_NAME" = "osx" ]
then
	# from brew
	export PKG_CONFIG_PATH=/usr/local/opt/libxml2/lib/pkgconfig
	export PATH="$PATH:/usr/local/opt/gettext/bin"

	mkdir m4
	ln -sf /usr/local/opt/gettext/share/aclocal/nls.m4 m4

	# disable OpenSSL on macOS since it is no more provided by Apple
	configure_args+=" --without-openssl"
fi

sh -x ./autogen.sh

echo "configure_args: $configure_args $@"
./configure $configure_args "$@"
