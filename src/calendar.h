/* fichier d'en tête gsbcalendar.h */
GtkWidget *gsb_calendar_new ( GtkWidget *entry ) ;
gboolean clavier_calendrier ( GtkCalendar *pCalendar,
			      GdkEventKey *ev,
			      GtkWidget *entry );
void date_selection ( GtkCalendar *pCalendar,
		      GtkWidget *entry );
gchar *gsb_today ( void ) ;
gboolean format_date ( GtkWidget *entree );

double gsb_strtod ( char *nptr, char **endptr );
