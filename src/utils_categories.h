/*START_DECLARATION*/
struct struct_categ *categ_par_no ( gint no_categorie );
struct struct_categ *categ_par_nom ( gchar *nom_categ,
				     gboolean creer,
				     gint type_categ,
				     gint no_derniere_sous_categ );
gchar *nom_categ_par_no ( gint no_categorie,
			  gint no_sous_categorie );
gchar *nom_sous_categ_par_no ( gint no_categorie,
			       gint no_sous_categorie );
struct struct_sous_categ *sous_categ_par_no ( gint no_categorie,
					      gint no_sous_categorie );
struct struct_sous_categ *sous_categ_par_nom ( struct struct_categ *categ,
					       gchar *nom_sous_categ,
					       gboolean creer );
void remove_transaction_from_category ( struct structure_operation * transaction,
					struct struct_categ * category,
					struct struct_sous_categ * sub_category );
void add_transaction_to_category ( struct structure_operation * transaction,
				   struct struct_categ * category,
				   struct struct_sous_categ * sub_category );
void calcule_total_montant_categ ( void );
void update_category_in_trees ( struct struct_categ * category, 
				struct struct_sous_categ * sub_category );
/*END_DECLARATION*/

