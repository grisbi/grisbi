/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                              */
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_metatree.h"
#include "categories_onglet.h"
#include "gsb_currency.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_automem.h"
#include "gsb_data_payee.h"
#include "gsb_file.h"
#include "imputation_budgetaire.h"
#include "navigation.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "utils_prefs.h"
#include "utils_widgets.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageMetatreePrivate   PrefsPageMetatreePrivate;

struct _PrefsPageMetatreePrivate
{
	GtkWidget *			vbox_metatree;

    GtkWidget *         box_metatree_action_2button;
	GtkWidget *			box_metatree_sort_transactions;
	GtkWidget *			box_metatree_unarchived_payees;
	GtkWidget *			checkbutton_metatree_assoc_mode;
    GtkWidget *			checkbutton_metatree_totals;
	GtkWidget *			checkbutton_metatree_unarchived_payees;
	GtkWidget *			combo_devise_totaux_categ;
	GtkWidget *			combo_devise_totaux_ib;
	GtkWidget *			combo_devise_totaux_tiers;
	GtkWidget *			grid_metatree_currencies;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageMetatree, prefs_page_metatree, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 *
 * \param
 * \param
 * \param
 *
 * \return FALSE
 **/
static gboolean prefs_page_onglet_metatree_action_changed (GtkWidget *checkbutton,
														   GdkEventButton *event,
														   gint *pointeur)
{
    if (pointeur)
    {
        gint value = 0;

        value = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (checkbutton), "pointer"));
        *pointeur = value;
        utils_prefs_gsb_file_set_modified ();
    }

    return FALSE;
}

/**
 * Cette fonction retrie la division affichée après un changement
 *
 * \param
 * \param
 * \param
 *
 * \return TRUE if OK else FALSE
 * */
static gboolean prefs_page_metatree_re_sort_divisions (GtkWidget *checkbutton,
													   GdkEventButton *event,
													   gpointer data)
{
    GtkWidget *division_tree;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path = NULL;
    gint page_number;

    page_number = gsb_gui_navigation_get_current_page ();
    switch (page_number)
    {
        case GSB_PAYEES_PAGE:
            division_tree = payees_get_tree_view ();
            selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (division_tree));
            if (gtk_tree_selection_get_selected (selection, &model, &iter))
                path = gtk_tree_model_get_path (model, &iter);
            payees_fill_list ();
            break;

        case GSB_CATEGORIES_PAGE:
            division_tree = categories_get_tree_view ();
            selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (division_tree));
            if (gtk_tree_selection_get_selected (selection, &model, &iter))
                path = gtk_tree_model_get_path (model, &iter);
            categories_fill_list ();
            break;

        case GSB_BUDGETARY_LINES_PAGE:
            division_tree = budgetary_lines_get_tree_view ();
            selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (division_tree));
            if (gtk_tree_selection_get_selected (selection, &model, &iter))
                path = gtk_tree_model_get_path (model, &iter);
            budgetary_lines_fill_list ();
            break;

        default:
            return FALSE;
            break;
    }

    if (path)
    {
        gtk_tree_path_up (path);
        gtk_tree_view_expand_to_path (GTK_TREE_VIEW (division_tree), path);
        gtk_tree_path_free (path);
    }

    gsb_file_set_modified (TRUE);

    /* return */
    return TRUE;
}

/**
 * Cette fonction est appellée suite au changement de l'ordre de tri des opérations
 * dans les divisions.
 *
 * \param
 * \param
 * \param
 *
 * \return FALSE
 **/
static gboolean prefs_page_metatree_sort_transactions_changed (GtkWidget *checkbutton,
															   GdkEventButton *event,
															   gint *pointeur)
{
    if (pointeur)
    {
        gint value = 0;

        value = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (checkbutton), "pointer"));
        *pointeur = value;
        utils_prefs_gsb_file_set_modified ();
    }

    prefs_page_metatree_re_sort_divisions (checkbutton, event, pointeur);

    return FALSE;
}

/**
 * Cette fonction modifie la liste des tiers utilisés
 *
 * \param
 * \param
 *
 * \return FALSE
 **/
static void prefs_page_metatree_unarchived_payees_toggled (GtkWidget *checkbutton,
												   gpointer null)
{
	gboolean toggle;

	toggle = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));
	if (toggle)
	{
		GSList *payees_list;
		GSList *tmp_list;
		gchar *hint;
		gchar *text;
		gint nbre_payees;
		gint nbre_unarchived;
		gint test;
		gint reponse_id;

		payees_list = gsb_data_payee_get_payees_list ();
		nbre_payees = g_slist_length (payees_list);
		tmp_list = gsb_data_payee_get_unarchived_payees_list ();
		nbre_unarchived = g_slist_length (tmp_list);
		nbre_payees = nbre_payees - nbre_unarchived;

		if (nbre_payees == 0)
		{
			text = g_strdup (_("There are no archived transactions."));
			hint = g_strdup (_("This feature is useless"));
			dialogue_warning_hint (text, hint);
			g_free (text);
			g_free (hint);
			g_signal_handlers_block_by_func (checkbutton, prefs_page_metatree_unarchived_payees_toggled, NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), FALSE);
			g_signal_handlers_unblock_by_func (checkbutton, prefs_page_metatree_unarchived_payees_toggled, NULL);

			return;
		}

		/* on compare le nombre de tiers des opérations archivées avec le nombre */
		/* de tiers des opérations non archivées. si >= 2 on conseille de le faire */
		test = nbre_payees / nbre_unarchived;
		if (test >= 2)
		{
			text = g_strdup_printf (_("The ratio of the number of payees of the archived transactions "
									  "(%d) to the number of payees of non-archived transactions (%d) "
									  "is greater than or equal to two.\n"
									  "It is advisable to use this feature: answer YES"),
									nbre_payees,
									nbre_unarchived);
			hint = g_strdup (_("This feature can be useful"));
			reponse_id = GTK_RESPONSE_YES;
		}
		else
		{
			text = g_strdup_printf (_("The ratio of the number of payees of the archived transactions "
									  "(%d) to the number of payees of non-archived transactions (%d) "
									  "is less than two.\n"
									  "It is not recommended to use this feature: answer NO"),
									nbre_payees,
									nbre_unarchived);
			hint = g_strdup (_("This feature is useless"));
			reponse_id = GTK_RESPONSE_NO;
		}

		if (dialogue_yes_no (text, hint, reponse_id))
		{
			payees_update_combofix (FALSE);
		}
		else
		{
			g_signal_handlers_block_by_func (checkbutton, prefs_page_metatree_unarchived_payees_toggled, NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), FALSE);
			g_signal_handlers_unblock_by_func (checkbutton, prefs_page_metatree_unarchived_payees_toggled, NULL);
		}
	}
	else
	{
		payees_update_combofix (FALSE);
	}
}

/**
 * Création de la page de gestion des metatree
 *
 * \param prefs
 *
 * \return
 */
static void prefs_page_metatree_setup_metatree_page (PrefsPageMetatree *page)
{
	GtkWidget *head_page;
	GtkWidget *vbox_button;
	gboolean is_loading;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;
	PrefsPageMetatreePrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_metatree_get_instance_private (page);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	w_etat = grisbi_win_get_w_etat ();
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Payees, categories and budgetaries"),
															   "gsb-organization-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_metatree), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_metatree), head_page, 0);

	/* tri des opérations */
	vbox_button = gsb_automem_radiobutton3_new (_("by number"),
												_("by increasing date"),
												_("by date descending"),
                        						&w_etat->metatree_sort_transactions,
												G_CALLBACK (prefs_page_metatree_sort_transactions_changed),
                        						&w_etat->metatree_sort_transactions,
												GTK_ORIENTATION_HORIZONTAL);
	/* set other widgets sensitives */
	if (is_loading)
	{
		/* set the variables for totals currencies */
		priv->combo_devise_totaux_tiers = gsb_currency_combobox_new (&w_etat->no_devise_totaux_tiers,
																	   payees_fill_list);
		priv->combo_devise_totaux_categ = gsb_currency_combobox_new (&w_etat->no_devise_totaux_categ,
																	   categories_fill_list);
		priv->combo_devise_totaux_ib = gsb_currency_combobox_new (&w_etat->no_devise_totaux_ib,
																	budgetary_lines_fill_list);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_metatree_totals),
									  w_etat->metatree_add_archive_in_totals);
		/* set option for french associations */
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_metatree_assoc_mode),
									  w_etat->metatree_assoc_mode);

		/* set for metatree_unarchived_payees */
		if (w_etat->combofix_force_payee)
		{
			gtk_widget_set_sensitive (priv->checkbutton_metatree_unarchived_payees, FALSE);
			w_etat->metatree_unarchived_payees = FALSE;
		}
		else
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_metatree_unarchived_payees),
										  w_etat->metatree_unarchived_payees);

	}
	else
	{
		priv->combo_devise_totaux_tiers = utils_prefs_create_combo_list_indisponible ();
		priv->combo_devise_totaux_categ = utils_prefs_create_combo_list_indisponible ();
		priv->combo_devise_totaux_ib = utils_prefs_create_combo_list_indisponible ();

		gtk_widget_set_sensitive (priv->grid_metatree_currencies, FALSE);
		gtk_widget_set_sensitive (priv->checkbutton_metatree_totals, FALSE);
		gtk_widget_set_sensitive (priv->checkbutton_metatree_assoc_mode, FALSE);
		gtk_widget_set_sensitive (priv->box_metatree_sort_transactions, FALSE);
		gtk_widget_set_sensitive (priv->box_metatree_unarchived_payees, FALSE);

	}
    gtk_grid_attach (GTK_GRID (priv->grid_metatree_currencies), priv->combo_devise_totaux_tiers, 1, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (priv->grid_metatree_currencies), priv->combo_devise_totaux_categ, 1, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (priv->grid_metatree_currencies), priv->combo_devise_totaux_ib, 1, 2, 1, 1);
	gtk_box_pack_start (GTK_BOX (priv->box_metatree_sort_transactions), vbox_button, FALSE, FALSE, 0);

	/* gestion des divisions*/
	vbox_button = gsb_automem_radiobutton3_gsettings_new (_("Expand the line"),
														  _("Edit the line"),
														  _("Manage the line"),
														  &a_conf->metatree_action_2button_press,
													 	  G_CALLBACK (prefs_page_onglet_metatree_action_changed),
													 	  &a_conf->metatree_action_2button_press,
													 	  GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start (GTK_BOX (priv->box_metatree_action_2button), vbox_button, FALSE, FALSE, 0);

	/* Connect signal */
    /* callback for checkbutton_metatree_assoc_mode */
    g_signal_connect (priv->checkbutton_metatree_assoc_mode,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->metatree_assoc_mode);

	/* callback for checkbutton_metatree_totals */
    g_signal_connect (priv->checkbutton_metatree_totals,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->metatree_add_archive_in_totals);

	g_signal_connect_after (priv->checkbutton_metatree_totals,
							"toggled",
							G_CALLBACK (prefs_page_metatree_re_sort_divisions),
							NULL);
    /* callback for checkbutton_metatree_unarchived_payees */
    g_signal_connect (priv->checkbutton_metatree_unarchived_payees,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->metatree_unarchived_payees);

	g_signal_connect_after (priv->checkbutton_metatree_unarchived_payees,
							"toggled",
							G_CALLBACK (prefs_page_metatree_unarchived_payees_toggled),
							NULL);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_metatree_init (PrefsPageMetatree *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_metatree_setup_metatree_page (page);
}

static void prefs_page_metatree_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_metatree_parent_class)->dispose (object);
}

static void prefs_page_metatree_class_init (PrefsPageMetatreeClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_metatree_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_metatree.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMetatree, vbox_metatree);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMetatree, grid_metatree_currencies);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMetatree, checkbutton_metatree_assoc_mode);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMetatree, checkbutton_metatree_totals);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMetatree, checkbutton_metatree_unarchived_payees);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMetatree, box_metatree_unarchived_payees);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMetatree, box_metatree_sort_transactions);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMetatree, box_metatree_action_2button);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
PrefsPageMetatree *prefs_page_metatree_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_METATREE_TYPE, NULL);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void prefs_page_metatree_sensitive_widget (const gchar *widget_name,
										   gboolean sensitive)
{
	GtkWidget *page;

	devel_debug (widget_name);
	page = grisbi_prefs_get_child_by_page_name ("metatree_num_page");
	if (page && PREFS_IS_PAGE_METATREE (page))
	{
		PrefsPageMetatreePrivate *priv;

		priv = prefs_page_metatree_get_instance_private (PREFS_PAGE_METATREE (page));
		if (strcmp (widget_name, "metatree_unarchived_payees") == 0)
		{
			GrisbiWinEtat *w_etat;

			w_etat = grisbi_win_get_w_etat ();
			if (sensitive && w_etat->metatree_unarchived_payees)
			{
				g_signal_handlers_block_by_func (priv->checkbutton_metatree_unarchived_payees,
												 prefs_page_metatree_unarchived_payees_toggled,
												 NULL);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_metatree_unarchived_payees),
											  FALSE);
				g_signal_handlers_unblock_by_func (priv->checkbutton_metatree_unarchived_payees,
												   prefs_page_metatree_unarchived_payees_toggled,
												   NULL);
			}

			gtk_widget_set_sensitive (priv->checkbutton_metatree_unarchived_payees, !sensitive);
		}
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *prefs_page_metatree_get_currency_combobox (const gchar *widget_name)
{
	GtkWidget *page;

	//~ devel_debug (widget_name);
	page = grisbi_prefs_get_child_by_page_name ("metatree_num_page");
	if (page && PREFS_IS_PAGE_METATREE (page))
	{
		PrefsPageMetatreePrivate *priv;

		priv = prefs_page_metatree_get_instance_private (PREFS_PAGE_METATREE (page));

		if (strcmp (widget_name, "combo_totaux_categ") == 0)
		{
			return priv->combo_devise_totaux_categ;
		}
		else if (strcmp (widget_name, "combo_totaux_ib") == 0)
		{
			return priv->combo_devise_totaux_ib;
		}
		if (strcmp (widget_name, "combo_totaux_tiers") == 0)
		{
			return priv->combo_devise_totaux_tiers;
		}
	}

	return NULL;
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

