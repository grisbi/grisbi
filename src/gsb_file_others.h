#ifndef _GSB_FILE_OTHERS_H
#define _GSB_FILE_OTHERS_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean gsb_file_others_load_budget ( gchar *filename );
gboolean gsb_file_others_load_budget_from_category ( const gchar *filename );
gboolean gsb_file_others_load_category ( gchar *filename );
gboolean gsb_file_others_load_report ( gchar *filename );
gboolean gsb_file_others_save_budget ( gchar *filename );
gboolean gsb_file_others_save_category ( gchar *filename );
gboolean gsb_file_others_save_report ( gchar *filename );
/* END_DECLARATION */
#endif
