#ifndef _GSB_CALENDAR_ENTRY_H
#define _GSB_CALENDAR_ENTRY_H (1)

/* fichier d'en tête gsbcalendar.h */
/* Constantes utilisées par la fonction inc_dec_date */
#define ONE_DAY 1
#define ONE_WEEK 7
#define ONE_MONTH 30
#define ONE_YEAR 365
#define SIZEOF_FORMATTED_STRING_DATE 200

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/*START_DECLARATION*/
GDate *gsb_calendar_entry_get_date ( GtkWidget *entry );
GtkWidget *gsb_calendar_entry_new ( gint set_today );
gboolean gsb_calendar_entry_set_color ( GtkWidget *entry,
					gboolean normal_color );
gboolean gsb_calendar_entry_set_date ( GtkWidget *entry,
				       const GDate *date );
/*END_DECLARATION*/


#endif

