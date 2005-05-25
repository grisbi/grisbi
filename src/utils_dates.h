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
gchar *gsb_today ( void );
gboolean modifie_date ( GtkWidget *entree );
GtkWidget * new_date_entry ( gchar ** value, GCallback hook );
gchar *renvoie_date_formatee ( GDate *date );
/* END_DECLARATION */
#endif
