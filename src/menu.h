#ifndef _MENU_H
#define _MENU_H (1)


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
gboolean affiche_derniers_fichiers_ouverts ( void );
void efface_derniers_fichiers_ouverts ( void );
gboolean gsb_gui_sensitive_menu_item ( gchar *root_menu_name,
                        gchar *submenu_name,
                        gchar *subsubmenu_name,
                        gboolean state );
gboolean gsb_gui_sensitive_menu_item_from_string ( gchar *item_name, gboolean state );
void gsb_gui_toggle_line_view_mode ( GtkRadioAction *action,
                        GtkRadioAction *current,
                        gpointer user_data );
gboolean gsb_gui_toggle_show_archived ( void );
gboolean gsb_gui_toggle_show_reconciled ( void );
GtkUIManager *gsb_menu_get_ui_manager ( void );
gboolean gsb_menu_transaction_operations_set_sensitive ( gboolean sensitive );
gboolean gsb_menu_update_accounts_in_menus ( void );
gboolean gsb_menu_update_view_menu ( gint account_number );
GtkWidget *init_menus ( GtkWidget *vbox );
gchar *menu_name ( gchar *menu, gchar *submenu, gchar *subsubmenu );
/* END_DECLARATION */
#endif
