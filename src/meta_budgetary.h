#ifndef _META_BUDGETARY_H
#define _META_BUDGETARY_H (1)

#include <glib.h>

/* START_INCLUDE_H */
#include "metatree.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
MetatreeInterface *budgetary_line_get_metatree_interface ( void );
gint budgetary_line_tree_currency ( void );
/* END_DECLARATION */
#endif
