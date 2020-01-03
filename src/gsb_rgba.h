#ifndef _GSB_RGBA_H
#define _GSB_RGBA_H (1)

#include <glib.h>
#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget * gsb_rgba_create_color_combobox          (void);
void        gsb_rgba_initialise_couleurs_par_defaut (void);
GdkRGBA *   gsb_rgba_get_couleur                    (const gchar *couleur);
gchar *     gsb_rgba_get_couleur_to_hexa_string     (const gchar *couleur);
gchar *     gsb_rgba_get_couleur_to_string          (const gchar *couleur);
GdkRGBA *   gsb_rgba_get_couleur_with_indice        (const gchar *couleur,
                                                     gint indice);
gchar *		gsb_rgba_get_couleur_with_indice_to_str	(const gchar *couleur,
                                                     gint indice);
gchar *     gsb_rgba_get_string_to_save             (void);
void		gsb_rgba_set_alt_colors					(void);
gboolean	gsb_rgba_set_css_color_property			(GdkRGBA *color,
													 gchar *property);
gboolean    gsb_rgba_set_couleur                    (const gchar *couleur,
                                                     const gchar *value);
gboolean    gsb_rgba_set_couleur_with_indice        (const gchar *couleur,
                                                     gint indice,
                                                     const gchar *value);
void        gsb_rgba_set_colors_to_default          (void);

/* END_DECLARATION */
#endif
