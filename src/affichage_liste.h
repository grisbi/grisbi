GtkWidget *onglet_affichage_liste ( void );
gboolean change_choix_ajustement_auto_colonnes ( GtkWidget *bouton );
gboolean change_largeur_colonne ( GtkWidget *clist,
				  gint colonne,
				  gint largeur );
GtkWidget *cree_menu_quatres_lignes ( void );
gboolean allocation_clist_affichage_liste ( GtkWidget *clist,
					    GtkAllocation *allocation );
gboolean pression_bouton_classement_liste ( GtkWidget *clist,
					    GdkEventButton *ev );
gboolean lache_bouton_classement_liste ( GtkWidget *clist,
					 GdkEventButton *ev );
void remplissage_tab_affichage_ope ( GtkWidget *clist );
void toggled_bouton_affichage_liste ( GtkWidget *bouton,
				      gint *no_bouton );
void recuperation_noms_colonnes_et_tips ( void );
void raz_affichage_ope ( void );
GtkWidget *onglet_affichage_operations ( void );
gboolean change_choix_ajustement_auto_colonnes ( GtkWidget *bouton );

