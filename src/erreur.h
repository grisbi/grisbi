/*START_DECLARATION*/
void affiche_log_message ( void );
gboolean fermeture_grisbi ( void );
void traitement_sigsegv ( gint signal_nb );
void initialize_debugging ( void );
gchar *get_debug_time ( void );
void debug_message ( gchar *prefixe, gchar *message, gint level, gboolean force_debug_display);
void print_backtrace ( void );
/*END_DECLARATION*/
