#ifndef _GSB_COLOR_H
#define _GSB_COLOR_H (1)

#include <glib.h>
#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *gsb_color_create_color_combobox ( void );
void gsb_color_initialise_couleurs_par_defaut ( void );
GdkColor *gsb_color_get_couleur ( gchar *couleur );
gchar *gsb_color_get_strings_to_save ( void );
gboolean gsb_color_set_couleur ( gchar *couleur, gint value );
void gsb_color_set_colors_to_default ( void );
/* END_DECLARATION */
#endif
