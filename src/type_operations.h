GtkWidget *onglet_types_operations ( void );
gboolean select_payment_method (GtkTreeSelection *selection,
				GtkTreeModel *model);
gboolean deselect_payment_method (GtkTreeSelection *selection,
				  GtkTreeModel *model);
void modification_entree_nom_type ( void );
void modification_type_numerotation_auto (void);
void modification_entree_type_dernier_no ( void );
void modification_type_signe ( gint *no_menu );
void ajouter_type_operation ( void );
void supprimer_type_operation ( void );
void inclut_exclut_les_neutres ( void );
void remplit_liste_tri_par_type ( gint no_compte );
void selection_type_liste_tri ( void );
void deselection_type_liste_tri ( void );
void deplacement_type_tri_haut ( GtkWidget * button, gpointer data );
void deplacement_type_tri_bas ( void );
void save_ordre_liste_type_tri ( void );
GtkWidget *creation_menu_types ( gint demande,
				 gint compte,
				 gint origine );
gint cherche_no_menu_type ( gint demande );
gint cherche_no_menu_type_associe ( gint demande,
				    gint origine );
gint cherche_no_menu_type_echeancier ( gint demande );
void changement_choix_type_formulaire ( struct struct_type_ope *type );
void changement_choix_type_echeancier ( struct struct_type_ope *type );
gchar *type_ope_name_by_no ( gint no_type_ope,
			     gint no_de_compte );

