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
#include "config.h"
#endif

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
#include "affichage.h"
#include "affichage_liste.h"
#include "bet_config.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "grisbi_settings.h"
#include "gsb_bank.h"
#include "gsb_currency_config.h"
#include "gsb_currency_link_config.h"
#include "gsb_dirs.h"
#include "gsb_form_config.h"
#include "gsb_fyear_config.h"
#include "gsb_payment_method_config.h"
#include "gsb_reconcile_config.h"
#include "gsb_reconcile_sort_config.h"
#include "parametres.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_gtkbuilder.h"
#include "utils_prefs.h"
#include "prefs/prefs_page_archives.h"
#include "prefs/prefs_page_display_adr.h"
#include "prefs/prefs_page_display_fonts.h"
#include "prefs/prefs_page_divers.h"
#include "prefs/prefs_page_files.h"
#include "prefs/prefs_page_import_asso.h"
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

    /* panel de gauche */
    GtkWidget *			left_sw;
    GtkWidget *      	left_treeview;

    /* notebook de droite */
    GtkWidget *         notebook_prefs;
	GtkWidget *     	vbox_import_page;

	/* pages num */
	gint 				form_num_page;
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
void grisbi_prefs_dialog_response  (GtkDialog *prefs,
                                    gint result_id)
{
	GSettings *settings;

    devel_debug (NULL);
	if (!prefs)
	{
        return;
	}

	/* on récupère la dimension de la fenêtre */
	gtk_window_get_size (GTK_WINDOW (prefs), &conf.prefs_width, &conf.prefs_height);

	settings = grisbi_settings_get_settings (SETTINGS_PREFS);

    g_settings_set_int (G_SETTINGS (settings),
                        "prefs-height",
                        conf.prefs_height);

    g_settings_set_int (G_SETTINGS (settings),
                        "prefs-panel-width",
                        conf.prefs_panel_width);

    g_settings_set_int (G_SETTINGS (settings),
                        "prefs-width",
                        conf.prefs_width);

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

	GSettings *settings;

	settings = grisbi_settings_get_settings (SETTINGS_PREFS);
    conf.prefs_height = allocation->height;
	conf.prefs_width = allocation->width;

    /* save settings_prefs */
	g_settings_set_int (G_SETTINGS (settings), "prefs-height", conf.prefs_height);
	g_settings_set_int (G_SETTINGS (settings), "prefs-width", conf.prefs_width);
	g_settings_set_int (G_SETTINGS (settings), "prefs-panel-width", conf.prefs_panel_width);

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
	GtkWidget *vbox_import_asso;
	GtkWidget *label;
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Import"), "gsb-import-32.png");

	gtk_box_pack_start (GTK_BOX (priv->vbox_import_page), head_page, FALSE, FALSE, 0);

	/* set notebook for import */
	notebook_import_pages = gtk_notebook_new ();
	utils_widget_set_padding (notebook_import_pages, MARGIN_BOX, 0);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook_import_pages), FALSE);
	gtk_box_pack_start (GTK_BOX (priv->vbox_import_page), notebook_import_pages, FALSE, FALSE, 0);

	/* set import settings */
	vbox_import_files = GTK_WIDGET (prefs_page_import_files_new (prefs));
	gtk_widget_set_margin_top (vbox_import_files, MARGIN_TOP);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook_import_pages), vbox_import_files, NULL);
	label = gtk_label_new (_("Files import"));
	utils_widget_set_padding (label, MARGIN_BOX, 0);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_import_pages), vbox_import_files, label);

	/* set manage associations */
	vbox_import_asso = GTK_WIDGET (prefs_page_import_asso_new (prefs));
	gtk_widget_set_margin_top (vbox_import_asso, MARGIN_TOP);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook_import_pages), vbox_import_asso, NULL);
	label = gtk_label_new (_("Associations for import"));
	utils_widget_set_padding (label, MARGIN_BOX, 0);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_import_pages), vbox_import_asso, label);

	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_prefs), priv->vbox_import_page, NULL);

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
static void grisbi_prefs_left_panel_populate_tree_model (GrisbiPrefs *prefs)
{
    GtkWidget *widget = NULL;
	GtkTreeStore *tree_model;
    gint page = 0;
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);

	tree_model = GTK_TREE_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (priv->left_treeview)));
	/* append group page "Generalities" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Generalities"), -1);

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

	/* append page Divers */
    widget = GTK_WIDGET (prefs_page_divers_new (prefs));
    utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Various settings"), page);
    page++;

	/* append page Accueil */
	widget = GTK_WIDGET (onglet_accueil ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Main page"), page);
	page++;

	/* append group page "Display" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Display"), -1);

	/* append page Fonts & logo */
	widget = GTK_WIDGET (prefs_page_display_fonts_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Fonts & Logo"), page);
	page++;

	/* append page Messages & warnings */
	widget = GTK_WIDGET (onglet_messages_and_warnings ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Messages & warnings"), page);
	page++;

	/* append page Addresses & titles */
	widget = GTK_WIDGET (prefs_page_display_adr_new (prefs));
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Addresses & titles"), page);
	page++;

	/* append page Payees, categories and budgetaries */
	widget = GTK_WIDGET (onglet_metatree ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Payees, categories and budgetaries"), page);
	page++;

	/* append page Elements of interface */
	widget = GTK_WIDGET (tab_display_toolbar ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Elements of interface"), page);
	page++;

	/* append group page "Transactions" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Transactions"), -1);

	/* append page List behavior */
	widget = GTK_WIDGET (onglet_affichage_operations ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("List behavior"), page);
	page++;

	/* append page Transactions list cells */
	widget = GTK_WIDGET (onglet_affichage_liste ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Transactions list cells"), page);
	page++;

	/* append page Messages before deleting */
	widget = GTK_WIDGET (onglet_delete_messages ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Messages before deleting"), page);
	page++;

	/* append page Reconciliation */
	widget = GTK_WIDGET (gsb_reconcile_config_create ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Reconciliation"), page);
	page++;

	/* append page Sort for reconciliation */
	widget = GTK_WIDGET (gsb_reconcile_sort_config_create ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Sort for reconciliation"), page);
	page++;

    /* append group page "Transaction form" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Transaction form"), -1);

	/* append page Content of form */
	widget = GTK_WIDGET (gsb_form_config_create_page ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Content"), page);
	priv->form_num_page = page;
	page++;

	/* append page Behavior */
	widget = GTK_WIDGET (onglet_diverse_form_and_lists ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Behavior"), page);
	page++;

	/* append page Completion */
	widget = GTK_WIDGET (onglet_form_completion ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Completion"), page);
	page++;

    /* append group page "Resources subtree" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Resources"), -1);

	/* append page Currencies */
	widget = GTK_WIDGET (gsb_currency_config_create_page ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Currencies"), page);
	page++;

	/* append page Currencies links */
	widget = GTK_WIDGET (gsb_currency_link_config_create_page ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Currencies links"), page);
	page++;

	/* append page Banks */
	widget = GTK_WIDGET (gsb_bank_create_page (FALSE));
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Banks"), page);
	page++;

	/* append page Financial years */
	widget = GTK_WIDGET (gsb_fyear_config_create_page ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Financial years"), page);
	page++;

	/* append page Payment methods */
	widget = GTK_WIDGET (gsb_payment_method_config_create ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Payment methods"), page);
	page++;

    /* append group page "Balance estimate" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Balance estimate"), -1);

	/* append page General Options */
	widget = GTK_WIDGET (bet_config_general_create_general_page ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("General Options"), page);
	page++;

	/* append page Accounts data */
	widget = GTK_WIDGET (bet_config_account_create_account_page ());
	utils_widget_set_padding (widget, MARGIN_BOX, 0);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Accounts data"), page);
	page++;
}

/**
 * création du tree_view qui liste les onglets de la fenêtre de dialogue
 *
 *
 *\return tree_view or NULL;
 * */
static GtkWidget *grisbi_prefs_left_tree_view_setup (GrisbiPrefs *prefs)
{
	GtkWidget *sw;
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
    /* Create sw */
	sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    /* Create treeView */
    tree_view = gtk_tree_view_new ();
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
    g_object_unref (G_OBJECT (model));

    /* set the color of selected row */
	gtk_widget_set_name (tree_view, "tree_view");

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
    priv->left_treeview = tree_view;

    /* Put the tree in the scroll */
    gtk_container_add (GTK_CONTAINER (sw), tree_view);

	return sw;
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void grisbi_prefs_init (GrisbiPrefs *prefs)
{
	GtkWidget *content_area;
	GtkWidget *prefs_paned;
	GtkWidget *tree;
	GtkWidget *hbox;
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);

    gtk_dialog_add_buttons (GTK_DIALOG (prefs), "gtk-close", GTK_RESPONSE_CLOSE, NULL);

	g_signal_connect (G_OBJECT (prefs), "size-allocate", (GCallback) grisbi_prefs_size_allocate, NULL);

	gtk_window_set_transient_for (GTK_WINDOW (prefs), GTK_WINDOW (grisbi_app_get_active_window (NULL)));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (prefs), TRUE);

	/* construct the prefs archi */
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (prefs));
	prefs_paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	g_signal_connect (G_OBJECT (prefs_paned), "size-allocate", (GCallback) grisbi_prefs_paned_size_allocate, NULL);

    /* Frame for preferences */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    priv->notebook_prefs = gtk_notebook_new ();
    gtk_notebook_set_show_border (GTK_NOTEBOOK (priv->notebook_prefs), FALSE);
    gtk_notebook_set_show_tabs  (GTK_NOTEBOOK (priv->notebook_prefs), FALSE);
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (priv->notebook_prefs), TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), priv->notebook_prefs, TRUE, TRUE, 0);

	/* initialise left_tree_view */
	tree = grisbi_prefs_left_tree_view_setup (prefs);

	/* construct paned */
    gtk_paned_pack1 (GTK_PANED (prefs_paned), tree, TRUE, FALSE);
    gtk_paned_pack2 (GTK_PANED (prefs_paned), hbox, TRUE, FALSE);

	gtk_container_set_border_width (GTK_CONTAINER(prefs_paned), MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (content_area), prefs_paned, TRUE, TRUE, 0);
	if (conf.prefs_height && conf.prefs_width)
	{
        gtk_window_set_default_size (GTK_WINDOW (prefs), conf.prefs_width, conf.prefs_height);
	}
    else
	{
        gtk_window_set_default_size (GTK_WINDOW (prefs), PREFS_MIN_WIN_WIDTH, PREFS_MIN_WIN_HEIGHT);
	}

	if (conf.prefs_panel_width)
	{
        gtk_paned_set_position (GTK_PANED (prefs_paned), conf.prefs_panel_width);
	}
    else
	{
		gtk_paned_set_position (GTK_PANED (prefs_paned), PREFS_MIN_PANED_WIDTH);
	}

	/* creation de la vbox pour la page import */
	priv->vbox_import_page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	/* remplissage du paned gauche */
    grisbi_prefs_left_panel_populate_tree_model (prefs);

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

    /* libération de l'objet prefs */
    G_OBJECT_CLASS (grisbi_prefs_parent_class)->finalize (object);
	grisbi_win_set_prefs_dialog (NULL, NULL);
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
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
GrisbiPrefs *grisbi_prefs_new (GrisbiWin *win)
{
	return g_object_new (GRISBI_PREFS_TYPE, "transient-for", win, NULL);
}

void grisbi_prefs_set_page_by_name (gchar *page_name)
{
	GrisbiPrefs *prefs;
	GrisbiPrefsPrivate *priv;

	devel_debug (page_name);

	prefs = grisbi_prefs_new (grisbi_app_get_active_window (NULL));
	priv = grisbi_prefs_get_instance_private (prefs);

	if (strcmp (page_name, "form_num_page") == 0)
	{
		utils_prefs_left_panel_tree_view_select_page (priv->left_treeview, priv->notebook_prefs, priv->form_num_page);
	}
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

