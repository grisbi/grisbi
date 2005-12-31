#ifndef OFX_H
#define OFX_H 1

/* START_INCLUDE_H */
#include "ofx.h"
#include "import.h"
/* END_INCLUDE_H */


/*START_DECLARATION*/
gboolean recuperation_donnees_ofx ( struct imported_file * imported );
/*END_DECLARATION*/


#ifndef _WIN32
/* On Windows, the Ofx Severity enumerate values are already used in wingdi.h, DELETE is used in winnt.h
 * This is a work around to this issues :
 *  INFO, WARN, ERROR, DELETE and REPLACE are defined in standard libofx.h;
 *  on windows there should be prefixed by OFX_ 
 */

#ifndef OFX_INFO
#define OFX_INFO    INFO
#endif

#ifndef OFX_WARN
#define OFX_WARN    WARN
#endif

#ifndef OFX_ERROR
#define OFX_ERROR   ERROR
#endif

#ifndef OFX_DELETE
#define OFX_DELETE  DELETE
#endif

#ifndef OFX_REPLACE
#define OFX_REPLACE REPLACE
#endif

#endif

#endif
