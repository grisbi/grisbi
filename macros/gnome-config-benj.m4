
dnl GNOME_CHECK_MODULES(GSTUFF, gtk+-2.0 >= 1.3 glib = 1.3.4, action-if, action-not)
dnl defines GSTUFF_LIBS, GSTUFF_CFLAGS, see gnome-config man page
dnl also defines GSTUFF_GNOME_ERRORS on error
AC_DEFUN(GNOME_CHECK_MODULES, [
  succeeded=yes

  if test -z "$GNOME_CONFIG"; then
    AC_PATH_PROG(GNOME_CONFIG, gnome-config, no)
  fi

  if test "$GNOME_CONFIG" = "no" ; then
     echo "*** The gnome-config script could not be found. Make sure it is"
     echo "*** in your path, or set the GNOME_CONFIG environment variable"
     echo "*** to the full path to gnome-config."
     echo "*** Or see http://www.freedesktop.org/software/gnomeconfig to get gnome-config."
     succeeded=no
  else
      AC_MSG_CHECKING(for $2)

      AC_MSG_CHECKING($1_LIBS)
      AC_MSG_CHECKING($1_CFLAGS)
      for i in $2
	do
	if $GNOME_CONFIG --cflags "$i" ; then
	    $1_TMPCFLAGS=`$GNOME_CONFIG --cflags "$i"`
	    $1_CFLAGS="$1_CFLAGS $1_TMPCFLAGS"
	    
	    $1_TMPLIBS=`$GNOME_CONFIG --libs "$i"`
	    $1_LIBS="$1_LIBS $1_TMPLIBS"
	else
	    $1_CFLAGS=""
	    $1_LIBS=""
	    succeeded=no
	    ifelse([$4], ,echo $$1_GNOME_ERRORS,)
	fi
      done
      
      AC_SUBST($1_CFLAGS)
      AC_SUBST($1_LIBS)
  fi

  if test $succeeded = yes; then
     ifelse([$3], , :, [$3])
  else
     ifelse([$4], , AC_MSG_ERROR([Library requirements ($2) not met; consider adjusting the GNOME_CONFIG_PATH environment variable if your libraries are in a nonstandard prefix so gnome-config can find them.]), [$4])
  fi
])


