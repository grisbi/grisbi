#!/bin/sh
#
# autogen.sh glue for CMU Cyrus IMAP
# $Id: autogen.sh,v 1.1.2.3 2004/04/12 16:38:01 gegeweb Exp $
#
# Requires: automake, autoconf, dpkg-dev
set -e

PATH_AUTOMAKE=/usr/share/automake

# test for slackware
if test -x /usr/share/automake-1.7
then
	PATH_AUTOMAKE=/usr/share/automake-1.7
fi

# Refresh GNU autotools toolchain.
for i in config.guess config.sub missing install-sh mkinstalldirs ; do
	test -r $PATH_AUTOMAKE/${i} && {
		rm -f ${i}
		cp $PATH_AUTOMAKE/${i} .
	}
	chmod 755 ${i}
done

aclocal -I macros
autoheader
automake --verbose --foreign --add-missing
autoconf

exit 0
