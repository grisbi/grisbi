#ifndef _FICHIERS_GESTION_H
#define _FICHIERS_GESTION_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "grisbi_app.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_file_automatic_backup_change_time ( GtkWidget *spinbutton,
                        gpointer null );
gboolean gsb_file_automatic_backup_start ( GtkWidget *checkbutton,
                        gpointer null );
gboolean gsb_file_close ( void );
const gchar *gsb_file_get_account_files_path ( void );
const gchar *gsb_file_get_backup_path ( void );
const gchar *gsb_file_get_import_files_path ( void );
const gchar *gsb_file_get_last_path ( void );
gboolean gsb_file_new ( void );
gboolean gsb_file_new_finish ( void );
void gsb_file_new_gui ( void );
gboolean gsb_file_open_direct_menu ( GtkMenuItem *item,
                        gint *file_number_ptr );
gboolean gsb_file_open_file ( gchar *filename );
gboolean gsb_file_open_from_commandline ( GSList *file_list );
gboolean gsb_file_open_menu ( void );
void gsb_file_remove_name_from_opened_list ( gchar *filename );
gboolean gsb_file_save ( void );
gboolean gsb_file_save_as ( void );
void gsb_file_set_account_files_path ( const gchar *path,
                        GrisbiAppConf *conf );
void gsb_file_set_backup_path ( const gchar *path,
                        GrisbiAppConf *conf );
void gsb_file_set_import_files_path ( const gchar *path,
                        GrisbiAppConf *conf );
void gsb_file_update_last_path ( const gchar *last_path );
void gsb_file_set_modified ( gboolean modified );
gboolean gsb_file_get_modified ( void );
/* END_DECLARATION */
#endif
