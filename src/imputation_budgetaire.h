#define START_DECLARATION
void creation_liste_imputation_combofix ( void );
struct struct_imputation *imputation_par_no ( gint no_imputation );
struct struct_imputation *imputation_par_nom ( gchar *nom_ib,
					       gboolean creer,
					       gint type_ib,
					       gint no_derniere_sous_ib );
void mise_a_jour_combofix_imputation ( void );
gchar *nom_imputation_par_no ( gint no_imputation,
			       gint no_sous_imputation );
gchar *nom_sous_imputation_par_no ( gint no_imputation,
				    gint no_sous_imputation );
GtkWidget *onglet_imputations ( void );
void remplit_arbre_imputation ( void );
struct struct_sous_imputation *sous_imputation_par_nom ( struct struct_imputation *imputation,
							 gchar *nom_sous_imputation,
							 gboolean creer );
#define END_DECLARATION

