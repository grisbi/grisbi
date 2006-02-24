#ifndef _GSB_DATA_FYEAR_H
#define _GSB_DATA_FYEAR_H (1)


/* START_INCLUDE_H */
#include "gsb_data_fyear.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_data_fyear_compare ( gint fyear_number_1,
			      gint fyear_number_2 );
GDate *gsb_data_fyear_get_begining_date ( gint fyear_number );
GDate *gsb_data_fyear_get_end_date ( gint fyear_number );
gboolean gsb_data_fyear_get_form_show ( gint fyear_number );
gint gsb_data_fyear_get_from_date ( GDate *date );
GSList *gsb_data_fyear_get_fyears_list ( void );
gchar *gsb_data_fyear_get_name ( gint fyear_number );
gint gsb_data_fyear_get_no_fyear ( gpointer fyear_ptr );
gboolean gsb_data_fyear_init_variables ( void );
gint gsb_data_fyear_new ( const gchar *name );
gboolean gsb_data_fyear_remove ( gint fyear_number );
gboolean gsb_data_fyear_set_begining_date ( gint fyear_number,
					    GDate *date );
gboolean gsb_data_fyear_set_end_date ( gint fyear_number,
				       GDate *date );
gboolean gsb_data_fyear_set_form_show ( gint fyear_number,
					gboolean showed_in_form );
gboolean gsb_data_fyear_set_name ( gint fyear_number,
				   const gchar *name );
gint gsb_data_fyear_set_new_number ( gint fyear_number,
				     gint new_no_fyear );
/* END_DECLARATION */
#endif
