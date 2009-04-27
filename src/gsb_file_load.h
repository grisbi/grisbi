#ifndef _GSB_FILE_LOAD_H
#define _GSB_FILE_LOAD_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void gsb_file_load_amount_comparison ( const gchar **attribute_names,
                        const gchar **attribute_values );
void gsb_file_load_budgetary ( const gchar **attribute_names,
                        const gchar **attribute_values );
void gsb_file_load_category ( const gchar **attribute_names,
                        const gchar **attribute_values );
void gsb_file_load_error ( GMarkupParseContext *context,
                        GError *error,
                        gpointer user_data );
gboolean gsb_file_load_open_file ( gchar *filename );
void gsb_file_load_report ( const gchar **attribute_names,
                        const gchar **attribute_values );
void gsb_file_load_sub_budgetary ( const gchar **attribute_names,
                        const gchar **attribute_values);
void gsb_file_load_sub_category ( const gchar **attribute_names,
                        const gchar **attribute_values);
void gsb_file_load_text_comparison ( const gchar **attribute_names,
                        const gchar **attribute_values );
/* END_DECLARATION */
#endif

