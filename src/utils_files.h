#ifndef _H_UTILS_FILES
#define _H_UTILS_FILES 1

#include <sys/stat.h>
#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/*START_DECLARATION*/
void 		utils_files_append_name_to_recent_array		(const gchar *filename);
GtkWidget *	utils_files_create_file_chooser 			(GtkWidget *parent,
														 gchar *titre);
gchar *		utils_files_create_sel_charset 				(GtkWidget *assistant,
														 const gchar *tmp_str,
														 const gchar *charmap_imported,
														 gchar *filename);
gboolean 	utils_files_create_XDG_dir 					(void);
gchar *		utils_files_get_ofx_charset 				(gchar *contents);
gint 		utils_files_get_utf8_line_from_file			(FILE *fichier,
														 gchar **string,
														 const gchar *coding_system);
void		utils_files_remove_name_to_recent_array		(const gchar *filename);
gchar *		utils_files_safe_file_name 					(gchar *filename);
gchar * 	utils_files_selection_get_last_directory	(GtkFileChooser *filesel,
														 gboolean ended);
FILE* 		utils_files_utf8_fopen 						(const gchar *utf8filename,
														 const gchar *mode);
gint 		utils_files_utf8_remove 					(const gchar *utf8filename);
/*END_DECLARATION*/

#endif
