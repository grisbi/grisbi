#ifndef _GSB_FILE_UTIL_H
#define _GSB_FILE_UTIL_H (1)
/* START_DECLARATION */
void gsb_file_util_change_permissions ( void );
gint gsb_file_util_compress_file ( gchar **file_content,
				   gulong length,
				   gboolean compress );
gulong gsb_file_util_crypt_file ( gchar * file_name, gchar **file_content,
				gboolean crypt, gulong length );
gboolean gsb_file_util_modify_lock ( gboolean create_swp );
void switch_t_r ( void );
/* END_DECLARATION */
#endif
