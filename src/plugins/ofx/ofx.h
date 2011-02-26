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


#endif /* GSB_OFX_H */
