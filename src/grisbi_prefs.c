/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2020 Pierre Biava (grisbi@pierre.biava.name)                 */
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

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
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
#include "mouse.h"
#include "parametres.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_gtkbuilder.h"
#include "utils_prefs.h"
#include "prefs/prefs_page_accueil.h"
#include "prefs/prefs_page_archives.h"
#include "prefs/prefs_page_bank.h"
#include "prefs/prefs_page_bet_account.h"
#include "prefs/prefs_page_bet_general.h"
#include "prefs/prefs_page_display_adr.h"
#include "prefs/prefs_page_display_fonts.h"
#include "prefs/prefs_page_display_form.h"
#include "prefs/prefs_page_display_gui.h"
#include "prefs/prefs_page_display_ope.h"
#include "prefs/prefs_page_divers.h"
#include "prefs/prefs_page_files.h"
#include "prefs/prefs_page_form_completion.h"
#include "prefs/prefs_page_form_options.h"
#include "prefs/prefs_page_fyear.h"
#include "prefs/prefs_page_import_asso.h"
#include "prefs/prefs_page_import_files.h"
#include "prefs/prefs_page_metatree.h"
#include "prefs/prefs_page_msg_delete.h"
#include "prefs/prefs_page_msg_warning.h"
#include "prefs/prefs_page_options_ope.h"
#include "prefs/prefs_page_reconcile.h"
#include "prefs/prefs_page_reconcile_sort.h"
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
	gint				metatree_num_page;
	gint				reconcile_sort_page_num;
 };


G_DEFINE_TYPE_WITH_PRIVATE (GrisbiPrefs, grisbi_prefs, GTK_TYPE_DIALOG)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 */
static gboolean grisbi_prefs_left_panel_tree_view_selection_changed (GtkTreeSelection *selection,
																	 GtkWidget *notebook)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
	GtkTreePath *path;
    gint selected;
	GrisbiWinRun *w_run;

    if (! gtk_tree_selection_get_selected (selection, &model, &iter))
	{
        return (FALSE);
	}

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	path = gtk_tree_model_get_path (model, &iter);
	if (w_run->prefs_selected_row)
		g_free (w_run->prefs_selected_row);

	w_run->prefs_selected_row = gtk_tree_path_to_string (path);
    gtk_tree_model_get (model, &iter, 1, &selected, -1);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), selected);
	gtk_tree_path_free (path);

    /* return */
    return FALSE;
}

/**
 *  expand or collapse the tree_view and select thee path when the widget is realized
 *
 * \param		tree_view
 * \param		w_run->prefs_selected_row et w_run->prefs_expand_tree
 *
 * \return
 **/
static void grisbi_prefs_left_tree_view_select_path_realize (GtkWidget *tree_view,
															 GrisbiWinRun *w_run)
{
    GtkTreePath *path;
	GtkTreeSelection *selection;
	gchar *str_path;
	gboolean first_path = TRUE;				/* cas où l'onglet fichiers est sélectionné vrai au démarrage */

	/* set the path */
	str_path = w_run->prefs_selected_row;

	if (strcmp (str_path, "0:0") == 0)
	{
		first_path = TRUE;
	}
	else
	{
		first_path = FALSE;
	}

    path = gtk_tree_path_new_from_string (str_path);

	/* expand or collapse all */
	if (w_run->prefs_expand_tree)
	{
		gtk_tree_view_expand_all (GTK_TREE_VIEW (tree_view));
	}
	else
	{
		gtk_tree_view_collapse_all (GTK_TREE_VIEW (tree_view));
		gtk_tree_view_expand_to_path (GTK_TREE_VIEW (tree_view), path);
	}

    /* selection de l'item sélectionnable */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	if (first_path)
	{
		GtkTreePath *tmp_path;

		tmp_path = gtk_tree_path_new_from_string ("0");
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), tmp_path, NULL, TRUE, 0.0, 0.0 );
		gtk_tree_path_free (tmp_path);
	}
	else
	{
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, FALSE, 0.0, 0.0 );
	}

	gtk_tree_selection_select_path (GTK_TREE_SELECTION (selection), path);
	gtk_tree_path_free (path);
}

/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 **/
static gboolean grisbi_prefs_left_treeview_button_press (GtkWidget *tree_view,
														 GdkEventButton *ev,
														 gpointer null)
{
    if (ev->type == GDK_2BUTTON_PRESS)
    {
        GtkTreePath *path = NULL;

        if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view), ev->x, ev->y, &path, NULL, NULL, NULL))
        {
            if (gtk_tree_view_row_expanded (GTK_TREE_VIEW (tree_view), path))
                gtk_tree_view_collapse_row (GTK_TREE_VIEW (tree_view), path);
            else
                gtk_tree_view_expand_row (GTK_TREE_VIEW (tree_view), path, FALSE);

            gtk_tree_path_free (path);

            return FALSE;
        }
    }

    return FALSE;
}

/**
 * Cette fonction réduit ou développe toutes les lignes du tree_view.
 * Le libellé du bouton est modifié en conséquence.
 *
 * \param le button de commande
 * \param le tree_view considéré
 *
 * \return
 **/
static void grisbi_prefs_collapse_expand_all_rows (GtkToggleButton *togglebutton,
												   GtkWidget *tree_view)
{
    GtkWidget *hbox_expand;
    GtkWidget *hbox_collapse;
	GrisbiWinRun *w_run;
	devel_debug (NULL);

    hbox_expand = g_object_get_data (G_OBJECT (togglebutton), "hbox_expand");
    hbox_collapse = g_object_get_data (G_OBJECT (togglebutton), "hbox_collapse");
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* on remet à FALSE la propriété "no-show-all" utilisée pour initialiser le bouton */
	/* voir etats_prefs_toggle_button_init_button_expand () */
	/* et grisbi_prefs_init () */
	if (gtk_widget_get_no_show_all (hbox_expand))
		gtk_widget_set_no_show_all (hbox_expand, FALSE);
	else
		gtk_widget_set_no_show_all (hbox_collapse, FALSE);

	if (gtk_toggle_button_get_active (togglebutton))
	{
		gtk_widget_hide (hbox_expand);
		gtk_widget_show_all (hbox_collapse);
		gtk_tree_view_expand_all (GTK_TREE_VIEW (tree_view));
		w_run->prefs_expand_tree = TRUE;
	}
	else
	{
		gtk_widget_show_all (hbox_expand);
		gtk_widget_hide (hbox_collapse);
		gtk_tree_view_collapse_all (GTK_TREE_VIEW (tree_view));
		w_run->prefs_expand_tree = FALSE;
	}
}

/**
 * récupère la largeur des préférences
 *
 * \param GtkWidget     prefs
 * \param GtkAllocation   allocation
 * \param gpointer       null
 *
 * \return           FALSE
 **/
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
 **/
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
 **/
static GtkWidget *grisbi_prefs_setup_import_page (GrisbiPrefs *prefs)
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
	gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook_import_pages), FALSE);
	gtk_box_pack_start (GTK_BOX (priv->vbox_import_page), notebook_import_pages, TRUE, TRUE, 0);

	/* set import settings */
	vbox_import_files = GTK_WIDGET (prefs_page_import_files_new (prefs));
	gtk_widget_set_margin_top (vbox_import_files, MARGIN_TOP);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook_import_pages), vbox_import_files, NULL);
	label = gtk_label_new (_("Files import"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_import_pages), vbox_import_files, label);

	/* set manage associations */
	vbox_import_asso = GTK_WIDGET (prefs_page_import_asso_new (prefs));
	gtk_widget_set_margin_top (vbox_import_asso, MARGIN_TOP);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook_import_pages), vbox_import_asso, NULL);
	label = gtk_label_new (_("Associations for import"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_import_pages), vbox_import_asso, label);
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_prefs), priv->vbox_import_page, NULL);
	gtk_widget_show (notebook_import_pages);

	return priv->vbox_import_page;
}

/* LEFT PANED */
 /**
 * remplit le model pour la configuration des états
 *
 * \param GtkTreeStore		model
 * \param GrisbiPrefs		prefs
 *
 * \return
 **/
static void grisbi_prefs_left_panel_populate_tree_model (GrisbiPrefs *prefs)
{
    GtkWidget *widget = NULL;
	GtkTreeStore *tree_model;
    gint page = 0;
	gboolean is_loading;
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);
	is_loading = grisbi_win_file_is_loading ();

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
    widget = grisbi_prefs_setup_import_page (prefs);
	if (is_loading == FALSE)
		gtk_widget_set_sensitive (widget, FALSE);
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Import"), page);
    page++;

	/* append page Divers */
    widget = GTK_WIDGET (prefs_page_divers_new (prefs));
    utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Various settings"), page);
    page++;

	/* append page Accueil */
	widget = GTK_WIDGET (prefs_page_accueil_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Main page"), page);
	page++;

	/* append group page "Display" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Display"), -1);

	/* append page Fonts & logo */
	widget = GTK_WIDGET (prefs_page_display_fonts_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Fonts & Logo"), page);
	page++;

	/* append page Messages & warnings */
	widget = GTK_WIDGET (prefs_page_msg_warning_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Messages & warnings"), page);
	page++;

	/* append page Addresses & titles */
	widget = GTK_WIDGET (prefs_page_display_adr_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Addresses & titles"), page);
	page++;

	/* append page Payees, categories and budgetaries */
	widget = GTK_WIDGET (prefs_page_metatree_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Payees, categories and budgetaries"), page);
	priv->metatree_num_page = page;
	page++;

	/* append page Elements of interface */
	widget = GTK_WIDGET (prefs_page_display_gui_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Elements of interface"), page);
	page++;

	/* append group page "Transactions" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Transactions"), -1);

	/* append page List behavior */
	widget = GTK_WIDGET (prefs_page_options_ope_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("List behavior"), page);
	page++;

	/* append page Transactions list cells */
	widget = GTK_WIDGET (prefs_page_display_ope_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Transactions list cells"), page);
	page++;

	/* append page Messages before deleting */
	widget = GTK_WIDGET (prefs_page_msg_delete_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Messages before deleting"), page);
	page++;

	/* append page Reconciliation */
	widget = GTK_WIDGET (prefs_page_reconcile_new (prefs));
	if (is_loading == FALSE)
		gtk_widget_set_sensitive (widget, FALSE);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Reconciliation"), page);
	page++;

	/* append page Sort for reconciliation */
	widget = GTK_WIDGET (prefs_page_reconcile_sort_new (prefs));
	if (is_loading == FALSE)
		gtk_widget_set_sensitive (widget, FALSE);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Sort for reconciliation"), page);
	priv->reconcile_sort_page_num = page;
	page++;

    /* append group page "Transaction form" */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Transaction form"), -1);

	/* append page Form behavior */
	widget = GTK_WIDGET (prefs_page_form_options_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Form behavior"), page);
	page++;

	/* append page Content of form */
	widget = GTK_WIDGET (prefs_page_display_form_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Content"), page);
	priv->form_num_page = page;
	page++;

	/* append page Completion */
	widget = GTK_WIDGET (prefs_page_form_completion_new (prefs));
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
	widget = GTK_WIDGET (prefs_page_bank_new (prefs));
	if (is_loading == FALSE)
		gtk_widget_set_sensitive (widget, FALSE);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Banks"), page);
	page++;

	/* append page Financial years */
	widget = GTK_WIDGET (prefs_page_fyear_new (prefs));
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
	widget = GTK_WIDGET (prefs_page_bet_general_new (prefs));
	if (is_loading == FALSE)
		gtk_widget_set_sensitive (widget, FALSE);
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("General Options"), page);
	page++;

	/* append page Accounts data */
	widget = GTK_WIDGET (prefs_page_bet_account_new (prefs));
	utils_prefs_left_panel_add_line (tree_model, priv->notebook_prefs, widget, _("Accounts data"), page);
	page++;
}

/**
 * création du tree_view qui liste les onglets de la fenêtre de dialogue
 *
 *
 *\return tree_view or NULL;
 **/
static GtkWidget *grisbi_prefs_left_tree_view_setup (GrisbiPrefs *prefs)
{
	GtkWidget *sw;
    GtkWidget *tree_view = NULL;
    GtkTreeStore *model = NULL;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;
	GrisbiWinRun *w_run;
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

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

    /* Choose which entries will be selectable */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    gtk_tree_selection_set_select_function (selection,
											utils_prefs_left_panel_tree_view_selectable_func,
											NULL,
											NULL);

	/* set headers hide */
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);

	/* expand all rows after the treeview widget has been realized */
    g_signal_connect (tree_view,
					  "realize",
					  G_CALLBACK (grisbi_prefs_left_tree_view_select_path_realize),
					  w_run);
    priv->left_treeview = tree_view;

	g_signal_connect (G_OBJECT (tree_view),
					  "button-press-event",
					  G_CALLBACK (grisbi_prefs_left_treeview_button_press),
					  NULL);

    /* Handle select */
    g_signal_connect (selection,
					  "changed",
					  G_CALLBACK (grisbi_prefs_left_panel_tree_view_selection_changed),
					  priv->notebook_prefs);

    /* Put the tree in the scroll */
    gtk_container_add (GTK_CONTAINER (sw), tree_view);

	return sw;
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_prefs_init (GrisbiPrefs *prefs)
{
	GtkWidget *content_area;
	GtkWidget *prefs_paned;
	GtkWidget *tree;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *button_hbox1;
	GtkWidget *button_hbox2;
	GtkWidget *button_vbox;
	GtkWidget *label;
	GtkWidget *image;
    gchar *filename;
	GrisbiWinRun *w_run;
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);
	grisbi_win_status_bar_message (_("Preferences start"));

	priv = grisbi_prefs_get_instance_private (prefs);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	gtk_dialog_add_buttons (GTK_DIALOG (prefs), "gtk-close", GTK_RESPONSE_CLOSE, NULL);

	g_signal_connect (G_OBJECT (prefs), "size-allocate", (GCallback) grisbi_prefs_size_allocate, NULL);

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
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	tree = grisbi_prefs_left_tree_view_setup (prefs);
	gtk_widget_set_vexpand (tree, TRUE);
	gtk_box_pack_start (GTK_BOX (vbox), tree, TRUE, TRUE, 0);

	/* set the expand collapse button */
	button = gtk_toggle_button_new ();
	gtk_widget_set_tooltip_text (button,
								 _("This state will be kept for the duration of this session"));

	/* construction du bouton */
	button_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	button_hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
	gtk_widget_set_halign (button_hbox1, GTK_ALIGN_CENTER);
	g_object_set_data (G_OBJECT (button), "hbox_expand", button_hbox1);
    filename = g_build_filename (gsb_dirs_get_pixmaps_dir (), "gsb-down-16.png", NULL);
	image = gtk_image_new_from_file (filename);
	g_free (filename);
	gtk_box_pack_start (GTK_BOX (button_hbox1), image, FALSE, FALSE, 0);
	label = gtk_label_new (_("Expand all"));
	gtk_box_pack_start (GTK_BOX (button_hbox1), label, FALSE, FALSE, 0);

	button_hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
	gtk_widget_set_halign (button_hbox2, GTK_ALIGN_CENTER);
	g_object_set_data (G_OBJECT (button), "hbox_collapse", button_hbox2);
    filename = g_build_filename (gsb_dirs_get_pixmaps_dir (), "gsb-up-16.png", NULL);
	image = gtk_image_new_from_file (filename);
	g_free (filename);
	gtk_box_pack_start (GTK_BOX (button_hbox2), image, FALSE, FALSE, 0);
	label = gtk_label_new (_("Collapse all"));
	gtk_box_pack_start (GTK_BOX (button_hbox2), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (button_vbox), button_hbox1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (button_vbox), button_hbox2, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (button), button_vbox);

	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	/* positionne le button en fonction de run->prefs_expand_tree */
	if (w_run->prefs_expand_tree)
	{
		gtk_widget_set_no_show_all (button_hbox1, TRUE);
		gtk_widget_hide (button_hbox1);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	}
	else
	{
		gtk_widget_set_no_show_all (button_hbox2, TRUE);
		gtk_widget_hide (button_hbox2);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
	}

    g_signal_connect (G_OBJECT (button),
					  "clicked",
					  G_CALLBACK (grisbi_prefs_collapse_expand_all_rows),
					  priv->left_treeview);

	/* construct paned */
    gtk_paned_pack1 (GTK_PANED (prefs_paned), vbox, TRUE, FALSE);
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

	grisbi_win_status_bar_message (_("Done"));
}

/**
 * finalise GrisbiPrefs
 *
 * \param object
 *
 * \return
 **/
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
 **/
static void grisbi_prefs_class_init (GrisbiPrefsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grisbi_prefs_finalize;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
GrisbiPrefs *grisbi_prefs_new (GrisbiWin *win)
{
	return g_object_new (GRISBI_PREFS_TYPE, "transient-for", win, NULL);
}

/**
 * retourne le widget enfant de la page passée en paramètre
 *
 * \param page_name
 *
 * \return the child of notebook page
 **/
GtkWidget *grisbi_prefs_get_child_by_page_name (const gchar *page_name)
{
	GtkWidget *widget = NULL;
	GrisbiPrefs *prefs;
	GrisbiWin *win;
	GrisbiPrefsPrivate *priv;

	devel_debug (page_name);
	win = grisbi_app_get_active_window (NULL);
	prefs = GRISBI_PREFS (grisbi_win_get_prefs_dialog (win));
	priv = grisbi_prefs_get_instance_private (prefs);

	if (strcmp (page_name, "metatree_num_page") == 0)
	{
		widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook_prefs), priv->metatree_num_page);
	}
	else if (strcmp (page_name, "reconcile_sort_page_num") == 0)
	{
		widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook_prefs), priv->reconcile_sort_page_num);
	}

	return widget;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_prefs_set_page_by_name (gchar *page_name)
{
	GrisbiPrefs *prefs;
	GrisbiWin *win;
	GrisbiPrefsPrivate *priv;
	gint result;

	devel_debug (page_name);
	win = grisbi_app_get_active_window (NULL);
	prefs = grisbi_prefs_new (win);
	priv = grisbi_prefs_get_instance_private (prefs);
	grisbi_win_set_prefs_dialog (win, GTK_WIDGET (prefs));

	gtk_window_present (GTK_WINDOW (prefs));
	gtk_widget_show_all (GTK_WIDGET (prefs));

	if (strcmp (page_name, "form_num_page") == 0)
	{
		utils_prefs_left_panel_tree_view_select_page (priv->left_treeview, priv->notebook_prefs, priv->form_num_page);
	}
	result = gtk_dialog_run (GTK_DIALOG (prefs));
	grisbi_prefs_dialog_response (GTK_DIALOG (prefs), result);
}

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
	if (result_id == GTK_RESPONSE_CLOSE)
	{
		GrisbiWinRun *w_run;

		grisbi_win_status_bar_message (_("Preferences stop"));
		if (!prefs)
		{
			grisbi_win_status_bar_message (_("Done"));

			return;
		}
		/* on récupère éventuellement la dimension de la fenêtre */
		w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
		settings = grisbi_settings_get_settings (SETTINGS_PREFS);
		if (w_run->resolution_screen_toggled == FALSE)
		{
			gtk_window_get_size (GTK_WINDOW (prefs), &conf.prefs_width, &conf.prefs_height);
			g_settings_set_int (G_SETTINGS (settings),
								"prefs-height",
								conf.prefs_height);

			g_settings_set_int (G_SETTINGS (settings),
								"prefs-panel-width",
								conf.prefs_panel_width);

			g_settings_set_int (G_SETTINGS (settings),
								"prefs-width",
								conf.prefs_width);
		}
		else
		{
			w_run->resolution_screen_toggled = FALSE;
			g_settings_reset (G_SETTINGS (settings), "prefs-height");
			g_settings_reset (G_SETTINGS (settings), "prefs-panel-width");
			g_settings_reset (G_SETTINGS (settings), "prefs-width");

			conf.prefs_height = g_settings_get_int (settings, "prefs-height");
			conf.prefs_panel_width = g_settings_get_int (settings, "prefs-panel-width");
			conf.prefs_width = g_settings_get_int (settings, "prefs-width");
		}
	}
	gtk_widget_destroy (GTK_WIDGET (prefs));
	grisbi_win_set_prefs_dialog (NULL, NULL);
	grisbi_win_status_bar_message (_("Done"));
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *grisbi_prefs_get_left_treeview (void)
{
	GrisbiPrefs *prefs;
	GrisbiWin *win;
	GrisbiPrefsPrivate *priv;

	win = grisbi_app_get_active_window (NULL);
	prefs = GRISBI_PREFS (grisbi_win_get_prefs_dialog (win));
	priv = grisbi_prefs_get_instance_private (prefs);

	return priv->left_treeview;
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

