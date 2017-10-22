/* fichier d'en tête include.h */
#ifndef _INCLUDE_H
#define _INCLUDE_H (1)


/* The following lines revert back to FPRINTF on Windows Systems(LIBINTL_FPRINTF seems BUGGY) */
#ifdef G_OS_WIN32
# undef fprintf
#endif

#endif /* _INCLUDE_H */
