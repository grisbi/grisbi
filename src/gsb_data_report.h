#ifndef _GSB_DATA_REPORT_H
#define _GSB_DATA_REPORT_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/** \struct
 * describe an report category list or report budget list
 * the selected category/budget list in the report struct is a list of struct_report_category
 * we have here the numbers of categories/budgets selected, and for each one, the sub-categories/budgets selected
 * */
typedef struct
{
    gint div_number;
    GSList *sub_div_numbers;	/* list of number of sub-categories/budgets selected for the category/budget */

} struct_categ_budget_sel;


/* START_DECLARATION */
gboolean gsb_data_report_check_categ_budget_in_report ( GSList *list_struct_report,
							gint div_number,
							gint sub_div_number );
gint gsb_data_report_compare_position ( gint report_number_1,
					gint report_number_2 );
gint gsb_data_report_dup ( gint report_number );
gboolean gsb_data_report_free_categ_budget_struct (GSList *categ_budget_sel_list);
gint gsb_data_report_get_account_group_reports ( gint report_number );
GSList *gsb_data_report_get_account_numbers ( gint report_number );
gint gsb_data_report_get_account_show_amount ( gint report_number );
gint gsb_data_report_get_account_show_name ( gint report_number );
gint gsb_data_report_get_account_use_chosen ( gint report_number );
gint gsb_data_report_get_amount_comparison_currency ( gint report_number );
GSList *gsb_data_report_get_amount_comparison_list ( gint report_number );
gint gsb_data_report_get_amount_comparison_only_report_non_null ( gint report_number );
gint gsb_data_report_get_amount_comparison_used ( gint report_number );
gint gsb_data_report_get_append_in_payee ( gint report_number );
gint gsb_data_report_get_budget_currency ( gint report_number );
gint gsb_data_report_get_budget_detail_used ( gint report_number );
gint gsb_data_report_get_budget_show_budget_amount ( gint report_number );
gint gsb_data_report_get_budget_show_name ( gint report_number );
gint gsb_data_report_get_budget_show_sub_budget ( gint report_number );
gint gsb_data_report_get_budget_show_sub_budget_amount ( gint report_number );
gint gsb_data_report_get_budget_show_without_budget ( gint report_number );
GSList *gsb_data_report_get_budget_struct ( gint report_number );
gint gsb_data_report_get_budget_used ( gint report_number );
gint gsb_data_report_get_category_currency ( gint report_number );
gint gsb_data_report_get_category_detail_used ( gint report_number );
gint gsb_data_report_get_category_show_category_amount ( gint report_number );
gint gsb_data_report_get_category_show_name ( gint report_number );
gint gsb_data_report_get_category_show_sub_category ( gint report_number );
gint gsb_data_report_get_category_show_sub_category_amount ( gint report_number );
gint gsb_data_report_get_category_show_without_category ( gint report_number );
GSList *gsb_data_report_get_category_struct ( gint report_number );
gint gsb_data_report_get_category_used ( gint report_number );
gint gsb_data_report_get_column_title_show ( gint report_number );
gint gsb_data_report_get_column_title_type ( gint report_number );
gint gsb_data_report_get_currency_general ( gint report_number );
gint gsb_data_report_get_date_type ( gint report_number );
GSList *gsb_data_report_get_financial_year_list ( gint report_number );
gint gsb_data_report_get_financial_year_split ( gint report_number );
gint gsb_data_report_get_financial_year_type ( gint report_number );
GSList *gsb_data_report_get_method_of_payment_list ( gint report_number );
gint gsb_data_report_get_method_of_payment_used ( gint report_number );
gint gsb_data_report_get_not_detail_split ( gint report_number );
gint gsb_data_report_get_payee_currency ( gint report_number );
gint gsb_data_report_get_payee_detail_used ( gint report_number );
GSList *gsb_data_report_get_payee_numbers ( gint report_number );
gint gsb_data_report_get_payee_show_name ( gint report_number );
gint gsb_data_report_get_payee_show_payee_amount ( gint report_number );
gint gsb_data_report_get_payee_used ( gint report_number );
gint gsb_data_report_get_period_split ( gint report_number );
gint gsb_data_report_get_period_split_day ( gint report_number );
gint gsb_data_report_get_period_split_type ( gint report_number );
GDate *gsb_data_report_get_personal_date_end ( gint report_number );
GDate *gsb_data_report_get_personal_date_start ( gint report_number );
gint gsb_data_report_get_report_by_name ( const gchar *name );
gint gsb_data_report_get_report_can_click ( gint report_number );
GSList *gsb_data_report_get_report_list ( void );
gchar *gsb_data_report_get_report_name ( gint report_number );
gint gsb_data_report_get_report_number ( gpointer report_pointer );
gint gsb_data_report_get_show_r ( gint report_number );
gint gsb_data_report_get_show_report_bank_references ( gint report_number );
gint gsb_data_report_get_show_report_budget ( gint report_number );
gint gsb_data_report_get_show_report_category ( gint report_number );
gint gsb_data_report_get_show_report_date ( gint report_number );
gint gsb_data_report_get_show_report_financial_year ( gint report_number );
gint gsb_data_report_get_show_report_marked ( gint report_number );
gint gsb_data_report_get_show_report_method_of_payment ( gint report_number );
gint gsb_data_report_get_show_report_method_of_payment_content ( gint report_number );
gint gsb_data_report_get_show_report_note ( gint report_number );
gint gsb_data_report_get_show_report_payee ( gint report_number );
gint gsb_data_report_get_show_report_sub_budget ( gint report_number );
gint gsb_data_report_get_show_report_sub_category ( gint report_number );
gint gsb_data_report_get_show_report_transaction_amount ( gint report_number );
gint gsb_data_report_get_show_report_transaction_number ( gint report_number );
gint gsb_data_report_get_show_report_transactions ( gint report_number );
gint gsb_data_report_get_show_report_value_date ( gint report_number );
gint gsb_data_report_get_show_report_voucher ( gint report_number );
gint gsb_data_report_get_sorting_report ( gint report_number );
GSList *gsb_data_report_get_sorting_type ( gint report_number );
gint gsb_data_report_get_split_credit_debit ( gint report_number );
GSList *gsb_data_report_get_text_comparison_list ( gint report_number );
gint gsb_data_report_get_text_comparison_used ( gint report_number );
GSList *gsb_data_report_get_transfer_account_numbers ( gint report_number );
gint gsb_data_report_get_transfer_choice ( gint report_number );
gint gsb_data_report_get_transfer_reports_only ( gint report_number );
gint gsb_data_report_get_use_financial_year ( gint report_number );
gboolean gsb_data_report_init_variables ( void );
gint gsb_data_report_max_number ( void );
gboolean gsb_data_report_move_report ( gint report_number,
				       gint dest_report_number );
gint gsb_data_report_new ( gchar *name );
gint gsb_data_report_new_with_number ( gint number );
gboolean gsb_data_report_remove ( gint no_report );
gboolean gsb_data_report_set_account_group_reports ( gint report_number,
						     gint account_group_reports );
gboolean gsb_data_report_set_account_numbers ( gint report_number,
					       GSList *account_numbers );
gboolean gsb_data_report_set_account_show_amount ( gint report_number,
						   gint account_show_amount );
gboolean gsb_data_report_set_account_show_name ( gint report_number,
						 gint account_show_name );
gboolean gsb_data_report_set_account_use_chosen ( gint report_number,
						   gint account_use_chosen );
gboolean gsb_data_report_set_amount_comparison_currency ( gint report_number,
							  gint amount_comparison_currency );
gboolean gsb_data_report_set_amount_comparison_list ( gint report_number,
						      GSList *amount_comparison_list );
gboolean gsb_data_report_set_amount_comparison_only_report_non_null ( gint report_number,
								      gint amount_comparison_only_report_non_null );
gboolean gsb_data_report_set_amount_comparison_used ( gint report_number,
						      gint amount_comparison_used );
gboolean gsb_data_report_set_append_in_payee ( gint report_number,
					       gint append_in_payee );
gboolean gsb_data_report_set_budget_currency ( gint report_number,
					       gint budget_currency );
gboolean gsb_data_report_set_budget_detail_used ( gint report_number,
						  gint budget_detail_used );
gboolean gsb_data_report_set_budget_show_budget_amount ( gint report_number,
							 gint budget_show_budget_amount );
gboolean gsb_data_report_set_budget_show_name ( gint report_number,
						gint budget_show_name );
gboolean gsb_data_report_set_budget_show_sub_budget ( gint report_number,
						      gint budget_show_sub_budget );
gboolean gsb_data_report_set_budget_show_sub_budget_amount ( gint report_number,
							 gint budget_show_sub_budget_amount );
gboolean gsb_data_report_set_budget_show_without_budget ( gint report_number,
							  gint budget_show_without_budget );
gboolean gsb_data_report_set_budget_struct ( gint report_number,
					     GSList *budget_select_struct );
gboolean gsb_data_report_set_budget_used ( gint report_number,
					   gint budget_used );
gboolean gsb_data_report_set_category_currency ( gint report_number,
						 gint category_currency );
gboolean gsb_data_report_set_category_detail_used ( gint report_number,
						    gint category_detail_used );
gboolean gsb_data_report_set_category_show_category_amount ( gint report_number,
							     gint category_show_category_amount );
gboolean gsb_data_report_set_category_show_name ( gint report_number,
						  gint category_show_name );
gboolean gsb_data_report_set_category_show_sub_category ( gint report_number,
							  gint category_show_sub_category );
gboolean gsb_data_report_set_category_show_sub_category_amount ( gint report_number,
								 gint category_show_sub_category_amount );
gboolean gsb_data_report_set_category_show_without_category ( gint report_number,
							      gint category_show_without_category );
gboolean gsb_data_report_set_category_struct ( gint report_number,
					       GSList *categ_select_struct );
gboolean gsb_data_report_set_category_used ( gint report_number,
					     gint category_used );
gboolean gsb_data_report_set_column_title_show ( gint report_number,
						 gint column_title_show );
gboolean gsb_data_report_set_column_title_type ( gint report_number,
						 gint column_title_type );
gboolean gsb_data_report_set_currency_general ( gint report_number,
						gint currency_general );
gboolean gsb_data_report_set_date_type ( gint report_number,
					 gint date_type );
gboolean gsb_data_report_set_financial_year_list ( gint report_number,
						   GSList *financial_year_list );
gboolean gsb_data_report_set_financial_year_split ( gint report_number,
						    gint financial_year_split );
gboolean gsb_data_report_set_financial_year_type ( gint report_number,
						   gint financial_year_type );
gboolean gsb_data_report_set_method_of_payment_list ( gint report_number,
						      GSList *method_of_payment_list );
gboolean gsb_data_report_set_method_of_payment_used ( gint report_number,
						      gint method_of_payment_used );
gboolean gsb_data_report_set_not_detail_split ( gint report_number,
						    gint not_detail_split );
gboolean gsb_data_report_set_payee_currency ( gint report_number,
					      gint payee_currency );
gboolean gsb_data_report_set_payee_detail_used ( gint report_number,
						 gint payee_detail_used );
gboolean gsb_data_report_set_payee_numbers ( gint report_number,
					     GSList *payee_numbers );
gboolean gsb_data_report_set_payee_show_name ( gint report_number,
					       gint payee_show_name );
gboolean gsb_data_report_set_payee_show_payee_amount ( gint report_number,
						       gint payee_show_payee_amount );
gboolean gsb_data_report_set_payee_used ( gint report_number,
					  gint payee_used );
gboolean gsb_data_report_set_period_split ( gint report_number,
					    gint period_split );
gboolean gsb_data_report_set_period_split_day ( gint report_number,
						gint period_split_day );
gboolean gsb_data_report_set_period_split_type ( gint report_number,
						 gint period_split_type );
gboolean gsb_data_report_set_personal_date_end ( gint report_number,
						 GDate *personal_date_end );
gboolean gsb_data_report_set_personal_date_start ( gint report_number,
						   GDate *personal_date_start );
gboolean gsb_data_report_set_report_can_click ( gint report_number,
						gint report_can_click );
gboolean gsb_data_report_set_report_name ( gint report_number,
					   const gchar *report_name );
gboolean gsb_data_report_set_show_r ( gint report_number,
				      gint show_r );
gboolean gsb_data_report_set_show_report_bank_references ( gint report_number,
							   gint show_report_bank_references );
gboolean gsb_data_report_set_show_report_budget ( gint report_number,
						  gint show_report_budget );
gboolean gsb_data_report_set_show_report_category ( gint report_number,
						    gint show_report_category );
gboolean gsb_data_report_set_show_report_date ( gint report_number,
						gint show_report_date );
gboolean gsb_data_report_set_show_report_financial_year ( gint report_number,
							  gint show_report_financial_year );
gboolean gsb_data_report_set_show_report_marked ( gint report_number,
						  gint show_report_marked );
gboolean gsb_data_report_set_show_report_method_of_payment ( gint report_number,
							     gint show_report_method_of_payment );
gboolean gsb_data_report_set_show_report_method_of_payment_content ( gint report_number,
								     gint show_report_method_of_payment_content );
gboolean gsb_data_report_set_show_report_note ( gint report_number,
						gint show_report_note );
gboolean gsb_data_report_set_show_report_payee ( gint report_number,
						 gint show_report_payee );
gboolean gsb_data_report_set_show_report_sub_budget ( gint report_number,
						      gint show_report_sub_budget );
gboolean gsb_data_report_set_show_report_sub_category ( gint report_number,
							gint show_report_sub_category );
gboolean gsb_data_report_set_show_report_transaction_amount ( gint report_number,
							      gint show_report_transaction_amount );
gboolean gsb_data_report_set_show_report_transaction_number ( gint report_number,
							      gint show_report_transaction_number );
gboolean gsb_data_report_set_show_report_transactions ( gint report_number,
							gint show_report_transactions );
gboolean gsb_data_report_set_show_report_value_date ( gint report_number,
						      gint show_report_value_date );
gboolean gsb_data_report_set_show_report_voucher ( gint report_number,
						   gint show_report_voucher );
gboolean gsb_data_report_set_sorting_report ( gint report_number,
					      gint sorting_report );
gboolean gsb_data_report_set_sorting_type ( gint report_number,
					    GSList *sorting_type );
gboolean gsb_data_report_set_split_credit_debit ( gint report_number,
						  gint split_credit_debit );
gboolean gsb_data_report_set_text_comparison_list ( gint report_number,
						    GSList *text_comparison_list );
gboolean gsb_data_report_set_text_comparison_used ( gint report_number,
						    gint text_comparison_used );
gboolean gsb_data_report_set_transfer_account_numbers ( gint report_number,
							GSList *transfer_account_numbers );
gboolean gsb_data_report_set_transfer_choice ( gint report_number,
					       gint transfer_choice );
gboolean gsb_data_report_set_transfer_reports_only ( gint report_number,
						     gint transfer_reports_only );
gboolean gsb_data_report_set_use_financial_year ( gint report_number,
						  gint use_financial_year );
/* END_DECLARATION */

#endif
