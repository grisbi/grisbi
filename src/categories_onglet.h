/*START_DECLARATION*/
struct struct_categ *categ_par_no ( gint no_categorie );
struct struct_categ *categ_par_nom ( gchar *nom_categ,
				     gboolean creer,
				     gint type_categ,
				     gint no_derniere_sous_categ );
void creation_liste_categ_combofix ( void );
void creation_liste_categories ( void );
void mise_a_jour_combofix_categ ( void );
gchar *nom_categ_par_no ( gint no_categorie,
			  gint no_sous_categorie );
gchar *nom_sous_categ_par_no ( gint no_categorie,
			       gint no_sous_categorie );
GtkWidget *onglet_categories ( void );
void remplit_arbre_categ ( void );
struct struct_sous_categ *sous_categ_par_nom ( struct struct_categ *categ,
					       gchar *nom_sous_categ,
					       gboolean creer );
/*END_DECLARATION*/

