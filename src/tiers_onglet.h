#define START_DECLARATION
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
gchar *tiers_name_by_no ( gint no_tiers,
			  gboolean return_null );
struct struct_tiers *tiers_par_no ( gint no_tiers );
struct struct_tiers *tiers_par_nom ( gchar *nom_tiers,
				     gboolean creer );
#define END_DECLARATION

