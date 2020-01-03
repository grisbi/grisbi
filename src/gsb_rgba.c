/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2016 Pierre Biava (grisbi@pierre.biava.name)                 */
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
static GdkRGBA couleur_grey_box;
static GdkRGBA couleur_insensitive_text;

/* Couleurs modifiables dans les préférences */
/* Couleurs des tree_view */
static GdkRGBA background_archive;
static GdkRGBA couleur_fond[2];
static GdkRGBA couleur_jour;
static GdkRGBA couleur_selection;
static GdkRGBA background_split;
static GdkRGBA text_color[2];
static GdkRGBA background_scheduled;

/* colors for the balance estimate module */
static GdkRGBA couleur_bet_division;
static GdkRGBA couleur_bet_future;
static GdkRGBA couleur_bet_solde;
static GdkRGBA couleur_bet_transfert;

/* Couleurs alternatives. Sert à mémoriser les couleurs en cas de changement pour un thème sombre */
static GdkRGBA alt_couleur_grey_box;
static GdkRGBA alt_couleur_insensitive_text;

/* Couleurs des tree_view */
static GdkRGBA alt_background_archive;
static GdkRGBA alt_couleur_fond[2];
static GdkRGBA alt_couleur_jour;
static GdkRGBA alt_couleur_selection;
static GdkRGBA alt_background_split;
static GdkRGBA alt_text_color[2];
static GdkRGBA alt_background_scheduled;

/* colors for the balance estimate module */
static GdkRGBA alt_couleur_bet_division;
static GdkRGBA alt_couleur_bet_future;
static GdkRGBA alt_couleur_bet_solde;
static GdkRGBA alt_couleur_bet_transfert;


/* Couleurs par défaut servent à l'initialisation de grisbi */
static GdkRGBA default_couleur_grey_box;
static GdkRGBA default_couleur_insensitive_text;

 /* Couleurs des tree_view */
static GdkRGBA default_background_archive;
static GdkRGBA default_couleur_fond[2];
static GdkRGBA default_couleur_jour;
static GdkRGBA default_couleur_selection;
static GdkRGBA default_background_split;
static GdkRGBA default_text_color[2];
static GdkRGBA default_background_scheduled;

/* colors for the balance estimate module */
static GdkRGBA default_couleur_bet_division;
static GdkRGBA default_couleur_bet_future;
static GdkRGBA default_couleur_bet_solde;
static GdkRGBA default_couleur_bet_transfert;

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
void gsb_rgba_initialise_couleurs_par_defaut (void)
{
    devel_debug (NULL);

    /* définition des couleurs paramétrables */
	if (conf.use_dark_theme)
	{
		/* couleurs pour le thème sombre */
		if (!gdk_rgba_parse (&default_couleur_grey_box, "rgb(66,66,66)"))
			notice_debug ("default_couleur_grey_box en erreur");

		if (!gdk_rgba_parse (&default_couleur_insensitive_text, "rgb(175,175,175)"))
			notice_debug ("default_couleur_insensitive_text en erreur");

		/* colors of the background */
		if (!gdk_rgba_parse (&default_couleur_fond[0], "rgb(76,76,76)"))
			notice_debug ("default_dark_couleur_fond[0] en erreur");

		if (!gdk_rgba_parse (&default_couleur_fond[1], "rgb(76,76,76)"))
			notice_debug ("default_dark_couleur_fond[1] en erreur");

		/* colors of the text */
		if (!gdk_rgba_parse (&default_text_color[0], "white"))
			notice_debug ("default_dark_default_text_color[0] en erreur");

		if (!gdk_rgba_parse (&default_text_color[1], "red"))
			notice_debug ("default_dark_text_color[1] en erreur");

		/* color of today */
		if (!gdk_rgba_parse (&default_couleur_jour, "rgb(150,150,150)"))
			notice_debug ("default_dark_couleur_jour en erreur");

		/* selection color */
		if (!gdk_rgba_parse (&default_couleur_selection, "rgb(245,155,155)"))
			notice_debug ("default_dark_couleur_selection en erreur");

		/* color of the non selectable transactions on scheduler */
		if (!gdk_rgba_parse (&default_background_scheduled, "grey"))
			notice_debug ("default_dark_background_scheduled en erreur");

		/* set the archive background color */
		if (!gdk_rgba_parse (&default_background_archive, "green"))
			notice_debug ("default_dark_background_archive en erreur");

		/* color for split children */
		if (!gdk_rgba_parse (&default_background_split, "black"))
			notice_debug ("default_dark_background_split en erreur");

		/* colors of the background bet divisions */
		if (!gdk_rgba_parse (&default_couleur_bet_division, "rgb(250,255,130)"))
			notice_debug ("default_dark_couleur_bet_division en erreur");

		/* colors of the background bet future data */
		if (!gdk_rgba_parse (&default_couleur_bet_future, "rgb(125,200,140)"))
			notice_debug ("default_dark_couleur_bet_future en erreur");

		/* colors of the background bet solde data */
		if (!gdk_rgba_parse (&default_couleur_bet_solde, "rgb(0,255,235)"))
			notice_debug ("default_dark_couleur_bet_solde en erreur");

		/* colors of the background bet transfert data */
		if (!gdk_rgba_parse (&default_couleur_bet_transfert, "rgb(255,146,233)"))
			notice_debug ("default_dark_couleur_bet_transfert en erreur");
	}
	else
	{
		/* Couleurs de base de grisbi */
		if (!gdk_rgba_parse (&default_couleur_grey_box, "rgb(192,192,192)"))
			notice_debug ("default_couleur_grey_box en erreur");

		if (!gdk_rgba_parse (&default_couleur_insensitive_text, "rgb(80,80,80)"))
			notice_debug ("default_couleur_insensitive_text en erreur");

		/* colors of the background */
		if (!gdk_rgba_parse (&default_couleur_fond[0], "rgb(215,215,255)"))
			notice_debug ("default_couleur_fond[0] en erreur");

		if (!gdk_rgba_parse (&default_couleur_fond[1], "grey100"))
			notice_debug ("default_couleur_fond[1] en erreur");

		/* colors of the text */
		if (!gdk_rgba_parse (&default_text_color[0], "black"))
			notice_debug ("default_default_text_color[0] en erreur");

		if (!gdk_rgba_parse (&default_text_color[1], "red"))
			notice_debug ("default_text_color[1] en erreur");

		/* color of today */
		if (!gdk_rgba_parse (&default_couleur_jour, "rgb(180,180,180)"))
			notice_debug ("default_couleur_jour en erreur");

		/* selection color */
		if (!gdk_rgba_parse (&default_couleur_selection, "rgb(245,155,155)"))
			notice_debug ("default_couleur_selection en erreur");

		/* color of the non selectable transactions on scheduler */
		if (!gdk_rgba_parse (&default_background_scheduled, "grey"))
			notice_debug ("default_background_scheduled en erreur");

		/* set the archive background color */
		if (!gdk_rgba_parse (&default_background_archive, "rgb(60,120,40)"))
			notice_debug ("default_background_archive en erreur");

		/* color for split children */
		if (!gdk_rgba_parse (&default_background_split, "rgb(240,200,200)"))
			notice_debug ("default_background_split en erreur");

		/* colors of the background bet divisions */
		if (!gdk_rgba_parse (&default_couleur_bet_division, "rgb(250,255,130)"))
			notice_debug ("default_couleur_bet_division en erreur");

		/* colors of the background bet future data */
		if (!gdk_rgba_parse (&default_couleur_bet_future, "rgb(125,200,140)"))
			notice_debug ("default_couleur_bet_future en erreur");

		/* colors of the background bet solde data */
		if (!gdk_rgba_parse (&default_couleur_bet_solde, "rgb(0,255,235)"))
			notice_debug ("default_couleur_bet_solde en erreur");

		/* colors of the background bet transfert data */
		if (!gdk_rgba_parse (&default_couleur_bet_transfert, "rgb(255,146,233)"))
			notice_debug ("default_couleur_bet_transfert en erreur");
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
    if (strcmp (couleur, "couleur_grey_box") == 0)
        return &couleur_grey_box;
	else if (strcmp (couleur, "couleur_insensitive_text") == 0)
        return &couleur_insensitive_text;
    else if (strcmp (couleur, "background_scheduled") == 0)
        return &background_scheduled;
    else if (strcmp (couleur, "background_archive") == 0)
        return &background_archive;
    else if (strcmp (couleur, "couleur_jour") == 0)
        return &couleur_jour;
    else if (strcmp (couleur, "couleur_selection") == 0)
        return &couleur_selection;
    else if (strcmp (couleur, "background_split") == 0)
        return &background_split;
    else if (strcmp (couleur, "couleur_bet_division") == 0)
        return &couleur_bet_division;
    else if (strcmp (couleur, "couleur_bet_future") == 0)
        return &couleur_bet_future;
    else if (strcmp (couleur, "couleur_bet_solde") == 0)
        return &couleur_bet_solde;
    else if (strcmp (couleur, "couleur_bet_transfert") == 0)
        return &couleur_bet_transfert;
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
    /* color of grey box */
    couleur_grey_box = default_couleur_grey_box;

	/* couleur insensitive pour widget */
	couleur_insensitive_text = default_couleur_insensitive_text;

    /* colors of the background */
    couleur_fond[0] = default_couleur_fond[0];
    couleur_fond[1] = default_couleur_fond[1];

    /* colors of the text */
    text_color[0] = default_text_color[0];
    text_color[1] = default_text_color[1];

    /* color of today */
    couleur_jour = default_couleur_jour;

    /* selection color */
    couleur_selection = default_couleur_selection;

    /* color of the non selectable transactions on scheduler */
    background_scheduled = default_background_scheduled;

    /* set the archive background color */
    background_archive = default_background_archive;

    /* color for split children */
    background_split = default_background_split;

    /* colors of the background bet divisions */
    couleur_bet_division = default_couleur_bet_division;

    /* colors of the background bet future data */
    couleur_bet_future = default_couleur_bet_future;

    /* colors of the background bet solde data */
    couleur_bet_solde = default_couleur_bet_solde;

    /* colors of the background bet transfert data */
    couleur_bet_transfert = default_couleur_bet_transfert;
}

/**
 * set the alternatives colors used in grisbi
 *
 * \param
 *
 * \return
 * */
void gsb_rgba_set_alt_colors (void)
{
    /* color of grey box */
    alt_couleur_grey_box = couleur_grey_box;

	/* couleur insensitive pour widget */
	alt_couleur_insensitive_text = couleur_insensitive_text;

	/* colors of the background */
    alt_couleur_fond[0] = couleur_fond[0];
    alt_couleur_fond[1] = couleur_fond[1];

    /* colors of the text */
    alt_text_color[0] = text_color[0];
    alt_text_color[1] = text_color[1];

    /* color of today */
    alt_couleur_jour = couleur_jour;

    /* selection color */
    alt_couleur_selection = couleur_selection;

    /* color of the non selectable transactions on scheduler */
    alt_background_scheduled = background_scheduled;

    /* set the archive background color */
    alt_background_archive = background_archive;

    /* color for split children */
    alt_background_split = background_split;

    /* colors of the background bet divisions */
    alt_couleur_bet_division = couleur_bet_division;

    /* colors of the background bet future data */
    alt_couleur_bet_future = couleur_bet_future;

    /* colors of the background bet solde data */
    alt_couleur_bet_solde = couleur_bet_solde;

    /* colors of the background bet transfert data */
    alt_couleur_bet_transfert = couleur_bet_transfert;
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
    { N_("Selection color"), &couleur_selection, &default_couleur_selection},
    { N_("Background of non selectable scheduled transactions"), &background_scheduled, &default_background_scheduled},
    { N_("Archive color"), &background_archive, &default_background_archive},
    { N_("Background of bet division"), &couleur_bet_division, &default_couleur_bet_division },
    { N_("Background of bet futur"), &couleur_bet_future, &default_couleur_bet_future },
    { N_("Background of bet solde"), &couleur_bet_solde, &default_couleur_bet_solde },
    { N_("Background of bet transfer"), &couleur_bet_transfert, &default_couleur_bet_transfert },
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
		gsb_rgba_set_alt_colors ();
		gsb_rgba_initialise_couleurs_par_defaut ();
		gsb_rgba_set_colors_to_default ();
		gsb_rgba_set_css_color_property (&couleur_selection, "couleur_selection");
		gsb_rgba_set_css_color_property (&couleur_grey_box, "couleur_grey_box");
		gsb_rgba_set_css_color_property (&couleur_insensitive_text, "couleur_insensitive_text");
		gsb_rgba_set_css_color_property (&text_color[0], "text_color_0");
	}

    /* save all colors */
    new_string = g_markup_printf_escaped ("\t<RGBA\n"
										  "\t\tCouleur_grey_box=\"%s\"\n"
										  "\t\tCouleur_insensitive_text=\"%s\"\n"
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
				                          "\t\tCouleur_bet_transfert=\"%s\"\n"
				                          "\t\tAlt_Couleur_grey_box=\"%s\"\n"
										  "\t\tAlt_Couleur_insensitive_text=\"%s\"\n"
				                          "\t\tAlt_Background_color_0=\"%s\"\n"
										  "\t\tAlt_Background_color_1=\"%s\"\n"
										  "\t\tAlt_Couleur_jour=\"%s\"\n"
										  "\t\tAlt_Background_scheduled=\"%s\"\n"
										  "\t\tAlt_Background_archive=\"%s\"\n"
										  "\t\tAlt_Couleur_selection=\"%s\"\n"
										  "\t\tAlt_Background_split=\"%s\"\n"
										  "\t\tAlt_Text_color_0=\"%s\"\n"
										  "\t\tAlt_Text_color_1=\"%s\"\n"
										  "\t\tAlt_Couleur_bet_division=\"%s\"\n"
										  "\t\tAlt_Couleur_bet_future=\"%s\"\n"
										  "\t\tAlt_Couleur_bet_solde=\"%s\"\n"
										  "\t\tAlt_Couleur_bet_transfert=\"%s\" />\n",
										  gdk_rgba_to_string (&couleur_grey_box),
										  gdk_rgba_to_string (&couleur_insensitive_text),
										  gdk_rgba_to_string (&couleur_fond[0]),
										  gdk_rgba_to_string (&couleur_fond[1]),
										  gdk_rgba_to_string (&couleur_jour),
										  gdk_rgba_to_string (&background_scheduled),
										  gdk_rgba_to_string (&background_archive),
										  gdk_rgba_to_string (&couleur_selection),
										  gdk_rgba_to_string (&background_split),
										  gdk_rgba_to_string (&text_color[0]),
										  gdk_rgba_to_string (&text_color[1]),
										  gdk_rgba_to_string (&couleur_bet_division),
										  gdk_rgba_to_string (&couleur_bet_future),
										  gdk_rgba_to_string (&couleur_bet_solde),
										  gdk_rgba_to_string (&couleur_bet_transfert),
										  gdk_rgba_to_string (&alt_couleur_grey_box),
										  gdk_rgba_to_string (&alt_couleur_insensitive_text),
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
gboolean gsb_rgba_set_css_color_property (GdkRGBA *color,
										  gchar *couleur)
{
	GtkCssProvider *css_provider = NULL;
    gchar *new_color_string;
	gchar *new_data = NULL;

	css_provider = grisbi_app_get_css_provider ();
	new_color_string = gdk_rgba_to_string (color);

	if (strcmp (couleur, "couleur_selection") == 0)
	{
		new_data = gsb_rgba_css_color_get_data_with_new_color ("@define-color couleur_selection ", new_color_string);
	}
	else if (strcmp (couleur, "couleur_grey_box") == 0)
	{
		new_data = gsb_rgba_css_color_get_data_with_new_color ("@define-color couleur_grey_box ", new_color_string);
	}

	else if (strcmp (couleur, "couleur_insensitive_text") == 0)
	{
		new_data = gsb_rgba_css_color_get_data_with_new_color ("@define-color couleur_insensitive_text ", new_color_string);
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
		conf.prefs_change_css_data = TRUE;
	}

	g_free (new_color_string);

	return conf.prefs_change_css_data;
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
