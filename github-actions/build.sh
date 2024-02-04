#!/bin/bash

# env

# exit on error
set -e

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

configure_args="$@"

# fail on warning
configure_args+=" --enable-real-werror"

echo "RUNNER_OS: $RUNNER_OS"
case "$RUNNER_OS" in
	"macOS")
		echo "macOS"

		# from brew
		export PKG_CONFIG_PATH=/usr/local/opt/libxml2/lib/pkgconfig:/usr/local/opt/openssl/lib/pkgconfig:/usr/local/opt/libffi/lib/pkgconfig
		export PATH="$PATH:/usr/local/opt/gettext/bin"
		;;

	"Windows")
		echo "Windows"

		# ignore undefined macro
# 		D:/a/_temp/msys64/tmp/inst/include/libgoffice-0.10/goffice/math/go-math.h:116:5: error: "_MSC_VER" is not defined, evaluates to 0 [-Werror=undef]
#   116 | #if _MSC_VER
#       |     ^~~~~~~~
# D:/a/_temp/msys64/tmp/inst/include/libgoffice-0.10/goffice/math/go-math.h:120:7: error: "GOFFICE_WITH_WINREG" is not defined, evaluates to 0 [-Werror=undef]
#   120 |  #if (GOFFICE_WITH_WINREG==1)
#       |       ^~~~~~~~~~~~~~~~~~~
		CFLAGS+=" -Wno-undef"

#  CCLD     grisbi.exe
#./.libs/lt-grisbi.c: In function 'lt_fatal':
#./.libs/lt-grisbi.c:615:1: error: function might be candidate for attribute 'noreturn' [-Werror=suggest-attribute=noreturn]
#  615 | lt_fatal (const char *file, int line, const char *message, ...)
#      | ^~~~~~~~
		CFLAGS+=" -Wno-suggest-attribute=noreturn"
		;;

	"Linux")
		echo "Linux"

		# runs the standard link-time optimizer
		CFLAGS+=" -flto=auto"
		;;
esac

export CFLAGS
echo "CFLAGS: $CFLAGS"

echo "configure_args: $configure_args"
./configure $configure_args
