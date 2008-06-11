#ifndef _GSB_FILE_UTIL_H
#define _GSB_FILE_UTIL_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_file_util_change_permissions ( void );
gboolean gsb_file_util_modify_lock ( gboolean create_swp );
gboolean gsb_file_util_test_overwrite ( const gchar *filename );
void switch_t_r ( void );
gboolean gsb_file_util_get_contents ( gchar *filename,
				      gchar **file_content,
				      gulong *length );
/* END_DECLARATION */
#endif
