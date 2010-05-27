# Cunit

if HAVE_CUNIT

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src

check_PROGRAMS = cunit_tests
TESTS = cunit_tests

cunit_tests_SOURCES = \
	main_cunit.c	\
	gsb_data_account_cunit.c	\
	gsb_real_cunit.c	\
	utils_dates_cunit.c	\
	\
	main_cunit.h	\
	gsb_data_account_cunit.h	\
	gsb_real_cunit.h	\
	utils_dates_cunit.h

cunit_tests_LDADD = $(top_builddir)/src/libgrisbi.la -lcunit $(GTK_LIBS)


CLEANFILES = *~

MAINTAINERCLEANFILES = Makefile.in

endif
