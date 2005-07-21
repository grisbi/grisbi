#ifndef _FENETRE_PRINCIPALE_H
#define _FENETRE_PRINCIPALE_H (1)

typedef enum GSB_GENERAL_NOTEBOOK_PAGES {
    GSB_HOME_PAGE,
    GSB_ACCOUNT_PAGE,
    GSB_SCHEDULER_PAGE,
    GSB_PAYEES_PAGE,
    GSB_CATEGORIES_PAGE,
    GSB_BUDGETARY_LINES_PAGE,
#ifdef HAVE_G2BANKING
    GSB_AQBANKING_PAGE,
#endif
    GSB_REPORTS_PAGE,
} GsbGeneralNotebookPages;

/* START_INCLUDE_H */
#include "fenetre_principale.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget * create_main_widget ( void );
void gsb_gui_headings_update ( gchar * title, gchar * suffix );
void gsb_gui_notebook_change_page ( GsbGeneralNotebookPages page );
gboolean gsb_gui_on_notebook_switch_page ( GtkNotebook *notebook,
					   GtkNotebookPage *page,
					   guint numero_page,
					   gpointer null );
/*END_DECLARATION*/

#endif
