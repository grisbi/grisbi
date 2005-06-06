#ifndef NAVIGATION_H
#define NAVIGATION_H


/** Holds data for the navigation tree.  */
enum navigation_cols { 
    NAVIGATION_PIX, NAVIGATION_PIX_VISIBLE,
    NAVIGATION_TEXT, NAVIGATION_FONT,
    NAVIGATION_PAGE, NAVIGATION_ACCOUNT,
    NAVIGATION_REPORT, NAVIGATION_SENSITIVE,
    NAVIGATION_TOTAL,
};


/* START_INCLUDE_H */
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget * create_navigation_pane ( void );
void gsb_gui_navigation_add_account ( gint account_nb );
void gsb_gui_navigation_remove_account ( gint account_nb );
void gsb_gui_navigation_update_account ( gint account_nb );
void gsb_gui_navigation_add_report ( struct struct_etat * report );
void gsb_gui_navigation_remove_report ( struct struct_etat * report );
void gsb_gui_navigation_update_report ( struct struct_etat * report );
/* END_DECLARATION */

#endif
