#!/bin/sh
#
# autogen.sh glue for Grisbi
# $Id: autogen.sh,v 1.1.2.12 2005/06/02 19:59:36 gegeweb Exp $
#
# Requires: automake, autoconf, dpkg-dev

if test -z "${PATH_AUTOMAKE}"
then
	PATH_AUTOMAKE=`ls -1d /usr/share/automake* 2>/dev/null | sort -gbu | tail -1`
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

if test -x /opt/local/share/aclocal
then
	aclocal -I /opt/local/share/aclocal -I macros
else
	aclocal -I macros
fi
autoheader
automake --verbose --foreign --add-missing --copy
autoconf

rm -rf autom4te.cache
chmod +x debian/rules

exit 0
