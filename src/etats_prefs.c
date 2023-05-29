/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)           */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include <gtk/gtk.h>
#include <stdlib.h>

/*START_INCLUDE*/
#include "etats_prefs.h"
#include "etats_prefs_private.h"
#include "etats_config.h"
#include "etats_page_amount.h"
#include "etats_page_accounts.h"
#include "etats_page_budget.h"
#include "etats_page_category.h"
#include "etats_page_payee.h"
#include "etats_page_period.h"
#include "etats_page_text.h"
#include "etats_page_transfer.h"
#include "grisbi_app.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_gtkbuilder.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
#ifdef OS_OSX
static const gchar *label_search_help = N_("Command-click\nto add to the selection");
#endif /* OS_OSX */
/*END_STATIC*/

/* Private structure type */
typedef struct _EtatsPrefsPrivate EtatsPrefsPrivate;

struct _EtatsPrefsPrivate
{
    GtkWidget *		hpaned;
	GtkWidget *		notebook_etats_prefs;

	gboolean		form_date_force_prev_year;
};

G_DEFINE_TYPE_WITH_PRIVATE (EtatsPrefs, etats_prefs, GTK_TYPE_DIALOG)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/* GTK_BUILDER */
/**
 * Crée un builder et récupère les widgets du fichier grisbi.ui
 *
 * \param
 *
 * \rerurn
 **/
static gboolean etats_prefs_initialise_builder (EtatsPrefs *prefs)
{
	EtatsPrefsPrivate *priv;

	/* Creation d'un nouveau GtkBuilder */
    etats_prefs_builder = gtk_builder_new ();

	if (etats_prefs_builder == NULL)
		return FALSE;

	/* Chargement du XML dans etats_prefs_builder */
	if (!utils_gtkbuilder_merge_ui_data_in_builder (etats_prefs_builder, "etats_prefs.ui"))
		return FALSE;

	priv = etats_prefs_get_instance_private (prefs);
	priv->hpaned = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "dialog_hpaned"));
	priv->notebook_etats_prefs = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "notebook_etats_prefs"));

    return TRUE;
}

/*FONCTIONS UTILITAIRES COMMUNES*/
/**
 * crée un nouveau tree_view initialisé avec model.
 * le modèle comporte deux colonnes : G_TYPE_STRING, G_TYPE_INT
 * le tree_view n'affiche que la colonne texte.
 *
 * \param
 * \param
 *
 * \return the tree_wiew
 **/
static GtkWidget *etats_prefs_tree_view_new_with_model (const gchar *treeview_name,
														GtkTreeModel *model)
{
    GtkWidget *tree_view;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, treeview_name));
    if (!tree_view)
        return NULL;

    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
    g_object_unref (G_OBJECT (model));

    /* set the color of selected row */
	gtk_widget_set_name (tree_view, "tree_view");

    /* set the column */
    cell = gtk_cell_renderer_text_new ();

    column = gtk_tree_view_column_new_with_attributes (NULL, cell, "text", 0, NULL);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_resizable (column, TRUE);

    return tree_view;
}

/**
 * initialise le tree_view avec son modèle et son type de sélection
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void etats_prefs_tree_view_init (const gchar *treeview_name,
										GtkTreeModel *(*function) (void),
										GtkSelectionMode type_selection,
										GCallback selection_callback)
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeSelection *selection;

    /* on récupère le model par appel à function */
    model = function ();

    tree_view = etats_prefs_tree_view_new_with_model (treeview_name, GTK_TREE_MODEL (model));
    gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (tree_view), TRUE);
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    gtk_tree_selection_set_mode (selection, type_selection);

    if (selection_callback)
        g_signal_connect (G_OBJECT (selection),
                    	  "changed",
                		  G_CALLBACK (selection_callback),
                 		  NULL);
}

/**
 * Sélectionne les iters en fonction des données de la liste
 *
 * \param liste des lignes à sélectionner
 * \param nom du tree_view concerné
 * \param numéro de la colonne contenant la donnée testée
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void etats_prefs_tree_view_select_rows_from_list (GSList *liste,
												  const gchar *treeview_name,
												  gint column)
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GSList *tmp_list;

    if (!liste)
        return;

    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, treeview_name));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
        do
        {
            gint tmp_number;

            gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, column, &tmp_number, -1);

            tmp_list = liste;
            while (tmp_list)
            {
                gint result;

                result = GPOINTER_TO_INT (tmp_list->data);

                if (result == tmp_number)
                    gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);

                tmp_list = tmp_list->next;
            }
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }
}

/**
 * Sélectionne les iters en fonction des données de la liste
 *
 * \param liste des lignes à sélectionner
 * \param nom du tree_view concerné
 * \param numéro de la colonne contenant la donnée testée
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void new_etats_prefs_tree_view_select_rows_from_list (GSList *liste,
													  GtkWidget *tree_view,
													  gint column)
{
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GSList *tmp_list;

    if (!liste)
        return;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
        do
        {
            gint tmp_number;

            gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, column, &tmp_number, -1);

            tmp_list = liste;
            while (tmp_list)
            {
                gint result;

                result = GPOINTER_TO_INT (tmp_list->data);

                if (result == tmp_number)
                    gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);

                tmp_list = tmp_list->next;
            }
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }
}

/*LEFT_PANEL*/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 */
static gboolean etats_prefs_left_panel_tree_view_selection_changed (GtkTreeSelection *selection,
                                                                    EtatsPrefs *prefs)
{
	GtkWidget *notebook;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint selected;
	EtatsPrefsPrivate *priv;

    if (! gtk_tree_selection_get_selected (selection, &model, &iter))
	{
        return (FALSE);
	}


	/* on recuper le notebook */
	priv = etats_prefs_get_instance_private (prefs);
	notebook = priv->notebook_etats_prefs;

	gtk_tree_model_get (model, &iter, 1, &selected, -1);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), selected);
	if (selected == PAYEE_PAGE_TYPE)
	{
		/* affiche le premier tiers concerné */
		etats_page_payee_show_first_row_selected (GTK_WIDGET (prefs));
	}

    /* return */
    return FALSE;
}

/**
 * If applicable, update report navigation tree style to reflect which
 * pages have been changed.
 *
 * \param page_number Page that contained an interface element just
 *                      changed that triggered this event.
 *
 * \return      FALSE
 **/
gboolean etats_prefs_left_panel_tree_view_update_style (GtkWidget *button,
															   gint *page_number)
{
    gint iter_page_number;

    iter_page_number = GPOINTER_TO_INT (page_number);

    if (iter_page_number)
    {
        GtkWidget *tree_view;
        GtkTreeModel *model;
        GtkTreeIter parent_iter;
        gint active;
        gboolean italic = 0;

        tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "treeview_left_panel"));
        model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
        active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
        italic = active;

        if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &parent_iter))
            return FALSE;

        do
        {
            GtkTreeIter iter;

            if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter, &parent_iter))
            {
                do
                {
                    gint page;

                    gtk_tree_model_get (GTK_TREE_MODEL (model),
                                &iter,
                                LEFT_PANEL_TREE_PAGE_COLUMN, &page,
                                -1);

                    if (page == iter_page_number)
                        gtk_tree_store_set (GTK_TREE_STORE (model),
                                &iter,
                                LEFT_PANEL_TREE_ITALIC_COLUMN, italic,
                                -1);
                }
                while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
            }
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &parent_iter));

        return TRUE;
    }

    /* return */
    return FALSE;
}

/**
 * selectionne une page
 *
 * \param
 *
 * \return
 **/
gboolean etats_prefs_left_panel_tree_view_select_last_page (void)
{
    GtkWidget *tree_view;
    GtkWidget *notebook;

    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "treeview_left_panel"));
    notebook = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "notebook_etats_prefs"));

    utils_prefs_left_panel_tree_view_select_page (tree_view, notebook, last_page);

    /* return */
    return FALSE;
}

static void etats_prefs_left_panel_notebook_change_page (GtkNotebook *notebook,
														 gpointer npage,
														 gint page,
														 gpointer user_data)
{
    last_page = page;
}

/**
 * remplit le model pour la configuration des états
 *
 * \param
 * \param
 *
 * \return
 **/
static void etats_prefs_left_panel_populate_tree_model (GtkTreeStore *tree_model,
														EtatsPrefs *prefs)
{
	GtkWidget *notebook;
    GtkWidget *widget = NULL;
    gint page = 0;
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (prefs);
	notebook = priv->notebook_etats_prefs;

    /* append group page */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Data selection"), -1);

    /* append page Dates */
	widget = GTK_WIDGET (etats_page_period_new (GTK_WIDGET (prefs)));
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Dates"), DATE_PAGE_TYPE);
    page++;

    /* append page Transferts */
    widget = GTK_WIDGET (etats_page_transfer_new (GTK_WIDGET (prefs)));
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Transfers"), TRANSFER_PAGE_TYPE);
    page++;

    /* append page Accounts */
    widget = GTK_WIDGET (etats_page_accounts_new (GTK_WIDGET (prefs)));
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Accounts"), ACCOUNT_PAGE_TYPE);
    page++;

    /* append page Payee */
    widget = GTK_WIDGET (etats_page_payee_new (GTK_WIDGET (prefs)));
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Payee"), PAYEE_PAGE_TYPE);
    page++;

    /* append page Categories */
    widget = GTK_WIDGET (etats_page_category_new (GTK_WIDGET (prefs)));
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Categories"), CATEGORY_PAGE_TYPE);
    page++;

    /* append page Budgetary lines */
    widget = GTK_WIDGET (etats_page_budget_new (GTK_WIDGET (prefs)));
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Budgetary lines"), BUDGET_PAGE_TYPE);
    page++;

    /* append page Text */
    widget = GTK_WIDGET (etats_page_text_new (GTK_WIDGET (prefs)));
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Texts"), TEXT_PAGE_TYPE);
    page++;

    /* append page Amounts */
    widget = GTK_WIDGET (etats_page_amount_new (GTK_WIDGET (prefs)));
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Amounts"), AMOUNT_PAGE_TYPE);
    page++;

    /* append page Payment methods */
    widget = etats_prefs_onglet_mode_paiement_create_page (page);
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Payment methods"), page);
    page++;

    /* append page Misc. */
    widget = etats_prefs_onglet_divers_create_page (page);
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Miscellaneous"), page);
    page++;

    /* remplissage de l'onglet d'organisation */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Data organization"), -1);

    /* Data grouping */
    widget = etats_prefs_onglet_data_grouping_create_page (page);
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Data grouping"), page);
    page++;

    /* Data separation */
    widget = etats_prefs_onglet_data_separation_create_page (page);
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Data separation"), page);
    page++;

    /* remplissage de l'onglet d'affichage */
    utils_prefs_left_panel_add_line (tree_model, NULL, NULL, _("Data display"), -1);

    /* append page Generalities */
    widget = etats_prefs_onglet_affichage_generalites_create_page (page);
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Generalities"), page);
    page++;

    /* append page divers */
    widget = etats_prefs_onglet_affichage_titles_create_page (page);
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Titles"), page);
    page++;

    /* append page Transactions */
    widget = etats_prefs_onglet_affichage_operations_create_page (page);
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Transactions"), page);
    page++;

    /* append page Currencies */
    widget = etats_prefs_onglet_affichage_devises_create_page (page);
    utils_prefs_left_panel_add_line (tree_model, notebook, widget, _("Currencies"), page);
}

/**
 * création du tree_view qui liste les onglets de la fenêtre de dialogue
 *
 * \param
 *
 *\return tree_view or NULL;
 **/
static GtkWidget *etats_prefs_left_panel_create_tree_view (EtatsPrefs *prefs)
{
    GtkWidget *tree_view = NULL;
    GtkWidget *notebook;
    GtkTreeStore *model = NULL;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;

    devel_debug (NULL);

    /* Création du model */
    model = gtk_tree_store_new (LEFT_PANEL_TREE_NUM_COLUMNS,
                        G_TYPE_STRING,  /* LEFT_PANEL_TREE_TEXT_COLUMN */
                        G_TYPE_INT,     /* LEFT_PANEL_TREE_PAGE_COLUMN */
                        G_TYPE_INT,     /* LEFT_PANEL_TREE_BOLD_COLUMN */
                        G_TYPE_INT);   /* LEFT_PANEL_TREE_ITALIC_COLUMN */

    /* Create container + TreeView */
    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "treeview_left_panel"));
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW (tree_view), FALSE);
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

    /* initialisation du notebook pour les pages de la configuration */
    notebook = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "notebook_etats_prefs"));
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
    gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);

    /* Handle select */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    g_signal_connect (selection,
                        "changed",
                        G_CALLBACK (etats_prefs_left_panel_tree_view_selection_changed),
                        prefs);

    /* Choose which entries will be selectable */
    gtk_tree_selection_set_select_function (selection,
                        utils_prefs_left_panel_tree_view_selectable_func, NULL, NULL);

    /* expand all rows after the treeview widget has been realized */
    g_signal_connect (tree_view,
                        "realize",
                        G_CALLBACK (utils_tree_view_set_expand_all_and_select_path_realize),
                        (gpointer) "0:0");

    /* remplissage du paned gauche */
    etats_prefs_left_panel_populate_tree_model (model, prefs);

    /* on met la connexion pour mémoriser la dernière page utilisée */
    g_signal_connect_after (notebook,
                        "switch-page",
                        G_CALLBACK (etats_prefs_left_panel_notebook_change_page),
                        NULL);

    /* show all widgets */
    gtk_widget_show_all (tree_view);


    return tree_view;
}

/*RIGHT_PANEL : ONGLET_MODE_PAIEMENT*/
/**
 * Sélectionne les iters en fonction des données de la liste
 *
 * \param liste des lignes à sélectionner
 * \param nom du tree_view concerné
 *
 * \return
 **/
void etats_prefs_onglet_mode_paiement_select_rows_from_list (GSList *liste,
                        									 const gchar *treeview_name)
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GSList *tmp_list;

    if (!liste)
        return;

    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, treeview_name));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
        do
        {
            gchar *tmp_str;

            gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &tmp_str, -1);

            tmp_list = liste;
            while (tmp_list)
            {
                gchar *str;

                str = tmp_list->data;

                if (strcmp (str, tmp_str) == 0)
                    gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);

                tmp_list = tmp_list->next;
            }
            g_free (tmp_str);
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }
}

/**
 * Création de l'onglet moyens de paiement
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_mode_paiement_create_page (gint page)
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *tree_view;
    GtkWidget *button;

    vbox_onglet =  GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "onglet_etat_mode_paiement"));

    vbox = new_vbox_with_title_and_icon (_("Payment methods"), "gsb-payment-32.png");

    gtk_box_pack_start (GTK_BOX (vbox_onglet), vbox, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (vbox_onglet), vbox, 0);

    etats_prefs_widget_set_sensitive ("vbox_mode_paiement_etat", FALSE);

	/* on adapte le label pour Mac_OSX */
#ifdef OS_OSX
	GtkLabel *label;

	label = GTK_LABEL (gtk_builder_get_object (etats_prefs_builder, "label_modes_search_help"));
	gtk_label_set_text (label, _(label_search_help));
	gtk_label_set_justify (label, GTK_JUSTIFY_CENTER);
#endif /* OS_OSX */

    /* on crée la liste des moyens de paiement */
    etats_prefs_tree_view_init ("treeview_mode_paiement",
                        etats_config_onglet_mode_paiement_get_model,
                        GTK_SELECTION_MULTIPLE,
                        NULL);

    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "treeview_mode_paiement"));

    /* on met la connection pour changer le style de la ligne du panneau de gauche */
    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "bouton_detaille_mode_paiement_etat"));
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
                        GINT_TO_POINTER (page));

    /* on met la connection pour rendre sensitif la vbox_generale_comptes_etat */
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "vbox_mode_paiement_etat"));

    /* on met la connection pour (dé)sélectionner tous les tiers */
    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder,
                        "togglebutton_select_all_mode_paiement"));
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (utils_togglebutton_select_unselect_all_rows),
                        tree_view);

    gtk_widget_show_all (vbox_onglet);

    return vbox_onglet;
}

/*RIGHT_PANEL : ONGLET_DIVERS*/
/**
 * If applicable, update report navigation tree style to reflect which
 * pages have been changed.
 *
 * \param page_number Page that contained an interface element just
 *                      changed that triggered this event.
 *
 * \return      FALSE
 **/
static gboolean etats_prefs_onglet_divers_update_style_left_panel (GtkWidget *button,
																   gint *page_number)
{
    gint active;
    gint index;

    index = etats_prefs_buttons_radio_get_active_index ("radiobutton_marked_all");
    active = etats_prefs_button_toggle_get_actif ("bouton_pas_detailler_ventilation");

    if (GTK_IS_RADIO_BUTTON (button))
    {
        if (active == 0)
            etats_prefs_left_panel_tree_view_update_style (button, page_number);
    }
    else
    {
        if (index == 0)
            etats_prefs_left_panel_tree_view_update_style (button, page_number);
    }

    return TRUE;
}

/**
 * Création de l'onglet Divers
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_divers_create_page (gint page)
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *button;

    vbox_onglet =  GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "onglet_etat_divers"));

    vbox = new_vbox_with_title_and_icon (_("Miscellaneous"), "gsb-generalities-32.png");

    gtk_box_pack_start (GTK_BOX (vbox_onglet), vbox, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (vbox_onglet), vbox, 0);

    /* on met la connection pour changer le style de la ligne du panneau de gauche */
    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "radiobutton_marked"));
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (etats_prefs_onglet_divers_update_style_left_panel),
                        GINT_TO_POINTER (page));

    /* on met la connection pour rendre sensitif la vbox_marked_buttons */
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "vbox_marked_buttons"));

    /* on met la connection pour changer le style de la ligne du panneau de gauche */
    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "radiobutton_marked_No_R"));
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (etats_prefs_onglet_divers_update_style_left_panel),
                        GINT_TO_POINTER (page));

    /* on met la connection pour changer le style de la ligne du panneau de gauche */
    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "bouton_pas_detailler_ventilation"));
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (etats_prefs_onglet_divers_update_style_left_panel),
                        GINT_TO_POINTER (page));

    gtk_widget_show_all (vbox_onglet);

    return vbox_onglet;
}

/*RIGHT_PANEL : ONGLET_DATA_GROUPING*/
/**
 * callback when treeview_data_grouping receive a drag and drop signal
 *
 * \param drag_dest
 * \param dest_path
 * \param selection_data
 *
 * \return FALSE
 **/
static gboolean etats_prefs_onglet_data_grouping_drag_data_received (GtkTreeDragDest *drag_dest,
																	 GtkTreePath *dest_path,
																	 GtkSelectionData *selection_data)
{
    if (dest_path && selection_data)
    {
        GtkTreeModel *model;
        GtkTreeIter src_iter;
        GtkTreeIter dest_iter;
        GtkTreePath *src_path;
        gint src_pos = 0;
        gint dest_pos = 0;
        gint src_type_data = 0;
        gint dest_type_data = 0;

        /* On récupère le model et le path d'origine */
        gtk_tree_get_row_drag_data (selection_data, &model, &src_path);

        /* On récupère les données des 2 lignes à modifier */
        if (gtk_tree_model_get_iter (model, &src_iter, src_path))
            gtk_tree_model_get (model, &src_iter, 1, &src_pos, 2, &src_type_data, -1);

        if (gtk_tree_model_get_iter (model, &dest_iter, dest_path))
            gtk_tree_model_get (model, &dest_iter, 1, &dest_pos, 2, &dest_type_data, -1);
        else
            return FALSE;

        /* on met à jour la liste des types pour le tri de données */
        etats_config_onglet_data_grouping_move_in_list (src_pos,
                        src_type_data,
                        dest_pos);

        return TRUE;
    }

    /* return */
    return FALSE;
}

/**
 * Fill the drag & drop structure with the path of selected column.
 * This is an interface function called from GTK, much like a callback.
 *
 * \param drag_source       Not used.
 * \param path              Original path for the gtk selection.
 * \param selection_data    A pointer to the drag & drop structure.
 *
 * \return FALSE, to allow future processing by the callback chain.
 **/
static gboolean etats_prefs_onglet_data_grouping_drag_data_get (GtkTreeDragSource *drag_source,
																GtkTreePath *path,
																GtkSelectionData *selection_data)
{
    if (path)
    {
        GtkWidget *tree_view;
        GtkTreeModel *model;

        tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "treeview_data_grouping"));
        model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

        gtk_tree_set_row_drag_data (selection_data, GTK_TREE_MODEL (model), path);
    }

    return FALSE;
}

/**
 * Checks the validity of the change of position
 * This is an interface function called from GTK, much like a callback.
 *
 * \param drag_dest         Not used.
 * \param path              Original path for the gtk selection.
 * \param selection_data    A pointer to the drag & drop structure.
 *
 * \return FALSE, to allow future processing by the callback chain.
 **/
static gboolean etats_prefs_onglet_data_grouping_drop_possible (GtkTreeDragDest *drag_dest,
																GtkTreePath *dest_path,
																GtkSelectionData *selection_data)
{
    GtkTreePath *orig_path;
    GtkTreeModel *model;
    gint src_pos = 0;
    gint dst_pos = 0;
    GtkTreeIter iter;

    gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);

    if (gtk_tree_model_get_iter (model, &iter, orig_path))
        gtk_tree_model_get (model, &iter, 1, &src_pos, -1);

    if (gtk_tree_model_get_iter (model, &iter, dest_path))
        gtk_tree_model_get (model, &iter, 1, &dst_pos, -1);

    if (dst_pos < 0 || dst_pos > 3)
        return FALSE;

    if (src_pos != dst_pos)
        return TRUE;
    else
        return FALSE;
}

/**
 * fonction de callback de changement de sélection
 *
 * \param selection
 * \param NULL
 *
 * \return
 **/
static void etats_prefs_onglet_data_grouping_selection_changed (GtkTreeSelection *selection,
																GtkWidget *tree_view)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gint pos;

        gtk_tree_model_get (model, &iter, 1, &pos, -1);
        switch (pos)
        {
            case 0:
                desensitive_widget (NULL,
                                GTK_WIDGET (gtk_builder_get_object (
                                etats_prefs_builder, "button_data_grouping_up")));
                sensitive_widget (NULL,
                                GTK_WIDGET (gtk_builder_get_object (
                                etats_prefs_builder, "button_data_grouping_down")));
                break;
            case 3:
                sensitive_widget (NULL,
                                GTK_WIDGET (gtk_builder_get_object (
                                etats_prefs_builder, "button_data_grouping_up")));
                desensitive_widget (NULL,
                                GTK_WIDGET (gtk_builder_get_object (
                                etats_prefs_builder, "button_data_grouping_down")));
                break;
            default:
                sensitive_widget (NULL,
                                GTK_WIDGET (gtk_builder_get_object (
                                etats_prefs_builder, "button_data_grouping_up")));
                sensitive_widget (NULL,
                                GTK_WIDGET (gtk_builder_get_object (
                                etats_prefs_builder, "button_data_grouping_down")));
                break;
        }
    }
}

/**
 * crée un nouveau tree_view initialisé avec model.
 * le modèle comporte 3 colonnes : G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT
 * le tree_view n'affiche que la colonne texte.
 *
 * \return the tree_wiew
 **/
static gboolean etats_prefs_onglet_data_grouping_init_tree_view (void)
{
	GtkWidget *window;
    GtkWidget *tree_view;
    GtkListStore *store;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeDragDestIface *dst_iface;
    GtkTreeDragSourceIface *src_iface;
    static GtkTargetEntry row_targets[] = {{(gchar*)"GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0}};

	window = GTK_WIDGET (grisbi_app_get_active_window (NULL));

    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "treeview_data_grouping"));
    if (!tree_view)
        return FALSE;

    /* colonnes du list_store :
     *  1 : chaine affichée
     *  2 : numéro de ligne dans le modèle
     *  3 : type de donnée : 1 Categ, 3 IB, 5 Account, 6 Payee.
     */
    store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT (store));

    /* set the color of selected row */
	gtk_widget_set_name (tree_view, "tree_view");

    /* set the column */
    cell = gtk_cell_renderer_text_new ();

    column = gtk_tree_view_column_new_with_attributes (NULL,
                        cell,
                        "text", 0,
                        NULL);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
                        GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_resizable (column, TRUE);

    /* Enable drag & drop */
    gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW (tree_view),
                        GDK_BUTTON1_MASK,
                        row_targets,
                        1,
                        GDK_ACTION_MOVE);
    gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (tree_view),
                        row_targets,
                        1,
                        GDK_ACTION_MOVE);
    gtk_tree_view_set_reorderable (GTK_TREE_VIEW (tree_view), TRUE);

    dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE (store);
    if (dst_iface)
    {
        dst_iface->drag_data_received = &etats_prefs_onglet_data_grouping_drag_data_received;
        dst_iface->row_drop_possible = &etats_prefs_onglet_data_grouping_drop_possible;
    }

    src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE (store);
    if (src_iface)
    {
        gtk_selection_add_target (window,
                      GDK_SELECTION_PRIMARY,
                      GDK_SELECTION_TYPE_ATOM,
                      1);
        src_iface->drag_data_get = &etats_prefs_onglet_data_grouping_drag_data_get;
    }

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    g_signal_connect (G_OBJECT (selection),
                        "changed",
                        G_CALLBACK (etats_prefs_onglet_data_grouping_selection_changed),
                        tree_view);

    /* return */
    return TRUE;
}

/**
 * callback when a button receive a clicked signal
 *
 * \param the button
 * \param a pointer for the direction of movement
 *
 * \return
 **/
static void etats_prefs_onglet_data_grouping_button_clicked (GtkWidget *button,
															 gpointer data)
{
    GtkWidget *tree_view;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter orig_iter;

    /* On récupère le model et le path d'origine */
    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "treeview_data_grouping"));
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (gtk_tree_selection_get_selected (selection, &model, &orig_iter))
    {
        GtkTreeIter dest_iter;
        GtkTreePath *path;
        gchar *string = NULL;
        gint orig_pos = 0;
        gint dest_pos;
        gint orig_type_data;
        gint dest_type_data;
        gint sens;

        sens = GPOINTER_TO_INT (data);

        path = gtk_tree_model_get_path (model, &orig_iter);

        /* On récupère les données des 2 lignes à modifier */
        gtk_tree_model_get (model, &orig_iter, 1, &orig_pos, 2, &orig_type_data, -1);

        if (sens == GTK_DIR_UP)
            gtk_tree_path_prev (path);
        else
            gtk_tree_path_next (path);

        if (gtk_tree_model_get_iter (model, &dest_iter, path))
            gtk_tree_model_get (model, &dest_iter, 1, &dest_pos, 2, &dest_type_data, -1);
        else
            return;
        /* on met à jour la ligne de destination */
        string = etats_config_onglet_data_grouping_get_string (orig_type_data, dest_pos);
        gtk_list_store_set (GTK_LIST_STORE (model), &dest_iter, 0, string, 2, orig_type_data, -1);

        g_free (string);

        /* on met à jour la ligne d'origine */
        string = etats_config_onglet_data_grouping_get_string (dest_type_data, orig_pos);
        gtk_list_store_set (GTK_LIST_STORE (model), &orig_iter, 0, string, 2, dest_type_data, -1);

        /* on garde la sélection sur le même iter */
        gtk_tree_selection_select_path (selection, path);

        g_free (string);
    }
}

/**
 * Création de l'onglet groupement des donnés
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_data_grouping_create_page (gint page)
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *button;

    vbox_onglet =  GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "page_data_grouping"));

    vbox = new_vbox_with_title_and_icon (_("Data grouping"), "gsb-organization-32.png");

    gtk_box_pack_start (GTK_BOX (vbox_onglet), vbox, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (vbox_onglet), vbox, 0);

    etats_prefs_onglet_data_grouping_init_tree_view ();

    /* on met la connection pour modifier l'ordre des données dans le tree_view data_grouping */
    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "button_data_grouping_up"));
    g_signal_connect (G_OBJECT (button),
                        "clicked",
                        G_CALLBACK (etats_prefs_onglet_data_grouping_button_clicked),
                        GINT_TO_POINTER (GTK_DIR_UP));

    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "button_data_grouping_down"));
    g_signal_connect (G_OBJECT (button),
                        "clicked",
                        G_CALLBACK (etats_prefs_onglet_data_grouping_button_clicked),
                        GINT_TO_POINTER (GTK_DIR_DOWN));

    gtk_widget_show_all (vbox_onglet);

    /* return */
    return vbox_onglet;
}

/*RIGHT_PANEL : ONGLET_DATA_SEPARATION*/
/**
 * Création de l'onglet séparation des donnés
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_data_separation_create_page (gint page)
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkComboBox *combo_1;
    GtkComboBox *combo_2;
    GtkTreeModel *model;

    vbox_onglet =  GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "page_data_separation"));

    vbox = new_vbox_with_title_and_icon (_("Data separation"), "gsb-organization-32.png");

    gtk_box_pack_start (GTK_BOX (vbox_onglet), vbox, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (vbox_onglet), vbox, 0);

    /* on met la connexion pour la séparation par exercice avec le bouton radio_button_utilise_exo */
    //~ g_signal_connect (G_OBJECT (gtk_builder_get_object (etats_prefs_builder, "radio_button_utilise_exo")),
                        //~ "toggled",
                        //~ G_CALLBACK (sens_desensitive_pointeur),
                        //~ gtk_builder_get_object (etats_prefs_builder, "bouton_separe_exo_etat"));

    /* on met la connexion pour rendre sensible la boite avec le bouton bouton_type_separe_plages_etat */
    g_signal_connect (G_OBJECT (gtk_builder_get_object (etats_prefs_builder, "bouton_separe_plages_etat")),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "paddingbox_data_separation2"));

    /* on crée le bouton avec les pérodes pour la séparation de l'état */
    model = GTK_TREE_MODEL (utils_list_store_create_from_string_array (data_separation_periodes));
    combo_1 = GTK_COMBO_BOX (gtk_builder_get_object (etats_prefs_builder, "bouton_type_separe_plages_etat"));
    gtk_combo_box_set_model (combo_1, model);
    utils_gtk_combo_box_set_text_renderer (GTK_COMBO_BOX (combo_1), 0);

    model = GTK_TREE_MODEL (utils_list_store_create_from_string_array (jours_semaine));
    combo_2 = GTK_COMBO_BOX (gtk_builder_get_object (etats_prefs_builder, "bouton_debut_semaine"));
    gtk_combo_box_set_model (combo_2, model);
    utils_gtk_combo_box_set_text_renderer (GTK_COMBO_BOX (combo_2), 0);

    /* on connecte le signal "changed" au bouton bouton_type_separe_plages_etat
     * pour rendre insensible le choix du jour de la semaine pour les choix
     * autres que la semaine. On le met ici pour que l'initialisation se fasse
     * proprement */
    g_signal_connect (G_OBJECT (combo_1),
                        "changed",
                        G_CALLBACK (etats_config_onglet_data_separation_combo_changed),
                        combo_2);

    gtk_widget_show_all (vbox_onglet);

    /* return */
    return vbox_onglet;
}

/*RIGHT_PANEL : ONGLET_AFFICHAGE_GENERALITES*/
/**
 * Création de l'onglet affichage des generalites
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_affichage_generalites_create_page (gint page)
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;

    vbox_onglet =  GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "affichage_etat_generalites"));

    vbox = new_vbox_with_title_and_icon (_("Generalities"), "gsb-generalities-32.png");

    gtk_box_pack_start (GTK_BOX (vbox_onglet), vbox, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (vbox_onglet), vbox, 0);

    gtk_widget_show_all (vbox_onglet);

    /* return */
    return vbox_onglet;
}

/*RIGHT_PANEL : ONGLET_AFFICHAGE_TITLES*/
/**
 * Création de l'onglet affichage de diverses données
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_affichage_titles_create_page (gint page)
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *button;

    vbox_onglet =  GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "affichage_etat_titles"));

    vbox = new_vbox_with_title_and_icon (_("Titles"), "gsb-title-32.png");

    gtk_box_pack_start (GTK_BOX (vbox_onglet), vbox, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (vbox_onglet), vbox, 0);

    /* on met les connexions pour sensibiliser désensibiliser les données associées */
    g_signal_connect (G_OBJECT (gtk_builder_get_object (etats_prefs_builder, "bouton_regroupe_ope_compte_etat")),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_affiche_sous_total_compte"));

    /* affichage possible des tiers */
    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "bouton_utilise_tiers_etat"));
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_afficher_noms_tiers"));

    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_affiche_sous_total_tiers"));

    /* affichage possible des categories */
    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "bouton_group_by_categ"));
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_afficher_noms_categ"));

    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_affiche_sous_total_categ"));

    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_afficher_sous_categ"));

    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_affiche_sous_total_sous_categ"));

    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_afficher_pas_de_sous_categ"));

    /* affichage possible des ib */
    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "bouton_utilise_ib_etat"));
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_afficher_noms_ib"));

    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_affiche_sous_total_ib"));

    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_afficher_sous_ib"));

    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_affiche_sous_total_sous_ib"));

    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "bouton_afficher_pas_de_sous_ib"));

    gtk_widget_show_all (vbox_onglet);

    /* return */
    return vbox_onglet;
}

/*RIGHT_PANEL : ONGLET_AFFICHAGE_OPERATIONS*/
/**
 * Création de l'onglet affichage de opérations
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_affichage_operations_create_page (gint page)
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *button;
    GtkComboBox *combo;
    GtkTreeModel *model;

    vbox_onglet =  GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "affichage_etat_operations"));

    vbox = new_vbox_with_title_and_icon (_("Transactions display"), "gsb-transdisplay-32.png");

    gtk_box_pack_start (GTK_BOX (vbox_onglet), vbox, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (vbox_onglet), vbox, 0);

    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "bouton_afficher_opes"));
    /* on met la connection pour changer le style de la ligne du panneau de gauche */
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
                        GINT_TO_POINTER (page));

    /* on met la connection pour rendre sensitif la vbox_show_transactions */
    g_signal_connect (G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (etats_prefs_builder, "vbox_show_transactions"));

    /* on crée le bouton avec les types de classement des opérations */
    model = GTK_TREE_MODEL (utils_list_store_create_from_string_array (etats_prefs_classement_operations));
    combo = GTK_COMBO_BOX (gtk_builder_get_object (etats_prefs_builder, "bouton_choix_classement_ope_etat"));
    gtk_combo_box_set_model (combo, model);
    utils_gtk_combo_box_set_text_renderer (GTK_COMBO_BOX (combo), 0);

    /* on met les connexions */
    g_signal_connect (G_OBJECT (gtk_builder_get_object (
                        etats_prefs_builder, "bouton_afficher_categ_opes")),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (
                        etats_prefs_builder, "bouton_afficher_sous_categ_opes"));

    g_signal_connect (G_OBJECT (gtk_builder_get_object (
                        etats_prefs_builder, "bouton_afficher_ib_opes")),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (
                        etats_prefs_builder, "bouton_afficher_sous_ib_opes"));

    g_signal_connect (G_OBJECT (gtk_builder_get_object (
                        etats_prefs_builder, "bouton_afficher_titres_colonnes")),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (
                        etats_prefs_builder, "bouton_titre_changement"));

    g_signal_connect (G_OBJECT (gtk_builder_get_object (
                        etats_prefs_builder, "bouton_afficher_titres_colonnes")),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        gtk_builder_get_object (
                        etats_prefs_builder, "bouton_titre_en_haut"));

    gtk_widget_show_all (vbox_onglet);

    return vbox_onglet;
}

/*RIGHT_PANEL : ONGLET_AFFICHAGE_DEVISES*/
/**
 * Création de l'onglet affichage des devises
 *
 * \param
 *
 * \return
 **/
static GtkWidget *etats_prefs_onglet_affichage_devises_create_page (gint page)
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;

    vbox_onglet =  GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, "affichage_etat_devises"));

    vbox = new_vbox_with_title_and_icon (_("Totals currencies"), "gsb-currencies-32.png");

    gtk_box_pack_start (GTK_BOX (vbox_onglet), vbox, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (vbox_onglet), vbox, 0);

    gtk_widget_show_all (vbox_onglet);

    etats_config_onglet_affichage_devises_make_combobox ();

    gtk_widget_show_all (vbox_onglet);

    return vbox_onglet;
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
/**
 * Initialise EtatsPrefs
 *
 * \param prefs
 *
 * \return
 **/
static void etats_prefs_init (EtatsPrefs *prefs)
{
	GrisbiWinEtat *w_etat;
	EtatsPrefsPrivate *priv;

	if (!etats_prefs_initialise_builder (prefs))
		exit (1);

	priv = etats_prefs_get_instance_private (prefs);
	gtk_dialog_add_buttons (GTK_DIALOG (prefs),
							"gtk-cancel",
							GTK_RESPONSE_CANCEL,
							"gtk-ok",
							GTK_RESPONSE_OK,
							NULL);

    gtk_window_set_title (GTK_WINDOW (prefs), _("Report properties"));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (prefs), TRUE);

    gtk_container_set_border_width (GTK_CONTAINER (prefs), MARGIN_BOX);
    gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (prefs))), 2);

	/* memorise la variable w_etat->form_date_force_prev_year */
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	if (w_etat->form_date_force_prev_year)
	{
		priv->form_date_force_prev_year = w_etat->form_date_force_prev_year;
		w_etat->form_date_force_prev_year = FALSE;
	}

    /* Recupération d'un pointeur sur le gtk_tree_view. */
    etats_prefs_left_panel_create_tree_view (prefs);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (prefs))),
                        priv->hpaned, TRUE, TRUE, 0);

    gtk_widget_show_all (priv->hpaned);
}

/**
 *  called when destroy EtatsPrefs
 *
 * \param object
 *
 * \return
 **/
static void etats_prefs_dispose (GObject *object)
{
	EtatsPrefsPrivate *priv;

	devel_debug (NULL);

	/* restauration de la variable w_etat->form_date_force_prev_year */
	priv = etats_prefs_get_instance_private (ETATS_PREFS (object));
	if (priv->form_date_force_prev_year)
	{
		GrisbiWinEtat *w_etat;

		w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
		w_etat->form_date_force_prev_year = TRUE;
	}

    /* libération de l'objet prefs */
    G_OBJECT_CLASS (etats_prefs_parent_class)->dispose (object);
}


/**
 * finalise EtatsPrefs
 *
 * \param object
 *
 * \return
 **/
static void etats_prefs_finalize (GObject *object)
{
    devel_debug (NULL);
/*    etats_prefs_dialog = NULL; */
    /* libération de l'objet prefs */
    G_OBJECT_CLASS (etats_prefs_parent_class)->finalize (object);
}


/**
 * Initialise EtatsPrefsClass
 *
 * \param
 *
 * \return
 **/
static void etats_prefs_class_init (EtatsPrefsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = etats_prefs_dispose;
    object_class->finalize = etats_prefs_finalize;
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
GtkWidget *etats_prefs_new (GtkWidget *parent)
{
    GtkWidget *dialog = NULL;

    dialog = g_object_new (ETATS_PREFS_TYPE, NULL);
    g_signal_connect (dialog, "destroy", G_CALLBACK (gtk_widget_destroy), NULL);

    if (GTK_WINDOW (parent) != gtk_window_get_transient_for (GTK_WINDOW (dialog)))
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));

    return dialog;
}

/*FONCTIONS UTILITAIRES COMMUNES*/
/**
 * retourne l'index du radiobutton actif.
 *
 * \param nom du radio_button
 *
 * \return index bouton actif
 **/
gint etats_prefs_buttons_radio_get_active_index (const gchar *button_name)
{
    GtkWidget *radiobutton;
    gint index = 0;

    radiobutton = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, button_name));

    index = utils_radiobutton_get_active_index (radiobutton);

    return index;
}

/**
 * rend actif le button qui correspond à l'index passé en paramètre.
 *
 * \param nom du radio_button
 * \param index du bouton à rendre actif
 *
 * \return index bouton actif
 **/
void etats_prefs_buttons_radio_set_active_index (const gchar *button_name,
												 gint index)
{
    GtkWidget *radiobutton;

    radiobutton = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, button_name));

    utils_radiobutton_set_active_index (radiobutton, index);
}

/**
 * retourne l'état du togglebutton dont le nom est passé en paramètre.
 *
 * \param widget name
 *
 * \return activ
 **/
gboolean etats_prefs_button_toggle_get_actif (const gchar *button_name)
{
    GtkWidget *button = NULL;

    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, button_name));
    if (!button)
        return FALSE;

    return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
}

/**
 *  Positionne letogglebutton demandé par son nom en fonction de actif
 *
 * \param widget name
 * \param actif
 *
 * \return TRUE if success FALSE otherwise
 **/
gboolean etats_prefs_button_toggle_set_actif (const gchar *button_name,
											  gboolean actif)
{
    GtkWidget *button = NULL;

    button = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, button_name));
    if (!button)
        return FALSE;

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), actif);

    return TRUE;
}

/**
 * free the gtk_builder
 *
 * \param
 *
 * \return
 **/
void etats_prefs_free_all_var (void)
{
    g_object_unref (G_OBJECT (etats_prefs_builder));
}

/**
 * récupère l'index l'iter selectionné
 *
 * \param nom du tree_view
 *
 * \return numéro de la ligne sélectionnée
 **/
gint etats_prefs_tree_view_get_single_row_selected (const gchar *treeview_name)
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;

    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, treeview_name));
    if (!tree_view)
        return -1;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gint index;

        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &index, -1);

        return index;
    }
    return -1;
}

/**
 * récupère l'index l'iter selectionné
 *
 * \param nom du tree_view
 *
 * \return numéro de la ligne sélectionnée
 **/
gint new_etats_prefs_tree_view_get_single_row_selected (GtkWidget *tree_view)
{
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gint index;

        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &index, -1);

        return index;
    }

	return -1;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void etats_prefs_tree_view_select_single_row (const gchar *treeview_name,
											  gint numero)
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;

    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, treeview_name));
    if (!tree_view)
        return;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
        do
        {
            gint index;

            gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &index, -1);

            if (numero == index)
            {
                gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);
                break;
            }
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void new_etats_prefs_tree_view_select_single_row (GtkWidget *tree_view,
												  gint numero)
{
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
        do
        {
            gint index;

            gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &index, -1);

            if (numero == index)
            {
                gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);
                break;
            }
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
GtkWidget *etats_prefs_widget_get_widget_by_name (const gchar *parent_name,
												  const gchar *child_name)
{
    return utils_gtkbuilder_get_widget_by_name (etats_prefs_builder, parent_name, child_name);
}

/**
 * récupère l'index l'iter selectionné
 *
 * \param nom du tree_view
 *
 * \return numéro de la ligne sélectionnée
 **/
GSList *etats_prefs_tree_view_get_list_rows_selected (const gchar *treeview_name)
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GSList *tmp_list = NULL;
    GList *rows_list;

    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, treeview_name));
    if (!tree_view)
        return NULL;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    rows_list = gtk_tree_selection_get_selected_rows (selection, &model);
    while (rows_list)
    {
        GtkTreePath *path;
        gint index;

        path = rows_list->data;

        gtk_tree_model_get_iter (model, &iter, path) ;
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &index, -1);

        tmp_list = g_slist_append (tmp_list, GINT_TO_POINTER (index));

        gtk_tree_path_free (path);
        rows_list = rows_list->next;
    }
    g_list_free (rows_list);

    return tmp_list;
}

GSList *new_etats_prefs_tree_view_get_list_rows_selected (GtkWidget *tree_view)
{
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GSList *tmp_list = NULL;
    GList *rows_list;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    rows_list = gtk_tree_selection_get_selected_rows (selection, &model);
    while (rows_list)
    {
        GtkTreePath *path;
        gint index;

        path = rows_list->data;

        gtk_tree_model_get_iter (model, &iter, path) ;
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &index, -1);
        tmp_list = g_slist_append (tmp_list, GINT_TO_POINTER (index));

        rows_list = rows_list->next;
    }
    g_list_free_full (rows_list, (GDestroyNotify) gtk_tree_path_free);

    return tmp_list;
}

/**
 *  rend sensible le widget demandé par son nom en fonction de sensible
 *
 *\param widget name
 *\param sensitive
 *
 * \return TRUE if success FALSE otherwise
 **/
gboolean etats_prefs_widget_set_sensitive (const gchar *widget_name,
										   gboolean sensitive)
{
    GtkWidget *widget = NULL;

    widget = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, widget_name));
    if (!widget)
        return FALSE;

    gtk_widget_set_sensitive (widget, sensitive);

    return TRUE;
}

/*RIGHT_PANEL : ONGLET_MODE_PAIEMENT*/
/**
 * récupère la liste des libellés des item sélectionnés
 *
 * \param nom du tree_view
 *
 * \return numéro de la ligne sélectionnée
 **/
GSList *etats_prefs_onglet_mode_paiement_get_list_rows_selected (const gchar *treeview_name)
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GSList *tmp_list = NULL;
    GList *rows_list;

    tree_view = GTK_WIDGET (gtk_builder_get_object (etats_prefs_builder, treeview_name));
    if (!tree_view)
        return NULL;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    rows_list = gtk_tree_selection_get_selected_rows (selection, &model);
    while (rows_list)
    {
        GtkTreePath *path;
        gchar *tmp_str;

        path = rows_list->data;

        gtk_tree_model_get_iter (model, &iter, path) ;
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &tmp_str, -1);

        tmp_list = g_slist_append (tmp_list, tmp_str);

        gtk_tree_path_free (path);
        rows_list = rows_list->next;
    }
    g_list_free (rows_list);

    return tmp_list;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *etats_prefs_get_page_by_number (GtkWidget *etats_prefs,
										   gint num_page)
{
	GtkWidget *page = NULL;
	EtatsPrefsPrivate *priv;

	priv = etats_prefs_get_instance_private (ETATS_PREFS (etats_prefs));
	page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook_etats_prefs), num_page);

	return page;
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
