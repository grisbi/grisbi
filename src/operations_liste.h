GtkWidget *creation_fenetre_operations ( void );
GtkWidget *initialisation_tree_view_operations ( void );
void creation_titres_tree_view ( void );
void update_titres_tree_view ( void );
void creation_listes_operations ( void );
void remplissage_liste_operations ( gint compte );
gchar *recherche_contenu_cellule ( struct structure_operation *operation,
				   gint no_affichage );
gboolean selectionne_ligne_souris ( GtkWidget *tree_view,
				    GdkEventButton *evenement );
gboolean traitement_clavier_liste ( GtkWidget *widget_variable,
				    GdkEventKey *evenement );
void selectionne_ligne ( gint compte,
			 gint nouvelle_ligne );
void ajuste_scrolling_liste_operations_a_selection ( gint compte );
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view );
struct structure_operation *cherche_operation_from_ligne ( gint ligne );
GtkTreeIter *cherche_iter_operation ( struct structure_operation *operation );
void edition_operation ( void );
void p_press (void);
void r_press (void);
void supprime_operation ( struct structure_operation *operation );
gboolean changement_taille_liste_ope ( GtkWidget *tree_view,
				       GtkAllocation *allocation );
void demande_mise_a_jour_tous_comptes ( void );
void verification_mise_a_jour_liste ( void );
void mise_a_jour_solde ( gint compte );
void verifie_ligne_selectionnee_visible ( void );
gboolean assert_selected_transaction ();
void new_transaction ( );
void remove_transaction ( );
void clone_selected_transaction ( );
struct structure_operation * clone_transaction ( struct structure_operation * operation );
void move_selected_operation_to_account ( GtkMenuItem * menu_item );
void move_operation_to_account ( struct structure_operation * transaction, 
				 gint account );
void schedule_selected_transaction ( );
struct operation_echeance * schedule_transaction ( struct structure_operation * transaction );
void popup_transaction_context_menu ( gboolean full );
void affichage_traits_liste_operation ( void );
gboolean click_sur_titre_colonne_operations ( GtkTreeViewColumn *colonne,
					      gint *no_colonne );
void changement_choix_tri_liste_operations ( gchar *nom_tri );
gint recupere_no_from_nom_champ_tri ( gchar *nom_tri );
gpointer recupere_classement_par_no ( gint no_tri );

