#ifndef _MENU_H
#define _MENU_H (1)

#include <gtk/gtk.h>

enum view_menu_action {
  HIDE_SHOW_TRANSACTION_FORM = 0,
  HIDE_SHOW_GRID,
  HIDE_SHOW_RECONCILED_TRANSACTIONS,
  ONE_LINE_PER_TRANSACTION,
  TWO_LINES_PER_TRANSACTION,
  THREE_LINES_PER_TRANSACTION,
  FOUR_LINES_PER_TRANSACTION,
  HIDE_SHOW_CLOSED_ACCOUNTS,
};

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */

/* RECENT FILES MANAGER */
void gsb_menu_recent_manager_purge_gsb_items ( GtkRecentManager *recent_manager );
void gsb_menu_recent_manager_remove_item ( GtkRecentManager *recent_manager,
						const gchar *path );
gchar **gsb_menu_recent_manager_get_recent_array ( void );
void gsb_menu_recent_manager_set_recent_array ( gchar **recent_array );

/* APP MENU */
void grisbi_cmd_new_window ( GSimpleAction *action,
                        GVariant *parameter,
                        gpointer user_data );
void grisbi_cmd_about ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_quit ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app );
/* PREFS MENU */
void grisbi_cmd_prefs ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app );
/* HELP MENU */
void grisbi_cmd_manual ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app );
void grisbi_cmd_quick_start ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app );
void grisbi_cmd_web_site ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app );
void grisbi_cmd_report_bug ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app );
void grisbi_cmd_day_tip ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app );

/* WIN MENU */
/* FILE MENU */
void grisbi_cmd_file_new ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_file_open_menu ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_file_open_direct_menu ( GtkRecentManager *recent_manager,
						GtkRecentChooser *chooser );
void grisbi_cmd_file_save ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_file_save_as ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_importer_fichier ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_export_accounts ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_create_archive ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_export_archive ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_debug_acc_file ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_obf_acc_file ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_obf_qif_file ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_debug_mode_toggle ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_file_close ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
/* EDIT MENU */
void grisbi_cmd_edit_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_new_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_remove_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_template_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_clone_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_convert_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_new_acc ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_remove_acc ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
/* VIEW MENU */
void grisbi_cmd_show_form_toggle ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_show_reconciled_toggle ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_show_archived_toggle ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_show_closed_acc_toggle ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_show_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );
void grisbi_cmd_reset_width_col ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data );



gboolean affiche_derniers_fichiers_ouverts ( void );
void efface_derniers_fichiers_ouverts ( void );
gboolean gsb_gui_sensitive_menu_item ( gchar *item_name, gboolean state );
void gsb_gui_toggle_line_view_mode ( GtkRadioAction *action,
                        GtkRadioAction *current,
                        gpointer user_data );
gboolean gsb_gui_toggle_show_archived ( void );
gboolean gsb_gui_toggle_show_reconciled ( void );
void gsb_menu_free_ui_manager ( void );
GtkUIManager *gsb_menu_get_ui_manager ( void );
void gsb_menu_free_ui_manager ( void );
gboolean gsb_menu_set_block_menu_cb ( gboolean etat );
gboolean gsb_menu_set_menus_select_scheduled_sensitive ( gboolean sensitive );
gboolean gsb_menu_set_menus_select_transaction_sensitive ( gboolean sensitive );
void gsb_menu_set_menus_view_account_sensitive ( gboolean sensitive );
void gsb_menu_set_menus_with_file_sensitive ( gboolean sensitive );
gboolean gsb_menu_update_accounts_in_menus ( void );
gboolean gsb_menu_update_view_menu ( gint account_number );


/* END_DECLARATION */
#endif
