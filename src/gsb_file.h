#ifndef _FICHIERS_GESTION_H
#define _FICHIERS_GESTION_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_file_close ( void );
const gchar *gsb_file_get_backup_path ( void );
const gchar *gsb_file_get_last_path ( void );
gboolean gsb_file_new ( void );
void gsb_file_new_gui ( void );
gboolean gsb_file_open_direct_menu ( GtkMenuItem *item,
				     gint *file_number_ptr );
gboolean gsb_file_open_file ( gchar *filename );
gboolean gsb_file_open_menu ( void );
void gsb_file_remove_name_from_opened_list ( gchar *filename );
gboolean gsb_file_save ( void );
gboolean gsb_file_save_as ( void );
void gsb_file_set_backup_path ( const gchar *backup_path );
void gsb_file_update_last_path ( const gchar *last_path );
void gsb_file_update_window_title ( void );
/* END_DECLARATION */
#endif
