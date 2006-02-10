#ifndef _GSB_DATA_CURRENCY_H
#define _GSB_DATA_CURRENCY_H (1)


/* START_INCLUDE_H */
#include "gsb_data_currency.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
GDate *gsb_data_currency_get_change_date ( gint currency_number );
gdouble gsb_data_currency_get_change_rate ( gint currency_number );
gint gsb_data_currency_get_change_to_euro ( gint currency_number );
gchar *gsb_data_currency_get_code ( gint currency_number );
gchar *gsb_data_currency_get_code_iso4217 ( gint currency_number );
gchar *gsb_data_currency_get_code_or_isocode ( gint currency_number );
gint gsb_data_currency_get_contra_currency_number ( gint currency_number );
GSList *gsb_data_currency_get_currency_list ( void );
gint gsb_data_currency_get_default_currency (void);
gint gsb_data_currency_get_link_currency ( gint currency_number );
gchar *gsb_data_currency_get_name ( gint currency_number );
gint gsb_data_currency_get_no_currency ( gpointer currency_ptr );
gint gsb_data_currency_get_number_by_code_iso4217 ( const gchar *code );
gint gsb_data_currency_get_number_by_name ( const gchar *name );
gboolean gsb_data_currency_init_variables ( void );
gint gsb_data_currency_new ( const gchar *name );
gboolean gsb_data_currency_remove ( gint currency_number );
gboolean gsb_data_currency_set_change_date ( gint currency_number,
					     GDate *change_date );
gboolean gsb_data_currency_set_change_rate ( gint currency_number,
					     gdouble change_rate);
gboolean gsb_data_currency_set_change_to_euro ( gint currency_number,
						gint change_to_euro);
gboolean gsb_data_currency_set_code ( gint currency_number,
				      const gchar *currency_code );
gboolean gsb_data_currency_set_code_iso4217 ( gint currency_number,
					      const gchar *currency_code_iso4217 );
gboolean gsb_data_currency_set_contra_currency_number ( gint currency_number,
							gint contra_currency_number);
gboolean gsb_data_currency_set_link_currency ( gint currency_number,
					       gint link_currency);
gboolean gsb_data_currency_set_name ( gint currency_number,
				      const gchar *name );
gint gsb_data_currency_set_new_number ( gint currency_number,
					gint new_no_currency );
/* END_DECLARATION */
#endif
