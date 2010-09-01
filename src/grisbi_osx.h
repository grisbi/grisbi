#ifndef _GRISBI_OSX_H
#define _GRISBI_OSX_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void grisbi_osx_app_active_cb ( GtkOSXApplication* app, gboolean* data );
GtkWidget *grisbi_osx_create_window ( gchar *title );
gboolean grisbi_osx_app_should_quit_cb ( GtkOSXApplication *app, gpointer data );
void grisbi_osx_app_will_quit_cb ( GtkOSXApplication *app, gpointer data );
void grisbi_osx_print_element ( void );
/* END_DECLARATION */


#endif
