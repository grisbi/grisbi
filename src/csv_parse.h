#ifndef CSV_PARSE_H
#define CSV_PARSE_H

/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */

/*START_DECLARATION */
gboolean 	csv_import_parse_balance 		(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_budget 		(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_category 		(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_cheque 		(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_credit 		(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_currency 		(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_date 			(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_debit 			(ImportTransaction *ope,
											 gchar *string);
GSList *	csv_import_parse_line 			(gchar **contents,
											 const gchar *separator);
gboolean 	csv_import_parse_notes 			(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_p_r 			(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_payee 			(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_payment_method	(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_split 			(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_sub_budget 	(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_sub_category 	(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_value_date 	(ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_validate_amount 		(gchar *string);
gboolean 	csv_import_validate_date 		(gchar *string);
gboolean 	csv_import_validate_number 		(gchar *string);
gboolean 	csv_import_validate_string 		(gchar *string);
/* END_DECLARATION */

#endif
