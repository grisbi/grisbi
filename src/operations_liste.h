GtkWidget *creation_fenetre_operations ( void );
GtkWidget *initialisation_notebook_operations ( void );
void creation_listes_operations ( void );
gint empeche_pression_titre_colonne ( GtkWidget *bouton );
void ajoute_nouvelle_liste_operation ( gint no_compte );
void onglet_compte_realize ( GtkWidget *onglet,
			     GtkWidget *liste );
void remplissage_liste_operations ( gint compte );
gchar *recherche_contenu_cellule ( struct structure_operation *operation,
				   gint no_affichage );
void selectionne_ligne_souris ( GtkCList *liste_operations,
				GdkEventButton *evenement,
				gpointer data);
gboolean traitement_clavier_liste ( GtkCList *liste_operations,
				     GdkEventKey *evenement,
				     gpointer data);
void ctrl_press ( void );
void fin_ctrl ( GtkCList *liste_operations,
		    GdkEventKey *evenement,
		    gpointer origine);
void selectionne_ligne ( gint compte );
void edition_operation ( void );
void p_press (void);
void r_press (void);
void supprime_operation ( struct structure_operation *operation );
gint recherche_operation_par_no ( struct structure_operation *operation,
				  gint *no_ope );
void changement_taille_liste_ope ( GtkWidget *clist,
				   GtkAllocation *allocation,
				   gint *compte );
void focus_a_la_liste ( void );
void demande_mise_a_jour_tous_comptes ( void );
void verification_mise_a_jour_liste ( void );
void mise_a_jour_solde ( gint compte );
void verifie_ligne_selectionnee_visible ( void );
void changement_taille_colonne ( GtkWidget *clist,
				 gint colonne,
				 gint largeur );
void new_transaction ( );
void remove_transaction ( );
void clone_selected_transaction ( );
struct structure_operation * clone_transaction ( struct structure_operation * operation );
void move_operation_to_account ( );
void popup_menu ( gboolean full );
