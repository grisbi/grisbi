void dialogue ( gchar * texte_dialogue );
void dialogue_error ( gchar *text );
void dialogue_warning ( gchar *text );
void dialogue_special ( GtkMessageType param, gchar * text );
gboolean blocage_boites_dialogues ( GtkWidget *dialog,
				    gpointer null );
gboolean question ( gchar *texte );
gboolean question_yes_no ( gchar *texte );
gchar *demande_texte ( gchar *titre_fenetre, gchar *question );
