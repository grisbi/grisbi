#ifndef CSV_PARSE_H
#define CSV_PARSE_H

/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean csv_import_parse_balance ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_budget ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_category ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_credit ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_currency ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_date ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_debit ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_notes ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_p_r ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_payee ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_split ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_sub_budget ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_sub_category ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_value_date ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_parse_voucher ( struct struct_ope_importation * ope, gchar * string );
gboolean csv_import_validate_amount ( gchar * string );
gboolean csv_import_validate_date ( gchar * string );
gboolean csv_import_validate_number ( gchar * string );
gboolean csv_import_validate_string ( gchar * string );
GSList * csv_parse_line ( gchar ** contents, gchar * separator );
/* END_DECLARATION */

#endif
