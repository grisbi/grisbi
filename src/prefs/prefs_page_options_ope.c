/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org                                               */
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
#include "prefs_page_options_ope.h"
#include "grisbi_app.h"
#include "gsb_file.h"
#include "gsb_rgba.h"
#include "gsb_transactions_list.h"
#include "navigation.h"
#include "structures.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageOptionsOpePrivate   PrefsPageOptionsOpePrivate;

struct _PrefsPageOptionsOpePrivate
{
	GtkWidget *			vbox_options_ope;

	GtkWidget *			checkbutton_retient_affichage_par_compte;
    GtkWidget *			checkbutton_show_transaction_gives_balance;
	GtkWidget *			checkbutton_force_credit_before_debit;
    GtkWidget *			checkbutton_show_transaction_selected_in_form;
	GtkWidget *			combo_display_one_line;
	GtkWidget *			combo_display_two_lines;
	GtkWidget *			combo_display_three_lines;
	GtkWidget *			combo_transactions_list_primary_sorting;
	GtkWidget *			combo_transactions_list_secondary_sorting;
    GtkWidget *         grid_display_modes;

};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageOptionsOpe, prefs_page_options_ope, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * update the account page if necessary
 *
 * \param self
 * \param NULL
 *
 * \return
 **/
static void prefs_page_options_ope_checkbutton_force_credit_before_debit_toggled (GtkCheckButton* self,
																				  gpointer user_data)
{
	gint current_page;

	devel_debug (NULL);
	current_page = gsb_gui_navigation_get_current_page ();
	if (current_page == GSB_ACCOUNT_PAGE)
		gsb_transactions_list_update_tree_view (gsb_gui_navigation_get_current_account(), TRUE);
}

/**
 * called when we change a button for the display mode
 *
 * \param button 	the combo box which changed
 * \param line_ptr	a gint* which is the line of the button (ie 1 line mode, 2 lines or 3 lines)
 *
 * \return FALSE
 **/
static gboolean prefs_page_options_ope_display_mode_button_changed (GtkWidget *button,
																	gint *line_ptr)
{
    gint line = GPOINTER_TO_INT (line_ptr);
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

    switch (line)
    {
	case 0:
	    /* 1 line visible mode */
	    w_run->display_one_line = gtk_combo_box_get_active (GTK_COMBO_BOX (button));
	    break;
	case 1:
	    /* 2 lines visibles mode */
	    w_run->display_two_lines = gtk_combo_box_get_active (GTK_COMBO_BOX (button));
	    break;
	case 2:
	    /* 3 lines visibles mode */
	    w_run->display_three_lines = gtk_combo_box_get_active (GTK_COMBO_BOX (button));
	    break;
    }

    /* update the visible account */
    gsb_transactions_list_update_tree_view (gsb_gui_navigation_get_current_account (), TRUE);

    gsb_file_set_modified (TRUE);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean prefs_page_options_ope_display_sort_changed (GtkComboBox *widget,
															 gint *pointeur)
{
    gint page_number;
    gint account_nb;
    gint value = 0;
    gint sort_type = 0;
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

    page_number = gsb_gui_navigation_get_current_page ();
    value = gtk_combo_box_get_active (widget);
    sort_type = GPOINTER_TO_INT (pointeur);

	/* On force le tri secondaire si on choisit PRIMARY_SORT_BY_VALUE_DATE_THEN_OPERATION_DATA */
    if (sort_type == PRIMARY_SORT)
    {
		a_conf->transactions_list_primary_sorting = value;
		if (value == PRIMARY_SORT_BY_VALUE_DATE_THEN_OPERATION_DATA)
		{
			GtkWidget *combo;

			combo = g_object_get_data (G_OBJECT (widget), "secondary_combo");
			g_signal_handlers_block_by_func (G_OBJECT (combo),
											 G_CALLBACK (prefs_page_options_ope_display_sort_changed),
											 pointeur);
			a_conf->transactions_list_secondary_sorting = SECONDARY_SORT_BY_VALUE_DATE_THEN_DATE;
			gtk_combo_box_set_active (GTK_COMBO_BOX (combo), a_conf->transactions_list_secondary_sorting);
			g_signal_handlers_unblock_by_func (G_OBJECT (combo),
											   G_CALLBACK (prefs_page_options_ope_display_sort_changed),
											   pointeur);
		}
	}
    gsb_file_set_modified (TRUE);

    switch (page_number)
    {
        case GSB_ACCOUNT_PAGE:
            account_nb = gsb_gui_navigation_get_current_account ();
            if (account_nb != -1)
                gsb_transactions_list_update_tree_view (account_nb, TRUE);
            break;
    }

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void prefs_page_options_ope_init_combo_sorting (PrefsPageOptionsOpe *page,
													   GrisbiAppConf *a_conf)
{
	GtkListStore *store = NULL;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	const gchar *str_color;
    gchar *options_tri_primaire[] = {
		_("Sort by value date (if fail, try with the date)"),
		_("Sort by value date and then by date"),
		_("Forced sort by transaction date"),
		NULL
    };
    gchar *options_tri_secondaire[] = {
		_("Sort by transaction number"),
		_("Sort by type of amount (credit debit)"),
		_("Sort by payee name (if fail, by transaction number)"),
		_("Sort by date and then by transaction number"),
		_("Sort by value date and then by date"),
		NULL
    };
	gint i = 0;
	PrefsPageOptionsOpePrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_options_ope_get_instance_private (page);

	str_color = gsb_rgba_get_couleur_to_string ("text_gsetting_option_normal");

	/* Primary sorting option for the transactions */
	store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
	for (i = 0; i < 3; i++)
	{
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, options_tri_primaire[i], 1, i, 2, str_color, -1);
    }

	gtk_combo_box_set_model (GTK_COMBO_BOX (priv->combo_transactions_list_primary_sorting),
							 GTK_TREE_MODEL (store));
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (priv->combo_transactions_list_primary_sorting), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (priv->combo_transactions_list_primary_sorting),
									renderer,
									"text", 0,
									"foreground", 2,
									NULL);
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_transactions_list_primary_sorting),
							  a_conf->transactions_list_primary_sorting);

	g_signal_connect (G_OBJECT (priv->combo_transactions_list_primary_sorting),
					  "changed",
					  G_CALLBACK (prefs_page_options_ope_display_sort_changed),
					  GINT_TO_POINTER (PRIMARY_SORT));

	/* Secondary sorting option for the transactions */
	store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
	for (i = 0; i < 5; i++)
	{
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, options_tri_secondaire[i], 1, i, 2, str_color, -1);
    }

	gtk_combo_box_set_model (GTK_COMBO_BOX (priv->combo_transactions_list_secondary_sorting),
							 GTK_TREE_MODEL (store));
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (priv->combo_transactions_list_secondary_sorting), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (priv->combo_transactions_list_secondary_sorting),
									renderer,
									"text", 0,
									"foreground", 2,
									NULL);
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_transactions_list_secondary_sorting),
							  a_conf->transactions_list_secondary_sorting);

	g_signal_connect (G_OBJECT (priv->combo_transactions_list_secondary_sorting),
					  "changed",
					  G_CALLBACK (prefs_page_options_ope_display_sort_changed),
					  GINT_TO_POINTER (SECONDARY_SORT));

	g_free ((gpointer)str_color);
}

/**
 * Création de la page de gestion des options_ope
 *
 * \param prefs
 *
 * \return
 */
static void prefs_page_options_ope_setup_options_ope_page (PrefsPageOptionsOpe *page)
{
	GtkWidget *head_page;
	gboolean is_loading;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;
	GrisbiWinRun *w_run;
	PrefsPageOptionsOpePrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_options_ope_get_instance_private (page);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Transaction list behavior"),
															   "gsb-transaction-list-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_options_ope), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_options_ope), head_page, 0);

    /* set checkbuttons */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_retient_affichage_par_compte),
								  w_etat->retient_affichage_par_compte);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_show_transaction_gives_balance),
								  a_conf->show_transaction_gives_balance);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_force_credit_before_debit),
								  w_etat->force_credit_before_debit);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_show_transaction_selected_in_form),
								  a_conf->show_transaction_selected_in_form);

	/* set combos sorting */
	prefs_page_options_ope_init_combo_sorting (page, a_conf);
	g_object_set_data (G_OBJECT (priv->combo_transactions_list_primary_sorting),
					   "secondary_combo",
					   priv->combo_transactions_list_secondary_sorting),
	g_object_set_data (G_OBJECT (priv->combo_transactions_list_secondary_sorting),
					   "primary_combo",
					   priv->combo_transactions_list_primary_sorting);

	/* set combo display lines */
	gtk_combo_box_set_active ( GTK_COMBO_BOX (priv->combo_display_one_line), w_run->display_one_line);
	gtk_combo_box_set_active ( GTK_COMBO_BOX (priv->combo_display_two_lines), w_run->display_two_lines);
	gtk_combo_box_set_active ( GTK_COMBO_BOX (priv->combo_display_three_lines), w_run->display_three_lines);

	if (!is_loading)
	{
		gtk_widget_set_sensitive (priv->checkbutton_retient_affichage_par_compte, FALSE);
		gtk_widget_set_sensitive (priv->grid_display_modes, FALSE);
	}

    /* Connect signal */
    g_signal_connect (G_OBJECT (priv->combo_display_one_line),
					  "changed",
					  G_CALLBACK (prefs_page_options_ope_display_mode_button_changed),
					  GINT_TO_POINTER (0));
    g_signal_connect (G_OBJECT (priv->combo_display_two_lines),
					  "changed",
					  G_CALLBACK (prefs_page_options_ope_display_mode_button_changed),
					  GINT_TO_POINTER (1));
    g_signal_connect (G_OBJECT (priv->combo_display_three_lines),
					  "changed",
					  G_CALLBACK (prefs_page_options_ope_display_mode_button_changed),
					  GINT_TO_POINTER (2));

    g_signal_connect (priv->checkbutton_retient_affichage_par_compte,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->retient_affichage_par_compte);
    g_signal_connect (priv->checkbutton_show_transaction_gives_balance,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->show_transaction_gives_balance);
    g_signal_connect (priv->checkbutton_force_credit_before_debit,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->force_credit_before_debit);

	/* update tree_view if necessary */
    g_signal_connect (priv->checkbutton_force_credit_before_debit,
					  "toggled",
					  G_CALLBACK (prefs_page_options_ope_checkbutton_force_credit_before_debit_toggled),
					  NULL);

    g_signal_connect (priv->checkbutton_show_transaction_selected_in_form,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->show_transaction_selected_in_form);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_options_ope_init (PrefsPageOptionsOpe *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_options_ope_setup_options_ope_page (page);
}

static void prefs_page_options_ope_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_options_ope_parent_class)->dispose (object);
}

static void prefs_page_options_ope_class_init (PrefsPageOptionsOpeClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_options_ope_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_options_ope.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageOptionsOpe, vbox_options_ope);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageOptionsOpe,
												  checkbutton_retient_affichage_par_compte);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageOptionsOpe,
												  checkbutton_show_transaction_gives_balance);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageOptionsOpe,
												  checkbutton_force_credit_before_debit);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageOptionsOpe,
												  checkbutton_show_transaction_selected_in_form);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageOptionsOpe,
												  combo_display_one_line);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageOptionsOpe,
												  combo_display_two_lines);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageOptionsOpe,
												  combo_display_three_lines);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageOptionsOpe,
												  combo_transactions_list_primary_sorting);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageOptionsOpe,
												  combo_transactions_list_secondary_sorting);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageOptionsOpe, grid_display_modes);
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
PrefsPageOptionsOpe *prefs_page_options_ope_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_OPTIONS_OPE_TYPE, NULL);
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

