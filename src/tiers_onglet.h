/*START_DECLARATION*/
struct struct_tiers *ajoute_nouveau_tiers ( gchar *tiers );
gboolean changement_taille_liste_tiers ( GtkWidget *clist,
					 GtkAllocation *allocation,
					 gpointer null );
gint classement_alphabetique_tree ( GtkWidget *tree,
				    GtkCListRow *ligne_1,
				    GtkCListRow *ligne_2 );
void creation_liste_tiers_combofix ( void );
void mise_a_jour_combofix_tiers ( void );
GtkWidget *onglet_tiers ( void );
void remplit_arbre_tiers ( void );
/*END_DECLARATION*/

