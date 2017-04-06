/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include "grisbi_prefs.h"
#include "affichage.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_dirs.h"
#include "parametres.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_gtkbuilder.h"
#include "utils_prefs.h"
#include "prefs/prefs_page_archives.h"
#include "prefs/prefs_page_files.h"
#include "prefs/prefs_page_import_files.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

struct _GrisbiPrefs
{
    GtkDialog dialog;
};

struct _GrisbiPrefsClass
{
    GtkDialogClass parent_class;
};

/* Private structure type */
typedef struct _GrisbiPrefsPrivate GrisbiPrefsPrivate;

struct _GrisbiPrefsPrivate
{
  GtkWidget           *prefs_paned;

    /* panel de gauche */
    GtkWidget *			left_sw;
    GtkWidget *      	left_treeview;
    GtkTreeStore *    	prefs_tree_model;
	GtkWidget *      	togglebutton_expand_prefs;

    /* notebook de droite */
    GtkWidget *         notebook_prefs;
	GtkWidget *     	vbox_import_page;
	GtkWidget *      	label_import_page_1;
	GtkWidget *      	label_import_page_2;
 };


G_DEFINE_TYPE_WITH_PRIVATE (GrisbiPrefs, grisbi_prefs, GTK_TYPE_DIALOG)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * callback pour la fermeture des preferences
 *
 * \param prefs_dialog
 * \param result_id
 *
 * \return
 **/
static void grisbi_prefs_dialog_response  (GtkDialog *prefs,
										   gint result_id)
{
    if (!prefs)
  {
        return;
  }

  /* on récupère la dimension de la fenêtre */
  gtk_window_get_size (GTK_WINDOW (prefs), &conf.prefs_width, &conf.prefs_height);

  gtk_widget_destroy (GTK_WIDGET (prefs));
}

/**
 * récupère la largeur des préférences
 *
 * \param GtkWidget     prefs
 * \param GtkAllocation   allocation
 * \param gpointer       null
 *
 * \return           FALSE
 * */
static gboolean grisbi_prefs_size_allocate (GtkWidget *prefs,
											GtkAllocation *allocation,
											gpointer null)
{
    conf.prefs_height = allocation->height;
	conf.prefs_width = allocation->width;

	return FALSE;
}

/**
 * save prefs hpahed width
 *
 * \param GtkWidget			hpaned
 * \param GtkAllocation 	allocation
 * \param gpointer			NULL
 *
 * \return FALSE
 */
static gboolean grisbi_prefs_paned_size_allocate (GtkWidget *prefs_hpaned,
												  GtkAllocation *allocation,
												  gpointer null)
{
    conf.prefs_panel_width = gtk_paned_get_position (GTK_PANED (prefs_hpaned));

	return FALSE;
}


/* RIGHT PANED */
/**
 * Création de la page pour l'importation. Cette page comporte deux onglets
 * - 1 pour les fichiers
 * - 1 pour les associations
 *
 * \param prefs
 *
 * \return
 */
static void grisbi_prefs_setup_import_page (GrisbiPrefs *prefs)
{
	GtkWidget *head_page;
	GtkWidget *notebook_import_pages;
	GtkWidget *vbox_import_files;
	//~ GtkWidget *vbox_import_asso;
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Import"), "import.png");

	gtk_box_pack_start (GTK_BOX (priv->vbox_import_page), head_page, FALSE, FALSE, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_prefs), priv->vbox_import_page, NULL);

	/* set notebook for import */
	notebook_import_pages = gtk_notebook_new ();
	gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook_import_pages), FALSE);
	gtk_box_pack_start (GTK_BOX (priv->vbox_import_page), notebook_import_pages, FALSE, FALSE, 0);

	/* set import settings */
	vbox_import_files = GTK_WIDGET (prefs_page_import_files_new (prefs));
	gtk_widget_set_margin_top (vbox_import_files, MARGIN_TOP);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook_import_pages), vbox_import_files, NULL);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_import_pages), vbox_import_files, priv->label_import_page_1);

	/* set manage associations */
	//~ vbox_import_asso = GTK_WIDGET (prefs_page_import_files_new (prefs));
	//~ gtk_widget_set_margin_top (vbox_import_asso, MARGIN_TOP);
	//~ gtk_notebook_append_page (GTK_NOTEBOOK (notebook_import_pages), vbox_import_asso, NULL);
	//~ gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_import_pages), vbox_import_asso, priv->label_import_page_2);

	gtk_widget_show (notebook_import_pages);
}

/* LEFT PANED */
 /**
 * remplit le model pour la configuration des états
 *
 * \param GtkTreeStore		model
 * \param GrisbiPrefs		prefs
 *
 * \return
 * */
static void grisbi_prefs_left_panel_populate_tree_model (GtkTreeStore *tree_model,
														 GrisbiPrefs *prefs)
{
    GtkWidget *widget = NULL;
    gint page = 0;
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);

	/* append group page "Main" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Main"), -1);

    /* append page Fichiers */
	widget = GTK_WIDGET (prefs_page_files_new (prefs));
    utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Files"), page);
    page++;

     /* append page Archives */
	widget = GTK_WIDGET (prefs_page_archives_new (prefs));
    utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Archives"), page);
    page++;

     /* append page Import */
    grisbi_prefs_setup_import_page (prefs);
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Import"), page);
    page++;

	widget = GTK_WIDGET (onglet_accueil ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Main page"), page);
	page++;

   /* append group page "Display" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Display"), -1);

     /* append page Fonts & logo */
	widget = GTK_WIDGET (onglet_display_fonts ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Fonts & Logo"), page);
	page++;

	/* append page Messages & warnings */
	widget = GTK_WIDGET (onglet_messages_and_warnings ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Messages & warnings"), page);
	page++;

    /* append group page "Transactions" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Transactions"), -1);

    /* append group page "Transaction form" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Transaction form"), -1);

    /* append group page "Resources" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Resources"), -1);

    /* append group page "Balance estimate" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Balance estimate"), -1);

    /* append group page "Graphiques" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Graphs"), -1);
}

/**
 * création du tree_view qui liste les onglets de la fenêtre de dialogue
 *
 *
 *\return tree_view or NULL;
 * */
static void grisbi_prefs_left_tree_view_setup (GrisbiPrefs *prefs)
{
    GtkWidget *tree_view = NULL;
    GtkTreeStore *model = NULL;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);

    /* Création du model */
    model = gtk_tree_store_new (LEFT_PANEL_TREE_NUM_COLUMNS,
								G_TYPE_STRING,  				/* LEFT_PANEL_TREE_TEXT_COLUMN */
								G_TYPE_INT,     				/* LEFT_PANEL_TREE_PAGE_COLUMN */
								G_TYPE_INT,     				/* LEFT_PANEL_TREE_BOLD_COLUMN */
								G_TYPE_INT);    				/* LEFT_PANEL_TREE_ITALIC_COLUMN */

    /* Create treeView */
    tree_view = priv->left_treeview;
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
    g_object_unref (G_OBJECT (model));

    /* set the color of selected row */
    utils_set_tree_view_selection_and_text_color (tree_view);

    /* make column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Categories",
													   cell,
													   "text", LEFT_PANEL_TREE_TEXT_COLUMN,
													   "weight", LEFT_PANEL_TREE_BOLD_COLUMN,
													   "style", LEFT_PANEL_TREE_ITALIC_COLUMN,
													   NULL);

    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));

    /* Handle select */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    g_signal_connect (selection,
					  "changed",
					  G_CALLBACK (utils_prefs_left_panel_tree_view_selection_changed),
					  priv->notebook_prefs);

    /* Choose which entries will be selectable */
    gtk_tree_selection_set_select_function (selection,
											utils_prefs_left_panel_tree_view_selectable_func,
											NULL,
											NULL);

	/* set headers hide */
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);

	/* expand all rows after the treeview widget has been realized */
    g_signal_connect (tree_view,
					  "realize",
					  G_CALLBACK (utils_tree_view_set_expand_all_and_select_path_realize),
					  "0:0");

    /* remplissage du paned gauche */
    grisbi_prefs_left_panel_populate_tree_model (model, prefs);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void grisbi_prefs_init (GrisbiPrefs *prefs)
{
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);
	gtk_widget_init_template (GTK_WIDGET (prefs));

    gtk_dialog_add_buttons (GTK_DIALOG (prefs),
                        GTK_STOCK_CLOSE,
                        GTK_RESPONSE_CLOSE,
                        NULL);

    gtk_window_set_destroy_with_parent (GTK_WINDOW (prefs), TRUE);

    /* set the default size */
    if (conf.prefs_width && conf.prefs_width > 830)
	{
        gtk_window_set_default_size (GTK_WINDOW (prefs), conf.prefs_width, conf.prefs_height);
	}
    else
	{
        gtk_window_set_default_size (GTK_WINDOW (prefs), 830, conf.prefs_height);
	}

	if (conf.prefs_panel_width > 200)
	{
        gtk_paned_set_position (GTK_PANED (priv->prefs_paned), conf.prefs_panel_width);
	}
    else
	{
		gtk_paned_set_position (GTK_PANED (priv->prefs_paned), 200);
	}

	/* initialise left_tree_view */
	grisbi_prefs_left_tree_view_setup (prefs);

	gtk_widget_show_all (GTK_WIDGET (prefs));
}
/**
 * finalise GrisbiPrefs
 *
 * \param object
 *
 * \return
 */
static void grisbi_prefs_finalize (GObject *object)
{
/*     GrisbiPrefs *prefs = GRISBI_PREFS (object);  */

    devel_debug (NULL);

    /* libération de l'objet prefs */
    G_OBJECT_CLASS (grisbi_prefs_parent_class)->finalize (object);
}

/**
 * Initialise GrisbiPrefsClass
 *
 * \param
 *
 * \return
 */
static void grisbi_prefs_class_init (GrisbiPrefsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grisbi_prefs_finalize;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/grisbi_prefs.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, prefs_paned);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, left_treeview);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, togglebutton_expand_prefs);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, notebook_prefs);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, vbox_import_page);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, label_import_page_1);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, label_import_page_2);


	/* signaux */
    gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), grisbi_prefs_dialog_response);
    gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), grisbi_prefs_size_allocate);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), grisbi_prefs_paned_size_allocate);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
GrisbiPrefs *grisbi_prefs_new (GrisbiWin *win)
{
  return g_object_new (GRISBI_PREFS_TYPE, "transient-for", win, NULL);
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

