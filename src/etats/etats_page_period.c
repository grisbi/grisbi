/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          http://www.grisbi.org                                                */
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
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "etats_page_period.h"
#include "dialog.h"
#include "etats_prefs.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_fyear.h"
#include "gsb_data_report.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_dates.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _EtatsPagePeriodPrivate   EtatsPagePeriodPrivate;

struct _EtatsPagePeriodPrivate
{
	GtkWidget *			vbox_etats_page_period;

	/* dates part */
	GtkWidget *         radio_button_utilise_dates;

	GtkWidget *         entree_date_finale_etat;
	GtkWidget *         entree_date_init_etat;
	GtkWidget *         hbox_date_finale;
	GtkWidget *			hbox_date_init;
	GtkWidget *			treeview_dates;
	GtkWidget *         vbox_utilisation_date;

	/* exo part */
	GtkWidget *         radio_button_utilise_exo;

	GtkWidget *         bouton_exo_courant;
	GtkWidget *         bouton_exo_detaille;
	GtkWidget *         bouton_exo_precedent;
	GtkWidget *         bouton_exo_tous;
	GtkWidget *         sw_exo;
	GtkWidget *         treeview_exo;
	GtkWidget *         vbox_utilisation_exo;

	/* parent */
	GtkWidget *			etats_prefs;
};

G_DEFINE_TYPE_WITH_PRIVATE (EtatsPagePeriod, etats_page_period, GTK_TYPE_BOX)

/* liste des plages de date possibles */
static const gchar *etat_period_liste_plages_dates[] =
{
    N_("All"),
    N_("Custom"),
    N_("Total to now"),
    N_("Current month"),
    N_("Current year"),
    N_("Current month to now"),
    N_("Current year to now"),
    N_("Previous month"),
    N_("Previous year"),
    N_("Last 30 days"),
    N_("Last 3 months"),
    N_("Last 6 months"),
    N_("Last 12 months"),
    NULL,
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkTreeModel *etats_page_period_get_model_dates (void)
{
    GtkListStore *list_store;

    list_store = utils_list_store_create_from_string_array (etat_period_liste_plages_dates);

    return GTK_TREE_MODEL (list_store);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkTreeModel *etats_page_period_get_model_exercices (void)
{
    GtkListStore *list_store;
    GSList *list_tmp;

    list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store), 0, GTK_SORT_DESCENDING);

    /* on remplit la liste des exercices */
    list_tmp = gsb_data_fyear_get_fyears_list ();
    while (list_tmp)
    {
        GtkTreeIter iter;
        gchar *name;
        gint fyear_number;

        fyear_number = gsb_data_fyear_get_no_fyear (list_tmp->data);
        name = my_strdup (gsb_data_fyear_get_name (fyear_number));

        gtk_list_store_append (list_store, &iter);
        gtk_list_store_set (list_store, &iter, 0, name, 1, fyear_number, -1);

        if (name)
            g_free (name);

        list_tmp = list_tmp->next;
    }

    return GTK_TREE_MODEL (list_store);
}

/**
 * rend accessible ou nom l'ensemble des données de date configurables
 *
 * \param page
 * \param TRUE rend sensible FALSE rend insensible les données
 *
 * \return
 **/
static void etats_page_period_date_interval_sensitive (EtatsPagePeriod *page,
													   gboolean show)
{
	EtatsPagePeriodPrivate *priv;

	priv = etats_page_period_get_instance_private (page);
	if (show > 1)
		show = 0;

	gtk_widget_set_sensitive (priv->entree_date_init_etat, show);
	gtk_widget_set_sensitive (priv->entree_date_finale_etat, show);
}

/**
 * If applicable, update report navigation tree style to reflect which
 * pages have been changed.
 *
 * \param page Page that contained an interface element just
 *                      changed that triggered this event.
 * \param
 *
 * \return
 **/
static void etats_page_period_update_style_left_panel (GtkWidget *button,
													   EtatsPagePeriod *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
	GtkWidget *tree_view;
    gint index;
	EtatsPagePeriodPrivate *priv;

	priv = etats_page_period_get_instance_private (page);
    index = utils_radiobutton_get_active_index (priv->radio_button_utilise_exo);
    if (index == 0)
    {
        if (etats_prefs_tree_view_get_single_row_selected (priv->treeview_dates) == 1)
        index = 1;
    }

	tree_view = etats_prefs_get_widget_by_name ("treeview_left_panel", priv->etats_prefs);
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, "0:0"))
    {
        gtk_tree_store_set (GTK_TREE_STORE (model),
							&iter,
							LEFT_PANEL_TREE_ITALIC_COLUMN, index,
							-1);
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
static void etats_page_period_selection_dates_changed (GtkTreeSelection *selection,
													   EtatsPagePeriod *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint selected;

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        return;

    gtk_tree_model_get (model, &iter, 1, &selected, -1);
    etats_page_period_date_interval_sensitive (page, selected);

    /* on regarde si on utilise des dates personalisées */
    etats_page_period_update_style_left_panel (NULL, page);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void etats_page_period_setup_treeview_treeview_exo (EtatsPagePeriod *page)
{
    GtkCellRenderer *cell;
	GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *column;
	EtatsPagePeriodPrivate *priv;

	priv = etats_page_period_get_instance_private (page);

	/* set model */
	model = etats_page_period_get_model_exercices ();
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_exo), model);
    g_object_unref (G_OBJECT (model));

	/* set the color of selected row */
	gtk_widget_set_name (priv->treeview_exo, "tree_view");

    /* set the column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (NULL, cell, "text", 0, NULL);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_exo), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_resizable (column, TRUE);

	/* set selection */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_exo));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void etats_page_period_setup_treeview_dates (EtatsPagePeriod *page)
{
    GtkCellRenderer *cell;
	GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *column;
	EtatsPagePeriodPrivate *priv;

	priv = etats_page_period_get_instance_private (page);

	/* set model */
	model = etats_page_period_get_model_dates ();
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_dates), model);
    g_object_unref (G_OBJECT (model));

    /* set the color of selected row */
	gtk_widget_set_name (priv->treeview_dates, "tree_view");

    /* set the column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (NULL, cell, "text", 0, NULL);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_dates), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_resizable (column, TRUE);

	/* set selection */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_dates));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (selection),
					  "changed",
					  G_CALLBACK (etats_page_period_selection_dates_changed),
					  page);
}

/**
 * Création de la page de gestion des dates
 *
 * \param
 *
 * \return
 **/
static void etats_page_period_setup_page (EtatsPagePeriod *page,
										  GtkWidget *etats_prefs)
{
	GtkWidget *head_page;
	EtatsPagePeriodPrivate *priv;

	devel_debug (NULL);

	priv = etats_page_period_get_instance_private (page);

	/* on sauvegarde etats_prefs pour plus tard */
	priv->etats_prefs = etats_prefs;

	/* set head page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Date selection"), "gsb-scheduler-32.png");
    gtk_box_pack_start (GTK_BOX (priv->vbox_etats_page_period), head_page, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (priv->vbox_etats_page_period), head_page, 0);

    /* on traite la partie gauche de l'onglet dates */
	etats_page_period_setup_treeview_dates (page);

	/* on ajoute les entrées pour saisir les dates personnalisées */
	priv->entree_date_init_etat = gsb_calendar_entry_new (FALSE);
    gtk_widget_set_size_request (priv->entree_date_init_etat, ENTRY_MIN_WIDTH, -1);
    gtk_box_pack_end (GTK_BOX (priv->hbox_date_init), priv->entree_date_init_etat, FALSE, FALSE, 0);

	priv->entree_date_finale_etat = gsb_calendar_entry_new (FALSE);
    gtk_widget_set_size_request (priv->entree_date_finale_etat, ENTRY_MIN_WIDTH, -1);
    gtk_box_pack_end (GTK_BOX (priv->hbox_date_finale), priv->entree_date_finale_etat, FALSE, FALSE, 0);

	/* on traite la partie droite de l'onglet dates */
	etats_page_period_setup_treeview_treeview_exo (page);

	/* on met la connection pour changer le style de la ligne du panneau de gauche */
    g_signal_connect (G_OBJECT (priv->radio_button_utilise_dates),
                      "toggled",
                      G_CALLBACK (etats_page_period_update_style_left_panel),
                      page);

    /* on met la connection pour rendre sensitif la frame vbox_utilisation_date */
    g_signal_connect (G_OBJECT (priv->radio_button_utilise_dates),
                      "toggled",
                      G_CALLBACK (sens_desensitive_pointeur),
                      priv->vbox_utilisation_date);

	/* on met la connection pour changer le style de la ligne du panneau de droite */
    g_signal_connect (G_OBJECT (priv->radio_button_utilise_exo),
                      "toggled",
                      G_CALLBACK (etats_page_period_update_style_left_panel),
                      page);

    /* on met la connection pour rendre sensitif la frame vbox_utilisation_exo */
    g_signal_connect (G_OBJECT (priv->radio_button_utilise_exo),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        priv->vbox_utilisation_exo);

    /* on connecte les signaux nécessaires pour gérer la sélection de l'exercice */
    g_signal_connect (G_OBJECT (priv->bouton_exo_detaille),
                   	 "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->sw_exo);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void etats_page_period_init (EtatsPagePeriod *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void etats_page_period_dispose (GObject *object)
{
	G_OBJECT_CLASS (etats_page_period_parent_class)->dispose (object);
}

static void etats_page_period_class_init (EtatsPagePeriodClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = etats_page_period_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/etats/etats_page_period.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, vbox_etats_page_period);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, bouton_exo_courant);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, bouton_exo_detaille);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, bouton_exo_precedent);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, bouton_exo_tous);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, hbox_date_finale);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, hbox_date_init);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, radio_button_utilise_dates);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, radio_button_utilise_exo);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, sw_exo);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, treeview_dates);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, treeview_exo);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, vbox_utilisation_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPagePeriod, vbox_utilisation_exo);
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
EtatsPagePeriod *etats_page_period_new (GtkWidget *etats_prefs)
{
	EtatsPagePeriod *page;


  	page = g_object_new (ETATS_PAGE_PERIOD_TYPE, NULL);
	etats_page_period_setup_page (page, etats_prefs);

	return page;
}

/**
 * Initialise les informations de l'onglet periode
 *
 * \param etats_prefs
 * \param report_number
 *
 * \return
 */
void etats_page_period_initialise_onglet (GtkWidget *etats_prefs,
										  gint report_number)
{
	EtatsPagePeriod *page;
	EtatsPagePeriodPrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_PERIOD (etats_prefs_get_page_by_number (etats_prefs, DATE_PAGE_TYPE));
	priv = etats_page_period_get_instance_private (page);

    if (gsb_data_report_get_use_financial_year (report_number))
    {
        gint financial_year_type;

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radio_button_utilise_exo), TRUE);

        financial_year_type = gsb_data_report_get_financial_year_type (report_number);
        utils_radiobutton_set_active_index (priv->bouton_exo_tous, financial_year_type);

        if (financial_year_type == 3)
		{
            etats_prefs_tree_view_select_rows_from_list (gsb_data_report_get_financial_year_list
															 (report_number),
															 priv->treeview_exo,
															 1);
			gtk_widget_set_sensitive (priv->sw_exo, TRUE);
		}
		else
			gtk_widget_set_sensitive (priv->sw_exo, FALSE);

        /* on initialise le tree_view des dates avec une valeur par défaut (mois en cours) */
        etats_prefs_tree_view_select_single_row (priv->treeview_dates, 3);

		/* on sensibilise les widgets contenant les datas */
		gtk_widget_set_sensitive (priv->vbox_utilisation_date, FALSE);
		gtk_widget_set_sensitive (priv->vbox_utilisation_exo, TRUE);
    }
    else
    {
		gint date_type;

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radio_button_utilise_dates), TRUE);
		date_type = gsb_data_report_get_date_type (report_number);
		etats_prefs_tree_view_select_single_row (priv->treeview_dates, date_type);

		/* on sensibilise les widgets contenant les datas */
		gtk_widget_set_sensitive (priv->vbox_utilisation_date, TRUE);
		gtk_widget_set_sensitive (priv->vbox_utilisation_exo, FALSE);

		/* on active le choix du type de date */
        if (date_type == 1)
        {
            GDate *date;

			/* sensiitive date entries */
			etats_page_period_date_interval_sensitive (page, TRUE);

			/* on remplit les dates perso si elles existent */
			date = gsb_data_report_get_personal_date_start (report_number);
            if (date && g_date_valid (date))
                gsb_calendar_entry_set_date (priv->entree_date_init_etat, date);

			date = gsb_data_report_get_personal_date_end (report_number);
            if (date && g_date_valid (date))
            	gsb_calendar_entry_set_date (priv->entree_date_finale_etat, date);
        }
    }
}

/**
 * Recuperation des informations
 *
 * \param etats_prefs
 * \param report_number
 *
 * \return
 **/
void etats_page_period_get_info (GtkWidget *etats_prefs,
								 gint report_number)
{
    gint active;
	EtatsPagePeriod *page;
	EtatsPagePeriodPrivate *priv;

	page = ETATS_PAGE_PERIOD (etats_prefs_get_page_by_number (etats_prefs, 0));
	priv = etats_page_period_get_instance_private (page);

	/* get datas */
	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->radio_button_utilise_exo));
    gsb_data_report_set_use_financial_year (report_number, active);
    if (active)
    {
        gint index;

        index = utils_radiobutton_get_active_index (priv->bouton_exo_tous);
        gsb_data_report_set_financial_year_type (report_number, index);
        if (index == 3)
        {
            gsb_data_report_free_financial_year_list (report_number);
            gsb_data_report_set_financial_year_list (report_number,
													 etats_prefs_tree_view_get_list_rows_selected
													 (priv->treeview_exo));
            if (utils_tree_view_all_rows_are_selected (GTK_TREE_VIEW (priv->treeview_exo)))
            {
                gchar *text;
                gchar *hint;

                hint = g_strdup (_("Performance issue."));
                text = g_strdup (_("All financial years have been selected.  Grisbi will run "
                                "faster without the \"Detail financial years\" option activated."));

                dialogue_hint (text, hint);
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_exo_tous), FALSE);
                gsb_data_report_set_financial_year_type (report_number, 0);

                g_free (text);
                g_free (hint);
            }
        }
    }
    else
    {
        gint item_selected;

        /* Check that custom dates are OK, but only if custom date range
         * has been selected. */
		item_selected = etats_prefs_tree_view_get_single_row_selected (priv->treeview_dates);
        if (item_selected == 1)
        {
            if (!gsb_date_check_entry (priv->entree_date_init_etat))
            {
                gchar *text;
                gchar *hint;

                text = g_strdup (_("Grisbi can't parse date.  For a list of date formats"
                                   " that Grisbi can use, refer to Grisbi manual."));
                hint = g_strdup_printf (_("Invalid initial date '%s'"),
										gtk_entry_get_text (GTK_ENTRY (priv->entree_date_init_etat)));
                dialogue_error_hint (text, hint);
                g_free (text);
                g_free (hint);

                return;
            }
            else
                gsb_data_report_set_personal_date_start (report_number,
														 gsb_calendar_entry_get_date (priv->entree_date_init_etat));

            if (!gsb_date_check_entry (priv->entree_date_finale_etat))
			{
                gchar *text;
                gchar *hint;

                text = g_strdup (_("Grisbi can't parse date.  For a list of date formats"
								   " that Grisbi can use, refer to Grisbi manual."));
                hint = g_strdup_printf (_("Invalid final date '%s'"),
										gtk_entry_get_text (GTK_ENTRY (priv->entree_date_finale_etat)));
                dialogue_error_hint (text, hint);
                g_free (text);
                g_free (hint);

                return;
            }
            else
                gsb_data_report_set_personal_date_end (report_number,
													   gsb_calendar_entry_get_date (priv->entree_date_finale_etat));
        }
        gsb_data_report_set_date_type (report_number, item_selected);
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
GtkWidget *etats_page_period_get_radio_button_utilise_exo (GtkWidget *etats_prefs)
{
	EtatsPagePeriod *page;
	EtatsPagePeriodPrivate *priv;

	page = ETATS_PAGE_PERIOD (etats_prefs_get_page_by_number (etats_prefs, 0));
	priv = etats_page_period_get_instance_private (page);

	return priv->radio_button_utilise_exo;
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

