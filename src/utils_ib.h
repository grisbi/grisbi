/*START_DECLARATION*/
struct struct_imputation *imputation_par_no ( gint no_imputation );
struct struct_sous_imputation *sous_imputation_par_no ( gint no_imputation,
							gint no_sous_imputation );
struct struct_imputation *imputation_par_nom ( gchar *nom_ib,
					       gboolean creer,
					       gint type_ib,
					       gint no_derniere_sous_ib );
gchar *nom_imputation_par_no ( gint no_imputation,
			       gint no_sous_imputation );
gchar *nom_sous_imputation_par_no ( gint no_imputation,
				    gint no_sous_imputation );
struct struct_sous_imputation *sous_imputation_par_nom ( struct struct_imputation *imputation,
							 gchar *nom_sous_imputation,
							 gboolean creer );
void remove_transaction_from_budgetary_line ( struct structure_operation * transaction,
					      struct struct_imputation * budgetary_line,
					      struct struct_sous_imputation * sub_budgetary_line );
void add_transaction_to_budgetary_line ( struct structure_operation * transaction,
					 struct struct_imputation * budgetary_line,
					 struct struct_sous_imputation * sub_budgetary_line );
void calcule_total_montant_budgetary_line ( void );
void calcule_total_montant_payee ( void );
/*END_DECLARATION*/

