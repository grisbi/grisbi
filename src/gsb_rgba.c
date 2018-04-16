/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2016 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
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
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/* Couleurs de base */
static GdkRGBA couleur_bleue;
static GdkRGBA couleur_jaune;

/* Couleurs des tree_view */
static GdkRGBA background_archive;
static GdkRGBA default_background_archive;

static GdkRGBA couleur_fond[2];
static GdkRGBA default_couleur_fond[2];

static GdkRGBA couleur_jour;
static GdkRGBA default_couleur_jour;

static GdkRGBA couleur_selection;
static GdkRGBA default_couleur_selection;

static GdkRGBA background_split;
static GdkRGBA default_background_split;

static GdkRGBA text_color[2];
static GdkRGBA default_text_color[2];

static GdkRGBA background_scheduled;
static GdkRGBA default_background_scheduled;

/* colors for the balance estimate module */
GdkRGBA couleur_bet_division;
GdkRGBA default_couleur_bet_division;

GdkRGBA couleur_bet_future;
GdkRGBA default_couleur_bet_future;

GdkRGBA couleur_bet_solde;
GdkRGBA default_couleur_bet_solde;

GdkRGBA couleur_bet_transfert;
GdkRGBA default_couleur_bet_transfert;

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
    if (!gdk_rgba_parse (&default_couleur_jour, "rgb(215,215,215)"))
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

    /* couleurs fixes */
    /* blue color */
    if (!gdk_rgba_parse (&couleur_bleue, "blue"))
        notice_debug ("couleur_bleue en erreur");

    /* yellow color */
    if (!gdk_rgba_parse (&couleur_jaune, "yellow"))
        notice_debug ("couleur_jaune en erreur");
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
    if (strcmp (couleur, "couleur_bleue") == 0)
        return &couleur_bleue;
    else if (strcmp (couleur, "background_scheduled") == 0)
        return &background_scheduled;
    else if (strcmp (couleur, "couleur_jaune") == 0)
        return &couleur_jaune;
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

    color = gsb_rgba_get_couleur (couleur);

    if (color == NULL)
        return FALSE;

    (void)gdk_rgba_parse (color, value);

    return TRUE;
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

    return &couleur_fond[0];
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

/**
 * initialize the colors used in grisbi
 *
 * \param
 *
 * \return
 * */
void gsb_rgba_set_colors_to_default (void)
{
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
    gchar *name;
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

    /* save the general informations */
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
    gdk_rgba_to_string (&couleur_bet_transfert));

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
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
