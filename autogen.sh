#!/bin/sh
#
# autogen.sh glue for Grisbi
# $Id: autogen.sh,v 1.1.2.10 2005/01/07 19:24:18 benj2 Exp $
#
# Requires: automake, autoconf, dpkg-dev

PATH_AUTOMAKE=/usr/share/automake

# test for some distribution...
# Y'a pas plus simple ?

if test -x /usr/share/automake-1.8
then
	PATH_AUTOMAKE=/usr/share/automake-1.8
fi

if test -x /usr/share/automake-1.7
then
	PATH_AUTOMAKE=/usr/share/automake-1.7
fi

if test -x /usr/share/automake-1.6
then
	PATH_AUTOMAKE=/usr/share/automake-1.6
fi

if test -x /usr/share/automake-1.4
then
	PATH_AUTOMAKE=/usr/share/automake-1.4
fi

# Refresh GNU autotools toolchain.
for i in config.guess config.sub missing install-sh mkinstalldirs depcomp ; do
	test -r $PATH_AUTOMAKE/${i} && {
		rm -f ${i}
		cp $PATH_AUTOMAKE/${i} .
	}
	if test -r ${i} ; then
	    chmod 755 ${i}
	fi
done

aclocal -I macros
autoheader
automake --verbose --foreign --add-missing
autoconf

rm -rf autom4te.cache
chmod +x debian/rules

exit 0
