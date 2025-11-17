#ifndef _GSB_DATA_MIX_H
#define _GSB_DATA_MIX_H (1)


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
void 			gsb_data_mix_clone_current_transaction 			(void);
void 			gsb_data_mix_delete_current_transaction 		(void);
void 			gsb_data_mix_edit_current_transaction 			(void);
gint 			gsb_data_mix_get_account_number 				(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_account_number_transfer 		(gint transaction_number,
																 gboolean is_transaction);
GsbReal 		gsb_data_mix_get_amount 						(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_automatic_transaction 			(gint transaction_number,
																 gboolean is_transaction);
const gchar *	gsb_data_mix_get_bank_references 				(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_budgetary_number 				(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_category_number 				(gint transaction_number,
																 gboolean is_transaction);
GSList *		gsb_data_mix_get_children 						(gint transaction_number,
																 gboolean return_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_currency_number 				(gint transaction_number,
																 gboolean is_transaction);
const GDate *	gsb_data_mix_get_date 							(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_financial_year_number 			(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_method_of_payment_number 		(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_mother_transaction_number 		(gint transaction_number,
																 gboolean is_transaction);
const gchar *	gsb_data_mix_get_notes 							(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_payee_number 					(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_split_of_transaction 			(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_sub_budgetary_number 			(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_sub_category_number 			(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_contra_transaction_number 		(gint transaction_number,
																 gboolean is_transaction);
const gchar *	gsb_data_mix_get_voucher 						(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_get_white_line 					(gint transaction_number,
																 gboolean is_transaction);
gint 			gsb_data_mix_new_transaction 					(gint account_number,
																 gboolean is_transaction);
void 			gsb_data_mix_new_transaction_by_menu 			(void);
gboolean 		gsb_data_mix_set_amount 						(gint transaction_number,
																 GsbReal amount,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_bank_references 				(gint transaction_number,
																 const gchar *bank_references,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_budgetary_number 				(gint transaction_number,
																 gint budgetary_number,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_category_number 				(gint transaction_number,
																 gint no_category,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_currency_number 				(gint transaction_number,
																 gint no_currency,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_date 							(gint transaction_number,
																 GDate *date,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_financial_year_number 			(gint transaction_number,
																 gint financial_year_number,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_method_of_payment_content 		(gint transaction_number,
																 const gchar *method_of_payment_content,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_method_of_payment_number 		(gint transaction_number,
																 gint number,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_mother_transaction_number 		(gint transaction_number,
																 gint mother_transaction_number,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_notes 							(gint transaction_number,
																 const gchar *notes,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_payee_number 					(gint transaction_number,
																 gint no_party,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_split_of_transaction 			(gint transaction_number,
																 gint is_split,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_sub_budgetary_number 			(gint transaction_number,
																 gint sub_budgetary_number,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_sub_category_number 			(gint transaction_number,
																 gint no_sub_category,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_contra_transaction_number	(gint transaction_number,
																 gint contra_transaction_number,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_value_date 					(gint transaction_number,
																 GDate *date,
																 gboolean is_transaction);
gboolean 		gsb_data_mix_set_voucher 						(gint transaction_number,
																 const gchar *voucher,
																 gboolean is_transaction);
/* END_DECLARATION */
#endif
