/* fichier d'en tête gsbcalendar.h */
GtkWidget *gsb_calendar_new ( GtkWidget *entry ) ;
gboolean clavier_calendrier ( GtkCalendar *pCalendar,
			      GdkEventKey *ev,
			      GtkWidget *entry );
void date_selection ( GtkCalendar *pCalendar,
		      GtkWidget *entry );
void calendar_destroyed ( GtkWidget *popup,
			 GdkEventKey *ev,
			 GtkEntry *entry );
/*
void calendar_destroy ( GtkButton *button,
			  GdkEventKey *ev,
			  GtkCalendar *pCalendar );*/

