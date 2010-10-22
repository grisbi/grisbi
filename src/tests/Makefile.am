# Cunit

if HAVE_CUNIT

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src $(GRISBI_CFLAGS) $(IGE_MAC_CFLAGS) $(CUNIT_CFLAGS)

check_PROGRAMS = cunit_tests
TESTS = cunit_tests

cunit_tests_SOURCES = \
	main_cunit.c	\
	gsb_data_account_cunit.c	\
	gsb_real_cunit.c	\
	utils_dates_cunit.c	\
	\
	gsb_data_account_cunit.h	\
	gsb_real_cunit.h	\
	utils_dates_cunit.h

cunit_tests_LDADD = $(top_builddir)/src/libgrisbi.la $(GRISBI_LIBS) $(IGE_MAC_LIBS) $(CUNIT_LIBS)


CLEANFILES = *~

endif
