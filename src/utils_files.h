#ifndef _H_UTILS_FILES
#define _H_UTILS_FILES 1

#include <sys/stat.h>
#include <gtk/gtk.h>

enum get_filename_returned_value {
    UTILS_FILES_FILENAME,
    UTILS_FILES_BACKUP_FILENAME,
};

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/*START_DECLARATION*/
gint get_utf8_line_from_file ( FILE *fichier,
                        gchar **string,
                        const gchar *coding_system );
gchar *safe_file_name ( gchar *filename );
G_MODULE_EXPORT FILE* utf8_fopen ( const gchar *utf8filename, gchar *mode );
gint utf8_remove ( const gchar *utf8filename );
gboolean utils_files_create_XDG_dir ( void );
GtkWidget *utils_files_create_file_chooser ( GtkWidget *parent, gchar * titre );
gchar *utils_files_create_sel_charset ( GtkWidget *assistant,
                        const gchar *tmp_str,
                        const gchar *charmap_imported,
                        gchar *filename );
void utils_files_display_dialog_error ( gint error_type,
                        const gchar *filename,
                        const gchar *error_str );
gchar *utils_files_get_ofx_charset ( gchar *contents );
/*END_DECLARATION*/

#endif
