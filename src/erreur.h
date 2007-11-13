#ifndef _ERREUR_H
#define _ERREUR_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void debug_message ( gchar *prefixe, gchar * file, gint line, const char * function, 
		     gchar *message, gint level, gboolean force_debug_display);
void r_devel_debug ( gchar * file, gint line, gchar * function, gchar *message );
void initialize_debugging ( void );
void r_notice_debug ( gchar * file, gint line, gchar * function, gchar *message );
void traitement_sigsegv ( gint signal_nb );
void r_warning_debug ( gchar * file, gint line, gchar * function, gchar *message );
/* END_DECLARATION */
#endif
