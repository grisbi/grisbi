#ifndef _PARAMETRES_H
#define _PARAMETRES_H (1)

/* Preference pages */
enum preference_page  {
    NOT_A_PAGE = -1,
    FILES_PAGE,
    ARCHIVE_PAGE,
    IMPORT_PAGE,
    IMPORT_ASSOCIATION_PAGE,
    SOFTWARE_PAGE,
    SCHEDULER_PAGE,
    MAIN_PAGE,
    FONTS_AND_LOGO_PAGE,
    MESSAGES_AND_WARNINGS_PAGE,
    ADDRESSES_AND_TITLES_PAGE,
    TOTALS_PAGE,
    TOOLBARS_PAGE,
    TRANSACTIONS_PAGE,
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
    NUM_PREFERENCES_PAGES
};

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/*START_DECLARATION*/
gboolean gsb_config_backup_dir_chosen ( GtkWidget *button,
                        GtkWidget *dialog );
gboolean gsb_config_metatree_sort_transactions ( GtkWidget *checkbutton,
                        gpointer null );
gboolean gsb_gui_encryption_toggled ( GtkWidget * checkbox, gpointer data );
gboolean gsb_preferences_menu_open ( GtkWidget *menu_item,
                        gpointer page_ptr );
gboolean preferences ( gint page );
/*END_DECLARATION*/

#endif

