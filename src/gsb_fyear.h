#ifndef _GSB_FYEAR_H
#define _GSB_FYEAR_H (1)


/* START_INCLUDE_H */
#include "gsb_fyear.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_fyear_get_fyear_from_combobox ( GtkWidget *combo_box,
					 GDate *date );
void gsb_fyear_init_variables ( void );
GtkWidget *gsb_fyear_make_combobox ( void );
gboolean gsb_fyear_set_combobox_history ( GtkWidget *combo_box,
					  gint fyear_number );
gboolean gsb_fyear_update_fyear_list ( void );
/* END_DECLARATION */
#endif
