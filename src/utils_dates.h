#ifndef _UTILS_DATES_H
#define _UTILS_DATES_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GDate *gdate_today ( void );
GDate *gsb_date_add_one_month ( GDate *date, gboolean free );
gboolean gsb_date_check_and_complete_entry ( GtkWidget *entry,
                        gboolean set_today );
gboolean gsb_date_check_entry ( GtkWidget *entry );
GDate *gsb_date_copy ( const GDate *date );
void gsb_date_free_last_date ( void );
gchar *gsb_date_get_compiled_time ( void );
gchar *gsb_date_get_format_date ( void );
GDate *gsb_date_get_last_day_of_month ( const GDate *date );
GDate *gsb_date_get_last_entry_date ( const gchar *string );
void gsb_date_set_format_date ( const gchar *format_date );
void gsb_date_set_last_date ( const gchar *date );
gchar *gsb_date_today ( void );
GDate *gsb_date_tomorrow ( void );
gchar *gsb_format_date ( gint day, gint month, gint year );
gchar *gsb_format_gdate ( const GDate *date );
gchar *gsb_format_gdate_safe ( const GDate *date );
GDate *gsb_parse_date_string ( const gchar *date_string );
GDate *gsb_parse_date_string_safe ( const gchar *date_string );
/* END_DECLARATION */
#endif
