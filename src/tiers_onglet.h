GtkWidget *onglet_tiers ( void );
void remplit_arbre_tiers ( void );
gint classement_alphabetique_tree ( GtkWidget *tree,
				    GtkCListRow *ligne_1,
				    GtkCListRow *ligne_2 );
void ouverture_node_tiers ( GtkWidget *arbre,
			    GtkCTreeNode *node,
			    gpointer null );
gboolean selection_ligne_tiers ( GtkCTree *arbre_tiers, GtkCTreeNode *noeud,
				 gint colonne, gpointer null );
gboolean verifie_double_click ( GtkWidget *liste, GdkEventButton *ev, 
				gpointer null );
gboolean enleve_selection_ligne_tiers ( void );
void modification_du_texte_tiers ( GtkText *texte,
				   gpointer null );
void clique_sur_modifier_tiers ( GtkWidget *bouton_modifier,
				 gpointer null );
void clique_sur_annuler_tiers ( GtkWidget *bouton_annuler,
				gpointer null );
struct struct_tiers *ajoute_nouveau_tiers ( gchar *tiers );
void supprimer_tiers ( GtkWidget *bouton,
		       gpointer null );
void creation_liste_tiers_combofix ( void );
gboolean changement_taille_liste_tiers ( GtkWidget *clist,
					 GtkAllocation *allocation,
					 gpointer null );
void mise_a_jour_combofix_tiers ( void );
gfloat *calcule_total_montant_tiers ( void );
gchar *calcule_total_montant_tiers_par_compte ( gint no_tiers,
					       gint no_compte );
void appui_sur_ajout_tiers ( void );
gboolean keypress_tiers ( GtkWidget *widget, GdkEventKey *ev, gint *no_origine );
gboolean expand_selected_tiers ( GtkWidget *liste, GdkEventButton *ev, gpointer null );


struct struct_tiers *tiers_par_no ( gint no_tiers );
struct struct_tiers *tiers_par_nom ( gchar *nom_tiers,
				     gboolean creer );
gchar *tiers_name_by_no ( gint no_tiers,
			  gboolean return_null );

