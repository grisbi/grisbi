#ifndef _UTILS_DATES_H
#define _UTILS_DATES_H (1)
/* START_INCLUDE_H */
#include "utils_dates.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void date_set_value ( GtkWidget * hbox, GDate ** value, gboolean update );
gboolean format_date ( GtkWidget *entree );
GDate *gdate_today ( void );
GtkWidget * get_entry_from_date_entry (GtkWidget * hbox);
GDate *gsb_date_copy ( GDate *date );
gchar * gsb_format_date ( gint day, gint month, gint year );
gchar * gsb_format_gdate ( GDate *date );
GDate *gsb_parse_date_string ( const gchar *date_string );
gchar *gsb_today ( void );
gboolean modifie_date ( GtkWidget *entree );
GtkWidget * new_date_entry ( gchar ** value, GCallback hook );
/* END_DECLARATION */
#endif
