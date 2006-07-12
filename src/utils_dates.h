#ifndef _UTILS_DATES_H
#define _UTILS_DATES_H (1)
/* START_INCLUDE_H */
#include "utils_dates.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
GDate *gdate_today ( void );
gboolean gsb_date_check_and_complete_entry ( GtkWidget *entry );
GDate *gsb_date_copy ( GDate *date );
gboolean gsb_date_free_last_date ( void );
gboolean gsb_date_set_last_date ( const gchar *date );
gchar *gsb_date_today ( void );
GtkWidget *gsb_editable_date_new ( gchar *value,
				   GCallback hook,
				   gpointer data,
				   GCallback default_func,
				   gint number_for_func );
void gsb_editable_date_set_value ( GtkWidget *hbox,
				   GDate *value,
				   gint number_for_func );
gchar * gsb_format_date ( gint day, gint month, gint year );
gchar * gsb_format_gdate ( GDate *date );
gchar * gsb_format_gdate_safe ( GDate *date );
GDate *gsb_parse_date_string ( const gchar *date_string );
GDate *gsb_parse_date_string_safe ( const gchar *date_string );
/* END_DECLARATION */
#endif
