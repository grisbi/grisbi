/*START_DECLARATION*/
void affiche_log_message ( void );
gboolean fermeture_grisbi ( void );
void traitement_sigsegv ( gint signal_nb );
void initialize_debugging ( void );
void debug_message ( gchar *prefixe, gchar *message, gint level, gboolean force_debug_display);
/*END_DECLARATION*/
