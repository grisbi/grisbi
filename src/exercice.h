GtkWidget *onglet_exercices ( void );
void ajout_exercice ( GtkWidget *bouton,
		      GtkWidget *clist );
void supprime_exercice ( GtkWidget *bouton,
			 GtkWidget *liste );
void selection_ligne_exercice ( GtkWidget *liste,
			      gint ligne,
			      gint colonne,
			      GdkEventButton *ev,
				GtkWidget *frame );
void deselection_ligne_exercice ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev,
				  GtkWidget *frame );
void applique_modif_exercice ( GtkWidget *liste );
void annuler_modif_exercice ( GtkWidget *bouton,
			      GtkWidget *liste );
gint recherche_exercice_par_nom ( struct struct_exercice *exercice,
				  gchar *nom );
gint recherche_exercice_par_no ( struct struct_exercice *exercice,
				 gint *no_exercice );
GtkWidget *creation_menu_exercices ( gint origine );
gint cherche_no_menu_exercice ( gint no_demande,
				GtkWidget *option_menu );
void affiche_exercice_par_date ( GtkWidget *entree_date,
				 GtkWidget *option_menu_exercice );
void association_automatique ( void );
gint recherche_exo_correspondant ( GDate *date );
