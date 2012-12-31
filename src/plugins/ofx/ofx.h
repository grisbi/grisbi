#ifndef GSB_OFX_H
#define GSB_OFX_H

/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
void ofx_plugin_register ( void );
gpointer ofx_plugin_run ( GtkWidget * assistant,
				    struct imported_file * imported );
/* END_DECLARATION */

#endif /* GSB_OFX_H */
