#ifndef _FENETRE_PRINCIPALE_H
#define _FENETRE_PRINCIPALE_H (1)

typedef enum GSB_GENERAL_NOTEBOOK_PAGES {
    GSB_HOME_PAGE,
    GSB_ACCOUNT_PAGE,
    GSB_SCHEDULER_PAGE,
    GSB_PAYEES_PAGE,
    GSB_SIMULATOR_PAGE, 
    GSB_CATEGORIES_PAGE,
    GSB_BUDGETARY_LINES_PAGE,
    GSB_REPORTS_PAGE,
    /** FIXME (later) : define an api so that plugin register here itself.  */
    GSB_AQBANKING_PAGE,
} GsbGeneralNotebookPages;

typedef enum GSB_ACCOUNT_NOTEBOOK_PAGES {
    GSB_TRANSACTIONS_PAGE,
    GSB_ESTIMATE_PAGE,
    GSB_HISTORICAL_PAGE,
    GSB_FINANCE_PAGE,
    GSB_PROPERTIES_PAGE,
} GsbaccountNotebookPages;

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget * create_main_widget ( void );
void gsb_gui_headings_update_suffix ( gchar * suffix );
void gsb_gui_headings_update_title ( gchar * title );
void gsb_gui_notebook_change_page ( GsbGeneralNotebookPages page );
void gsb_gui_on_account_change_page ( GsbaccountNotebookPages page );
gboolean gsb_gui_on_account_switch_page ( GtkNotebook *notebook,
                        GtkNotebookPage *page,
                        guint page_number,
                        gpointer null );
void gsb_gui_sensitive_headings ( gboolean sensitive );
gboolean gsb_gui_update_show_headings ();
/*END_DECLARATION*/

#endif
