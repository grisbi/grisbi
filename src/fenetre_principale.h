#ifndef _FENETRE_PRINCIPALE_H
#define _FENETRE_PRINCIPALE_H (1)

typedef enum GSB_GENERAL_NOTEBOOK_PAGES {
    GSB_HOME_PAGE,
    GSB_ACCOUNT_PAGE,
    GSB_SCHEDULER_PAGE,
    GSB_PAYEES_PAGE,
#ifdef ENABLE_BALANCE_ESTIMATE 
    GSB_BALANCE_ESTIMATE_PAGE, 
#endif /*_BALANCE_ESTIMATE_TAB_H*/
    GSB_CATEGORIES_PAGE,
    GSB_BUDGETARY_LINES_PAGE,
    GSB_REPORTS_PAGE,
    /** FIXME (later) : define an api so that plugin register here itself.  */
    GSB_AQBANKING_PAGE,
} GsbGeneralNotebookPages;

/* START_INCLUDE_H */
#include "././balance_estimate_tab.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget * create_main_widget ( void );
void gsb_gui_headings_update ( gchar * title, gchar * suffix );
void gsb_gui_headings_update_suffix ( gchar * suffix );
void gsb_gui_notebook_change_page ( GsbGeneralNotebookPages page );
void gsb_gui_sensitive_headings ( gboolean sensitive );
/*END_DECLARATION*/

#endif
