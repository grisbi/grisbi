GtkWidget *creation_fenetre_operations ( void );
GtkWidget *creation_tree_view_operations ( void );
void creation_titres_tree_view ( void );
void update_titres_tree_view ( void );
void verification_list_store_termine ( gint no_compte );
void remplissage_liste_operations ( gint compte );
void ajoute_operations_compte_dans_list_store ( gint compte,
						gint par_partie );
void remplit_ligne_operation ( struct structure_operation *operation,
			       GtkTreeIter *iter );
void affiche_ligne_ope ( struct structure_operation *operation,
			 GtkTreeIter *iter,
			 gint ligne );
gchar *recherche_contenu_cellule ( struct structure_operation *operation,
				   gint no_affichage );
void update_couleurs_background ( gint compte,
				  GtkTreeIter *iter_debut );
void update_soldes_list_store ( gint compte,
				GtkTreeIter *iter_debut );
gdouble solde_debut_affichage ( gint no_compte );
gdouble calcule_solde_compte ( gint no_compte );
gdouble calcule_solde_pointe_compte ( gint no_compte );
gboolean selectionne_ligne_souris ( GtkWidget *tree_view,
				    GdkEventButton *evenement );
gboolean traitement_clavier_liste ( GtkWidget *widget_variable,
				    GdkEventKey *evenement );
void selectionne_ligne ( struct structure_operation *nouvelle_operation_selectionnee );
void ajuste_scrolling_liste_operations_a_selection ( gint compte );
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view );
struct structure_operation *cherche_operation_from_ligne ( gint ligne,
							   gint no_compte );
struct structure_operation *operation_from_iter ( GtkTreeIter *iter,
						  gint no_compte );
GtkTreeIter *cherche_iter_operation ( struct structure_operation *operation );
gint cherche_ligne_operation ( struct structure_operation *operation );
void edition_operation ( void );
void p_press (void);
void r_press (void);
void supprime_operation ( struct structure_operation *operation );
gboolean changement_taille_liste_ope ( GtkWidget *tree_view,
				       GtkAllocation *allocation );
void demande_mise_a_jour_tous_comptes ( void );
gboolean assert_selected_transaction ();
void new_transaction ( );
void remove_transaction ( );
void clone_selected_transaction ( );
struct structure_operation * clone_transaction ( struct structure_operation * operation );
void move_selected_operation_to_account ( GtkMenuItem * menu_item );
gboolean move_operation_to_account ( struct structure_operation * transaction, 
				     gint account );
void schedule_selected_transaction ( );
struct operation_echeance * schedule_transaction ( struct structure_operation * transaction );
void popup_transaction_context_menu ( gboolean full );
gboolean affichage_traits_liste_operation ( void );
gboolean click_sur_titre_colonne_operations ( GtkTreeViewColumn *colonne,
					      gint *no_colonne );
gboolean changement_choix_tri_liste_operations ( gchar *nom_tri );
gint recupere_no_from_nom_champ_tri ( gchar *nom_tri );
gpointer recupere_classement_par_no ( gint no_tri );
void mise_a_jour_labels_soldes ( void );
void my_list_store_sort ( gint no_compte,
			 GSList *liste_avant_tri,
			 GSList *liste_apres_tri );
GSList *cree_slist_affichee ( gint no_compte );
void mise_a_jour_affichage_r ( gint affichage_r );
void mise_a_jour_affichage_lignes ( gint nb_lignes );

