void nouveau_fichier ( void );
void initialisation_variables_nouveau_fichier ( void );
void initialisation_graphiques_nouveau_fichier ( void );
void ouvrir_fichier ( void );
void ouverture_fichier_par_menu ( gpointer null,
				  gint no_fichier );
gboolean fermer_fichier ( void );
gint question_fermer_sans_enregistrer ( void );
gboolean enregistrement_fichier ( gint origine );
gboolean enregistrer_fichier_sous ( void );
void ouverture_confirmee ( void );
void affiche_titre_fenetre ( void );
void remove_file_from_last_opened_files_list ( gchar *nom_fichier );

