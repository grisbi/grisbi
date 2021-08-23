#ifndef _FICHIERS_GESTION_H
#define _FICHIERS_GESTION_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean        gsb_file_automatic_backup_change_time   (GtkWidget *spinbutton,
                                                         gpointer null);
gboolean        gsb_file_automatic_backup_start         (GtkWidget *checkbutton,
                                                         gpointer null);
gboolean        gsb_file_close                          (void);
void 			gsb_file_copy_old_file 					(const gchar *filename);
void            gsb_file_free_last_path                 (void);
void            gsb_file_free_backup_path               (void);
const gchar *   gsb_file_get_backup_path                (void);
const gchar *   gsb_file_get_last_path                  (void);
void            gsb_file_init_last_path                 (const gchar *last_path);
gboolean        gsb_file_new_finish                     (void);
gboolean        gsb_file_open_file                      (const gchar *filename);
gboolean        gsb_file_open_menu                      (void);
gboolean        gsb_file_quit							(void);
void			gsb_file_remove_account_file			(GtkWidget *menu_item,
														 const gchar *filename);
gboolean        gsb_file_save                           (void);
gboolean        gsb_file_save_as                        (void);
void            gsb_file_set_backup_path                (const gchar *path);
void            gsb_file_set_modified                   (gboolean modified);
gboolean        gsb_file_get_modified                   (void);
void            gsb_file_update_last_path               (const gchar *last_path);
/* END_DECLARATION */
#endif
