#ifndef _GSB_RGBA_H
#define _GSB_RGBA_H (1)

#include <glib.h>
#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

enum CssRules
{
	CSS_HOME_RULES,
	CSS_TRANSACTIONS_RULES,
	CSS_BET_RULES,
	CSS_PREFS_RULES
};

/* START_DECLARATION */
void		gsb_rgba_create_color_combobox_from_ui  (GtkWidget *combo,
													 gint type);
GdkRGBA *   gsb_rgba_get_couleur                    (const gchar *couleur);
gchar *     gsb_rgba_get_couleur_to_hexa_string     (const gchar *couleur);
gchar *     gsb_rgba_get_couleur_to_string          (const gchar *couleur);
GdkRGBA *   gsb_rgba_get_couleur_with_indice        (const gchar *couleur,
                                                     gint indice);
gchar *		gsb_rgba_get_couleur_with_indice_to_str	(const gchar *couleur,
                                                     gint indice);
gchar *     gsb_rgba_get_string_to_save             (void);
gint		gsb_rgba_get_type_theme					(const gchar *theme_name);
void        gsb_rgba_initialise_couleurs			(const gchar *css_data);
void		gsb_rgba_set_alt_colors					(const gchar *alt_color,
													 const gchar *value);
gboolean    gsb_rgba_set_couleur                    (const gchar *couleur,
                                                     const gchar *value);
gboolean    gsb_rgba_set_couleur_with_indice        (const gchar *couleur,
                                                     gint indice,
                                                     const gchar *value);


/* CSS FUNCTIONS */
void		gsb_css_count_change_dec				(gboolean doublon);
void		gsb_css_count_change_inc				(void);
void		gsb_css_count_change_init				(void);

gchar *		gsb_css_get_filename					(void);
gchar *		gsb_css_get_property_from_name			(const gchar *name,
													 const gchar *property);
void		gsb_css_load_css_data_from_file			(GtkCssProvider *css_provider);
void		gsb_css_set_color_property				(GdkRGBA *color,
													 gchar *property);
void 		gsb_css_set_property_from_name			(const gchar *name,
													 const gchar *value);
void		gsb_css_set_property_from_selector		(const gchar *selector,
													 const gchar *property,
													 const gchar *value);
gboolean	gsb_css_test_user_css_file				(void);
/* END_DECLARATION */
#endif
