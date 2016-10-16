#ifndef _META_PAYEE_H
#define _META_PAYEE_H (1)

#include <glib.h>

/* START_INCLUDE_H */
#include "metatree.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
MetatreeInterface *payee_get_metatree_interface ( void );
gint payee_tree_currency ( void );
/* END_DECLARATION */
#endif
