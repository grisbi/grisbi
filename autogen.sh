#!/bin/sh
#
# Requires: automake, autoconf, dpkg-dev

PATH_AUTOMAKE=/usr/share/automake

# test for slackware
if test -x /usr/share/automake-1.7
then
	PATH_AUTOMAKE=/usr/share/automake-1.7
fi

# test for Red-Hat
if test -x /usr/share/automake-1.6
then
	PATH_AUTOMAKE=/usr/share/automake-1.6
fi

# Refresh GNU autotools toolchain.
for i in config.guess config.sub missing install-sh mkinstalldirs ; do
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

exit 0
