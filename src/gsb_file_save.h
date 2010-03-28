#ifndef _GSB_FILE_SAVE_H
#define _GSB_FILE_SAVE_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gulong gsb_file_save_append_part ( gulong iterator,
                        gulong *length_calculated,
                        gchar **file_content,
                        gchar *new_string );
gulong gsb_file_save_budgetary_part ( gulong iterator,
                        gulong *length_calculated,
                        gchar **file_content );
gulong gsb_file_save_category_part ( gulong iterator,
                        gulong *length_calculated,
                        gchar **file_content );
gulong gsb_file_save_report_part ( gulong iterator,
                        gulong *length_calculated,
                        gchar **file_content,
                        gboolean current_report );
gboolean gsb_file_save_save_file ( const gchar *filename,
                        gboolean compress,
                        gint archive_number );
const gchar *my_safe_null_str ( const gchar *string );
/* END_DECLARATION */

#endif

