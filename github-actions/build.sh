#!/bin/bash

# env

# Warnings enabled
CFLAGS=""
CFLAGS="-Wall"
CFLAGS+=" -Wextra"
CFLAGS+=" -std=c11"

#CFLAGS+=" -Wbad-function-cast"
CFLAGS+=" -Wcast-align"
CFLAGS+=" -Wchar-subscripts"
CFLAGS+=" -Wempty-body"
CFLAGS+=" -Wformat"
CFLAGS+=" -Wformat-security"
CFLAGS+=" -Winit-self"
CFLAGS+=" -Winline"
CFLAGS+=" -Wmissing-declarations"
#CFLAGS+=" -Wmissing-include-dirs"
CFLAGS+=" -Wmissing-prototypes"
CFLAGS+=" -Wnested-externs"
CFLAGS+=" -Wold-style-definition"
CFLAGS+=" -Wpointer-arith"
CFLAGS+=" -Wredundant-decls"
#CFLAGS+=" -Wshadow"
CFLAGS+=" -Wstrict-prototypes"
CFLAGS+=" -Wswitch-enum"
CFLAGS+=" -Wundef"
CFLAGS+=" -Wuninitialized"
CFLAGS+=" -Wunused"
CFLAGS+=" -Wwrite-strings"
#CFLAGS+=" -Wdeclaration-after-statement"
CFLAGS+=" -Wmissing-noreturn"
#CFLAGS+=" -Wmissing-format-attribute"

# warnings disabled on purpose
CFLAGS+=" -Wno-unused-parameter"

configure_args=""

# fail on warning
configure_args+=" --enable-real-werror"

if [ "$RUNNER_OS" = "macOS" ]
then
	# disable deprecated warnings since gdk-pixbuf fails to
	# build with:
	# /usr/local/Cellar/gdk-pixbuf/2.38.1_1/include/gdk-pixbuf-2.0/gdk-pixbuf/gdk-pixbuf-animation.h:122:85: warning: 'GTimeVal' is deprecated: Use 'GDateTime' instead [-Wdeprecated-declarations]
	CFLAGS+=" -Wno-deprecated-declarations"
else
	# /usr/include/libgsf-1/gsf/gsf-utils.h:303:9: error: 'GParameter' is deprecated [-Werror=deprecated-declarations]
	CFLAGS+=" -Wno-deprecated-declarations"
fi

# enable goffice
configure_args+=" --with-goffice"

export CFLAGS
echo "CFLAGS: $CFLAGS"

if [ "$RUNNER_OS" = "macOS" ]
then
	# from brew
	export PKG_CONFIG_PATH=/usr/local/opt/libxml2/lib/pkgconfig:/usr/local/opt/openssl/lib/pkgconfig:/usr/local/opt/libffi/lib/pkgconfig
	export PATH="$PATH:/usr/local/opt/gettext/bin"

	mkdir m4
	ln -sf /usr/local/opt/gettext/share/aclocal/nls.m4 m4
fi

# exit on error
set -e

echo "configure_args: $configure_args $@"
./configure $configure_args "$@"
