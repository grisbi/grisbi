#ifndef _GSB_BUDGET_DATA_H
#define _GSB_BUDGET_DATA_H (1)

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_data_budget_add_transaction_to_budget ( gint transaction_number,
						 gint budget_id,
						 gint sub_budget_id );
gsb_real gsb_data_budget_get_balance ( gint no_budget );
GSList *gsb_data_budget_get_budgets_list ( void );
gsb_real gsb_data_budget_get_direct_balance ( gint no_budget );
gpointer gsb_data_budget_get_empty_budget ( void );
gchar *gsb_data_budget_get_name ( gint no_budget,
				  gint no_sub_budget,
				  const gchar *return_value_error );
GSList *gsb_data_budget_get_name_list ( gboolean set_debit,
					gboolean set_credit );
gint gsb_data_budget_get_nb_direct_transactions ( gint no_budget );
gint gsb_data_budget_get_nb_transactions ( gint no_budget );
gint gsb_data_budget_get_no_budget ( gpointer budget_ptr );
gint gsb_data_budget_get_no_sub_budget ( gpointer sub_budget_ptr );
gint gsb_data_budget_get_number_by_name ( const gchar *name,
					  gboolean create,
					  gint budget_type );
gpointer gsb_data_budget_get_structure ( gint no_budget );
gsb_real gsb_data_budget_get_sub_budget_balance ( gint no_budget,
						  gint no_sub_budget );
GSList *gsb_data_budget_get_sub_budget_list ( gint no_budget );
gchar *gsb_data_budget_get_sub_budget_name ( gint no_budget,
					     gint no_sub_budget,
					     const gchar *return_value_error );
gint gsb_data_budget_get_sub_budget_nb_transactions ( gint no_budget,
						      gint no_sub_budget );
gint gsb_data_budget_get_sub_budget_number_by_name ( gint budget_number,
						     const gchar *name,
						     gboolean create );
gpointer gsb_data_budget_get_sub_budget_structure ( gint no_budget,
						    gint no_sub_budget );
gint gsb_data_budget_get_type ( gint no_budget );
gboolean gsb_data_budget_init_variables ( void );
gint gsb_data_budget_new_sub_budget_with_number ( gint number,
						  gint budget_number);
gint gsb_data_budget_new_with_number ( gint number );
gboolean gsb_data_budget_remove ( gint no_budget );
void gsb_data_budget_remove_transaction_from_budget ( gint transaction_number );
void gsb_data_budget_set_budget_from_string ( gint transaction_number,
					      const gchar *string,
					      gboolean is_transaction );
gboolean gsb_data_budget_set_name ( gint no_budget,
				    const gchar *name );
gboolean gsb_data_budget_set_sub_budget_name ( gint no_budget,
					       gint no_sub_budget,
					       const gchar *name );
gboolean gsb_data_budget_set_type ( gint no_budget,
				    gint budget_type );
gboolean gsb_data_budget_sub_budget_remove ( gint no_budget,
					     gint no_sub_budget );
void gsb_data_budget_update_counters ( void );
gchar * gsb_debug_duplicate_budget_check ();
gboolean gsb_debug_duplicate_budget_fix ();
/* END_DECLARATION */
#endif
