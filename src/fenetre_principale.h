#ifndef _FENETRE_PRINCIPALE_H
#define _FENETRE_PRINCIPALE_H (1)

#include <gtk/gtk.h>

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
} GsbAccountNotebookPages;

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_gui_fill_general_notebook ( GtkWidget *notebook );
GtkWidget *gsb_gui_get_account_page ( void );
GtkWidget *gsb_gui_get_general_notebook (void );
void gsb_gui_notebook_change_page ( GsbGeneralNotebookPages page );
void gsb_gui_on_account_change_page ( GsbAccountNotebookPages page );
gboolean gsb_gui_on_account_switch_page ( GtkNotebook *notebook,
                        gpointer page,
                        guint page_number,
                        gpointer null );
void gsb_gui_update_all_toolbars ( void );
/*END_DECLARATION*/

#endif
