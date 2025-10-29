#ifndef _GSB_PAYEE_DATA_H
#define _GSB_PAYEE_DATA_H (1)


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void 			gsb_data_payee_add_transaction_to_payee 		(gint transaction_number);
void 			gsb_data_payee_free_name_and_report_list 		(GSList *liste);
GsbReal			gsb_data_payee_get_balance 						(gint no_payee);
const gchar *	gsb_data_payee_get_description 					(gint no_payee);
gpointer 		gsb_data_payee_get_empty_payee 					(void);
gint			gsb_data_payee_get_ignore_case 					(gint no_payee);
const gchar *	gsb_data_payee_get_name 						(gint no_payee,
																 gboolean can_return_null);
GSList *		gsb_data_payee_get_name_and_report_list 		(void);
gint 			gsb_data_payee_get_nb_transactions 				(gint no_payee);
gint 			gsb_data_payee_get_no_payee 					(gpointer payee_ptr);
gint 			gsb_data_payee_get_number_by_name 				(const gchar *name,
																 gboolean create);
GSList *		gsb_data_payee_get_payees_list 					(void);
GSList *		gsb_data_payee_get_search_payee_list			(const gchar *text,
																 gboolean ignore_case);
const gchar *	gsb_data_payee_get_search_string 				(gint no_payee);
gpointer 		gsb_data_payee_get_structure 					(gint no_payee);
GSList *		gsb_data_payee_get_unarchived_payees_list		(void);
gint 			gsb_data_payee_get_unused_payees 				(void);
gint			gsb_data_payee_get_use_regex 					(gint no_payee);
gboolean 		gsb_data_payee_init_variables 					(gboolean cleanup);
gint 			gsb_data_payee_new 								(const gchar *name);
gboolean 		gsb_data_payee_remove 							(gint no_payee);
void 			gsb_data_payee_remove_transaction_from_payee 	(gint transaction_number);
gint 			gsb_data_payee_remove_unused 					(void);
gboolean 		gsb_data_payee_set_description 					(gint no_payee,
																 const gchar *description);
gboolean		gsb_data_payee_set_ignore_case 					(gint no_payee,
																 gint ignore_case);
gboolean 		gsb_data_payee_set_name 						(gint no_payee,
																 const gchar *name);
gint 			gsb_data_payee_set_new_number 					(gint no_payee,
																 gint new_no_payee);
gboolean 		gsb_data_payee_set_search_string 				(gint no_payee,
																 const gchar *search_string);
gboolean		gsb_data_payee_set_use_regex 					(gint no_payee,
																 gint use_regex);
void 			gsb_data_payee_update_counters 					(void);
gboolean 		gsb_data_payee_compare_payees_by_name 			(gpointer payee_ptr_a,
																 gpointer payee_ptr_b);
/* END_DECLARATION */
#endif
