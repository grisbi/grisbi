/*START_DECLARATION*/
gchar *tiers_name_by_no ( gint no_tiers, gboolean return_null );
struct struct_tiers *tiers_par_no ( gint no_tiers );
struct struct_tiers *tiers_par_nom ( gchar *nom_tiers, gboolean creer );
void calcule_total_montant_payee ( void );
void add_transaction_to_payee ( struct structure_operation * transaction, 
				struct struct_tiers * payee );
void remove_transaction_from_payee ( struct structure_operation * transaction,
				     struct struct_tiers * payee );
/*END_DECLARATION*/

