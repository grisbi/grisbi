gboolean fermeture_grisbi ( void );
void dialogue ( gchar * texte_dialogue );
gboolean blocage_boites_dialogues ( GtkWidget *dialog,
				    gpointer null );
gboolean question ( gchar *texte );
gboolean question_yes_no ( gchar *texte );
gchar *demande_texte ( gchar *titre_fenetre,
		       gchar *question );
void affiche_log_message ( void );
void traitement_sigsegv ( gint signal_nb );
