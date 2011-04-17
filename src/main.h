#ifndef _MAIN_H
#define _MAIN_H (1)

#include <glib.h>
#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean gsb_main_grisbi_close ( void );
int main (int argc, char **argv);
gchar *gsb_main_get_print_locale_var ( void );
gchar *gsb_main_get_print_dir_var ( void );
GtkWidget *gsb_main_get_main_window ( void );
gboolean gsb_main_set_grisbi_title ( gint account_number );
/* END_DECLARATION */
#endif
