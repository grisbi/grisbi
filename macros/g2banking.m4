# $Id: g2banking.m4,v 1.1 2004/12/29 19:40:26 benj2 Exp $
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for G2Banking

AC_DEFUN(AC_G2BANKING, [
dnl searches for g2banking
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: g2banking_dir
dnl          g2banking_libs
dnl          g2banking_data
dnl          g2banking_includes
dnl          have_g2banking

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if g2banking support desired)
AC_ARG_ENABLE(g2banking,
  [  --enable-g2banking       enable g2banking support (default=detect)],
  enable_g2banking="$enableval",
  enable_g2banking="yes")
AC_MSG_RESULT($enable_g2banking)

have_g2banking="no"
g2banking_dir=""
g2banking_data=""
g2banking_libs=""
g2banking_includes=""
if test "$enable_g2banking" != "no"; then
  AC_MSG_CHECKING(for g2banking)
  AC_ARG_WITH(g2banking-dir, [  --with-g2banking-dir=DIR
                          uses g2banking from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/g2banking-config"; then
          g2banking_dir="$li";
          break
      fi
  done
  if test -z "$g2banking_dir"; then
      AC_MSG_RESULT([not found ])
      have_g2banking="no"
  else
      AC_MSG_RESULT($g2banking_dir)
      AC_MSG_CHECKING(for g2banking libs)
      g2banking_libs="`$g2banking_dir/bin/g2banking-config --libraries`"
      AC_MSG_RESULT($g2banking_libs)
      AC_MSG_CHECKING(for g2banking includes)
      g2banking_includes="`$g2banking_dir/bin/g2banking-config --includes`"
      AC_MSG_RESULT($g2banking_includes)
      AC_MSG_CHECKING(for g2banking data)
      g2banking_data="`$g2banking_dir/bin/g2banking-config --data`"
      AC_MSG_RESULT($g2banking_data)

      AC_MSG_CHECKING(if g2banking test desired)
      AC_ARG_ENABLE(g2banking,
        [  --enable-g2banking-test  enable g2banking-test (default=yes)],
         enable_g2banking_test="$enableval",
         enable_g2banking_test="yes")
      AC_MSG_RESULT($enable_g2banking_test)
      AC_MSG_CHECKING(for G2Banking version >=$vma.$vmi.$vpl.$vbld)
      if test "$enable_g2banking_test" != "no"; then
        g2banking_versionstring="`$g2banking_dir/bin/g2banking-config --vstring`.`$g2banking_dir/bin/g2banking-config --vbuild`"
        AC_MSG_RESULT([found $g2banking_versionstring])
        if test "$vma" -gt "`$g2banking_dir/bin/g2banking-config --vmajor`"; then
          AC_MSG_ERROR([Your G2Banking version is way too old.
          Please update from http://www.aquamaniac.de/g2banking/])
        elif test "$vma" = "`$g2banking_dir/bin/g2banking-config --vmajor`"; then
          if test "$vmi" -gt "`$g2banking_dir/bin/g2banking-config --vminor`"; then
            AC_MSG_ERROR([Your G2Banking version is too old.
              Please update from http://www.aquamaniac.de/g2banking/])
          elif test "$vmi" = "`$g2banking_dir/bin/g2banking-config --vminor`"; then
              if test "$vpl" -gt "`$g2banking_dir/bin/g2banking-config --vpatchlevel`"; then
                AC_MSG_ERROR([Your G2Banking version is a little bit too old.
                Please update from http://www.aquamaniac.de/g2banking/])
              elif test "$vpl" = "`$g2banking_dir/bin/g2banking-config --vpatchlevel`"; then
                if test "$vbld" -gt "`$g2banking_dir/bin/g2banking-config --vbuild`"; then
                  AC_MSG_ERROR([Your G2Banking version is a little bit too old. 
      Please update to the latest CVS version. Instructions for accessing 
      CVS can be found on http://www.aquamaniac.de/g2banking/])
                 fi
               fi
          fi
        fi
        have_g2banking="yes"
        #AC_MSG_RESULT(yes)
        AC_DEFINE_UNQUOTED(HAVE_G2BANKING, 1, [Defines if your system has the g2banking package])
      else
        have_g2banking="yes"
        AC_MSG_RESULT(assuming yes)
        AC_DEFINE_UNQUOTED(HAVE_G2BANKING, 1, [Defines if your system has the g2banking package])
      fi
   fi
dnl end of "if enable-g2banking"
fi

AC_SUBST(g2banking_dir)
AC_SUBST(g2banking_libs)
AC_SUBST(g2banking_data)
AC_SUBST(g2banking_includes)
])
