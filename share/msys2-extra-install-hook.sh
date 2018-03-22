#!/bin/sh
#
# Provided to fix issues in dependencies tracking with MSYS2/Windows ldd (ntldd)
# that misses two libraries (libcroco & librsvg). This prevents whole autotools
# system building a consistent/complete hierarchy through its "make install", and,
# thus, may brake the Windows package(s).
#
# Formerly, missing libraries were copied by Makefile.am itself but I (Frédéric)
# chose to propose this independant hook file to clarify goals and method.

# This file is executed by Makefile (install-exec-hook rule) with
# $(prefix) as its only, mandatory argument.
if test "$1"x = ""x; then
	echo "Usage $0: <destination>"
	exit 1
fi

# Detects architecture
ipkgpostfix="w64-x86_64"
if test "$MSYSTEM"x == "MINGW32"x; then
	ipkgpostfix="w64-i686";
fi

libcroco=`pacman -Ql mingw-$ipkgpostfix-libcroco | grep ".dll$" | cut -f2 -d' '`
librsvg=`pacman -Ql mingw-$ipkgpostfix-librsvg | grep "rsvg\-.*\.dll$" | cut -f2 -d' '`

for i in $libcroco $librsvg; do
	cp -v $i $1 || exit 1
done
