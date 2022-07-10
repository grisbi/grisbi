#!/bin/bash

# env

# Warnings enabled
CFLAGS=""
CFLAGS="-Wall"
CFLAGS+=" -Wextra"
CFLAGS+=" -std=c11"

CFLAGS+=" -Wcast-align"
CFLAGS+=" -Wchar-subscripts"
CFLAGS+=" -Wempty-body"
CFLAGS+=" -Wformat=2"
CFLAGS+=" -Wformat-security"
CFLAGS+=" -Winit-self"
CFLAGS+=" -Winline"
CFLAGS+=" -Wmissing-declarations"
CFLAGS+=" -Wmissing-prototypes"
CFLAGS+=" -Wnested-externs"
CFLAGS+=" -Wold-style-definition"
CFLAGS+=" -Wpointer-arith"
CFLAGS+=" -Wredundant-decls"
CFLAGS+=" -Wshadow"
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

# etats_page_amount.c: In function ‘etats_page_amount_retire_ligne_liste_comparaisons’:
# /usr/lib/x86_64-linux-gnu/glib-2.0/include/glibconfig.h:98:37: error: cast from function call of type ‘gpointer’ {aka ‘void *’} to non-matching type ‘long int’ [-Werror=bad-function-cast]
#    98 | #define GPOINTER_TO_INT(p) ((gint)  (glong) (p))
#       |                                     ^
# etats_page_amount.c:181:30: note: in expansion of macro ‘GPOINTER_TO_INT’
#   181 |   amount_comparison_number = GPOINTER_TO_INT (g_slist_nth_data (gsb_data_report_get_amount_comparison_list
#       |                              ^~~~~~~~~~~~~~~
# /usr/lib/x86_64-linux-gnu/glib-2.0/include/glibconfig.h:98:37: error: cast from function call of type ‘gpointer’ {aka ‘void *’} to non-matching type ‘long int’ [-Werror=bad-function-cast]
#    98 | #define GPOINTER_TO_INT(p) ((gint)  (glong) (p))
#       |                                     ^
# etats_page_amount.c:199:30: note: in expansion of macro ‘GPOINTER_TO_INT’
#   199 |   amount_comparison_number = GPOINTER_TO_INT (g_slist_nth_data (gsb_data_report_get_amount_comparison_list
#       |                              ^~~~~~~~~~~~~~~
#CFLAGS+=" -Wbad-function-cast"

# cc1: error: -pthread: No such file or directory [-Werror=missing-include-dirs]
#CFLAGS+=" -Wmissing-include-dirs"

CFLAGS+=" -O2"
CFLAGS+=" -D_FORTIFY_SOURCE=2"

configure_args=""

# fail on warning
configure_args+=" --enable-real-werror"

if [ "$RUNNER_OS" = "macOS" ]
then
	# do nothing
	echo
else
	if [ $(lsb_release -rs) = "20.04" ]
	then
		# disable on Ubuntu 20.04
		# /usr/include/libgsf-1/gsf/gsf-utils.h:303:9: error: 'GParameter' is deprecated [-Werror=deprecated-declarations]
		CFLAGS+=" -Wno-deprecated-declarations"
	fi

	# runs the standard link-time optimizer
	CFLAGS+=" -flto=auto"
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
