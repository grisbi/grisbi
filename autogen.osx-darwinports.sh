#!/bin/sh
#
# autogen.sh glue for Grisbi
# Modified 2005/05/22 by G. Niel for use with Mac OS X Darwinports
#
# Requires: automake, autoconf, dpkg-dev

PATH_AUTOMAKE=/opt/local/share/automake
export PATH=/opt/local/bin:$PATH

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
