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
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/* Couleurs actives peuvent être celles de grisbi ou pour un thème sombre */
/* Couleurs non modifiables */
static GdkRGBA couleur_gsetting_option;

/* Couleurs modifiables dans les préférences */
static GdkRGBA background_archive;
static GdkRGBA background_scheduled;
static GdkRGBA background_split;
static GdkRGBA couleur_fond[2];
static GdkRGBA couleur_jour;
static GdkRGBA couleur_selection_bg;
static GdkRGBA couleur_selection_fg;
static GdkRGBA text_color[2];

/* colors for the balance estimate module */
static GdkRGBA background_bet_division;
static GdkRGBA background_bet_future;
static GdkRGBA background_bet_solde;
static GdkRGBA background_bet_transfert;

/* Couleurs alternatives. Sert à mémoriser les anciennes couleurs de grisbi pour compatibilité descendante */
/* Couleurs des tree_view */
static GdkRGBA alt_background_archive;
static GdkRGBA alt_background_scheduled;
static GdkRGBA alt_background_split;
static GdkRGBA alt_couleur_fond[2];
static GdkRGBA alt_couleur_jour;
static GdkRGBA alt_couleur_selection;
static GdkRGBA alt_text_color[2];

/* colors for the balance estimate module */
static GdkRGBA alt_couleur_bet_division;
static GdkRGBA alt_couleur_bet_future;
static GdkRGBA alt_couleur_bet_solde;
static GdkRGBA alt_couleur_bet_transfert;


/* Couleurs par défaut servent à l'initialisation de grisbi */
static GdkRGBA default_couleur_gsetting_option;

 /* Couleurs des tree_view */
static GdkRGBA default_background_archive;
static GdkRGBA default_couleur_fond[2];
static GdkRGBA default_couleur_jour;
static GdkRGBA default_couleur_selection_bg;
static GdkRGBA default_couleur_selection_fg;
static GdkRGBA default_background_split;
static GdkRGBA default_text_color[2];
static GdkRGBA default_background_scheduled;

/* colors for the balance estimate module */
static GdkRGBA default_background_bet_division;
static GdkRGBA default_background_bet_future;
static GdkRGBA default_background_bet_solde;
static GdkRGBA default_background_bet_transfert;

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
	gchar *css_data;
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
 * initialize the default colors for grisbi
 *
 * \param
 *
 * \return
 * */
void gsb_rgba_initialise_couleurs_par_defaut (const gchar *css_data)
{
	gchar **tab_colors;
	guint i = 0;

    devel_debug (NULL);

	tab_colors = g_strsplit (css_data, ";", -1);
	for (i = 0; i < g_strv_length (tab_colors) -1; i++)
	{
		if (g_strrstr (tab_colors[i], "@define-color"))
		{
			gchar **tab;

			tab = g_strsplit (tab_colors[i], " ", -1);

			if (strcmp (tab[1], "background_archive") == 0)
			{
				/* set the archive background color */
				if (!gdk_rgba_parse (&default_background_archive, tab[2]))
					notice_debug ("default_background_archive en erreur");
			}
			else if (strcmp (tab[1], "background_scheduled") == 0)
			{
				/* color of the non selectable transactions on scheduler */
				if (!gdk_rgba_parse (&default_background_scheduled, tab[2]))
					notice_debug ("default_background_scheduled en erreur");
			}
			else if (strcmp (tab[1], "background_split") == 0)
			{
				/* color for split children */
				if (!gdk_rgba_parse (&default_background_split, tab[2]))
					notice_debug ("default_background_split en erreur");
			}
			else if (strcmp (tab[1], "couleur_fond_0") == 0)
			{
				/* background color of the tree_view line */
				if (!gdk_rgba_parse (&default_couleur_fond[0], tab[2]))
					notice_debug ("default_couleur_fond[0] en erreur");
			}
			else if (strcmp (tab[1], "couleur_fond_1") == 0)
			{
				/* background color of the tree_view line */
				if (!gdk_rgba_parse (&default_couleur_fond[1], tab[2]))
					notice_debug ("default_couleur_fond[1] en erreur");
			}
			else if (strcmp (tab[1], "couleur_jour") == 0)
			{
				/* color of today */
				if (!gdk_rgba_parse (&default_couleur_jour, tab[2]))
					notice_debug ("default_couleur_jour en erreur");
			}
			else if (strcmp (tab[1], "couleur_selection_bg") == 0)
			{
				if (!gdk_rgba_parse (&default_couleur_selection_bg, tab[2]))
					notice_debug ("default_couleur_selection_fg en erreur");
			}
			else if (strcmp (tab[1], "couleur_selection_fg") == 0)
			{
				if (!gdk_rgba_parse (&couleur_selection_fg, tab[2]))
					notice_debug ("default_couleur_selection_fg en erreur");
			}
			else if (strcmp (tab[1], "text_color_0") == 0)
			{
				/* colors of the text */
				if (!gdk_rgba_parse (&default_text_color[0], tab[2]))
					notice_debug ("default_text_color[0] en erreur");
			}
			else if (strcmp (tab[1], "text_color_1") == 0)
			{
				/* colors of the text */
				if (!gdk_rgba_parse (&default_text_color[1], tab[2]))
					notice_debug ("default_text_color[1] en erreur");
			}
			else if (strcmp (tab[1], "background_bet_division") == 0)
			{
				/* colors of the background bet divisions */
				if (!gdk_rgba_parse (&default_background_bet_division, tab[2]))
					notice_debug ("default_background_bet_division en erreur");
			}
			else if (strcmp (tab[1], "background_bet_future") == 0)
			{
				/* colors of the background bet future data */
				if (!gdk_rgba_parse (&default_background_bet_future, tab[2]))
					notice_debug ("default_background_bet_future en erreur");
			}
			else if (strcmp (tab[1], "background_bet_solde") == 0)
			{
				/* colors of the background bet solde data */
				if (!gdk_rgba_parse (&default_background_bet_solde, tab[2]))
					notice_debug ("default_background_bet_solde en erreur");
			}
			else if (strcmp (tab[1], "background_bet_transfert") == 0)
			{
				/* colors of the background bet transfert data */
				if (!gdk_rgba_parse (&default_background_bet_transfert, tab[2]))
					notice_debug ("default_background_bet_transfert en erreur");
			}
			else if (strcmp (tab[1], "couleur_gsetting_option") == 0)
			{
				if (!gdk_rgba_parse (&default_couleur_gsetting_option, tab[2]))
					notice_debug ("default_couleur_gsetting_option en erreur");
			}
			g_strfreev (tab);
		}
	}
}

/**
 * retourne la couleur demandée.
 *
 * \param gchar couleur
 *
 * \return a GdkRGBA
 * */
GdkRGBA *gsb_rgba_get_couleur (const gchar *couleur)
{
	/* couleurs actives */
	if (strcmp (couleur, "couleur_gsetting_option") == 0)
        return &couleur_gsetting_option;
    else if (strcmp (couleur, "background_archive") == 0)
        return &background_archive;
    else if (strcmp (couleur, "background_scheduled") == 0)
        return &background_scheduled;
    else if (strcmp (couleur, "background_split") == 0)
        return &background_split;
    else if (strcmp (couleur, "couleur_jour") == 0)
        return &couleur_jour;
    else if (strcmp (couleur, "couleur_selection_bg") == 0)
        return &couleur_selection_bg;
    else if (strcmp (couleur, "couleur_selection_fg") == 0)
        return &couleur_selection_fg;
    else if (strcmp (couleur, "background_bet_division") == 0)
        return &background_bet_division;
    else if (strcmp (couleur, "background_bet_future") == 0)
        return &background_bet_future;
    else if (strcmp (couleur, "background_bet_solde") == 0)
        return &background_bet_solde;
    else if (strcmp (couleur, "background_bet_transfert") == 0)
        return &background_bet_transfert;
	/* anciennes couleurs juste pour compatibilité descendante */
    else if (strcmp (couleur, "alt_background_archive") == 0)
        return &alt_background_archive;
    else if (strcmp (couleur, "alt_background_scheduled") == 0)
        return &alt_background_scheduled;
    else if (strcmp (couleur, "alt_background_split") == 0)
        return &alt_background_split;
    else if (strcmp (couleur, "alt_couleur_jour") == 0)
        return &alt_couleur_jour;
    else if (strcmp (couleur, "couleur_selection_bg") == 0)
        return &couleur_selection_bg;
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
 * */
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
 * */
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
 * */
GdkRGBA *gsb_rgba_get_couleur_with_indice (const gchar *couleur,
                        gint indice)
{

    if (strcmp (couleur, "couleur_fond") == 0)
        return &couleur_fond[indice];
    else if (strcmp (couleur, "text_color") == 0)
        return &text_color[indice];

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
 * */
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
 * initialize the colors used in grisbi
 *
 * \param
 *
 * \return
 * */
void gsb_rgba_set_colors_to_default (void)
{
	devel_debug (NULL);
	/* couleur des options gérées par gsettings */
	couleur_gsetting_option = default_couleur_gsetting_option;

    /* colors of the background */
    couleur_fond[0] = default_couleur_fond[0];
    couleur_fond[1] = default_couleur_fond[1];

    /* colors of the text */
    text_color[0] = default_text_color[0];
    text_color[1] = default_text_color[1];

    /* color of today */
    couleur_jour = default_couleur_jour;

    /* selection color */
    couleur_selection_bg = default_couleur_selection_bg;
    couleur_selection_fg = default_couleur_selection_fg;

    /* color of the non selectable transactions on scheduler */
    background_scheduled = default_background_scheduled;

    /* set the archive background color */
    background_archive = default_background_archive;

    /* color for split children */
    background_split = default_background_split;

    /* colors of the background bet divisions */
    background_bet_division = default_background_bet_division;

    /* colors of the background bet future data */
    background_bet_future = default_background_bet_future;

    /* colors of the background bet solde data */
    background_bet_solde = default_background_bet_solde;

    /* colors of the background bet transfert data */
    background_bet_transfert = default_background_bet_transfert;
}

/**
 * create a list of customable colors
 *
 * \param
 *
 * \return a GtkComboBox
 * */
GtkWidget *gsb_rgba_create_color_combobox (void)
{
    GtkWidget *combobox;
    GtkListStore *store;
    gint i;
    GtkCellRenderer *renderer;

    struct config_color {
    const gchar *name;
    GdkRGBA *color;
    GdkRGBA *default_color;

    } config_colors[] = {
    { N_("Transaction list background 1"), &couleur_fond[0], &default_couleur_fond[0]},
    { N_("Transaction list background 2"), &couleur_fond[1], &default_couleur_fond[1]},
    { N_("Color for the operation that gives the balance today"), &couleur_jour, &default_couleur_jour},
    { N_("Color of transaction's text"), &text_color[0], &default_text_color[0]},
    { N_("Text of unfinished split transaction"), &text_color[1], &default_text_color[1]},
    { N_("Children of split transaction"), &background_split, &default_background_split},
    { N_("Selection color_bg"), &couleur_selection_bg, &default_couleur_selection_bg},
    { N_("Background of non selectable scheduled transactions"), &background_scheduled, &default_background_scheduled},
    { N_("Archive color"), &background_archive, &default_background_archive},
    { N_("Background of bet division"), &background_bet_division, &default_background_bet_division },
    { N_("Background of bet futur"), &background_bet_future, &default_background_bet_future },
    { N_("Background of bet solde"), &background_bet_solde, &default_background_bet_solde },
    { N_("Background of bet transfer"), &background_bet_transfert, &default_background_bet_transfert },
    { NULL, 0, 0},
    };

    /* the store contains the name of the color we can modify and
     * a pointer to the corresponding variable */
    store = gtk_list_store_new (3,
                        G_TYPE_STRING,
                        G_TYPE_POINTER,
                        G_TYPE_POINTER);
    /* fill the store */
    for (i = 0 ; config_colors[i].name != NULL ; i++)
    {
    GtkTreeIter iter;

    gtk_list_store_append (GTK_LIST_STORE (store),
                        &iter);
    gtk_list_store_set (GTK_LIST_STORE (store),
                        &iter,
                        0, gettext (config_colors[i].name),
                        1, config_colors[i].color,
                        2, config_colors[i].default_color,
                        -1);
    }

    /* create the combobox */
    combobox = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer,
                        "text", 0,
                        NULL);
    return combobox;
}

/**
 * construit la chaine de sauvegarde des couleurs de Grisbi
 *
 *
 * \return new_string;
 * */
gchar *gsb_rgba_get_string_to_save (void)
{
    gchar *new_string = NULL;

    devel_debug (NULL);

	if (conf.tmp_use_dark_theme != conf.use_dark_theme)
	{
		conf.use_dark_theme = conf.tmp_use_dark_theme;
		//~ gsb_rgba_initialise_couleurs_par_defaut ();
		//~ gsb_rgba_set_colors_to_default ();
		gsb_rgba_set_css_color_property (&couleur_selection_bg, "couleur_selection_bg");
		gsb_rgba_set_css_color_property (&couleur_gsetting_option, "couleur_gsetting_option");
		gsb_rgba_set_css_color_property (&text_color[0], "text_color_0");
	}

    /* save all colors */
    new_string = g_markup_printf_escaped ("\t<RGBA\n"
										  "\t\tBackground_color_0=\"%s\"\n"
                        				  "\t\tBackground_color_1=\"%s\"\n"
                        				  "\t\tCouleur_jour=\"%s\"\n"
                        				  "\t\tBackground_scheduled=\"%s\"\n"
				                          "\t\tBackground_archive=\"%s\"\n"
				                          "\t\tCouleur_selection=\"%s\"\n"
				                          "\t\tBackground_split=\"%s\"\n"
				                          "\t\tText_color_0=\"%s\"\n"
				                          "\t\tText_color_1=\"%s\"\n"
				                          "\t\tCouleur_bet_division=\"%s\"\n"
				                          "\t\tCouleur_bet_future=\"%s\"\n"
				                          "\t\tCouleur_bet_solde=\"%s\"\n"
				                          "\t\tCouleur_bet_transfert=\"%s\" />\n",
										  gdk_rgba_to_string (&alt_couleur_fond[0]),
										  gdk_rgba_to_string (&alt_couleur_fond[1]),
										  gdk_rgba_to_string (&alt_couleur_jour),
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
 * */
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
 * \param
 *
 * \return
 **/
void gsb_rgba_set_css_color_property (GdkRGBA *color,
									  gchar *couleur)
{
	GtkCssProvider *css_provider = NULL;
    gchar *new_color_string;
	gchar *new_data = NULL;

	css_provider = grisbi_app_get_css_provider ();
	new_color_string = gdk_rgba_to_string (color);

	if (strcmp (couleur, "couleur_selection_bg") == 0)
	{
		new_data = gsb_rgba_css_color_get_data_with_new_color ("@define-color couleur_selection_bg ", new_color_string);
	}

	else if (strcmp (couleur, "couleur_gsetting_option") == 0)
	{
		new_data = gsb_rgba_css_color_get_data_with_new_color ("@define-color couleur_gsetting_option ", new_color_string);
	}

	else if (strcmp (couleur, "text_color_0") == 0)
	{
		new_data = gsb_rgba_css_color_get_data_with_new_color ("@define-color text_color_0 ", new_color_string);
	}

	if (new_data)
	{
		gtk_css_provider_load_from_data (css_provider, new_data, -1, NULL);
		grisbi_app_set_css_data (new_data);

		g_free (new_data);
	}

	g_free (new_color_string);
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
