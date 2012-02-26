/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2012 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_app.h"
#include "grisbi_prefs.h"
#include "utils.h"
#include "utils_gtkbuilder.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/


#define GRISBI_PREFS_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GRISBI_TYPE_PREFS, GrisbiPrefsPrivate))

static GtkBuilder *grisbi_prefs_builder = NULL;
static GtkWidget *grisbi_prefs_dialog = NULL;


struct _GrisbiPrefsPrivate
{
    GtkWidget           *hpaned;

    /* tree_store du panel de gauche */
    GtkWidget           *treeview_left_panel;
    GtkTreeStore        *prefs_tree_model;
};


G_DEFINE_TYPE(GrisbiPrefs, grisbi_prefs, GTK_TYPE_DIALOG)

/**
 * finalise GrisbiPrefs
 *
 * \param object
 *
 * \return
 */
static void grisbi_prefs_finalize ( GObject *object )
{
/*     GrisbiPrefs *prefs = GRISBI_PREFS ( object );  */

    devel_debug (NULL);

    /* libération de l'objet prefs */
    G_OBJECT_CLASS ( grisbi_prefs_parent_class )->finalize ( object );
}


/**
 * Initialise GrisbiPrefsClass
 *
 * \param
 *
 * \return
 */
static void grisbi_prefs_class_init ( GrisbiPrefsClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS ( klass );

    object_class->finalize = grisbi_prefs_finalize;

    g_type_class_add_private ( object_class, sizeof( GrisbiPrefsPrivate ) );
}


/* GTK_BUILDER */
/**
 * callback pour la fermeture des preferences
 *
 * \param prefs_dialog
 * \param result_id
 *
 * \return
 **/
static void grisbi_prefs_dialog_response  (GtkDialog *prefs,
                        gint result_id )
{
    if ( !prefs )
        return;

    switch (result_id)
    {
        case GTK_RESPONSE_HELP:

            g_signal_stop_emission_by_name ( prefs, "response" );

            break;

        default:
            gtk_widget_destroy ( GTK_WIDGET ( prefs ) );
    }
}


/* GTK_BUILDER */
/**
 * Crée un builder et récupère les widgets du fichier grisbi.ui
 *
 * \param
 *
 * \rerurn
 * */
static gboolean grisbi_prefs_initialise_builder ( void )
{
    /* Creation d'un nouveau GtkBuilder */
    grisbi_prefs_builder = gtk_builder_new ( );
    if ( grisbi_prefs_builder == NULL )
        return FALSE;

    /* récupère les widgets */
    if ( utils_gtkbuilder_merge_ui_data_in_builder ( grisbi_prefs_builder, "grisbi_prefs.ui" ) )
        return TRUE;
    else
        return FALSE;
}


/*LEFT_PANEL*/
/**
 * remplit le model pour la configuration des états
 *
 * \param
 * \param
 *
 * \return
 * */
static void grisbi_prefs_left_panel_populate_tree_model ( GtkTreeStore *tree_model,
                        GtkWidget *notebook )
{
    GtkWidget *widget;
    GtkTreeIter iter;
    gint page = 0;

    /* append group page "Main" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Main"), -1 );
page++;

    /* append group page "Display" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Display"), -1 );
page++;

    /* append group page "Transactions" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Transactions"), -1 );
page++;

    /* append group page "Transaction form" */

    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Transaction form"), -1 );
page++;

    /* append group page "Resources" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Resources"), -1 );
page++;

    /* append group page "Balance estimate" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Balance estimate"), -1 );
page++;




    /* append page Generalities */
/*     widget = etats_config_ui_onglet_affichage_generalites_create_page ( page );
 *     utils_ui_left_panel_add_line ( tree_model, &iter, notebook, widget, _("Generalities"), page );
 *     page++;
 */


    /* fin de fonction */
}


/**
 * création du tree_view qui liste les onglets de la fenêtre de dialogue
 *
 *
 *\return tree_view or NULL;
 * */
static GtkWidget *grisbi_prefs_left_panel_create_tree_view ( void )
{
    GtkWidget *tree_view = NULL;
    GtkWidget *notebook;
    GtkTreeStore *model = NULL;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;

    devel_debug (NULL);

    /* Création du model */
    model = gtk_tree_store_new ( LEFT_PANEL_TREE_NUM_COLUMNS,
                        G_TYPE_STRING,  /* LEFT_PANEL_TREE_TEXT_COLUMN */
                        G_TYPE_INT,     /* LEFT_PANEL_TREE_PAGE_COLUMN */
                        G_TYPE_INT,     /* LEFT_PANEL_TREE_BOLD_COLUMN */
                        G_TYPE_INT );   /* LEFT_PANEL_TREE_ITALIC_COLUMN */

    /* Create container + TreeView */
    tree_view = GTK_WIDGET ( gtk_builder_get_object ( grisbi_prefs_builder, "treeview_left_panel" ) );
    gtk_tree_view_set_model ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_MODEL ( model ) );
    g_object_unref ( G_OBJECT ( model ) );

    /* set the color of selected row */
    utils_set_tree_view_selection_and_text_color ( tree_view );

    /* make column */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( "Categories",
                        cell,
                        "text", LEFT_PANEL_TREE_TEXT_COLUMN,
                        "weight", LEFT_PANEL_TREE_BOLD_COLUMN,
                        "style", LEFT_PANEL_TREE_ITALIC_COLUMN,
                        NULL );
    gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN ( column ), GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_VIEW_COLUMN ( column ) );

    /* initialisation du notebook pour les pages de la configuration */
    notebook = GTK_WIDGET ( gtk_builder_get_object ( grisbi_prefs_builder, "notebook_prefs" ) );
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( notebook ), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK ( notebook ), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER ( notebook ), 0 );

    /* Handle select */
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    g_signal_connect ( selection,
                        "changed",
                        G_CALLBACK ( utils_ui_left_panel_tree_view_selection_changed ),
                        NULL );

    /* Choose which entries will be selectable */
    gtk_tree_selection_set_select_function ( selection,
                        utils_ui_left_panel_tree_view_selectable_func, NULL, NULL );

    /* expand all rows after the treeview widget has been realized */
    g_signal_connect ( tree_view,
                        "realize",
                        G_CALLBACK ( utils_tree_view_set_expand_all_and_select_path_realize ),
                        "0:0" );

    /* remplissage du paned gauche */
    grisbi_prefs_left_panel_populate_tree_model ( model, notebook );

    /* show all widgets */
    gtk_widget_show_all ( tree_view );

    return tree_view;
}


/*CREATE OBJECT */
/**
 * Initialise GrisbiPrefs
 *
 * \param prefs
 *
 * \return
 */
static void grisbi_prefs_init ( GrisbiPrefs *prefs )
{
    GrisbiAppConf *conf;

    devel_debug (NULL);
    conf = grisbi_app_get_conf ( );

    prefs->priv = GRISBI_PREFS_GET_PRIVATE ( prefs );

    if ( !grisbi_prefs_initialise_builder ( ) )
        exit ( 1 );

    gtk_dialog_add_buttons ( GTK_DIALOG ( prefs ),
                        GTK_STOCK_CLOSE,
                        GTK_RESPONSE_CLOSE,
                        GTK_STOCK_HELP,
                        GTK_RESPONSE_HELP,
                        NULL );

    gtk_window_set_title ( GTK_WINDOW ( prefs ), _("Grisbi preferences") );
    gtk_dialog_set_has_separator ( GTK_DIALOG ( prefs ), FALSE );
    gtk_window_set_destroy_with_parent ( GTK_WINDOW ( prefs ), TRUE );

    gtk_container_set_border_width ( GTK_CONTAINER ( prefs ), 5 );
    gtk_box_set_spacing (GTK_BOX ( gtk_dialog_get_content_area ( GTK_DIALOG ( prefs ) ) ), 2 );
    gtk_container_set_border_width ( GTK_CONTAINER ( gtk_dialog_get_action_area ( GTK_DIALOG ( prefs ) ) ), 5 );
    gtk_box_set_spacing (GTK_BOX ( gtk_dialog_get_action_area (GTK_DIALOG ( prefs ) ) ), 6 );

    /* set the default size */
    if ( conf->prefs_width )
        gtk_window_set_default_size ( GTK_WINDOW ( prefs ),
                        conf->prefs_width, -1 );

    /* create the tree_view */
    prefs->priv->treeview_left_panel = grisbi_prefs_left_panel_create_tree_view ( );

    /* connect the signals */
    g_signal_connect ( prefs,
                        "response",
                        G_CALLBACK ( grisbi_prefs_dialog_response ),
                        NULL);

    prefs->priv->hpaned = GTK_WIDGET ( gtk_builder_get_object ( grisbi_prefs_builder, "dialog_hpaned" ) );
    gtk_box_pack_start ( GTK_BOX ( gtk_dialog_get_content_area ( GTK_DIALOG ( prefs ) ) ),
                        prefs->priv->hpaned, TRUE, TRUE, 0 );
    g_object_unref ( prefs->priv->hpaned );

    /* return */
}


/**
 * show the preferences dialog
 *
 * \param parent
 *
 * \return
 **/
void grisb_show_prefs_dialog ( GrisbiWindow *parent )
{
    if ( !GRISBI_IS_WINDOW ( parent ) )
        return;

    if ( grisbi_prefs_dialog == NULL )
    {
        grisbi_prefs_dialog = GTK_WIDGET ( g_object_new ( GRISBI_TYPE_PREFS, NULL ) );
        g_signal_connect ( grisbi_prefs_dialog,
                        "destroy",
                        G_CALLBACK ( gtk_widget_destroyed ),
                        &grisbi_prefs_dialog );
    }

    if ( GTK_WINDOW ( parent) != gtk_window_get_transient_for ( GTK_WINDOW ( grisbi_prefs_dialog ) ) )
    {
        gtk_window_set_transient_for ( GTK_WINDOW ( grisbi_prefs_dialog ), GTK_WINDOW ( parent ) );
    }

    gtk_window_present ( GTK_WINDOW ( grisbi_prefs_dialog ) );

}


/**
 *
 *
 * \param parent
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

