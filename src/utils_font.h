#ifndef _UTILS_FONT_H
#define _UTILS_FONT_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
GtkWidget *utils_font_create_button ( gchar **fontname,
				      GCallback hook,
				      gpointer data  );
/* END_DECLARATION */
#endif
