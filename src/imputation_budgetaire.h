GtkWidget *onglet_imputations ( void );
void remplit_arbre_imputation ( void );
void ouverture_node_imputation ( GtkWidget *arbre,
				 GtkCTreeNode *node,
				 gpointer null );
void selection_ligne_imputation ( GtkCTree *arbre,
				  GtkCTreeNode *noeud,
				  gint colonne,
				  gpointer null );
void verifie_double_click_imputation ( GtkWidget *liste,
				       GdkEventButton *ev,
				       gpointer null );
void enleve_selection_ligne_imputation ( void );
void modification_du_texte_imputation ( void );
void clique_sur_modifier_imputation ( void );
void clique_sur_annuler_imputation ( void );
void supprimer_imputation ( void );
void supprimer_sous_imputation ( void );
void creation_liste_imputations ( void );
void creation_liste_imputation_combofix ( void );
void mise_a_jour_imputation ( void );
struct struct_imputation *ajoute_nouvelle_imputation ( gchar *imputation );
struct struct_sous_imputation *ajoute_nouvelle_sous_imputation ( gchar *sous_imputation,
								 struct struct_imputation *imputation );
gint recherche_imputation_par_nom ( struct struct_imputation *imputation,
				    gchar *nom_imputation );
gint recherche_sous_imputation_par_nom ( struct struct_sous_imputation *sous_imputation,
					 gchar *nom_sous_imputation );
gint recherche_imputation_par_no ( struct struct_imputation *imputation,
				   gint *no_imputation );
gint recherche_sous_imputation_par_no ( struct struct_sous_imputation *sous_imputation,
					gint *no_sous_imputation );
void fusion_categories_imputation ( void );
void calcule_total_montant_imputation ( void );
gchar *calcule_total_montant_imputation_par_compte ( gint imputation,
						     gint sous_imputation,
						     gint no_compte );
void appui_sur_ajout_imputation ( void );
void appui_sur_ajout_sous_imputation ( void );
void exporter_ib ( void );
void importer_ib ( void );
