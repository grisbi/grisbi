void nouveau_fichier ( void );
void ouvrir_fichier ( void );
void ouverture_fichier_par_menu ( gpointer null,
				  gint no_fichier );
void fichier_selectionne ( GtkWidget *selection_fichier);
void ouverture_confirmee ( void );
gboolean enregistrement_fichier ( gint origine );
gboolean enregistrer_fichier_sous ( void );
gint question_fermer_sans_enregistrer ( void );
gchar *demande_nom_enregistrement ( void );
gboolean fermer_fichier ( void );
void fermeture_confirmee ( void );
void affiche_titre_fenetre ( void );
gboolean enregistrement_backup ( void );
void ajoute_nouveau_fichier_liste_ouverture ( gchar *path_fichier );
void remove_file_from_last_opened_files_list ( gchar *nom_fichier );

