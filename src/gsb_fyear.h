#ifndef _GSB_FYEAR_H
#define _GSB_FYEAR_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_fyear_get_fyear_from_combobox ( GtkWidget *combo_box,
					 const GDate *date );
void gsb_fyear_init_variables ( void );
GtkWidget *gsb_fyear_make_combobox ( gboolean set_automatic );
gboolean gsb_fyear_set_automatic ( gboolean set_automatic );
gboolean gsb_fyear_set_combobox_history ( GtkWidget *combo_box,
					  gint fyear_number );
gboolean gsb_fyear_update_fyear_list ( void );
/* END_DECLARATION */
#endif
