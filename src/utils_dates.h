#ifndef _UTILS_DATES_H
#define _UTILS_DATES_H (1)
/* START_INCLUDE_H */
#include "utils_dates.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void date_set_value ( GtkWidget * hbox, GDate ** value, gboolean update );
GDate *gdate_today ( void );
GtkWidget * get_entry_from_date_entry (GtkWidget * hbox);
gboolean gsb_date_check_and_complete_entry ( GtkWidget *entry );
GDate *gsb_date_copy ( GDate *date );
gboolean gsb_date_free_last_date ( void );
gboolean gsb_date_set_last_date ( const gchar *date );
gchar *gsb_date_today ( void );
gchar * gsb_format_date ( gint day, gint month, gint year );
gchar * gsb_format_gdate ( GDate *date );
GDate *gsb_parse_date_string ( const gchar *date_string );
GtkWidget * new_date_entry ( gchar ** value, GCallback hook );
/* END_DECLARATION */
#endif
