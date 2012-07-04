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
#include "grisbi_window.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean affiche_derniers_fichiers_ouverts ( void );
void efface_derniers_fichiers_ouverts ( void );
gboolean gsb_gui_sensitive_menu_account_name ( gint account_number,
                        gboolean state );
gboolean gsb_gui_sensitive_menu_item ( gchar *item_name,
                        gboolean state );
void gsb_gui_toggle_line_view_mode ( GtkRadioAction *action,
                        GtkRadioAction *current,
                        gpointer user_data );
gboolean gsb_gui_toggle_show_archived ( void );
gboolean gsb_gui_toggle_show_closed_accounts ( void );
gboolean gsb_gui_toggle_show_form ( void );
gboolean gsb_gui_toggle_show_reconciled ( void );
void gsb_menu_full_screen_mode ( void );
GtkUIManager *gsb_menu_get_ui_manager ( void );
gboolean gsb_menu_reinit_largeur_col_menu ( void );
void gsb_menu_sensitive ( gboolean sensitive );
gboolean gsb_menu_set_block_menu_cb ( gboolean etat );
void gsb_menu_preferences ( GtkAction *action,
                        GrisbiWindow *window );
gboolean gsb_menu_transaction_operations_set_sensitive ( gboolean sensitive );
gboolean gsb_menu_update_accounts_in_menus ( void );
gboolean gsb_menu_update_view_menu ( gint account_number );
gboolean help_bugreport ( void );
gboolean help_manual ( void );
gboolean help_quick_start ( void );
gboolean help_translation ( void );
gboolean help_website ( void );
/* END_DECLARATION */
#endif
