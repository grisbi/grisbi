#!/bin/sh
#
# autogen.sh glue for CMU Cyrus IMAP
# $Id: autogen.slack.sh,v 1.1.2.1 2004/04/11 19:49:58 gegeweb Exp $
#
# Requires: automake, autoconf, dpkg-dev
set -e

# Refresh GNU autotools toolchain.
for i in config.guess config.sub missing install-sh mkinstalldirs ; do
	test -r /usr/share/automake-1.7/${i} && {
		rm -f ${i}
		cp /usr/share/automake-1.7/${i} .
	}
	chmod 755 ${i}
done

aclocal -I macros
autoheader
automake --verbose --foreign --add-missing
autoconf

exit 0
