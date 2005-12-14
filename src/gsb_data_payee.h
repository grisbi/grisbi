#ifndef _GSB_PAYEE_DATA_H
#define _GSB_PAYEE_DATA_H (1)



/* START_DECLARATION */
void gsb_data_payee_add_transaction_to_payee ( gint transaction_number );
gdouble gsb_data_payee_get_balance ( gint no_payee );
gchar *gsb_data_payee_get_description ( gint no_payee );
gpointer gsb_data_payee_get_empty_payee ( void );
gchar *gsb_data_payee_get_name ( gint no_payee,
				 gboolean can_return_null);
GSList *gsb_data_payee_get_name_and_report_list ( void );
GSList *gsb_data_payee_get_name_list ( void );
gint gsb_data_payee_get_nb_transactions ( gint no_payee );
gint gsb_data_payee_get_no_payee ( gpointer payee_ptr );
gint gsb_data_payee_get_number_by_name ( gchar *name,
					 gboolean create );
GSList *gsb_data_payee_get_payees_list ( void );
gpointer gsb_data_payee_get_structure ( gint no_payee );
gboolean gsb_data_payee_init_variables ( void );
gint gsb_data_payee_new ( gchar *name );
gboolean gsb_data_payee_remove ( gint no_payee );
void gsb_data_payee_remove_transaction_from_payee ( gint transaction_number );
gboolean gsb_data_payee_set_description ( gint no_payee,
					  const gchar *description );
gboolean gsb_data_payee_set_name ( gint no_payee,
			      const gchar *name );
gint gsb_data_payee_set_new_number ( gint no_payee,
				     gint new_no_payee );
void gsb_data_payee_update_counters ( void );
/* END_DECLARATION */
#endif
