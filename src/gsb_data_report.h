#ifndef _GSB_DATA_REPORT_H
#define _GSB_DATA_REPORT_H (1)

#include <glib.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/** \struct
 * describe an report category list or report budget list
 * the selected category/budget list in the report struct is a list of struct_report_category
 * we have here the numbers of categories/budgets selected, and for each one, the sub-categories/budgets selected
 * */
typedef struct _CategBudgetSel	CategBudgetSel;
typedef struct _SearchDataReport SearchDataReport;
typedef struct _ReportStruct	ReportStruct;

/** \ReportStruct
 * describe an report
 **/
struct _ReportStruct
{
    /** @name general stuff */
    gint report_number;
    gchar *report_name;
	gboolean compl_name_used;						/* TRUE si utilisation d'un complément au nom du rapport */
	gint compl_name_function;						/* Date ou Date + heure système */
	gint compl_name_position;						/* Devant, derrière ou dessous le nom du rapport */
	gchar *export_pdf_name;							/* Nom du fichier pour l'export pdf */
	gboolean search_report;							/* type recherche à ne pas sauvegarder */

    /** @name what we show of the transactions */
    gint show_m;                                    /**< 0=all the reports, 1=report not marked R, 2=report marked P,R or T */
    gint show_p;                                    /**< 0=report not marked P, 1=report marked P */
    gint show_r;                                    /**< 0=report not marked R, 1=report marked R */
    gint show_t;                                    /**< 0=report not marked T, 1=report marked T */
    gint show_report_transactions;
    gint show_report_transaction_amount;
    gint show_report_date;
    gint show_report_value_date;
    gint show_report_payee;
    gint show_report_category;
    gint show_report_sub_category;
    gint show_report_budget;
    gint show_report_sub_budget;
    gint show_report_note;
    gint show_report_voucher;
    gint show_report_bank_references;
    gint show_report_transaction_number;
    gint show_report_method_of_payment;
    gint show_report_method_of_payment_content;
    gint show_report_marked;
    gint show_report_financial_year;
    gint show_report_account_name;

    /** @name stuff showed in the report */
    gint sorting_report;                            /**< 0=date, 1=tr number, 2=payee, 3=categ, 4=budget, 5=notes, 6=method payment, 7=method paym content, 8=voucher, 9=bank ref, 10=marked number */

    gint not_detail_split;							/* TRUE = ne pas détailler les opérations ventilées (onglet divers) */
    gint split_credit_debit;

    gint currency_general;
    gint column_title_show;
    gint column_title_type;                         /* 0 = botton, 1 = each section */
    gint append_in_payee;                           /* TRUE : the name of the report will be in the payee list */
    gint report_can_click;                          /* TRUE : we can click on the reports */
    gint ignore_archives;                           /* TRUE ignore les opérations archivées choix manuel */


    /** @name period part of the report */
    /** exercices */
    gint use_financial_year;                        /* TRUE : use the financial year, FALSE : use the dates */
    gint financial_year_type;                       /* 0=all, 1=current, 2=last, 3=personnal */
    GSList *financial_year_list;                    /* list of the numbers of financials years used */
    gint financial_year_split;                      /* TRUE : split by financial year */

    /** dates */
    gint date_type;                                 /* 0=all, 1=perso ... */
    gint date_select_value;                         /* 0=date (default), 1=value date */
    GDate *personal_date_start;
    GDate *personal_date_end;

    /** affichage */
    gint period_split;                              /* TRUE : split by period */
    gint period_split_type;                         /* 0=day, 1=week, 2=month, 3=year */
    gint period_split_day;                          /* 0 = monday ... */

    /** à compléter  */
    GSList *sorting_type;                           /* list of numbers : 1=categ,2=sub-categ,3=budget,4=sub-budget,5=account,6=payee */

    /** @name account part of the report */
    gint account_use_chosen;
    GSList *account_numbers;
    gint account_group_reports;
    gint account_show_amount;
    gint account_show_name;

    /** @name transfer part of the report */
    gint transfer_choice;                           /* 0: no transfer / 1: transfers only on liabilities and assets accounts/2:transfer outside the report/3:perso */
    GSList *transfer_account_numbers;
    gint transfer_reports_only;

    /** @name category part of the report */
    gint category_used;								/* TRUE = Regrouper les opérations par catégorie */
    gint category_detail_used;						/* TRUE = Détailler les catégories utilisées (onglet Catégories) */
    GSList *categ_select_struct;                    /* list of CategBudgetSel containing the selected categories and sub-categories */
    gint category_show_sub_category;
    gint category_show_category_amount;
    gint category_show_sub_category_amount;
    gint category_currency;
    gint category_show_without_category;
    gint category_show_name;

    /** @name budget part of the report */
    gint budget_used;
    gint budget_detail_used;
    GSList *budget_select_struct;                   /* list of CategBudgetSel containing the selected budgets and sub-budgets */
    gint budget_show_sub_budget;
    gint budget_show_budget_amount;
    gint budget_show_sub_budget_amount;
    gint budget_currency;
    gint budget_show_without_budget;
    gint budget_show_name;

    /** @name payee part of the report */
    gint payee_used;
    gint payee_detail_used;
    GSList *payee_numbers;
    gint payee_show_payee_amount;
    gint payee_currency;
    gint payee_show_name;

    /** @name text comparison part of the report */
    gint text_comparison_used;
    GSList *text_comparison_list;

    /** @name amount comparison part of the report */
    gint amount_comparison_used;
    gint amount_comparison_currency;
    GSList *amount_comparison_list;
    gint amount_comparison_only_report_non_null;

    /** @name method of payment part of the report */
    gint method_of_payment_used;
    GSList *method_of_payment_list;
};

struct _CategBudgetSel
{
    gint div_number;
    GSList *sub_div_numbers;	/* list of number of sub-categories/budgets selected for the category/budget */
};


struct _SearchDataReport
{
	gint		search_type;			/* 1 = text, 2 = amount */
	gint		spin_value;				/* valeur du spin button */
	gint 		page_num;				/* GSB_PAYEES_PAGE, GSB_CATEGORIES_PAGE, GSB_BUDGETARY_LINES_PAGE */
	gboolean	ignore_case;
	gboolean	ignore_sign;
	gboolean	search_in_archive;
	gboolean	search_delta_amount;
	gboolean	search_save_report;
};

/* START_DECLARATION */
gboolean 	gsb_data_report_check_categ_budget_in_report 				(GSList *list_struct_report,
																		 gint div_number,
																		 gint sub_div_number);
gint 		gsb_data_report_compare_position 							(gint report_number_1,
																		 gint report_number_2);
gint 		gsb_data_report_dup 										(gint report_number);
void 		gsb_data_report_free_account_numbers_list 					(gint report_number);
void 		gsb_data_report_free_categ_budget_struct_list 				(GSList *categ_budget_sel_list);
void 		gsb_data_report_free_financial_year_list 					(gint report_number);
void 		gsb_data_report_free_method_of_payment_list 				(gint report_number);
void 		gsb_data_report_free_payee_numbers_list 					(gint report_number);
void 		gsb_data_report_free_sorting_type_list 						(gint report_number);
void 		gsb_data_report_free_transfer_account_numbers_list			(gint report_number);
gint 		gsb_data_report_get_account_group_reports 					(gint report_number);
GSList *	gsb_data_report_get_account_numbers_list 					(gint report_number);
gint 		gsb_data_report_get_account_show_amount 					(gint report_number);
gint 		gsb_data_report_get_account_show_name 						(gint report_number);
gint 		gsb_data_report_get_account_use_chosen 						(gint report_number);
gint 		gsb_data_report_get_amount_comparison_currency 				(gint report_number);
GSList *	gsb_data_report_get_amount_comparison_list 					(gint report_number);
gint 		gsb_data_report_get_amount_comparison_only_report_non_null 	(gint report_number);
gint 		gsb_data_report_get_amount_comparison_used 					(gint report_number);
gint 		gsb_data_report_get_append_in_payee 						(gint report_number);
gint 		gsb_data_report_get_budget_currency 						(gint report_number);
gint 		gsb_data_report_get_budget_detail_used 						(gint report_number);
gint		gsb_data_report_get_budget_show_budget_amount 				(gint report_number);
gint 		gsb_data_report_get_budget_show_name 						(gint report_number);
gint 		gsb_data_report_get_budget_show_sub_budget 					(gint report_number);
gint 		gsb_data_report_get_budget_show_sub_budget_amount 			(gint report_number);
gint 		gsb_data_report_get_budget_show_without_budget 				(gint report_number);
GSList *	gsb_data_report_get_budget_struct_list 						(gint report_number);
gint 		gsb_data_report_get_budget_used 							(gint report_number);
gint 		gsb_data_report_get_category_currency 						(gint report_number);
gint 		gsb_data_report_get_category_detail_used 					(gint report_number);
gint 		gsb_data_report_get_category_show_category_amount 			(gint report_number);
gint 		gsb_data_report_get_category_show_name 						(gint report_number);
gint 		gsb_data_report_get_category_show_sub_category 				(gint report_number);
gint 		gsb_data_report_get_category_show_sub_category_amount 		(gint report_number);
gint 		gsb_data_report_get_category_show_without_category 			(gint report_number);
GSList *	gsb_data_report_get_category_struct_list 					(gint report_number);
gint 		gsb_data_report_get_category_used 							(gint report_number);
gint 		gsb_data_report_get_column_title_show 						(gint report_number);
gint 		gsb_data_report_get_column_title_type 						(gint report_number);
gint 		gsb_data_report_get_compl_name_function 					(gint report_number);
gint 		gsb_data_report_get_compl_name_position 					(gint report_number);
gboolean 	gsb_data_report_get_compl_name_used 						(gint report_number);
gint 		gsb_data_report_get_currency_general 						(gint report_number);
gint 		gsb_data_report_get_date_type 								(gint report_number);
gint 		gsb_data_report_get_date_select_value 						(gint report_number);
gchar *		gsb_data_report_get_export_pdf_name 						(gint report_number);
GSList *	gsb_data_report_get_financial_year_list 					(gint report_number);
gint 		gsb_data_report_get_financial_year_split					(gint report_number);
gint 		gsb_data_report_get_financial_year_type 					(gint report_number);
gint 		gsb_data_report_get_ignore_archives 						(gint report_number);
GSList *	gsb_data_report_get_method_of_payment_list 					(gint report_number);
gint 		gsb_data_report_get_method_of_payment_used 					(gint report_number);
gint 		gsb_data_report_get_not_detail_split 						(gint report_number);
gint 		gsb_data_report_get_payee_currency 							(gint report_number);
gint 		gsb_data_report_get_payee_detail_used 						(gint report_number);
GSList *	gsb_data_report_get_payee_numbers_list 						(gint report_number);
gint 		gsb_data_report_get_payee_show_name 						(gint report_number);
gint 		gsb_data_report_get_payee_show_payee_amount 				(gint report_number);
gint 		gsb_data_report_get_payee_used 								(gint report_number);
gint 		gsb_data_report_get_period_split 							(gint report_number);
GDateWeekday	gsb_data_report_get_period_split_day 					(gint report_number);
gint 		gsb_data_report_get_period_split_type 						(gint report_number);
GDate *		gsb_data_report_get_personal_date_end 						(gint report_number);
GDate *		gsb_data_report_get_personal_date_start						(gint report_number);
gint 		gsb_data_report_get_report_by_name 							(const gchar *name);
gint 		gsb_data_report_get_report_can_click 						(gint report_number);
GSList *	gsb_data_report_get_report_list 							(void);
gchar *		gsb_data_report_get_report_name 							(gint report_number);
gint 		gsb_data_report_get_report_number 							(gpointer report_pointer);
gboolean	gsb_data_report_get_search_report							(gint report_number);
gint 		gsb_data_report_get_show_m 									(gint report_number);
gint 		gsb_data_report_get_show_p 									(gint report_number);
gint 		gsb_data_report_get_show_r 									(gint report_number);
gint 		gsb_data_report_get_show_report_account_name				(gint report_number);
gint 		gsb_data_report_get_show_report_bank_references 			(gint report_number);
gint 		gsb_data_report_get_show_report_budget 						(gint report_number);
gint 		gsb_data_report_get_show_report_category 					(gint report_number);
gint 		gsb_data_report_get_show_report_date 						(gint report_number);
gint 		gsb_data_report_get_show_report_financial_year 				(gint report_number);
gint 		gsb_data_report_get_show_report_marked 						(gint report_number);
gint 		gsb_data_report_get_show_report_method_of_payment 			(gint report_number);
gint 		gsb_data_report_get_show_report_method_of_payment_content 	(gint report_number);
gint 		gsb_data_report_get_show_report_note 						(gint report_number);
gint 		gsb_data_report_get_show_report_payee 						(gint report_number);
gint 		gsb_data_report_get_show_report_sub_budget 					(gint report_number);
gint 		gsb_data_report_get_show_report_sub_category 				(gint report_number);
gint 		gsb_data_report_get_show_report_transaction_amount 			(gint report_number);
gint 		gsb_data_report_get_show_report_transaction_number 			(gint report_number);
gint 		gsb_data_report_get_show_report_transactions 				(gint report_number);
gint 		gsb_data_report_get_show_report_value_date 					(gint report_number);
gint 		gsb_data_report_get_show_report_voucher 					(gint report_number);
gint 		gsb_data_report_get_show_t 									(gint report_number);
gint 		gsb_data_report_get_sorting_report 							(gint report_number);
GSList *	gsb_data_report_get_sorting_type_list 						(gint report_number);
gint 		gsb_data_report_get_split_credit_debit 						(gint report_number);
GSList *	gsb_data_report_get_text_comparison_list 					(gint report_number);
gint 		gsb_data_report_get_text_comparison_used 					(gint report_number);
GSList *	gsb_data_report_get_transfer_account_numbers_list 			(gint report_number);
gint 		gsb_data_report_get_transfer_choice 						(gint report_number);
gint 		gsb_data_report_get_transfer_reports_only 					(gint report_number);
gint 		gsb_data_report_get_use_financial_year 						(gint report_number);
gboolean 	gsb_data_report_init_variables 								(void);
gint 		gsb_data_report_max_number 									(void);
gboolean 	gsb_data_report_move_report 								(gint report_number,
																		 gint dest_report_number);
gint 		gsb_data_report_new 										(const gchar *name);
gint 		gsb_data_report_new_with_number 							(gint number);
void 		gsb_data_report_renum_account_number_0 						(gint new_account_number);
gboolean 	gsb_data_report_remove 										(gint no_report);
void		gsb_data_report_remove_from_account							(gint deleted_account);
gboolean 	gsb_data_report_set_account_group_reports 					(gint report_number,
																		 gint account_group_reports);
gboolean 	gsb_data_report_set_account_numbers_list 					(gint report_number,
																		 GSList *account_numbers);
gboolean 	gsb_data_report_set_account_show_amount 					(gint report_number,
																		 gint account_show_amount);
gboolean 	gsb_data_report_set_account_show_name 						(gint report_number,
																		 gint account_show_name);
gboolean 	gsb_data_report_set_account_use_chosen 						(gint report_number,
																		 gint account_use_chosen);
gboolean 	gsb_data_report_set_amount_comparison_currency 				(gint report_number,
																		 gint amount_comparison_currency);
gboolean 	gsb_data_report_set_amount_comparison_list 					(gint report_number,
																		 GSList *amount_comparison_list);
gboolean 	gsb_data_report_set_amount_comparison_only_report_non_null 	(gint report_number,
																		 gint amount_comparison_only_report_non_null);
gboolean 	gsb_data_report_set_amount_comparison_used 					(gint report_number,
																		 gint amount_comparison_used);
gboolean 	gsb_data_report_set_append_in_payee 						(gint report_number,
																		 gint append_in_payee);
gboolean 	gsb_data_report_set_budget_currency 						(gint report_number,
																		 gint budget_currency);
gboolean 	gsb_data_report_set_budget_detail_used 						(gint report_number,
																		 gint budget_detail_used);
gboolean 	gsb_data_report_set_budget_show_budget_amount 				(gint report_number,
																		 gint budget_show_budget_amount);
gboolean 	gsb_data_report_set_budget_show_name 						(gint report_number,
																		 gint budget_show_name);
gboolean 	gsb_data_report_set_budget_show_sub_budget 					(gint report_number,
																		 gint budget_show_sub_budget);
gboolean 	gsb_data_report_set_budget_show_sub_budget_amount 			(gint report_number,
																		 gint budget_show_sub_budget_amount);
gboolean 	gsb_data_report_set_budget_show_without_budget 				(gint report_number,
																		 gint budget_show_without_budget);
gboolean 	gsb_data_report_set_budget_struct_list 						(gint report_number,
                        												 GSList *budget_select_struct);
gboolean 	gsb_data_report_set_budget_used 							(gint report_number,
                        												 gint budget_used);
gboolean 	gsb_data_report_set_category_currency 						(gint report_number,
                        												 gint category_currency);
gboolean 	gsb_data_report_set_category_detail_used 					(gint report_number,
                        												 gint category_detail_used);
gboolean 	gsb_data_report_set_category_show_category_amount 			(gint report_number,
                        												 gint category_show_category_amount);
gboolean 	gsb_data_report_set_category_show_name 						(gint report_number,
                        												 gint category_show_name);
gboolean 	gsb_data_report_set_category_show_sub_category 				(gint report_number,
                        												 gint category_show_sub_category);
gboolean 	gsb_data_report_set_category_show_sub_category_amount 		(gint report_number,
                        												 gint category_show_sub_category_amount);
gboolean 	gsb_data_report_set_category_show_without_category 			(gint report_number,
                        												 gint category_show_without_category);
gboolean 	gsb_data_report_set_category_struct_list 					(gint report_number,
                        												 GSList *categ_select_struct);
gboolean 	gsb_data_report_set_category_used 							(gint report_number,
                        												 gint category_used);
gboolean 	gsb_data_report_set_column_title_show 						(gint report_number,
                        												 gint column_title_show);
gboolean 	gsb_data_report_set_column_title_type 						(gint report_number,
                        												 gint column_title_type);
gboolean 	gsb_data_report_set_compl_name_function 					(gint report_number,
																		gint compl_name_function);
gboolean 	gsb_data_report_set_compl_name_position 					(gint report_number,
																		gint compl_name_position);
gboolean 	gsb_data_report_set_compl_name_used 						(gint report_number,
																		 gint compl_name_used);
gboolean 	gsb_data_report_set_currency_general 						(gint report_number,
                        												 gint currency_general);
gboolean 	gsb_data_report_set_date_type 								(gint report_number,
                        												 gint date_type);
gboolean 	gsb_data_report_set_date_select_value 						(gint report_number,
                        												 gint date_select_value);
gboolean	gsb_data_report_set_export_pdf_name 						(gint report_number,
																		 gchar *export_pdf_name);
gboolean 	gsb_data_report_set_financial_year_list 					(gint report_number,
                        												 GSList *financial_year_list);
gboolean	 gsb_data_report_set_financial_year_split 					(gint report_number,
                        												 gint financial_year_split);
gboolean 	gsb_data_report_set_financial_year_type 					(gint report_number,
                        												 gint financial_year_type);
gboolean 	gsb_data_report_set_ignore_archives 						(gint report_number,
                        												 gint ignore_archives);
gboolean 	gsb_data_report_set_method_of_payment_list 					(gint report_number,
                        												 GSList *method_of_payment_list);
gboolean 	gsb_data_report_set_method_of_payment_used 					(gint report_number,
                        												 gint method_of_payment_used);
gboolean 	gsb_data_report_set_not_detail_split 						(gint report_number,
                        												 gint not_detail_split);
gboolean 	gsb_data_report_set_payee_currency 							(gint report_number,
																		 gint payee_currency);
gboolean 	gsb_data_report_set_payee_detail_used 						(gint report_number,
                        												 gint payee_detail_used);
gboolean 	gsb_data_report_set_payee_numbers_list 						(gint report_number,
                        												 GSList *payee_numbers);
gboolean 	gsb_data_report_set_payee_show_name 						(gint report_number,
                        												 gint payee_show_name);
gboolean 	gsb_data_report_set_payee_show_payee_amount 				(gint report_number,
                        												 gint payee_show_payee_amount);
gboolean 	gsb_data_report_set_payee_used 								(gint report_number,
                        												 gint payee_used);
gboolean 	gsb_data_report_set_period_split 							(gint report_number,
                        												 gint period_split);
gboolean 	gsb_data_report_set_period_split_day 						(gint report_number,
                        												 gint period_split_day);
gboolean 	gsb_data_report_set_period_split_type 						(gint report_number,
                        												 gint period_split_type);
gboolean	 gsb_data_report_set_personal_date_end 						(gint report_number,
                        												 GDate *personal_date_end);
gboolean 	gsb_data_report_set_personal_date_start 					(gint report_number,
                        												 GDate *personal_date_start);
gboolean 	gsb_data_report_set_report_can_click 						(gint report_number,
                        												 gint report_can_click);
gboolean 	gsb_data_report_set_report_name 							(gint report_number,
                        												 const gchar *report_name);
void		gsb_data_report_set_search_report							(gint report_number);
gboolean 	gsb_data_report_set_show_m 									(gint report_number,
                        												 gint show_m);
gboolean 	gsb_data_report_set_show_p 									(gint report_number,
                        												 gint show_p);
gboolean 	gsb_data_report_set_show_r 									(gint report_number,
                        												 gint show_r);
gboolean 	gsb_data_report_set_show_report_account_name				(gint report_number,
                        												 gint show_report_account_name);
gboolean 	gsb_data_report_set_show_report_bank_references 			(gint report_number,
                        												 gint show_report_bank_references);
gboolean 	gsb_data_report_set_show_report_budget 						(gint report_number,
                        												 gint show_report_budget);
gboolean 	gsb_data_report_set_show_report_category 					(gint report_number,
                        												 gint show_report_category);
gboolean 	gsb_data_report_set_show_report_date 						(gint report_number,
                        												 gint show_report_date);
gboolean 	gsb_data_report_set_show_report_financial_year 				(gint report_number,
                        												 gint show_report_financial_year);
gboolean 	gsb_data_report_set_show_report_marked 						(gint report_number,
                        												 gint show_report_marked);
gboolean 	gsb_data_report_set_show_report_method_of_payment 			(gint report_number,
                        												 gint show_report_method_of_payment);
gboolean 	gsb_data_report_set_show_report_method_of_payment_content 	(gint report_number,
                        												 gint show_report_method_of_payment_content);
gboolean 	gsb_data_report_set_show_report_note 						(gint report_number,
                        												 gint show_report_note);
gboolean 	gsb_data_report_set_show_report_payee 						(gint report_number,
                        												 gint show_report_payee);
gboolean 	gsb_data_report_set_show_report_sub_budget 					(gint report_number,
                        												 gint show_report_sub_budget);
gboolean 	gsb_data_report_set_show_report_sub_category 				(gint report_number,
                        												 gint show_report_sub_category);
gboolean	gsb_data_report_set_show_report_transaction_amount			(gint report_number,
                        												 gint show_report_transaction_amount);
gboolean 	gsb_data_report_set_show_report_transaction_number 			(gint report_number,
                        												 gint show_report_transaction_number);
gboolean 	gsb_data_report_set_show_report_transactions 				(gint report_number,
                        												 gint show_report_transactions);
gboolean 	gsb_data_report_set_show_report_value_date 					(gint report_number,
                        												 gint show_report_value_date);
gboolean 	gsb_data_report_set_show_report_voucher 					(gint report_number,
                        												 gint show_report_voucher);
gboolean 	gsb_data_report_set_show_t 									(gint report_number,
                        												 gint show_t);
gboolean 	gsb_data_report_set_sorting_report 							(gint report_number,
                        												 gint sorting_report);
gboolean 	gsb_data_report_set_sorting_type_list 						(gint report_number,
                        												 GSList *sorting_type);
gboolean 	gsb_data_report_set_split_credit_debit 						(gint report_number,
                        												 gint split_credit_debit);
gboolean 	gsb_data_report_set_text_comparison_list 					(gint report_number,
                        												 GSList *text_comparison_list);
gboolean 	gsb_data_report_set_text_comparison_used 					(gint report_number,
                        												 gint text_comparison_used);
gboolean 	gsb_data_report_set_transfer_account_numbers_list 			(gint report_number,
                        												 GSList *transfer_account_numbers);
gboolean 	gsb_data_report_set_transfer_choice 						(gint report_number,
                        												 gint transfer_choice);
gboolean 	gsb_data_report_set_transfer_reports_only 					(gint report_number,
                        												 gint transfer_reports_only);
gboolean 	gsb_data_report_set_use_financial_year 						(gint report_number,
                        												 gint use_financial_year);
gint 		gsb_data_report_test_ignore_archives 						(gint report_number);
/* END_DECLARATION */

#endif
