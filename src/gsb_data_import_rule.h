#ifndef _GSB_DATA_IMPORT_RULE_H
#define _GSB_DATA_IMPORT_RULE_H (1)

#include <glib.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean 		gsb_data_import_rule_account_has_rule 			(gint account_number);
gint 			gsb_data_import_rule_get_account 				(gint import_rule_number);
gint 			gsb_data_import_rule_get_action 				(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_charmap 				(gint import_rule_number);
gint 			gsb_data_import_rule_get_csv_account_id_col		(gint import_rule_number);
gint 			gsb_data_import_rule_get_csv_account_id_row		(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_csv_fields_str			(gint import_rule_number);
gint 			gsb_data_import_rule_get_csv_first_line_data	(gint import_rule_number);
gint 			gsb_data_import_rule_get_csv_headers_present	(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_csv_separator			(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_csv_skipped_lines_str	(gint import_rule_number);
gint			gsb_data_import_rule_get_csv_spec_action		(gint import_rule_number);
gint			gsb_data_import_rule_get_csv_spec_amount_col	(gint import_rule_number);
gint			gsb_data_import_rule_get_csv_spec_text_col		(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_csv_spec_text_str		(gint import_rule_number);
gint 			gsb_data_import_rule_get_currency 				(gint import_rule_number);
GSList *		gsb_data_import_rule_get_from_account 			(gint account_number);
gboolean 		gsb_data_import_rule_get_invert 				(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_last_file_name 		(gint import_rule_number);
GSList *		gsb_data_import_rule_get_list 					(void);
const gchar *	gsb_data_import_rule_get_name 					(gint import_rule_number);
gint 			gsb_data_import_rule_get_number 				(gpointer rule_ptr);
const gchar *	gsb_data_import_rule_get_type					(gint import_rule_number);
gboolean 		gsb_data_import_rule_init_variables 			(void);
gint 			gsb_data_import_rule_new 						(const gchar *name);
gboolean 		gsb_data_import_rule_remove 					(gint import_rule_number);
gboolean 		gsb_data_import_rule_set_account 				(gint import_rule_number,
																 gint account_number);
gboolean 		gsb_data_import_rule_set_action 				(gint import_rule_number,
																 gint action);
gboolean 		gsb_data_import_rule_set_charmap 				(gint import_rule_number,
																 const gchar *charmap);
gboolean		gsb_data_import_rule_set_csv_account_id_col		(gint import_rule_number,
																 gint account_id_col);
gboolean		gsb_data_import_rule_set_csv_account_id_row		(gint import_rule_number,
																 gint csv_account_id_row);
gboolean		gsb_data_import_rule_set_csv_fields_str			(gint import_rule_number,
																 const gchar *csv_fields_str);
gboolean		gsb_data_import_rule_set_csv_first_line_data	(gint import_rule_number,
																 gint csv_first_line_data);
gboolean		gsb_data_import_rule_set_csv_headers_present	(gint import_rule_number,
																 gboolean csv_headers_present);
gboolean		gsb_data_import_rule_set_csv_separator			(gint import_rule_number,
																 const gchar *csv_separator);
gboolean		gsb_data_import_rule_set_csv_skipped_lines_str	(gint import_rule_number,
																 const gchar *csv_skipped_lines_str);
gboolean		gsb_data_import_rule_set_csv_spec_action		(gint import_rule_number,
																 gint csv_spec_action);
gboolean		gsb_data_import_rule_set_csv_spec_amount_col	(gint import_rule_number,
																 gint csv_spec_amount_col);
gboolean		gsb_data_import_rule_set_csv_spec_text_col		(gint import_rule_number,
																 gint csv_spec_text_col);
gboolean		gsb_data_import_rule_set_csv_spec_text_str		(gint import_rule_number,
																 const gchar *csv_spec_text_str);
gboolean 		gsb_data_import_rule_set_currency 				(gint import_rule_number,
																 gint currency_number);
gboolean 		gsb_data_import_rule_set_invert 				(gint import_rule_number,
																 gboolean invert);
gboolean 		gsb_data_import_rule_set_last_file_name 		(gint import_rule_number,
																 const gchar *last_file_name);
gboolean 		gsb_data_import_rule_set_name 					(gint import_rule_number,
																 const gchar *name);
gint 			gsb_data_import_rule_set_new_number 			(gint import_rule_number,
																 gint new_no_import_rule);
gboolean 		gsb_data_import_rule_set_type 					(gint import_rule_number,
																 const gchar *type);
/* END_DECLARATION */
#endif
