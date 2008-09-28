#ifndef _GSB_DATA_IMPORT_RULE_H
#define _GSB_DATA_IMPORT_RULE_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean gsb_data_import_rule_account_has_rule ( gint account_number );
gint gsb_data_import_rule_get_account ( gint import_rule_number );
gint gsb_data_import_rule_get_action ( gint import_rule_number );
const gchar *gsb_data_import_rule_get_charmap ( gint import_rule_number );
gint gsb_data_import_rule_get_currency ( gint import_rule_number );
GSList *gsb_data_import_rule_get_from_account ( gint account_number );
gboolean gsb_data_import_rule_get_invert ( gint import_rule_number );
const gchar *gsb_data_import_rule_get_last_file_name ( gint import_rule_number );
GSList *gsb_data_import_rule_get_list ( void );
const gchar *gsb_data_import_rule_get_name ( gint import_rule_number );
gint gsb_data_import_rule_get_number ( gpointer rule_ptr );
gboolean gsb_data_import_rule_init_variables ( void );
gint gsb_data_import_rule_new ( const gchar *name );
gboolean gsb_data_import_rule_remove ( gint import_rule_number );
gboolean gsb_data_import_rule_set_account ( gint import_rule_number,
					    gint account_number );
gboolean gsb_data_import_rule_set_action ( gint import_rule_number,
					   gint action );
gboolean gsb_data_import_rule_set_charmap ( gint import_rule_number,
					 const gchar *charmap );
gboolean gsb_data_import_rule_set_currency ( gint import_rule_number,
					     gint currency_number );
gboolean gsb_data_import_rule_set_invert ( gint import_rule_number,
					   gboolean invert );
gboolean gsb_data_import_rule_set_last_file_name ( gint import_rule_number,
						   const gchar *last_file_name );
gboolean gsb_data_import_rule_set_name ( gint import_rule_number,
					 const gchar *name );
gint gsb_data_import_rule_set_new_number ( gint import_rule_number,
					   gint new_no_import_rule );
/* END_DECLARATION */
#endif
