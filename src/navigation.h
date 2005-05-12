#ifndef NAVIGATION_H
#define NAVIGATION_H

GtkWidget * create_navigation_pane ( void );


/** Holds data for the navigation tree.  */
enum navigation_cols { 
    NAVIGATION_PIX, NAVIGATION_PIX_VISIBLE,
    NAVIGATION_TEXT, NAVIGATION_FONT,
    NAVIGATION_PAGE, NAVIGATION_ACCOUNT,
    NAVIGATION_REPORT, NAVIGATION_TOTAL,
};

#endif
