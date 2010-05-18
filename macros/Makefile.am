## Please update this variable if any new macros are created

MACROS=						\
  aclocal-include.m4				\
  compiler-flags.m4				\
  curses.m4					\
  gperf-check.m4				\
  linger.m4					\
  need-declaration.m4				

EXTRA_DIST= gettext.m4 iconv.m4 lib-ld.m4 lib-link.m4 lib-prefix.m4 nls.m4 po.m4 progtest.m4 $(MACROS) autogen.sh
MAINTAINERCLEANFILES=macros.dep

@MAINT@macros.dep: Makefile.am
@MAINT@	@echo '$$(top_srcdir)/aclocal.m4: $(MACROS:%=macros/%)' > $@
