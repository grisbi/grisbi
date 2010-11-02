#ifndef _GSB_FILE_UTIL_H
#define _GSB_FILE_UTIL_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_file_util_change_permissions ( void );
void gsb_file_util_display_warning_permissions ( void );
gboolean gsb_file_util_get_contents ( gchar *filename,
				      gchar **file_content,
				      gulong *length );
gboolean gsb_file_util_modify_lock ( gboolean create_lock );
gboolean gsb_file_util_test_overwrite ( const gchar *filename );
void switch_t_r ( void );
/* END_DECLARATION */
#endif
