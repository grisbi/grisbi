#ifndef _GSB_FILE_UTIL_H
#define _GSB_FILE_UTIL_H (1)
/* START_DECLARATION */
void gsb_file_util_change_permissions ( void );
gchar *gsb_file_util_compress_file ( gchar *file_content,
				     gboolean compress );
gchar *gsb_file_util_crypt_file ( gchar *file_content,
				  gboolean crypt,
				  gint length );
gboolean gsb_file_util_modify_lock ( gboolean create_swp );
void switch_t_r ( void );
/* END_DECLARATION */
#endif
