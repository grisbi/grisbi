/*START_DECLARATION*/
gboolean affichage_traits_liste_operation ( void );
gboolean changement_taille_liste_ope ( GtkWidget *tree_view,
				       GtkAllocation *allocation );
GtkTreeIter *cherche_iter_operation ( struct structure_operation *transaction,
				      gint no_account );
gint cherche_ligne_operation ( struct structure_operation *transaction,
			       gint no_account );
void clone_selected_transaction ();
void creation_colonnes_tree_view_par_compte ( gint no_account );
GtkWidget *creation_fenetre_operations ( void );
GtkWidget *creation_tree_view_operations_par_compte ( gint no_account );
void demande_mise_a_jour_tous_comptes ( void );
gint find_p_r_col ();
gboolean gsb_account_list_set_breakdowns_visible ( gint no_account,
						   struct structure_operation *transaction,
						   gint visible );
gchar *gsb_transactions_get_category_real_name ( struct structure_operation *transaction );
gboolean gsb_transactions_list_append_transaction ( struct structure_operation *transaction,
						    gint no_account );
struct structure_operation *gsb_transactions_list_append_white_breakdown ( struct structure_operation *transaction );
gboolean gsb_transactions_list_button_press ( GtkWidget *tree_view,
					      GdkEventButton *ev );
struct structure_operation *gsb_transactions_list_clone_transaction ( struct structure_operation *transaction );
gboolean gsb_transactions_list_delete_transaction ( struct structure_operation *transaction );
gboolean gsb_transactions_list_edit_current_transaction ( void );
gboolean gsb_transactions_list_fill_row ( struct structure_operation *transaction,
					  GtkTreeIter *iter,
					  GtkListStore *store,
					  gint line_in_transaction );
gboolean gsb_transactions_list_fill_store ( gint no_account,
					    gboolean by_part );
gboolean gsb_transactions_list_key_press ( GtkWidget *widget,
					   GdkEventKey *ev );
gboolean gsb_transactions_list_move_to_current_transaction ( gint no_account );
gboolean gsb_transactions_list_set_background_color ( gint no_account );
gboolean gsb_transactions_list_set_current_transaction ( struct structure_operation *new_transaction,
							 gint no_account );
gboolean gsb_transactions_list_set_transactions_balances ( gint no_account );
gboolean gsb_transactions_list_set_visibles_rows_on_account ( gint no_account );
gboolean gsb_transactions_list_set_visibles_rows_on_transaction ( struct structure_operation *transaction );
void mise_a_jour_affichage_lignes ( gint nb_lignes );
void mise_a_jour_affichage_r ( gint affichage_r );
void mise_a_jour_labels_soldes ( void );
void move_selected_operation_to_account_nb ( gint *account );
void new_transaction () ;
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view );
void remove_transaction ();
void remplissage_liste_operations ( gint compte );
void update_titres_tree_view ( void );
void verification_list_store_termine ( gint no_account );
/*END_DECLARATION*/

