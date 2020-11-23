/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                           */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>

 /*START_INCLUDE*/
#include "gsb_rgba.h"
#include "grisbi_app.h"
#include "gsb_dirs.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct	_ComboColor		ComboColor;

struct _ComboColor
{
    const gchar *name;
	const gchar *css_name;
    GdkRGBA *color;
    GdkRGBA *old_color;
};

/*START_STATIC*/
/*HOME_COLORS*/
static GdkRGBA background_ligne_compte;
static GdkRGBA background_grey_box;
static GdkRGBA couleur_nom_compte_normal;
static GdkRGBA couleur_nom_compte_hover;
static GdkRGBA couleur_solde_normal;
static GdkRGBA couleur_solde_hover;
static GdkRGBA couleur_solde_alarme_low_normal;
static GdkRGBA couleur_solde_alarme_low_hover;
static GdkRGBA couleur_solde_alarme_high_normal;
static GdkRGBA couleur_solde_alarme_high_hover;

static ComboColor config_home_colors [] = {
    {N_("Background color of account row"), "background_ligne_compte", &background_ligne_compte, 0},
    {N_("Background color of logo title"), "background_grey_box", &background_grey_box, 0},
    {N_("Color of account text"), "couleur_nom_compte_normal", &couleur_nom_compte_normal, 0},
    {N_("Color of account text hovered"), "couleur_nom_compte_hover", &couleur_nom_compte_hover, 0},
    {N_("Color of balance text"), "couleur_solde_normal", &couleur_solde_normal, 0},
    {N_("Color of balance text hovered"), "couleur_solde_hover", &couleur_solde_hover, 0},
    {N_("Color of balance text in low alarm"), "couleur_solde_alarme_low_normal", &couleur_solde_alarme_low_normal, 0},
    {N_("Color of balance text hovered in low alarm"), "couleur_solde_alarme_low_hover", &couleur_solde_alarme_low_hover, 0},
    {N_("Color of balance text in high alarm"), "couleur_solde_alarme_high_normal", &couleur_solde_alarme_high_normal, 0},
    {N_("Color of balance text hovered in high alarm"), "couleur_solde_alarme_high_hover", &couleur_solde_alarme_high_hover, 0},
    {NULL, 0, 0, 0},
   };

/*TRANSACTIONS_LIST_COLORS*/
static GdkRGBA background_archive;
static GdkRGBA background_jour;
static GdkRGBA background_scheduled;
static GdkRGBA background_selection;
static GdkRGBA background_split;
static GdkRGBA couleur_fond_0;
static GdkRGBA couleur_fond_1;
static GdkRGBA text_selection;
static GdkRGBA text_color_0;
static GdkRGBA text_color_1;
static GdkRGBA text_color_2;
static GdkRGBA text_unfinished_split;
static GdkRGBA text_insensitive_entry;

static ComboColor config_transactions_colors [] = {
    {N_("Background color 1 in lists of transactions"), "couleur_fond_0", &couleur_fond_0, 0},
    {N_("Background color 2 in lists of transactions"), "couleur_fond_1", &couleur_fond_1, 0},
    {N_("Background of the transaction that gives the balance today"), "background_jour", &background_jour, 0},
    {N_("Background color of split transaction children"), "background_split", &background_split, 0},
    {N_("Background color of selected transaction"), "background_selection", &background_selection, 0},
    {N_("Background color of virtual scheduled transactions"), "background_scheduled", &background_scheduled, 0},
    {N_("Background color of archives"), "background_archive", &background_archive, 0},
    {N_("Color of transaction text"), "text_color_0", &text_color_0, 0},
    {N_("Color of unfinished split transaction text"), "text_unfinished_split", &text_unfinished_split, 0},
    {N_("Text color of a negative balance"), "text_color_1", &text_color_1, 0},
    {N_("Color of selected transaction text"), "text_selection", &text_selection, 0},
    {N_("Color of unsensitive text"), "text_insensitive_entry", &text_insensitive_entry, 0},
    {NULL, 0, 0, 0},
   };

/*BET_COLORS*/
/* colors for the balance estimate module */
static GdkRGBA background_bet_division;
static GdkRGBA background_bet_future;
static GdkRGBA background_bet_solde;
static GdkRGBA background_bet_transfert;

static ComboColor config_bet_colors [] = {
    {N_("Background color of historical data"), "background_bet_division", &background_bet_division, 0},
    {N_("Background color of futur data"), "background_bet_future", &background_bet_future, 0},
    {N_("Background color of month balance"), "background_bet_solde", &background_bet_solde, 0},
    {N_("Background color of transfer data"), "background_bet_transfert", &background_bet_transfert, 0},
    {NULL, 0, 0, 0},
   };

/*PREFS_RULES*/
static GdkRGBA text_gsetting_option_normal;
static GdkRGBA text_gsetting_option_hover;

static ComboColor config_prefs_colors [] = {
    {N_("Color of gsetting text"), "text_gsetting_option_normal", &text_gsetting_option_normal, 0},
    {N_("Color of gsetting text hovered"), "text_gsetting_option_hover", &text_gsetting_option_hover, 0},
    {NULL, 0, 0, 0},
   };

static ComboColor config_indiponible [] = {
    {N_("Not available"), NULL, 0, 0},
    {NULL, 0, 0, 0},
   };

/* Couleurs alternatives. Sert à mémoriser les anciennes couleurs de grisbi pour compatibilité descendante */
/* Couleurs des tree_view */
static GdkRGBA alt_background_archive;
static GdkRGBA alt_background_scheduled;
static GdkRGBA alt_background_split;
static GdkRGBA alt_couleur_fond[2];
static GdkRGBA alt_background_jour;
static GdkRGBA alt_couleur_selection;
static GdkRGBA alt_text_color[2];

/* colors for the balance estimate module */
static GdkRGBA alt_couleur_bet_division;
static GdkRGBA alt_couleur_bet_future;
static GdkRGBA alt_couleur_bet_solde;
static GdkRGBA alt_couleur_bet_transfert;
/*END_STATIC*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Cette fonction remplace la valeur de la ligne de couleur recherchée
 *
 * \param	line_needle		ligne de couleur recherchée
 * \param	new_color_value	nouvelle couleur
 *
 * \return	new_css_data	renvoie les données css mises à jour
 **/
static gchar *gsb_rgba_css_color_get_data_with_new_color (const gchar *line_needle,
														  const gchar *new_color_value)
{
	const gchar *css_data;
	gchar *first_part = NULL;
	gchar *new_data = NULL;
	gchar **tab_property;
	gchar *tmp_str = NULL;
	gchar **tmp_tab;

	css_data = grisbi_app_get_css_data ();
	tmp_str = g_strrstr (css_data, line_needle);
	if (tmp_str)
	{
		first_part = g_strndup (css_data, tmp_str - css_data);
		tab_property = g_strsplit (tmp_str, ";", 2);
		tmp_tab = g_strsplit (tab_property[0], " ", -1);
		new_data = g_strconcat (first_part, tmp_tab[0], " ", tmp_tab[1], " ", new_color_value, ";", tab_property[1], NULL);

		g_free (first_part);
		g_strfreev (tab_property);
		g_strfreev (tmp_tab);
	}

	return new_data;
}
/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * initialize the colors for grisbi from CSS data
 *
 * \param
 *
 * \return
 **/
void gsb_rgba_initialise_couleurs (const gchar *css_data)
{
	gchar **tab_colors;
	guint i = 0;

    devel_debug (NULL);

    if (strlen(css_data) == 0)
	    return;

	tab_colors = g_strsplit (css_data, ";", -1);
	for (i = 0; i < g_strv_length (tab_colors) -1; i++)
	{
		if (g_strrstr (tab_colors[i], "@define-color"))
		{
			GdkRGBA *color;
			gchar **tab;

			tab = g_strsplit (tab_colors[i], " ", 3);

			color = gsb_rgba_get_couleur (tab[1]);
			if (!gdk_rgba_parse (color, tab[2]))
					notice_debug (tab[1]);
			g_strfreev (tab);
		}
	}
	g_strfreev (tab_colors);
}

/**
 * retourne la couleur demandée.
 *
 * \param gchar couleur
 *
 * \return a GdkRGBA
 **/
GdkRGBA *gsb_rgba_get_couleur (const gchar *couleur)
{
	/* couleurs actives */
	/* home colors */
    if (strcmp (couleur, "background_ligne_compte") == 0)
        return &background_ligne_compte;
    else if (strcmp (couleur, "background_grey_box") == 0)
        return &background_grey_box;
    else if (strcmp (couleur, "couleur_nom_compte_normal") == 0)
        return &couleur_nom_compte_normal;
    else if (strcmp (couleur, "couleur_nom_compte_hover") == 0)
        return &couleur_nom_compte_hover;
    else if (strcmp (couleur, "couleur_solde_normal") == 0)
        return &couleur_solde_normal;
    else if (strcmp (couleur, "couleur_solde_hover") == 0)
        return &couleur_solde_hover;
    else if (strcmp (couleur, "couleur_solde_alarme_low_normal") == 0)
        return &couleur_solde_alarme_low_normal;
    else if (strcmp (couleur, "couleur_solde_alarme_low_hover") == 0)
        return &couleur_solde_alarme_low_hover;
    else if (strcmp (couleur, "couleur_solde_alarme_high_normal") == 0)
        return &couleur_solde_alarme_high_normal;
    else if (strcmp (couleur, "couleur_solde_alarme_high_hover") == 0)
        return &couleur_solde_alarme_high_hover;

	/* transactions colors */
    else if (strcmp (couleur, "background_archive") == 0)
        return &background_archive;
    else if (strcmp (couleur, "background_scheduled") == 0)
        return &background_scheduled;
    else if (strcmp (couleur, "background_split") == 0)
        return &background_split;
	else if (strcmp (couleur, "couleur_fond_0") == 0)
		return &couleur_fond_0;
	else if (strcmp (couleur, "couleur_fond_1") == 0)
		return &couleur_fond_1;
    else if (strcmp (couleur, "background_jour") == 0)
        return &background_jour;
    else if (strcmp (couleur, "background_selection") == 0)
        return &background_selection;
    else if (strcmp (couleur, "text_selection") == 0)
        return &text_selection;
	else if (strcmp (couleur, "text_color_0") == 0)
		return &text_color_0;
	else if (strcmp (couleur, "text_color_1") == 0)
		return &text_color_1;
	else if (strcmp (couleur, "text_color_2") == 0)
		return &text_color_2;
    else if (strcmp (couleur, "text_insensitive_entry") == 0)
        return &text_insensitive_entry;
    else if (strcmp (couleur, "text_unfinished_split") == 0)
        return &text_unfinished_split;

	/* bet colors */
    else if (strcmp (couleur, "background_bet_division") == 0)
        return &background_bet_division;
    else if (strcmp (couleur, "background_bet_future") == 0)
        return &background_bet_future;
    else if (strcmp (couleur, "background_bet_solde") == 0)
        return &background_bet_solde;
    else if (strcmp (couleur, "background_bet_transfert") == 0)
        return &background_bet_transfert;

	/* other colors */
	else if (strcmp (couleur, "text_gsetting_option_normal") == 0)
        return &text_gsetting_option_normal;
	else if (strcmp (couleur, "text_gsetting_option_hover") == 0)
		return &text_gsetting_option_hover;

	/* anciennes couleurs juste pour compatibilité descendante */
    else if (strcmp (couleur, "alt_background_archive") == 0)
        return &alt_background_archive;
    else if (strcmp (couleur, "alt_background_scheduled") == 0)
        return &alt_background_scheduled;
    else if (strcmp (couleur, "alt_background_split") == 0)
        return &alt_background_split;
    else if (strcmp (couleur, "alt_background_jour") == 0)
        return &alt_background_jour;
    else if (strcmp (couleur, "alt_couleur_selection") == 0)
        return &alt_couleur_selection;
    else if (strcmp (couleur, "alt_couleur_bet_division") == 0)
        return &alt_couleur_bet_division;
    else if (strcmp (couleur, "alt_couleur_bet_future") == 0)
        return &alt_couleur_bet_future;
    else if (strcmp (couleur, "alt_couleur_bet_solde") == 0)
        return &alt_couleur_bet_solde;
    else if (strcmp (couleur, "alt_couleur_bet_transfert") == 0)
        return &alt_couleur_bet_transfert;
    return NULL;
}

/**
 * set the colors
 *
 * \param gchar couleur
 * \param gchar composante de la couleur (red, green, blue)
 * \param gchar value
 *
 * \return TRUE
 **/
gboolean gsb_rgba_set_couleur (const gchar *couleur,
                               const gchar *value)
{
    GdkRGBA *color;
	gboolean result;
    color = gsb_rgba_get_couleur (couleur);

    if (color == NULL)
        return FALSE;

    result = gdk_rgba_parse (color, value);

    return result;
}

/**
 * retourne la couleur demandée.
 *
 * \param gchar couleur
 *
 * \return textual specification of color where r, g, and b are represented as
 * integers in the range 0 to 255, and a is represented as floating point value
 * in the range 0 to 1.
 **/
gchar *gsb_rgba_get_couleur_to_string (const gchar *couleur)
{
    GdkRGBA *color;
    gchar *string;

    color = gsb_rgba_get_couleur (couleur);
    string = gdk_rgba_to_string (color);

    return string;
}

/**
 * retourne la couleur demandée.
 *
 *\param gchar couleur
 *\param gint indice du tableau
 *
 **/
GdkRGBA *gsb_rgba_get_couleur_with_indice (const gchar *couleur,
                        gint indice)
{
    if (strcmp (couleur, "couleur_fond") == 0)
	{
		if (indice == 0)
        	return &couleur_fond_0;
		else
			return &couleur_fond_1;
	}
    else if (strcmp (couleur, "text_color") == 0)
	{
		if (indice == 0)
        	return &text_color_0;
		else if (indice == 1)
        	return &text_color_1;
		else
			return &text_color_2;
	}

	/* anciennes couleurs juste pour compatibilité descendante */
    if (strcmp (couleur, "alt_couleur_fond") == 0)
        return &alt_couleur_fond[indice];
    else if (strcmp (couleur, "alt_text_color") == 0)
        return &alt_text_color[indice];

    return NULL;
}

/**
 * set the colors
 *
 * \param gchar couleur
 * \param gint indice du tableau
 * \param gchar composante de la couleur (ref, green, blue)
 *
 * \return TRUE
 **/
gboolean gsb_rgba_set_couleur_with_indice (const gchar *couleur,
                                           gint indice,
                                           const gchar *value)
{
    GdkRGBA *color;

    color = gsb_rgba_get_couleur_with_indice (couleur, indice);

    if (color == NULL)
        return FALSE;

    (void)gdk_rgba_parse (color, value);

    return TRUE;
}

gchar *gsb_rgba_get_couleur_with_indice_to_str	(const gchar *couleur,
												 gint indice)
{
    GdkRGBA *color;
	gchar *str_color = NULL;

    color = gsb_rgba_get_couleur_with_indice (couleur, indice);

    if (color == NULL)
		return NULL;

	str_color = gdk_rgba_to_string (color);

	return str_color;
}

/**
 * create a list of customable colors
 *
 * \param
 *
 * \return a GtkComboBox
 **/
void gsb_rgba_create_color_combobox_from_ui (GtkWidget *combo,
											 gint type)
{
    GtkListStore *store;
    gint i;
    GtkCellRenderer *renderer;
	ComboColor *config_colors;

	switch (type)
	{
		case CSS_HOME_RULES:
			config_colors = config_home_colors;
		break;

		case CSS_TRANSACTIONS_RULES:
			config_colors = config_transactions_colors;
		break;

		case CSS_BET_RULES:
			config_colors = config_bet_colors;
		break;

		case CSS_PREFS_RULES:
			config_colors = config_prefs_colors;
		break;
		default:
			config_colors = config_indiponible;
	}

    /* the store contains the name of the color we can modify and
     * a pointer to the corresponding variable */
    store = gtk_list_store_new (4,
								G_TYPE_STRING,		/* combo name */
								G_TYPE_STRING,		/* css name	*/
								G_TYPE_POINTER,		/* rgba color */
								G_TYPE_POINTER);	/* old rgba color */
    /* fill the store */
    for (i = 0 ; config_colors[i].name != NULL ; i++)
    {
		GtkTreeIter iter;

		gtk_list_store_append (GTK_LIST_STORE (store), &iter);
		gtk_list_store_set (GTK_LIST_STORE (store),
							&iter,
							0, gettext (config_colors[i].name),
							1, config_colors[i].css_name,
							2, config_colors[i].color,
							3, config_colors[i].old_color,
							-1);
    }

    /* create the combobox */
    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer, "text", 0, NULL);
}

/**
 * construit la chaine de sauvegarde des couleurs de Grisbi
 *
 *
 * \return new_string;
 **/
gchar *gsb_rgba_get_string_to_save (void)
{
    gchar *new_string = NULL;

    devel_debug (NULL);

    /* save all colors */
    new_string = g_markup_printf_escaped ("\t<RGBA\n"
										  "\t\tBackground_color_0=\"%s\"\n"
                        				  "\t\tBackground_color_1=\"%s\"\n"
                        				  "\t\tCouleur_jour=\"%s\"\n"
                        				  "\t\tBackground_scheduled=\"%s\"\n"
				                          "\t\tBackground_archive=\"%s\"\n"
				                          "\t\tSelection=\"%s\"\n"
				                          "\t\tBackground_split=\"%s\"\n"
				                          "\t\tText_color_0=\"%s\"\n"
				                          "\t\tText_color_1=\"%s\"\n"
				                          "\t\tCouleur_bet_division=\"%s\"\n"
				                          "\t\tCouleur_bet_future=\"%s\"\n"
				                          "\t\tCouleur_bet_solde=\"%s\"\n"
				                          "\t\tCouleur_bet_transfert=\"%s\" />\n",
										  gdk_rgba_to_string (&alt_couleur_fond[0]),
										  gdk_rgba_to_string (&alt_couleur_fond[1]),
										  gdk_rgba_to_string (&alt_background_jour),
										  gdk_rgba_to_string (&alt_background_scheduled),
										  gdk_rgba_to_string (&alt_background_archive),
										  gdk_rgba_to_string (&alt_couleur_selection),
										  gdk_rgba_to_string (&alt_background_split),
										  gdk_rgba_to_string (&alt_text_color[0]),
										  gdk_rgba_to_string (&alt_text_color[1]),
										  gdk_rgba_to_string (&alt_couleur_bet_division),
										  gdk_rgba_to_string (&alt_couleur_bet_future),
										  gdk_rgba_to_string (&alt_couleur_bet_solde),
										  gdk_rgba_to_string (&alt_couleur_bet_transfert));

    return new_string;
}

/**
 * retourne couleur sous forme hexadécimale
 *
 * \param gchar     couleur à convertir
 *
 * \return
 **/
gchar *gsb_rgba_get_couleur_to_hexa_string  (const gchar *couleur)
{
    GdkRGBA *rgba;

	rgba = gsb_rgba_get_couleur (couleur);

    return g_strdup_printf ("#%02X%02X%02X",
                              (int)(0.5 + CLAMP (rgba->red, 0., 1.) * 255.),
                              (int)(0.5 + CLAMP (rgba->green, 0., 1.) * 255.),
                              (int)(0.5 + CLAMP (rgba->blue, 0., 1.) * 255.));
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_rgba_get_type_theme (const gchar *theme_name)
{
	gchar *tmp_theme_name;
	gint type_theme;

	tmp_theme_name = g_ascii_strdown (theme_name, -1);

	if (g_strstr_len (tmp_theme_name, -1, "dark"))
	{
		type_theme = 2;
	}
	else if (g_strstr_len (tmp_theme_name, -1, "light"))
	{
		type_theme = 3;
	}
	else
	{
		type_theme = 1;
	}
	g_free (tmp_theme_name);

	return type_theme;
}

/* CSS FUNCTIONS */
/**
 * retourne le nom du fichier css actif : soit local si il existe
 * sinon le fichier de base de grisbi
 *
 * \param
 *
 * \return string must be freed
 **/
gchar *gsb_css_get_filename (void)
{
	gchar *css_filename = NULL;
	gchar *tmp_str = NULL;
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	if (a_conf->use_type_theme == 2)
		tmp_str = g_strdup ("grisbi-dark.css");
	else
		tmp_str = g_strdup ("grisbi.css");

	css_filename = g_build_filename (gsb_dirs_get_user_config_dir (), tmp_str, NULL);
	if (g_file_test (css_filename, G_FILE_TEST_EXISTS) == FALSE)
	{
		g_free (css_filename);
    	css_filename = g_build_filename (gsb_dirs_get_ui_dir (), tmp_str, NULL);
	}

	g_free (tmp_str);

	return css_filename;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gchar *gsb_css_get_property_from_name (const gchar *name,
									   const gchar *property)
{
	const gchar *css_data;
	gchar *tmp_str = NULL;

	css_data = grisbi_app_get_css_data ();
	tmp_str = g_strrstr (css_data, name);
	if (tmp_str)
	{
		gchar *start_str;
		gchar *end_str;
		gchar *rule;

		start_str = g_strstr_len (tmp_str, -1, "{");
		end_str = g_strstr_len (tmp_str, -1, "}");
		rule = g_strndup (start_str+1, (end_str-start_str-1));
		tmp_str = g_strrstr (rule, property);
		if (tmp_str)
		{
			gchar **tab_property;
			gsize length;

			tab_property = g_strsplit (tmp_str, ":", 2);

			/* deleting ";" at the end of string */
			length = strlen (tab_property[1]);
			tmp_str = g_strndup (tab_property[1], length -2);

			g_free (rule);
			g_strfreev (tab_property);

			return tmp_str;
		}
		else
			g_free (rule);
	}

	return NULL;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void gsb_css_set_color_property (GdkRGBA *color,
								 gchar *couleur)
{
	GtkCssProvider *css_provider = NULL;
	gchar *line_needle;
    gchar *new_color_string;
	gchar *new_data = NULL;

	css_provider = grisbi_app_get_css_provider ();
	new_color_string = gdk_rgba_to_string (color);

	line_needle = g_strconcat ("@define-color ", couleur, " ", NULL);
	new_data = gsb_rgba_css_color_get_data_with_new_color (line_needle, new_color_string);
	if (new_data)
	{
		GrisbiAppConf *a_conf;

		a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
		gtk_css_provider_load_from_data (css_provider, new_data, -1, NULL);
		grisbi_app_set_css_data (new_data);
		a_conf->prefs_change_css_data = TRUE;

		g_free (new_data);
	}

	g_free (new_color_string);
	g_free (line_needle);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void gsb_css_set_property_from_name (const gchar *name,
									 const gchar *value)
{
	const gchar *css_data;
	gchar *tmp_str = NULL;

	css_data = grisbi_app_get_css_data ();
	tmp_str = g_strrstr (css_data, name);
	if (tmp_str)
	{
		GtkCssProvider *css_provider = NULL;
		gchar *first_part = NULL;
		gchar *new_data = NULL;
		gchar **tab_rule;
		GrisbiAppConf *a_conf;

		a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
		first_part = g_strndup (css_data, tmp_str - css_data);
		tab_rule = g_strsplit (tmp_str, "#", 3);
		new_data = g_strconcat (first_part, value, "\n#", tab_rule[2], NULL);

		css_provider = grisbi_app_get_css_provider ();
		gtk_css_provider_load_from_data (css_provider, new_data, -1, NULL);
		grisbi_app_set_css_data (new_data);
		a_conf->prefs_change_css_data = TRUE;

		g_free (first_part);
		g_free (new_data);
		g_strfreev (tab_rule);
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_css_test_user_css_file (void)
{
	gchar *css_filename = NULL;
	gchar *tmp_str = NULL;
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	if (a_conf->use_type_theme == 2)
		tmp_str = g_strdup ("grisbi-dark.css");
	else
		tmp_str = g_strdup ("grisbi.css");

	css_filename = g_build_filename (gsb_dirs_get_user_config_dir (), tmp_str, NULL);
	if (g_file_test (css_filename, G_FILE_TEST_EXISTS))
	{
		g_free (css_filename);
		g_free (tmp_str);

		return TRUE;
	}

	g_free (tmp_str);

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_css_load_css_data_from_file (GtkCssProvider *css_provider)
{
	GFile *file = NULL;
	gchar *css_data = NULL;
	gchar *css_filename;

	if (!css_provider)
	{
		css_provider = gtk_css_provider_new ();
	}
	css_filename = gsb_css_get_filename ();
	file = g_file_new_for_path (css_filename);
	gtk_css_provider_load_from_file (css_provider, file, NULL);
	g_free (css_filename);
	g_object_unref (file);

	css_data = gtk_css_provider_to_string (css_provider);
	grisbi_app_set_css_data (css_data);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
