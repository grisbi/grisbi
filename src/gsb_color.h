#ifndef _GSB_COLOR_H
#define _GSB_COLOR_H (1)

#include <glib.h>
#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *gsb_color_create_color_combobox ( void );
void gsb_color_initialise_couleurs_par_defaut ( void );
GdkColor *gsb_color_get_couleur ( const gchar *couleur );
gchar *gsb_color_get_couleur_to_string ( const gchar *couleur );
GdkColor *gsb_color_get_couleur_with_indice ( const gchar *couleur,
                        gint indice );
gchar *gsb_color_get_strings_to_save ( void );
gboolean gsb_color_set_couleur ( const gchar *couleur,
                        gchar *component,
                        gint value );
gboolean gsb_color_set_couleur_with_indice ( const gchar *couleur,
                        gint indice,
                        gchar *component,
                        gint value );
void gsb_color_set_colors_to_default ( void );
/* END_DECLARATION */
#endif
