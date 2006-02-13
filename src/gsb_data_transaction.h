#ifndef _GSB_TRANSACTION_DATA_H
#define _GSB_TRANSACTION_DATA_H (1)


/* START_INCLUDE_H */
#include "gsb_data_transaction.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_data_transaction_copy_transaction ( gint source_transaction_number,
						 gint target_transaction_number );
gint gsb_data_transaction_get_account_number ( gint no_transaction );
gint gsb_data_transaction_get_account_number_transfer ( gint no_transaction );
gdouble gsb_data_transaction_get_adjusted_amount ( gint no_transaction );
gdouble gsb_data_transaction_get_adjusted_amount_for_currency ( gint no_transaction,
								gint return_currency_number );
gdouble gsb_data_transaction_get_amount ( gint no_transaction );
gint gsb_data_transaction_get_automatic_transaction ( gint no_transaction );
gchar *gsb_data_transaction_get_bank_references ( gint no_transaction );
gint gsb_data_transaction_get_breakdown_of_transaction ( gint no_transaction );
gint gsb_data_transaction_get_budgetary_number ( gint no_transaction );
gint gsb_data_transaction_get_category_number ( gint no_transaction );
gint gsb_data_transaction_get_change_between ( gint no_transaction );
gint gsb_data_transaction_get_currency_number ( gint no_transaction );
GDate *gsb_data_transaction_get_date ( gint no_transaction );
gdouble gsb_data_transaction_get_exchange_fees ( gint no_transaction );
gdouble gsb_data_transaction_get_exchange_rate ( gint no_transaction );
gint gsb_data_transaction_get_financial_year_number ( gint no_transaction );
gint gsb_data_transaction_get_last_number (void);
gint gsb_data_transaction_get_marked_transaction ( gint no_transaction );
gchar *gsb_data_transaction_get_method_of_payment_content ( gint no_transaction );
gint gsb_data_transaction_get_method_of_payment_number ( gint no_transaction );
gint gsb_data_transaction_get_mother_transaction_number ( gint no_transaction );
gchar *gsb_data_transaction_get_notes ( gint no_transaction );
gint gsb_data_transaction_get_party_number ( gint no_transaction );
gpointer gsb_data_transaction_get_pointer_to_transaction ( gint transaction_number );
gint gsb_data_transaction_get_reconcile_number ( gint no_transaction );
gchar *gsb_data_transaction_get_str_adjusted_amount_for_currency ( gint no_transaction,
								   gint return_currency_number );
gchar *gsb_data_transaction_get_str_amount ( gint no_transaction,
					     gboolean absolute );
gint gsb_data_transaction_get_sub_budgetary_number ( gint no_transaction );
gint gsb_data_transaction_get_sub_category_number ( gint no_transaction );
gchar *gsb_data_transaction_get_transaction_id ( gint no_transaction );
gint gsb_data_transaction_get_transaction_number ( gpointer transaction_pointer );
gint gsb_data_transaction_get_transaction_number_transfer ( gint no_transaction );
GSList *gsb_data_transaction_get_transactions_list ( void );
GDate *gsb_data_transaction_get_value_date ( gint no_transaction );
gchar *gsb_data_transaction_get_voucher ( gint no_transaction );
gboolean gsb_data_transaction_init_variables ( void );
gint gsb_data_transaction_new_transaction ( gint no_account );
gint gsb_data_transaction_new_transaction_with_number ( gint no_account,
							gint transaction_number );
gint gsb_data_transaction_new_white_line ( gint mother_transaction_number);
gboolean gsb_data_transaction_remove_transaction ( gint transaction_number );
gboolean gsb_data_transaction_set_account_number ( gint no_transaction,
						   gint no_account );
gboolean gsb_data_transaction_set_account_number_transfer ( gint no_transaction,
							    gint account_number_transfer );
gboolean gsb_data_transaction_set_amount ( gint no_transaction,
					   gdouble amount );
gboolean gsb_data_transaction_set_automatic_transaction ( gint no_transaction,
							  gint automatic_transaction );
gboolean gsb_data_transaction_set_bank_references ( gint no_transaction,
						    const gchar *bank_references );
gboolean gsb_data_transaction_set_breakdown_of_transaction ( gint no_transaction,
							     gint is_breakdown );
gboolean gsb_data_transaction_set_budgetary_number ( gint no_transaction,
						     gint budgetary_number );
gboolean gsb_data_transaction_set_category_number ( gint no_transaction,
						    gint no_category );
gboolean gsb_data_transaction_set_change_between ( gint no_transaction,
						   gint value );
gboolean gsb_data_transaction_set_currency_number ( gint no_transaction,
						    gint no_currency );
gboolean gsb_data_transaction_set_date ( gint no_transaction,
					 GDate *date );
gboolean gsb_data_transaction_set_exchange_fees ( gint no_transaction,
						  gdouble rate );
gboolean gsb_data_transaction_set_exchange_rate ( gint no_transaction,
						  gdouble rate );
gboolean gsb_data_transaction_set_financial_year_number ( gint no_transaction,
							  gint financial_year_number );
gboolean gsb_data_transaction_set_marked_transaction ( gint no_transaction,
						       gint marked_transaction );
gboolean gsb_data_transaction_set_method_of_payment_content ( gint no_transaction,
							      const gchar *method_of_payment_content );
gboolean gsb_data_transaction_set_method_of_payment_number ( gint no_transaction,
							     gint number );
gboolean gsb_data_transaction_set_mother_transaction_number ( gint no_transaction,
							      gint mother_transaction_number );
gboolean gsb_data_transaction_set_notes ( gint no_transaction,
					  const gchar *notes );
gboolean gsb_data_transaction_set_party_number ( gint no_transaction,
						 gint no_party );
gboolean gsb_data_transaction_set_reconcile_number ( gint no_transaction,
						     gint reconcile_number );
gboolean gsb_data_transaction_set_sub_budgetary_number ( gint no_transaction,
							 gint sub_budgetary_number );
gboolean gsb_data_transaction_set_sub_category_number ( gint no_transaction,
							gint no_sub_category );
gboolean gsb_data_transaction_set_transaction_id ( gint no_transaction,
						   const gchar *transaction_id );
gboolean gsb_data_transaction_set_transaction_number_transfer ( gint no_transaction,
							 gint transaction_number_transfer );
gboolean gsb_data_transaction_set_value_date ( gint no_transaction,
					       GDate *date );
gboolean gsb_data_transaction_set_voucher ( gint no_transaction,
					    const gchar *voucher );
/* END_DECLARATION */


#endif

