#ifndef _GSB_ACCOUNT_H
#define _GSB_ACCOUNT_H (1)


/* START_INCLUDE_H */
#include "gsb_data_account.h"
#include "structures.h"
/* END_INCLUDE_H */


/** \struct
 * kind of the account
 * */

enum _kind_account
{
    GSB_TYPE_BANK        = 0,
    GSB_TYPE_CASH        = 1,
    GSB_TYPE_LIABILITIES = 2,
    GSB_TYPE_ASSET       = 3
};
typedef enum _kind_account	kind_account;


/* START_DECLARATION */
gboolean gsb_data_account_delete ( gint no_account );
gint gsb_data_account_first_number ( void );
gpointer gsb_data_account_get_account_button ( gint no_account );
gint gsb_data_account_get_accounts_amount ( void );
gint gsb_data_account_get_bank ( gint no_account );
gchar *gsb_data_account_get_bank_account_key ( gint no_account );
gchar *gsb_data_account_get_bank_account_number ( gint no_account );
gchar *gsb_data_account_get_bank_branch_code ( gint no_account );
gint gsb_data_account_get_closed_account ( gint no_account );
gint gsb_data_account_get_column_sort ( gint no_account,
				   gint no_column );
gchar *gsb_data_account_get_comment ( gint no_account );
gint gsb_data_account_get_currency ( gint no_account );
gint gsb_data_account_get_current_account ( void );
gdouble gsb_data_account_get_current_balance ( gint no_account );
GDate *gsb_data_account_get_current_reconcile_date ( gint no_account );
gint gsb_data_account_get_current_transaction_number ( gint no_account );
gint gsb_data_account_get_default_credit ( gint no_account );
gint gsb_data_account_get_default_debit ( gint no_account );
struct organisation_formulaire *gsb_data_account_get_form_organization ( gint no_account );
gchar *gsb_data_account_get_holder_address ( gint no_account );
gchar *gsb_data_account_get_holder_name ( gint no_account );
gchar *gsb_data_account_get_id ( gint no_account );
gdouble gsb_data_account_get_init_balance ( gint no_account );
kind_account gsb_data_account_get_kind ( gint no_account );
GSList *gsb_data_account_get_list_accounts ( void );
gdouble gsb_data_account_get_marked_balance ( gint no_account );
GSList *gsb_data_account_get_method_payment_list ( gint no_account );
gdouble gsb_data_account_get_mini_balance_authorized ( gint no_account );
gboolean gsb_data_account_get_mini_balance_authorized_message ( gint no_account );
gdouble gsb_data_account_get_mini_balance_wanted ( gint no_account );
gboolean gsb_data_account_get_mini_balance_wanted_message ( gint no_account );
gchar *gsb_data_account_get_name ( gint no_account );
gint gsb_data_account_get_nb_rows ( gint no_account );
gint gsb_data_account_get_no_account ( gpointer account_ptr );
gint gsb_data_account_get_no_account_by_name ( gchar *account_name );
gboolean gsb_data_account_get_r ( gint no_account );
gdouble gsb_data_account_get_reconcile_balance ( gint no_account );
gint gsb_data_account_get_reconcile_last_number ( gint no_account );
gint gsb_data_account_get_reconcile_sort_type ( gint no_account );
gint gsb_data_account_get_sort_column ( gint no_account );
GSList *gsb_data_account_get_sort_list ( gint no_account );
gint gsb_data_account_get_sort_type ( gint no_account );
gint gsb_data_account_get_split_neutral_payment ( gint no_account );
gboolean gsb_data_account_get_update_list ( gint no_account );
gdouble gsb_data_account_get_vertical_adjustment_value ( gint no_account );
gboolean gsb_data_account_init_variables ( void );
gint gsb_data_account_new ( kind_account account_kind );
gboolean gsb_data_account_reorder ( GSList *new_order );
gboolean gsb_data_account_set_account_button ( gint no_account,
					  gpointer account_button );
gint gsb_data_account_set_account_number ( gint no_account,
				      gint new_no );
gboolean gsb_data_account_set_bank ( gint no_account,
				gint bank );
gboolean gsb_data_account_set_bank_account_key ( gint no_account,
					    gchar *bank_account_key );
gboolean gsb_data_account_set_bank_account_number ( gint no_account,
					       gchar *bank_account_number );
gboolean gsb_data_account_set_bank_branch_code ( gint no_account,
					    gchar *bank_branch_code );
gboolean gsb_data_account_set_closed_account ( gint no_account,
					  gint closed_account );
gboolean gsb_data_account_set_column_sort ( gint no_account,
				       gint no_column,
				       gint column_sort );
gboolean gsb_data_account_set_comment ( gint no_account,
				   gchar *comment );
gboolean gsb_data_account_set_currency ( gint no_account,
				    gint currency );
gboolean gsb_data_account_set_current_account ( gint no_current_account );
gboolean gsb_data_account_set_current_balance ( gint no_account,
					   gdouble balance );
gboolean gsb_data_account_set_current_reconcile_date ( gint no_account,
						  GDate *date );
gboolean gsb_data_account_set_current_transaction_number ( gint no_account,
						      gint transaction_number );
gboolean gsb_data_account_set_default_credit ( gint no_account,
					  gint default_credit );
gboolean gsb_data_account_set_default_debit ( gint no_account,
					 gint default_debit );
gboolean gsb_data_account_set_form_organization ( gint no_account,
					     struct organisation_formulaire *form_organization );
gboolean gsb_data_account_set_holder_address ( gint no_account,
					  gchar *holder_address );
gboolean gsb_data_account_set_holder_name ( gint no_account,
				       gchar *holder_name );
gboolean gsb_data_account_set_id ( gint no_account,
			      gchar *id );
gboolean gsb_data_account_set_init_balance ( gint no_account,
					gdouble balance );
gboolean gsb_data_account_set_kind ( gint no_account,
				kind_account account_kind );
gboolean gsb_data_account_set_marked_balance ( gint no_account,
					  gdouble balance );
gboolean gsb_data_account_set_method_payment_list ( gint no_account,
					       GSList *list );
gboolean gsb_data_account_set_mini_balance_authorized ( gint no_account,
						   gdouble balance );
gboolean gsb_data_account_set_mini_balance_authorized_message ( gint no_account,
							   gboolean value );
gboolean gsb_data_account_set_mini_balance_wanted ( gint no_account,
					       gdouble balance );
gboolean gsb_data_account_set_mini_balance_wanted_message ( gint no_account,
						       gboolean value );
gboolean gsb_data_account_set_name ( gint no_account,
				gchar *name );
gboolean gsb_data_account_set_nb_rows ( gint no_account,
				   gint nb_rows );
gboolean gsb_data_account_set_r ( gint no_account,
			     gboolean show_r );
gboolean gsb_data_account_set_reconcile_balance ( gint no_account,
					     gdouble balance );
gboolean gsb_data_account_set_reconcile_last_number ( gint no_account,
						 gint number );
gboolean gsb_data_account_set_reconcile_sort_type ( gint no_account,
					       gint sort_type );
gboolean gsb_data_account_set_sort_column ( gint no_account,
				       gint sort_column );
gboolean gsb_data_account_set_sort_list ( gint no_account,
				     GSList *list );
gboolean gsb_data_account_set_sort_type ( gint no_account,
					  gint sort_type );
gboolean gsb_data_account_set_split_neutral_payment ( gint no_account,
						 gint split_neutral_payment );
gboolean gsb_data_account_set_update_list ( gint no_account,
				       gboolean value );
gboolean gsb_data_account_set_vertical_adjustment_value ( gint no_account,
						     gint vertical_adjustment_value );
/* END_DECLARATION */


#endif
