#ifndef CSV_PARSE_H
#define CSV_PARSE_H

/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */

/*START_DECLARATION */
gboolean 	csv_import_parse_balance 		(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_budget 		(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_category 		(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_credit 		(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_currency 		(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_date 			(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_debit 			(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_notes 			(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_p_r 			(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_payee 			(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_split 			(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_sub_budget 	(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_sub_category 	(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_value_date 	(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_cheque 		(struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_parse_payment_method (struct ImportTransaction *ope,
											 gchar *string);
gboolean 	csv_import_validate_amount 		(gchar *string);
gboolean 	csv_import_validate_date 		(gchar *string);
gboolean 	csv_import_validate_number 		(gchar *string);
gboolean 	csv_import_validate_string 		(gchar *string);
GSList *	csv_parse_line 					(gchar **contents,
											 const gchar *separator);
/* END_DECLARATION */

#endif
