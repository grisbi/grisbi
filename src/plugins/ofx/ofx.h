#ifndef GSB_OFX_H
#define GSB_OFX_H

/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
G_MODULE_EXPORT extern void ofx_plugin_register ( void );
G_MODULE_EXPORT extern gpointer ofx_plugin_run ( GtkWidget * assistant,
				    struct imported_file * imported );
/* END_DECLARATION */


/* It seems the following applies to everyone, otherwise OFX_* are not defined */
/* #ifdef _WIN32 */
/* On Windows, the Ofx Severity enumerate values are already used in wingdi.h, DELETE is used in winnt.h
 * This is a work around to this issues :
 *  INFO, WARN, ERROR, DELETE and REPLACE are used in standard libofx.h;
 *  on windows they should be prefixed by OFX_
 */

#ifndef _MINGW

#ifndef OFX_INFO
#define OFX_INFO    INFO
#endif

#ifndef OFX_WARN
#define OFX_WARN    WARN
#endif

#ifndef OFX_ERROR
#if defined(_MSC_VER) || defined(_MINGW)
#undef ERROR
#endif /* _MSC_VER */
#define OFX_ERROR   ERROR
#endif

#ifndef OFX_DELETE
#define OFX_DELETE  DELETE
#endif

#ifndef OFX_REPLACE
#define OFX_REPLACE REPLACE
#endif

#endif /* _MIN_GW */

/* #endif _WIN32 */

#endif /* GSB_OFX_H */
