/* fichier d'en tête include.h */
#ifndef _INCLUDE_H
#define _INCLUDE_H (1)


/* The following lines revert back to FPRINTF on Windows Systems(LIBINTL_FPRINTF seems BUGGY) */
#ifdef _WIN32
# undef fprintf
#endif

/* g_fopen returns an incompatible pointer on windows with MSVC. This is a fix*/
#ifdef _MSC_VER
#undef g_fopen
#define g_fopen fopen
#endif

#endif /* _INCLUDE_H */
