/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2011 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include "gsb_color.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/* Couleurs de base */
static GdkColor couleur_bleue;

static GdkColor couleur_grise;
static GdkColor default_couleur_grise;

static GdkColor couleur_jaune;

/* Couleurs des tree_view */
static GdkColor archive_background_color;
static GdkColor default_archive_background_color;

static GdkColor couleur_fond[2];
static GdkColor default_couleur_fond[2];

static GdkColor couleur_jour;
static GdkColor default_couleur_jour;

static GdkColor couleur_selection;
static GdkColor default_couleur_selection;

static GdkColor split_background;
static GdkColor default_split_background;

static GdkColor text_color[2];
static GdkColor default_text_color[2];

/* Couleur des entry en erreur */
static GdkColor entry_error_color;
static GdkColor default_entry_error_color;

/* colors of the amounts in the first page */
GdkColor couleur_nom_compte_normal;
GdkColor couleur_nom_compte_prelight;

GdkColor couleur_solde_alarme_orange_normal;
GdkColor couleur_solde_alarme_orange_prelight;

GdkColor couleur_solde_alarme_rouge_normal;
GdkColor couleur_solde_alarme_rouge_prelight;

GdkColor couleur_solde_alarme_verte_normal;
GdkColor couleur_solde_alarme_verte_prelight;

/* colors for the balance estimate module */
GdkColor couleur_bet_division;
GdkColor default_couleur_bet_division;

GdkColor couleur_bet_future;
GdkColor default_couleur_bet_future;

GdkColor couleur_bet_solde;
GdkColor default_couleur_bet_solde;

GdkColor couleur_bet_transfert;
GdkColor default_couleur_bet_transfert;


/**
 * initialize the default colors for grisbi
 *
 * \param
 *
 * \return
 * */
void gsb_color_initialise_couleurs_par_defaut ( void )
{
    devel_debug (NULL);

    /* définition des couleurs paramétrables */

    /* colors of the background */
    if ( !gdk_color_parse ( "#D6D8D6D8FFFF", &default_couleur_fond[0] ) )
        notice_debug ("default_couleur_fond[0] en erreur");

    if ( !gdk_color_parse ( "#FFFFFFFFFFFF", &default_couleur_fond[1] ) )
        notice_debug ("default_couleur_fond[1] en erreur");

    /* colors of the text */
    if ( !gdk_color_parse ( "#000", &default_text_color[0] ) )
        notice_debug ("default_default_text_color[0] en erreur");

    if ( !gdk_color_parse ( "#FFFF00000000", &default_text_color[1] ) )
        notice_debug ("default_text_color[1] en erreur");

    /* color of today */
    if ( !gdk_color_parse ( "#D8D8D8D8D8D8", &default_couleur_jour ) )
        notice_debug ("default_couleur_jour en erreur");

    /* selection color */
    if ( !gdk_color_parse ( "#F6189C409C40", &default_couleur_selection ) )
        notice_debug ("default_couleur_selection en erreur");

    /* color of the non selectable transactions on scheduler */
    if ( !gdk_color_parse ( "#C350C350C350", &default_couleur_grise ) )
        notice_debug ("default_couleur_grise en erreur");

    /* set the archive background color */
    if ( !gdk_color_parse ( "#3A9875302710", &default_archive_background_color ) )
        notice_debug ("default_archive_background_color en erreur");

    /* color for split children */
    if ( !gdk_color_parse ( "#EFEFC5C5C5C5", &default_split_background ) )
        notice_debug ("default_split_background en erreur");

    /* color for wrong entry */
    if ( !gdk_color_parse ( "#FFFF00000000", &default_entry_error_color ) )
        notice_debug ("default_entry_error_color en erreur");

    /* colors of the background bet divisions */
    if ( !gdk_color_parse ( "#FC26FFFF82BA", &default_couleur_bet_division ) )
        notice_debug ("default_couleur_bet_division en erreur");

    /* colors of the background bet future data */
    if ( !gdk_color_parse ( "#7C55C7478C44", &default_couleur_bet_future ) )
        notice_debug ("default_couleur_bet_future en erreur");

    /* colors of the background bet solde data */
    if ( !gdk_color_parse ( "#090EFFFFEBAB", &default_couleur_bet_solde ) )
        notice_debug ("default_couleur_bet_solde en erreur");

    /* colors of the background bet transfert data */
    if ( !gdk_color_parse ( "#EA8392F6EA34", &default_couleur_bet_transfert ) )
        notice_debug ("default_couleur_bet_transfert en erreur");

    /* Initialisation des couleurs non paramétrables */

    /* Initialisation des couleurs des différents labels */
    if ( !gdk_color_parse ( "#00007FFF0000", &couleur_solde_alarme_verte_normal ) )
        notice_debug ("couleur_solde_alarme_verte_normal en erreur");

    if ( !gdk_color_parse ( "#0000E6650000", &couleur_solde_alarme_verte_prelight ) )
        notice_debug ("couleur_solde_alarme_verte_prelight en erreur");

    if ( !gdk_color_parse ( "#E66599990000", &couleur_solde_alarme_orange_normal ) )
        notice_debug ("couleur_solde_alarme_orange_normal en erreur");

    if ( !gdk_color_parse ( "#FFFFCCCC0000", &couleur_solde_alarme_orange_prelight ) )
        notice_debug ("couleur_solde_alarme_orange_prelight en erreur");

    if ( !gdk_color_parse ( "#999900000000", &couleur_solde_alarme_rouge_normal ) )
        notice_debug ("couleur_solde_alarme_rouge_normal en erreur");

    if ( !gdk_color_parse ( "#FFFF00000000", &couleur_solde_alarme_rouge_prelight ) )
        notice_debug ("couleur_solde_alarme_rouge_prelight en erreur");

    if ( !gdk_color_parse ( "#000", &couleur_nom_compte_normal ) )
        notice_debug ("couleur_nom_compte_normal en erreur");

    if ( !gdk_color_parse ( "#9C289C289C28", &couleur_nom_compte_prelight ) )
        notice_debug ("couleur_nom_compte_prelight en erreur");

    /* blue color */
    if ( !gdk_color_parse ( "#1F41F4FFF", &couleur_bleue ) )
        notice_debug ("couleur_bleue en erreur");

    /* yellow color */
    if ( !gdk_color_parse ( "#C567FFFF0000", &couleur_jaune ) )
        notice_debug ("couleur_jaune en erreur");
}


/**
 * retourne la couleur demandée.
 *
 * \param gchar couleur
 *
 * \return a GdkColor
 * */
GdkColor *gsb_color_get_couleur ( const gchar *couleur )
{
    if ( strcmp ( couleur, "couleur_bleue" ) == 0 )
        return &couleur_bleue;
    else if ( strcmp ( couleur, "couleur_grise" ) == 0 )
        return &couleur_grise;
    else if ( strcmp ( couleur, "couleur_jaune" ) == 0 )
        return &couleur_jaune;
    else if ( strcmp ( couleur, "archive_background_color" ) == 0 )
        return &archive_background_color;
    else if ( strcmp ( couleur, "couleur_jour" ) == 0 )
        return &couleur_jour;
    else if ( strcmp ( couleur, "couleur_selection" ) == 0 )
        return &couleur_selection;
    else if ( strcmp ( couleur, "split_background" ) == 0 )
        return &split_background;
    else if ( strcmp ( couleur, "entry_error_color" ) == 0 )
        return &entry_error_color;
    else if ( strcmp ( couleur, "couleur_nom_compte_normal" ) == 0 )
        return &couleur_nom_compte_normal;
    else if ( strcmp ( couleur, "couleur_nom_compte_prelight" ) == 0 )
        return &couleur_nom_compte_prelight;
    else if ( strcmp ( couleur, "couleur_solde_alarme_orange_normal" ) == 0 )
        return &couleur_solde_alarme_orange_normal;
    else if ( strcmp ( couleur, "couleur_solde_alarme_orange_prelight" ) == 0 )
        return &couleur_solde_alarme_orange_prelight;
    else if ( strcmp ( couleur, "couleur_solde_alarme_rouge_normal" ) == 0 )
        return &couleur_solde_alarme_rouge_normal;
    else if ( strcmp ( couleur, "couleur_solde_alarme_rouge_prelight" ) == 0 )
        return &couleur_solde_alarme_rouge_prelight;
    else if ( strcmp ( couleur, "couleur_solde_alarme_verte_normal" ) == 0 )
        return &couleur_solde_alarme_verte_normal;
    else if ( strcmp ( couleur, "couleur_solde_alarme_verte_prelight" ) == 0 )
        return &couleur_solde_alarme_verte_prelight;
    else if ( strcmp ( couleur, "couleur_bet_division" ) == 0 )
        return &couleur_bet_division;
    else if ( strcmp ( couleur, "couleur_bet_future" ) == 0 )
        return &couleur_bet_future;
    else if ( strcmp ( couleur, "couleur_bet_solde" ) == 0 )
        return &couleur_bet_solde;
    else if ( strcmp ( couleur, "couleur_bet_transfert" ) == 0 )
        return &couleur_bet_transfert;

    return NULL;
}


/**
 * set the colors
 *
 * \param gchar couleur
 * \param gchar composante de la couleur (ref, green, blue)
 *
 * \return TRUE
 * */
gboolean gsb_color_set_couleur ( const gchar *couleur,
                        gchar *component,
                        gint value )
{
    GdkColor *color;

    color = gsb_color_get_couleur ( couleur );

    if ( color == NULL )
        return FALSE;

    if ( strcmp ( component, "red" ) == 0 )
        color -> red = value;
    else if ( strcmp ( component, "green" ) == 0 )
        color -> green = value;
    else if ( strcmp ( component, "blue" ) == 0 )
        color -> blue = value;

    return TRUE;
}


/**
 * retourne la couleur demandée.
 *
 * \param gchar couleur
 *
 * \return textual specification of color in the hexadecimal form #rrrrggggbbbb
 * */
gchar *gsb_color_get_couleur_to_string ( const gchar *couleur )
{
    GdkColor *color;
    gchar *string;

    color = gsb_color_get_couleur ( couleur );
    string = gdk_color_to_string ( color );

    return string;
}


/**
 * retourne la couleur demandée.
 *
 *\param gchar couleur
 *\param gint indice du tableau
 *
 * */
GdkColor *gsb_color_get_couleur_with_indice ( const gchar *couleur,
                        gint indice )
{

    if ( strcmp ( couleur, "couleur_fond" ) == 0 )
        return &couleur_fond[indice];
    else if ( strcmp ( couleur, "text_color" ) == 0 )
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
gboolean gsb_color_set_couleur_with_indice ( const gchar *couleur,
                        gint indice,
                        gchar *component,
                        gint value )
{
    GdkColor *color;

    color = gsb_color_get_couleur_with_indice ( couleur, indice );

    if ( color == NULL )
        return FALSE;

    if ( strcmp ( component, "red" ) == 0 )
        color -> red = value;
    else if ( strcmp ( component, "green" ) == 0 )
        color -> green = value;
    else if ( strcmp ( component, "blue" ) == 0 )
        color -> blue = value;

    return TRUE;
}


/**
 * initialize the colors used in grisbi
 *
 * \param
 *
 * \return
 * */
void gsb_color_set_colors_to_default ( void )
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
    couleur_grise = default_couleur_grise;

    /* set the archive background color */
    archive_background_color = default_archive_background_color;

    /* color for split children */
    split_background = default_split_background;

    /* color for wrong entry */
    entry_error_color = default_entry_error_color;

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
GtkWidget *gsb_color_create_color_combobox ( void )
{
    GtkWidget *combobox;
    GtkListStore *store;
    gint i;
    GtkCellRenderer *renderer;

    struct config_color {
    gchar *name;
    GdkColor *color;
    GdkColor *default_color;

    } config_colors[] = {
    { N_("Transaction list background 1"), &couleur_fond[0], &default_couleur_fond[0]},
    { N_("Transaction list background 2"), &couleur_fond[1], &default_couleur_fond[1]},
    { N_("Color for the operation that gives the balance today"), &couleur_jour, &default_couleur_jour},
    { N_("Color of transaction's text"), &text_color[0], &default_text_color[0]},
    { N_("Text of unfinished split transaction"), &text_color[1], &default_text_color[1]},
    { N_("Children of split transaction"), &split_background, &default_split_background},
    { N_("Selection color"), &couleur_selection, &default_couleur_selection},
    { N_("Background of non selectable scheduled transactions"), &couleur_grise, &default_couleur_grise},
    { N_("Archive color"), &archive_background_color, &default_archive_background_color},
    { N_("Background of invalid date entry"), &entry_error_color, &default_entry_error_color },
    { N_("Background of bet division"), &couleur_bet_division, &default_couleur_bet_division },
    { N_("Background of bet futur"), &couleur_bet_future, &default_couleur_bet_future },
    { N_("Background of bet solde"), &couleur_bet_solde, &default_couleur_bet_solde },
    { N_("Background of bet transfer"), &couleur_bet_transfert, &default_couleur_bet_transfert },
    { NULL, 0, 0},
    };

    /* the store contains the name of the color we can modify and
     * a pointer to the corresponding variable */
    store = gtk_list_store_new ( 3,
                        G_TYPE_STRING,
                        G_TYPE_POINTER,
                        G_TYPE_POINTER );
    /* fill the store */
    for ( i = 0 ; config_colors[i].name != NULL ; i++ )
    {
    GtkTreeIter iter;

    gtk_list_store_append ( GTK_LIST_STORE (store),
                        &iter );
    gtk_list_store_set ( GTK_LIST_STORE (store),
                        &iter,
                        0, gettext ( config_colors[i].name ),
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
gchar *gsb_color_get_strings_to_save ( void )
{
    gchar *new_string = NULL;

    /* save the general informations */
    new_string = g_markup_printf_escaped ( "\t<Color\n"
                        "\t\tBackground_color_0_red=\"%d\"\n"
                        "\t\tBackground_color_0_green=\"%d\"\n"
                        "\t\tBackground_color_0_blue=\"%d\"\n"
                        "\t\tBackground_color_1_red=\"%d\"\n"
                        "\t\tBackground_color_1_green=\"%d\"\n"
                        "\t\tBackground_color_1_blue=\"%d\"\n"
                        "\t\tCouleur_jour_red=\"%d\"\n"
                        "\t\tCouleur_jour_green=\"%d\"\n"
                        "\t\tCouleur_jour_blue=\"%d\"\n"
                        "\t\tBackground_scheduled_red=\"%d\"\n"
                        "\t\tBackground_scheduled_green=\"%d\"\n"
                        "\t\tBackground_scheduled_blue=\"%d\"\n"
                        "\t\tBackground_archive_red=\"%d\"\n"
                        "\t\tBackground_archive_green=\"%d\"\n"
                        "\t\tBackground_archive_blue=\"%d\"\n"
                        "\t\tSelection_red=\"%d\"\n"
                        "\t\tSelection_green=\"%d\"\n"
                        "\t\tSelection_blue=\"%d\"\n"
                        "\t\tBackground_split_red=\"%d\"\n"
                        "\t\tBackground_split_green=\"%d\"\n"
                        "\t\tBackground_split_blue=\"%d\"\n"
                        "\t\tText_color_0_red=\"%d\"\n"
                        "\t\tText_color_0_green=\"%d\"\n"
                        "\t\tText_color_0_blue=\"%d\"\n"
                        "\t\tText_color_1_red=\"%d\"\n"
                        "\t\tText_color_1_green=\"%d\"\n"
                        "\t\tText_color_1_blue=\"%d\"\n"
                        "\t\tEntry_error_color_red=\"%d\"\n"
                        "\t\tEntry_error_color_green=\"%d\"\n"
                        "\t\tEntry_error_color_blue=\"%d\"\n"
                        "\t\tCouleur_bet_division_red=\"%d\"\n"
                        "\t\tCouleur_bet_division_green=\"%d\"\n"
                        "\t\tCouleur_bet_division_blue=\"%d\"\n"
                        "\t\tCouleur_bet_future_red=\"%d\"\n"
                        "\t\tCouleur_bet_future_green=\"%d\"\n"
                        "\t\tCouleur_bet_future_blue=\"%d\"\n"
                        "\t\tCouleur_bet_solde_red=\"%d\"\n"
                        "\t\tCouleur_bet_solde_green=\"%d\"\n"
                        "\t\tCouleur_bet_solde_blue=\"%d\"\n"
                        "\t\tCouleur_bet_transfert_red=\"%d\"\n"
                        "\t\tCouleur_bet_transfert_green=\"%d\"\n"
                        "\t\tCouleur_bet_transfert_blue=\"%d\" />\n",

    couleur_fond[0].red,
    couleur_fond[0].green,
    couleur_fond[0].blue,
    couleur_fond[1].red,
    couleur_fond[1].green,
    couleur_fond[1].blue,
    couleur_jour.red,
    couleur_jour.green,
    couleur_jour.blue,
    couleur_grise.red,
    couleur_grise.green,
    couleur_grise.blue,
    archive_background_color.red,
    archive_background_color.green,
    archive_background_color.blue,
    couleur_selection.red,
    couleur_selection.green,
    couleur_selection.blue,
    split_background.red,
    split_background.green,
    split_background.blue,
    text_color[0].red,
    text_color[0].green,
    text_color[0].blue,
    text_color[1].red,
    text_color[1].green,
    text_color[1].blue,
    entry_error_color.red,
    entry_error_color.green,
    entry_error_color.blue,
    couleur_bet_division.red,
    couleur_bet_division.green,
    couleur_bet_division.blue,
    couleur_bet_future.red,
    couleur_bet_future.green,
    couleur_bet_future.blue,
    couleur_bet_solde.red,
    couleur_bet_solde.green,
    couleur_bet_solde.blue,
    couleur_bet_transfert.red,
    couleur_bet_transfert.green,
    couleur_bet_transfert.blue );

    return new_string;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
