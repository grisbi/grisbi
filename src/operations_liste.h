GtkWidget *creation_fenetre_operations ( void );
GtkWidget *creation_tree_view_operations_par_compte ( gint no_compte );
void creation_colonnes_tree_view_par_compte ( gint no_compte );
void verification_list_store_termine ( gint no_compte );
void edition_operation ( void );
void ajuste_scrolling_liste_operations_a_selection ( gint compte );
gboolean changement_taille_liste_ope ( GtkWidget *tree_view,
				       GtkAllocation *allocation );
void demande_mise_a_jour_tous_comptes ( void );
void update_titres_tree_view ( void );
gboolean affichage_traits_liste_operation ( void );
void mise_a_jour_affichage_r ( gint affichage_r );
void mise_a_jour_affichage_lignes ( gint nb_lignes );
void remplissage_liste_operations ( gint compte );
void selectionne_ligne ( struct structure_operation *nouvelle_operation_selectionnee );
gboolean traitement_clavier_liste ( GtkWidget *widget_variable,
				    GdkEventKey *evenement );
struct structure_operation *cherche_operation_from_ligne ( gint ligne,
							   gint no_compte );
void mise_a_jour_labels_soldes ( void );
gdouble calcule_solde_compte ( gint no_compte );
gdouble calcule_solde_pointe_compte ( gint no_compte );
void ajoute_operations_compte_dans_list_store ( gint compte,
						gint par_partie );
void update_couleurs_background ( gint compte,
				  GtkTreeIter *iter_debut );
void update_soldes_list_store ( gint compte,
				GtkTreeIter *iter_debut );
void new_transaction ( );
void remove_transaction ( );
void clone_selected_transaction ( );
gpointer recupere_classement_par_no ( gint no_tri );
void supprime_operation ( struct structure_operation *operation );
void remplit_ligne_operation ( struct structure_operation *operation,
			       GtkTreeIter *iter );
GtkTreeIter *cherche_iter_operation ( struct structure_operation *operation );
gint cherche_ligne_operation ( struct structure_operation *operation );
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view );

