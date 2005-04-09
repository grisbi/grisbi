/*START_DECLARATION*/
void add_transaction_to_payee ( struct structure_operation * transaction,
				struct struct_tiers * payee );
struct struct_tiers *ajoute_nouveau_tiers ( gchar *tiers );
void calcule_total_montant_payee ( void );
void remove_transaction_from_payee ( struct structure_operation * transaction,
				     struct struct_tiers * payee );
gchar *tiers_name_by_no ( gint no_tiers,
			  gboolean return_null );
struct struct_tiers *tiers_par_no ( gint no_tiers );
struct struct_tiers *tiers_par_nom ( gchar *nom_tiers,
				     gboolean creer );
/*END_DECLARATION*/

