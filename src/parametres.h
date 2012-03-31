#ifndef _PARAMETRES_H
#define _PARAMETRES_H (1)

#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "structures.h"
/*END_INCLUDE*/


/* Preference pages */
enum preference_page  {
    NOT_A_PAGE = -1,
    FILES_PAGE,
    ARCHIVE_PAGE,
    IMPORT_PAGE,
    IMPORT_ASSOCIATION_PAGE,
    SOFTWARE_PAGE,
    SCHEDULER_PAGE,
    LOCALISATION_PAGE,
    MAIN_PAGE,
    FONTS_AND_LOGO_PAGE,
    MESSAGES_AND_WARNINGS_PAGE,
    ADDRESSES_AND_TITLES_PAGE,
    TOTALS_PAGE,
    TOOLBARS_PAGE,
    TRANSACTIONS_PAGE,
    TRANSACTIONS_CELLULES_PAGE,
    DELETE_MESSAGES_PAGE,
    RECONCILIATION_PAGE,
    RECONCILIATION_SORT_PAGE,
    TRANSACTION_FORM_PAGE,
    TRANSACTION_FORM_BEHAVIOR_PAGE,
    TRANSACTION_FORM_COMPLETION_PAGE,
    CURRENCIES_PAGE,
    CURRENCY_LINKS_PAGE,
    BANKS_PAGE,
    FINANCIAL_YEARS_PAGE,
    METHODS_OF_PAYMENT_PAGE,
    BET_GENERAL_PAGE,
    BET_ACCOUNT_PAGE,
    NUM_PREFERENCES_PAGES
};

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/*START_DECLARATION*/
gboolean gsb_config_backup_dir_chosen ( GtkWidget *button,
                        GtkWidget *dialog );
GtkWidget *gsb_config_date_format_chosen ( GtkWidget *parent, gint sens );
gboolean gsb_config_metatree_re_sort_divisions ( GtkWidget *checkbutton,
                        GdkEventButton *event,
                        gpointer data );
GtkWidget *gsb_config_number_format_chosen ( GtkWidget *parent, gint sens );
gboolean gsb_gui_encryption_toggled ( GtkWidget * checkbox, gpointer data );
gboolean gsb_preferences_menu_open ( GtkWidget *menu_item,
                        gpointer page_ptr );
void parametres_affiche_derniers_fichiers_ouverts ( void );
void parametres_files_set_account_file_path ( const gchar *path,
                        GrisbiAppConf *conf );
void parametres_files_set_backup_path ( const gchar *path,
                        GrisbiAppConf *conf );
void parametres_files_set_import_files_path ( const gchar *path,
                        GrisbiAppConf *conf );
void parametres_files_set_modified ( gboolean modified );
void parametres_import_associations_check_add_button ( void );
GtkWidget *parametres_import_associations_get_combo_payees ( GrisbiWindowEtat *etat );
void parametres_import_associations_init_callback ( void );
void parametres_import_associations_init_treeview ( GtkWidget *tree_view );
gboolean preferences ( gint page );
/*END_DECLARATION*/

#endif

