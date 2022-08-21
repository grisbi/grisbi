#ifndef _MENU_H
#define _MENU_H (1)

#include <gtk/gtk.h>

enum ViewMenuAction {
  HIDE_SHOW_TRANSACTION_FORM = 0,
  HIDE_SHOW_GRID,
  HIDE_SHOW_RECONCILED_TRANSACTIONS,
  ONE_LINE_PER_TRANSACTION,
  TWO_LINES_PER_TRANSACTION,
  THREE_LINES_PER_TRANSACTION,
  FOUR_LINES_PER_TRANSACTION,
  HIDE_SHOW_CLOSED_ACCOUNTS
};

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gchar *		gsb_menu_normalise_label_name 		(const gchar *chaine);

/* APP MENU */
void        grisbi_cmd_about                    (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
/* PREFS MENU */
void        grisbi_cmd_prefs                    (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
/* HELP MENU */
void        grisbi_cmd_manual                   (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_quick_start              (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_web_site                 (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_report_bug               (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_day_tip                  (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);

/* WIN MENU */
/* FILE MENU */
void        grisbi_cmd_file_new                 (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_file_open_menu           (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_file_open_direct_menu    (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_file_save                (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_file_save_as             (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_importer_fichier         (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_export_accounts          (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_create_archive           (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_export_archive           (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_debug_acc_file           (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_obf_acc_file             (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_obf_qif_file             (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_debug_mode_toggle        (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_file_close               (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
/* EDIT MENU */
void        grisbi_cmd_edit_ope                 (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_new_ope                  (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_remove_ope               (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_template_ope             (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_clone_ope                (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_convert_ope              (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_move_to_account_menu     (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_new_acc                  (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_remove_acc               (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void		grisbi_cmd_search_acc				(GSimpleAction *action,
												 GVariant *parameter,
												 gpointer app);
/* VIEW MENU */
void        grisbi_cmd_show_form_toggle         (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_show_reconciled_toggle   (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_show_archived_toggle     (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_show_closed_acc_toggle   (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_show_ope_radio           (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);
void        grisbi_cmd_reset_width_col          (GSimpleAction *action,
                                                 GVariant *parameter,
                                                 gpointer app);



void        gsb_menu_gui_sensitive_win_menu_item            (const gchar *item_name,
                                                             gboolean state);
gboolean    gsb_menu_gui_toggle_show_archived               (void);
gboolean    gsb_menu_gui_toggle_show_form                   (void);
gboolean    gsb_menu_gui_toggle_show_reconciled             (void);
void        gsb_menu_set_menus_select_scheduled_sensitive   (gboolean sensitive);
void        gsb_menu_set_menus_select_transaction_sensitive (gboolean sensitive);
void        gsb_menu_set_menus_view_account_sensitive       (gboolean sensitive);
void        gsb_menu_set_menus_with_file_sensitive          (gboolean sensitive);
gboolean    gsb_menu_update_view_menu                       (gint account_number);


/* END_DECLARATION */
#endif
