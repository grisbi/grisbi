/*START_DECLARATION*/
gchar *tiers_name_by_no ( gint no_tiers,
			  gboolean return_null );
struct struct_tiers *tiers_par_no ( gint no_tiers );
struct struct_tiers *tiers_par_nom ( gchar *nom_tiers,
				     gboolean creer );
/*END_DECLARATION*/

