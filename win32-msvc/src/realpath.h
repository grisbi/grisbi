#ifndef _GRISBI_WIN32_MSVC_REALPATH_H
#define _GRISBI_WIN32_MSVC_REALPATH_H (1)

/* START_DECLARATION */

#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)

//char *realpath(const char *path, char resolved_path[]);

/* END_DECLARATION */

#endif//_GRISBI_WIN32_MSVC_REALPATH_H
