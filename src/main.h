#ifndef _MAIN_H
#define _MAIN_H (1)

#include <glib.h>
#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
int main ( int argc, char **argv );

gchar *grisbi_app_get_print_dir_var ( void );
gchar *grisbi_app_get_print_locale_var ( void );
gboolean grisbi_app_quit ( void );
gboolean grisbi_win_set_grisbi_title ( gint account_number );
/* END_DECLARATION */
#endif
