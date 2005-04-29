#ifndef _ERREUR_H
#define _ERREUR_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void affiche_log_message ( void );
void debug_message ( gchar *prefixe, gchar *message, gint level, gboolean force_debug_display);
gboolean fermeture_grisbi ( void );
void initialize_debugging ( void );
void traitement_sigsegv ( gint signal_nb );
/* END_DECLARATION */
#endif
