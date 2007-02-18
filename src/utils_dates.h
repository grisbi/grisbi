#ifndef _UTILS_DATES_H
#define _UTILS_DATES_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GDate *gdate_today ( void );
gboolean gsb_date_check_and_complete_entry ( GtkWidget *entry,
					     gboolean set_today );
GDate *gsb_date_copy ( const GDate *date );
gboolean gsb_date_free_last_date ( void );
gboolean gsb_date_set_last_date ( const gchar *date );
gchar *gsb_date_today ( void );
gchar *gsb_format_date ( gint day, gint month, gint year );
gchar *gsb_format_gdate ( const GDate *date );
gchar * gsb_format_gdate_safe ( const GDate *date );
GDate *gsb_parse_date_string ( const gchar *date_string );
GDate *gsb_parse_date_string_safe ( const gchar *date_string );
/* END_DECLARATION */
#endif
