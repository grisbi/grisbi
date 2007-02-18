#ifndef _ERREUR_H
#define _ERREUR_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void debug_message ( gchar *prefixe, gchar *message, gint level, gboolean force_debug_display);
void devel_debug ( gchar *message );
gboolean fermeture_grisbi ( void );
void initialize_debugging ( void );
void notice_debug ( gchar *message );
void traitement_sigsegv ( gint signal_nb );
void warning_debug ( gchar *message );
/* END_DECLARATION */
#endif
