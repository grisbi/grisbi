#!/bin/sh
#
# autogen.sh glue for CMU Cyrus IMAP
# $Id: autogen.sh,v 1.1.2.1 2004/04/11 16:23:15 benj2 Exp $
#
# Requires: automake, autoconf, dpkg-dev
set -e

# Refresh GNU autotools toolchain.
for i in config.guess config.sub missing install-sh mkinstalldirs ; do
	test -r /usr/share/automake/${i} && {
		rm -f ${i}
		cp /usr/share/automake/${i} .
	}
	chmod 755 ${i}
done

aclocal -I macros
autoheader
#automake --verbose --foreign --add-missing
autoconf

exit 0
