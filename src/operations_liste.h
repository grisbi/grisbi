/*START_DECLARATION*/
gboolean affichage_traits_liste_operation ( void );
void ajoute_operations_compte_dans_list_store ( gint compte,
						gint par_partie );
gboolean changement_taille_liste_ope ( GtkWidget *tree_view,
				       GtkAllocation *allocation );
GtkTreeIter *cherche_iter_operation ( struct structure_operation *operation,
				      gint no_account );
gint cherche_ligne_operation ( struct structure_operation *operation,
			       gint no_account );
void clone_selected_transaction ();
void creation_colonnes_tree_view_par_compte ( gint no_compte );
GtkWidget *creation_fenetre_operations ( void );
GtkWidget *creation_tree_view_operations_par_compte ( gint no_compte );
void demande_mise_a_jour_tous_comptes ( void );
void edition_operation ( void );
gint find_p_r_col ();
gchar *gsb_transactions_get_category_real_name ( struct structure_operation *transaction );
gboolean gsb_transactions_list_button_press ( GtkWidget *tree_view,
					      GdkEventButton *ev );
gboolean gsb_transactions_list_key_press ( GtkWidget *widget,
					   GdkEventKey *ev );
gboolean gsb_transactions_list_move_to_current_transaction ( gint no_account );
gboolean gsb_transactions_list_set_background_color ( gint no_account );
gboolean gsb_transactions_list_set_current_transaction ( struct structure_operation *new_transaction,
							 gint no_account );
gboolean gsb_transactions_list_set_transactions_balances ( gint no_account );
void hide_balance_labels ( void );
void mise_a_jour_affichage_lignes ( gint nb_lignes );
void mise_a_jour_affichage_r ( gint affichage_r );
void mise_a_jour_labels_soldes ( void );
void move_selected_operation_to_account_nb ( gint *account );
void new_transaction () ;
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view );
void remove_transaction ();
void remplissage_liste_operations ( gint compte );
void remplit_ligne_operation ( struct structure_operation *operation,
			       GtkTreeIter *iter );
gboolean set_visibles_rows_on_account ( gint no_account );
void show_balance_labels ( void );
void supprime_operation ( struct structure_operation *operation );
void update_titres_tree_view ( void );
void verification_list_store_termine ( gint no_compte );
/*END_DECLARATION*/

