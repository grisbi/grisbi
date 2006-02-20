#ifndef GSB_FILE_DEBUG_H
#define GSB_FILE_DEBUG_H (1)

/* START_INCLUDE_H */
#include "gsb_file_debug.h"
/* END_INCLUDE_H */


struct gsb_debug_test {
    gchar * name;
    gchar * description;
    gchar * instructions;
    gchar * (* test) ();
    gboolean (* fix) ();
};


/* START_DECLARATION */
gboolean gsb_file_debug ( void );
/* END_DECLARATION */

#endif
