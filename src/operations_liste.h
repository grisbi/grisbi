#define START_DECLARATION
gboolean affichage_traits_liste_operation ( void );
void ajoute_operations_compte_dans_list_store ( gint compte,
						gint par_partie );
void ajuste_scrolling_liste_operations_a_selection ( gint compte );
gdouble calcule_solde_compte ( gint no_compte );
gdouble calcule_solde_pointe_compte ( gint no_compte );
gboolean changement_taille_liste_ope ( GtkWidget *tree_view,
				       GtkAllocation *allocation );
GtkTreeIter *cherche_iter_operation ( struct structure_operation *operation );
gint cherche_ligne_operation ( struct structure_operation *operation );
struct structure_operation *cherche_operation_from_ligne ( gint ligne,
							   gint no_compte );
void classe_liste_operations ( gint no_compte );
void clone_selected_transaction ();
void creation_colonnes_tree_view_par_compte ( gint no_compte );
GtkWidget *creation_fenetre_operations ( void );
GtkWidget *creation_tree_view_operations_par_compte ( gint no_compte );
void demande_mise_a_jour_tous_comptes ( void );
void edition_operation ( void );
void mise_a_jour_affichage_lignes ( gint nb_lignes );
void mise_a_jour_affichage_r ( gint affichage_r );
void mise_a_jour_labels_soldes ( void );
void move_selected_operation_to_account_nb ( gint *account );
void new_transaction () ;
gpointer recupere_classement_par_no ( gint no_classement );
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view );
void remove_transaction ();
void remplissage_liste_operations ( gint compte );
void remplit_ligne_operation ( struct structure_operation *operation,
			       GtkTreeIter *iter );
void selectionne_ligne ( struct structure_operation *nouvelle_operation_selectionnee );
gboolean selectionne_ligne_souris ( GtkWidget *tree_view,
				    GdkEventButton *evenement );
void supprime_operation ( struct structure_operation *operation );
gboolean traitement_clavier_liste ( GtkWidget *widget_variable,
				    GdkEventKey *evenement );
void update_couleurs_background ( gint compte,
				  GtkTreeIter *iter_debut );
void update_fleches_classement_tree_view ( gint no_compte );
void update_soldes_list_store ( gint compte,
				GtkTreeIter *iter_debut );
void update_titres_tree_view ( void );
void verification_list_store_termine ( gint no_compte );
#define END_DECLARATION

