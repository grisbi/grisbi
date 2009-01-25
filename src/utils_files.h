#ifndef _H_UTILS_FILES
#define _H_UTILS_FILES 1

#include "config.h"
#include <sys/stat.h>

enum get_filename_returned_value {
    UTILS_FILES_FILENAME,
    UTILS_FILES_BACKUP_FILENAME,
};

/* START_INCLUDE_H */
/* END_INCLUDE_H */

#ifdef _WIN32
#define C_DIRECTORY_SEPARATOR "\\"
#else
#define C_DIRECTORY_SEPARATOR "/"
#endif

/*START_DECLARATION*/
gint get_utf8_line_from_file ( FILE *fichier,
			       gchar **string,
			       const gchar *coding_system );
GtkWidget * my_file_chooser ();
gchar* my_get_XDG_grisbi_data_dir(void);
gchar* my_get_XDG_grisbirc_dir(void);
gchar* my_get_grisbirc_dir(void);
gchar* my_get_gsb_file_default_dir(void);
gchar * safe_file_name ( gchar* filename );
FILE* utf8_fopen(const gchar* utf8filename,gchar* mode);
gint utf8_remove(const gchar* utf8filename);
gboolean utils_files_create_XDG_dir (void);
GtkWidget *utils_files_create_file_chooser ( GtkWidget *parent, gchar * titre );
/*END_DECLARATION*/

#endif
