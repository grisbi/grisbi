#ifndef _GSB_FILE_UTIL_H
#define _GSB_FILE_UTIL_H (1)

#include <glib.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_file_util_change_permissions ( void );
void gsb_file_util_display_warning_permissions ( void );
gboolean gsb_file_util_get_contents (const gchar *filename,
				      gchar **file_content,
				      gulong *length );
gboolean gsb_file_util_modify_lock (const gchar *filename,
									gboolean create_lock);
gboolean gsb_file_util_test_overwrite ( const gchar *filename );
/* END_DECLARATION */
#endif
