#ifndef _GRISBI_OSX_H
#define _GRISBI_OSX_H (1)

#ifdef GTKOSXAPPLICATION
#include <gtkosxapplication.h>
#endif /* GTKOSXAPPLICATION */

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void grisbi_osx_app_active_cb ( GtkOSXApplication* app, gboolean* data );
void grisbi_osx_app_update_menus_cb ( void );
gchar *grisbi_osx_get_locale_dir ( void );
gchar *grisbi_osx_get_pixmaps_dir ( void );
gchar *grisbi_osx_get_plugins_dir ( void );

GtkWidget *grisbi_osx_init_menus ( GtkWidget *window, GtkWidget *vbox );
/* END_DECLARATION */


#endif
