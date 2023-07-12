/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2010-2018 Pierre Biava (grisbi@pierre.biava.name)        */
/*          https://www.grisbi.org/                                            */
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

#include "include.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "bet_finance_ui.h"
#include "dialog.h"
#include "export_csv.h"
#include "grisbi_app.h"
#include "gsb_automem.h"
#include "gsb_combo_box.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_scheduled.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_form_widget.h"
#include "gsb_real.h"
#include "gsb_rgba.h"
#include "mouse.h"
#include "navigation.h"
#include "print_tree_view_list.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_files.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/* notebook pour la simulation de crédits */
static GtkWidget *finance_notebook;
/* toolbars */
static GtkWidget *account_toolbar;
static GtkWidget *amortization_toolbar;
static GtkWidget *simulator_toolbar;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

#define DUREE_MIN_MOIS 		 3		/* durée mini du prêt en mois */
#define DUREE_MAX_MOIS 		18		/* durée maxi du prêt en mois */
#define DUREE_MIN_ANNEE		 1		/* durée mini du prêt en année */
#define DUREE_MOY_ANNEE		15		/* durée moyennedu prêt en années */
#define DUREE_MAX_ANNEE		30		/* durée maxi du prêt en années */

enum BetFinanceAmortizationColumns
{
    BET_AMORTIZATION_NUMBER_COLUMN,
    BET_AMORTIZATION_DATE_COLUMN,
    BET_AMORTIZATION_CAPITAL_DU_COLUMN,
    BET_AMORTIZATION_INTERETS_COLUMN,
    BET_AMORTIZATION_PRINCIPAL_COLUMN,
    BET_AMORTIZATION_FRAIS_COLUMN,
    BET_AMORTIZATION_ECHEANCE_COLUMN,
    BET_AMORTIZATION_BACKGROUND_COLOR,
    BET_AMORTIZATION_NBRE_COLUMNS
};

enum BetFinanceDataColumns
{
    BET_FINANCE_DURATION_COLUMN,
    BET_FINANCE_NBRE_ECHEANCE_COLUMN,
    BET_FINANCE_DEVISE_COLUMN,
    BET_FINANCE_CAPITAL_COLUMN,
    BET_FINANCE_CAPITAL_DOUBLE,
    BET_FINANCE_TAUX_COLUMN,
    BET_FINANCE_TAUX_PERIODIQUE_DOUBLE,
    BET_FINANCE_HORS_FRAIS_COLUMN,
    BET_FINANCE_HORS_FRAIS_DOUBLE,
    BET_FINANCE_FRAIS_COLUMN,
    BET_FINANCE_FRAIS_DOUBLE,
    BET_FINANCE_ECHEANCE_COLUMN,
    BET_FINANCE_ECHEANCE_DOUBLE,
    BET_FINANCE_COUT_COLUMN,
    BET_FINANCE_COUT_DOUBLE,
    BET_FINANCE_BACKGROUND_COLOR,
    BET_FINANCE_NBRE_COLUMNS
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/* TRAITEMENTS */
/**
 *
 *
 * \param
 * \param
 * \param force la maj de s_loan->capital_du
 *
 * \return
 **/
static AmortissementStruct *bet_finance_get_echeance_at_date (LoanStruct *s_loan,
															  const GDate *date,
															  gboolean maj_s_loan_capital_du)
{
	AmortissementStruct *s_amortissement = NULL;

	//devel_debug (gsb_format_gdate (date));
	s_amortissement = bet_data_finance_structure_amortissement_init ();
	if (g_date_compare (date, s_loan->first_date) == 0
		&& s_loan->first_is_different)
	{
			s_amortissement->echeance = s_loan->first_capital;
			s_amortissement->echeance += s_loan->amount_fees + s_loan->first_interests;
			s_amortissement->interets = s_loan->first_interests;
			s_amortissement->frais = s_loan->amount_fees;
			s_amortissement->principal = s_loan->first_capital;
	}
	else if (s_loan->type_taux == 2)
	{
		gdouble taux_periodique;

		taux_periodique = bet_data_finance_get_taux_periodique (s_loan->annual_rate, s_loan->type_taux);
		s_amortissement->echeance = s_loan->fixed_due_amount;
		s_amortissement->interets = bet_data_finance_get_interets (s_loan->capital_du, taux_periodique);
		s_amortissement->frais = bet_data_finance_get_fees_for_type_taux_2 (s_loan->capital_du,
																			s_loan->percentage_fees);
		s_amortissement->principal = bet_data_finance_get_principal (s_amortissement->echeance,
																	 s_amortissement->interets,
																	 s_amortissement->frais);
	}
	else
	{
		gdouble taux_periodique;
//~ printf ("s_loan->capital_du avant = %f\n", s_loan->capital_du);
		taux_periodique = bet_data_finance_get_taux_periodique (s_loan->annual_rate, s_loan->type_taux);
		s_amortissement->echeance = bet_data_finance_get_echeance (s_loan->capital, taux_periodique, s_loan->duree);
		s_amortissement->echeance += s_loan->amount_fees;
		s_amortissement->interets = bet_data_finance_get_interets (s_loan->capital_du, taux_periodique);
		s_amortissement->frais = s_loan->amount_fees;
		s_amortissement->principal = bet_data_finance_get_principal (s_amortissement->echeance,
																	 s_amortissement->interets,
																	 s_amortissement->frais);
	}

	s_amortissement->capital_du = s_loan->capital_du - s_amortissement->principal;
//~ printf ("capital_du = %f principal = %f interêts = %f frais = %f\n",
		//~ s_amortissement->capital_du, s_amortissement->principal, s_amortissement->interets, s_amortissement->frais);

	if (maj_s_loan_capital_du)
		s_loan->capital_du -= s_amortissement->principal;
//~ printf ("s_loan->capital_du après = %f\n\n", s_loan->capital_du);
	return s_amortissement;
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void bet_finance_fill_data_ligne (GtkTreeModel *model,
										 EcheanceStruct *s_echeance,
										 const gchar *unit)
{
    GtkTreeIter iter;
    gchar *str_duree;
    gchar *str_capital;
    gchar *str_taux;
    gchar *str_frais;
    gchar *str_echeance;
    gchar *str_totale;
    gchar *str_total_cost;
	gchar *tmp_str;
    gchar buffer[256];
    gint nbre_char;

	tmp_str = utils_str_itoa (s_echeance->duree);
    str_duree = g_strconcat (tmp_str, " ", unit, " ", NULL);
	g_free (tmp_str);

    str_capital = utils_real_get_string_with_currency (gsb_real_double_to_real (s_echeance->capital),
													   s_echeance->devise,
													   TRUE);

    nbre_char = g_sprintf (buffer, "%.*f", BET_TAUX_DIGITS, s_echeance->taux);
    str_taux =  g_strndup (buffer, nbre_char + 1);

    str_frais = utils_real_get_string_with_currency (gsb_real_double_to_real (s_echeance->frais),
													 s_echeance->devise,
													 TRUE);

    str_echeance = utils_real_get_string_with_currency (gsb_real_double_to_real (s_echeance->echeance),
														s_echeance->devise,
														TRUE);

    str_totale = utils_real_get_string_with_currency (gsb_real_double_to_real (s_echeance->total_echeance),
													  s_echeance->devise,
													  TRUE);

    str_total_cost = utils_real_get_string_with_currency (gsb_real_double_to_real (s_echeance->total_cost),
														  s_echeance->devise,
														  TRUE);

    gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (model),
                        &iter,
                        BET_FINANCE_DURATION_COLUMN, str_duree,
                        BET_FINANCE_NBRE_ECHEANCE_COLUMN, s_echeance->nbre_echeances,
                        BET_FINANCE_DEVISE_COLUMN, s_echeance->devise,
                        BET_FINANCE_CAPITAL_COLUMN, str_capital,
                        BET_FINANCE_CAPITAL_DOUBLE, s_echeance->capital,
                        BET_FINANCE_TAUX_COLUMN, str_taux,
                        BET_FINANCE_TAUX_PERIODIQUE_DOUBLE, s_echeance->taux_periodique,
                        BET_FINANCE_HORS_FRAIS_COLUMN, str_echeance,
                        BET_FINANCE_HORS_FRAIS_DOUBLE, s_echeance->echeance,
                        BET_FINANCE_FRAIS_COLUMN, str_frais,
                        BET_FINANCE_FRAIS_DOUBLE, s_echeance->frais,
                        BET_FINANCE_ECHEANCE_COLUMN, str_totale,
                        BET_FINANCE_ECHEANCE_DOUBLE, s_echeance->total_echeance,
                        BET_FINANCE_COUT_COLUMN, str_total_cost,
                        BET_FINANCE_COUT_DOUBLE, s_echeance->total_cost,
                        - 1);

    g_free (str_duree);
    g_free (str_capital);
    g_free (str_taux);
    g_free (str_frais);
    g_free (str_echeance);
    g_free (str_totale);
	g_free (str_total_cost);
}

/**
 * Affiche une ligne du tableau d'amortissement en mois
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void bet_finance_calcule_show_months_tab (GtkTreeModel *model,
												 EcheanceStruct *s_echeance,
												 gdouble taux_frais)
{
    gint index;

    for (index = DUREE_MIN_MOIS; index <= DUREE_MAX_MOIS; index++)
    {
        s_echeance->duree = index;
        s_echeance->nbre_echeances = index;
        s_echeance->frais = bet_data_finance_get_frais_par_echeance (s_echeance->capital,
																	 taux_frais,
																	 s_echeance->nbre_echeances);

        s_echeance->echeance = bet_data_finance_get_echeance (s_echeance->capital,
															  s_echeance->taux_periodique,
															  s_echeance->nbre_echeances);

        s_echeance->total_echeance = s_echeance->echeance + s_echeance->frais;
        s_echeance->total_cost = bet_data_finance_get_total_cost (s_echeance);

        bet_finance_fill_data_ligne (model, s_echeance, _("months"));

        s_echeance->duree = 0;
        s_echeance->nbre_echeances = 0;
        s_echeance->echeance = 0;
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
static void bet_finance_fill_amortization_ligne (GtkTreeModel *model,
												 AmortissementStruct *s_amortissement)
{
    GtkTreeIter iter;
    gchar *str_capital_du = NULL;
    gchar *str_interets = NULL;
    gchar *str_principal = NULL;

    str_capital_du = utils_real_get_string_with_currency (gsb_real_double_to_real (s_amortissement->capital_du),
														  s_amortissement->devise,
														  TRUE);

    str_interets = utils_real_get_string_with_currency (gsb_real_double_to_real (s_amortissement->interets),
														s_amortissement-> devise,
														TRUE);

    str_principal = utils_real_get_string_with_currency (gsb_real_double_to_real (s_amortissement->principal),
														 s_amortissement-> devise,
														 TRUE);

    gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);

    if (s_amortissement->origin == SPP_ORIGIN_FINANCE)
    {
        gtk_tree_store_set (GTK_TREE_STORE (model),
							&iter,
							BET_AMORTIZATION_NUMBER_COLUMN, s_amortissement->num_echeance,
							BET_AMORTIZATION_DATE_COLUMN, s_amortissement->str_date,
							BET_AMORTIZATION_CAPITAL_DU_COLUMN, str_capital_du,
							BET_AMORTIZATION_INTERETS_COLUMN, str_interets,
							BET_AMORTIZATION_PRINCIPAL_COLUMN, str_principal,
							BET_AMORTIZATION_FRAIS_COLUMN, s_amortissement->str_frais,
							BET_AMORTIZATION_ECHEANCE_COLUMN, s_amortissement->str_echeance,
							- 1);
    }
    else
    {
        gtk_tree_store_set (GTK_TREE_STORE (model),
							&iter,
							BET_AMORTIZATION_NUMBER_COLUMN, s_amortissement->num_echeance,
							BET_AMORTIZATION_CAPITAL_DU_COLUMN, str_capital_du,
							BET_AMORTIZATION_INTERETS_COLUMN, str_interets,
							BET_AMORTIZATION_PRINCIPAL_COLUMN, str_principal,
							BET_AMORTIZATION_FRAIS_COLUMN, s_amortissement->str_frais,
							BET_AMORTIZATION_ECHEANCE_COLUMN, s_amortissement->str_echeance,
							- 1);
    }

    g_free (str_capital_du);
    g_free (str_interets);
    g_free (str_principal);
}

/**
 * remplit le tableau d'amortissement
 *
 * /param menu item
 * /param row selected
 *
 * \return
 **/
static void bet_finance_fill_amortization_array (GtkWidget *menu_item,
												 GtkTreeSelection *tree_selection)
{
    GtkWidget *page;
    GtkWidget *label;
    GtkWidget *tree_view;
    GtkTreeModel *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path;
    gchar *str_duree;
    gchar *str_capital;
    gchar *str_taux;
    gint index = 0;
    gint nbre_echeances;
    gdouble taux_periodique;
    AmortissementStruct *s_amortissement;

    devel_debug (NULL);
    if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (tree_selection), &model, &iter))
		return;

    gtk_notebook_next_page (GTK_NOTEBOOK (finance_notebook));

    /* initialise les données utiles */
    s_amortissement = bet_data_finance_structure_amortissement_init ();
    gtk_tree_model_get (model,
                        &iter,
                        BET_FINANCE_DURATION_COLUMN, &str_duree,
                        BET_FINANCE_NBRE_ECHEANCE_COLUMN, &nbre_echeances,
                        BET_FINANCE_DEVISE_COLUMN, &s_amortissement->devise,
                        BET_FINANCE_CAPITAL_COLUMN, &str_capital,
                        BET_FINANCE_CAPITAL_DOUBLE, &s_amortissement->capital_du,
                        BET_FINANCE_TAUX_COLUMN, &str_taux,
                        BET_FINANCE_TAUX_PERIODIQUE_DOUBLE, &taux_periodique,
                        BET_FINANCE_FRAIS_COLUMN, &s_amortissement->str_frais,
                        BET_FINANCE_FRAIS_DOUBLE, &s_amortissement->frais,
                        BET_FINANCE_ECHEANCE_COLUMN, &s_amortissement->str_echeance,
                        BET_FINANCE_ECHEANCE_DOUBLE, &s_amortissement->echeance,
                        -1);

    /* met à jour le titre du tableau d'amortissement */
    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (finance_notebook), 1);
    label = g_object_get_data (G_OBJECT (page), "capital");
    gtk_label_set_label (GTK_LABEL (label), str_capital);
    label = g_object_get_data (G_OBJECT (page), "bet_finance_taux");
    gtk_label_set_label (GTK_LABEL (label), str_taux);
    label = g_object_get_data (G_OBJECT (page), "duree");
    gtk_label_set_label (GTK_LABEL (label), str_duree);

    g_free (str_duree);
    g_free (str_capital);
    g_free (str_taux);

    /* remplit le tableau d'amortissement */
    tree_view = g_object_get_data (G_OBJECT (page), "tree_view");
    store = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    gtk_tree_store_clear (GTK_TREE_STORE (store));

    for (index = 1; index <= nbre_echeances; index++)
    {
        s_amortissement->num_echeance = index;
        s_amortissement->interets = bet_data_finance_get_interets (s_amortissement->capital_du,
																   taux_periodique);

        if (index == nbre_echeances)
        {
			gchar *amount;

			s_amortissement->echeance = bet_data_finance_get_last_echeance (s_amortissement->capital_du,
																			s_amortissement->interets,
																			s_amortissement->frais);
            g_free (s_amortissement->str_echeance);
			amount = utils_real_get_string_with_currency (gsb_real_double_to_real (s_amortissement->echeance),
														  s_amortissement-> devise,
														  TRUE);
            s_amortissement->str_echeance = amount;
            s_amortissement->principal = s_amortissement->capital_du;
        }
        else
            s_amortissement->principal = bet_data_finance_get_principal (s_amortissement->echeance,
																		 s_amortissement->interets,
																		 s_amortissement->frais);

        bet_finance_fill_amortization_ligne (store, s_amortissement);
        s_amortissement->capital_du -= s_amortissement->principal;
    }

    utils_set_tree_store_background_color (tree_view, BET_AMORTIZATION_BACKGROUND_COLOR);
    path = gtk_tree_path_new_first ();
    gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, TRUE, 0.0, 0.0);
    gtk_tree_selection_select_path (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)), path);

    gtk_tree_path_free (path);
    bet_data_finance_structure_amortissement_free (s_amortissement);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void bet_finance_struct_amortization_free (AmortissementStruct *s_amortissement)
{
    if (s_amortissement->str_date)
        g_free (s_amortissement->str_date);
    if (s_amortissement->str_echeance)
        g_free (s_amortissement->str_echeance);
    if (s_amortissement->str_frais)
        g_free (s_amortissement->str_frais);

    g_free (s_amortissement);
}

/* IHM */
/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_finance_ui_calculer_clicked (GtkButton *button,
											 GtkWidget *widget)
{
    GtkWidget *combobox;
    GtkWidget *bouton;
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreePath *path;
    gdouble taux_frais;
    gint duree_min, duree_max;
    gint type_taux;
    gint index = 0;
    EcheanceStruct *s_echeance;

    tree_view = g_object_get_data (G_OBJECT (widget), "tree_view");
    if (!GTK_IS_TREE_VIEW (tree_view))
        return;

    s_echeance = g_malloc0 (sizeof (EcheanceStruct));

    /* devise */
    combobox = g_object_get_data (G_OBJECT (widget), "devise");
    if (combobox)
        s_echeance->devise = gsb_currency_get_currency_from_combobox (combobox);

    /* capital */
    s_echeance->capital = bet_finance_get_number_from_string (widget, "capital");

    if (s_echeance->capital == 0)
    {
        gchar *tmp_str;

        tmp_str = g_strdup (_("You must enter at least one value for the capital"));
        dialogue_error (tmp_str);
        g_free (tmp_str);
		g_free (s_echeance);

		return;
    }
    else if (s_echeance->capital == G_MININT64)
	{
 		g_free (s_echeance);

		return;
	}

    /* rate */
    s_echeance->taux = bet_finance_get_number_from_string (widget, "taux");

    /* Duration */
    combobox = g_object_get_data (G_OBJECT (widget), "duree");
    if (combobox)
        index = gsb_combo_box_get_index (combobox);

    switch (index)
    {
        case 0:
            duree_min = DUREE_MIN_MOIS;
            duree_max = DUREE_MAX_MOIS;
            break;
        case 1:
            duree_min = DUREE_MIN_ANNEE;
            duree_max = DUREE_MOY_ANNEE;
            break;
        case 2:
            duree_min = DUREE_MOY_ANNEE;
            duree_max = DUREE_MAX_ANNEE;
            break;
        default :
            duree_min = DUREE_MIN_ANNEE;
            duree_max = DUREE_MOY_ANNEE;
            break;
    }

    /* frais */
    taux_frais = bet_finance_get_number_from_string (widget, "frais");

    /* type de taux */
    bouton = g_object_get_data (G_OBJECT (widget), "type_taux");
    type_taux = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bouton));

    /* taux periodique */
    s_echeance->taux_periodique = bet_data_finance_get_taux_periodique (s_echeance->taux,
																		type_taux);

    /* réinitialisation du model */
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    gtk_tree_store_clear (GTK_TREE_STORE (model));

    if (index == 0)
        bet_finance_calcule_show_months_tab (model, s_echeance, taux_frais);
    else
    {
        for (index = duree_min; index <= duree_max; index++)
        {
            s_echeance->duree = index;
            s_echeance->nbre_echeances = index * 12;
            s_echeance->frais = bet_data_finance_get_frais_par_echeance (s_echeance->capital,
																		 taux_frais,
																		 s_echeance->nbre_echeances);

            s_echeance->echeance = bet_data_finance_get_echeance (s_echeance->capital,
																  s_echeance->taux_periodique,
																  s_echeance->nbre_echeances);

            s_echeance->total_echeance = s_echeance->echeance + s_echeance->frais;
            s_echeance->total_cost = bet_data_finance_get_total_cost (s_echeance);

            if (s_echeance->duree == 1)
                bet_finance_fill_data_ligne (model, s_echeance, _("year"));
            else
                bet_finance_fill_data_ligne (model, s_echeance, _("years"));

            s_echeance->duree = 0;
            s_echeance->nbre_echeances = 0;
            s_echeance->echeance = 0;
        }
    }

    utils_set_tree_store_background_color (tree_view, BET_FINANCE_BACKGROUND_COLOR);
    path = gtk_tree_path_new_first ();
    gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, TRUE, 0.0, 0.0);
    gtk_tree_selection_select_path (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)), path);

    gtk_tree_path_free (path);
    g_free (s_echeance);

    gsb_file_set_modified (TRUE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean bet_finance_ui_capital_entry_changed (GtkWidget *entry,
													  GtkWidget *page)
{
    const gchar *text;
    gdouble capital;
    gboolean valide;

    text = gtk_entry_get_text (GTK_ENTRY (entry));
    capital = utils_str_strtod (text, NULL);

    if (strlen (text) == 0 || capital == 0)
    {
        gtk_widget_set_name (entry, "form_entry");
	    return FALSE;
    }

    valide = gsb_form_widget_get_valide_amout_entry (text);
    if (valide)
    {
        /* the entry is valid, make it normal */
        gtk_widget_set_name (entry, "form_entry");
    }
    else
    {
	    /* the entry is not valid, make it red */
        gtk_widget_set_name (entry, "form_entry_error");
    }

    return FALSE;
}

/**
 * called when press a key on an element of the form
 *
 * \param widget which receive the signal
 * \param ev
 * \param page
 *
 * \return FALSE
 **/
static gboolean bet_finance_ui_capital_entry_key_press_event (GtkWidget *widget,
															  GdkEventKey *ev,
															  GtkWidget *page)
{
    gchar *str_capital;

    switch (ev->keyval)
    {
        case GDK_KEY_1:
        case GDK_KEY_2:
        case GDK_KEY_3:
        case GDK_KEY_4:
        case GDK_KEY_5:
        case GDK_KEY_6:
        case GDK_KEY_7:
        case GDK_KEY_8:
        case GDK_KEY_9:
        case GDK_KEY_0:
            break;

        case GDK_KEY_Escape :
            str_capital = utils_real_get_string_with_currency (gsb_real_double_to_real (etat.bet_capital),
															   etat.bet_currency,
															   FALSE);
            gtk_entry_set_text (GTK_ENTRY (widget), str_capital);
            gtk_editable_set_position (GTK_EDITABLE (widget), -1);
            g_free (str_capital);
            return TRUE;
            break;

        case GDK_KEY_ISO_Left_Tab:
            etat.bet_capital = bet_finance_get_number_from_string (page, "capital");
            bet_finance_ui_calculer_clicked (NULL, page);
            return TRUE;
            break;

        case GDK_KEY_Tab :
            etat.bet_capital = bet_finance_get_number_from_string (page, "capital");
            bet_finance_ui_calculer_clicked (NULL, page);
            return TRUE;
            break;

        case GDK_KEY_KP_Enter :
        case GDK_KEY_Return :
            etat.bet_capital = bet_finance_get_number_from_string (page, "capital");
            bet_finance_ui_calculer_clicked (NULL, page);
            return TRUE;
            break;
    }

    return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_finance_ui_switch_amortization_initial_date (GtkWidget *widget,
															 GtkWidget *tree_view)
{
    GtkWidget *tmp_button;
    gint account_number;
    gboolean amortization_initial_date;
	GrisbiAppConf *a_conf;

	devel_debug (NULL);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
    amortization_initial_date = GPOINTER_TO_INT (g_object_get_data ( G_OBJECT (tree_view),
																	"amortization_initial_date"));
    amortization_initial_date = !amortization_initial_date;

    g_object_set_data (G_OBJECT (tree_view),
					   "amortization_initial_date",
					   GINT_TO_POINTER (amortization_initial_date));

	if (GTK_IS_BUTTON (widget))
		tmp_button = widget;
	else
		tmp_button = g_object_get_data (G_OBJECT (tree_view), "amortization_initial_date_button");

    if (a_conf->display_toolbar != GTK_TOOLBAR_ICONS)
    {
		if (amortization_initial_date)
			gtk_tool_button_set_label (GTK_TOOL_BUTTON (tmp_button), _("At today"));
		else
			gtk_tool_button_set_label (GTK_TOOL_BUTTON (tmp_button), _("Start date"));
    }

	if (amortization_initial_date)
		gtk_widget_set_tooltip_text (GTK_WIDGET (tmp_button), _("Show amortization schedule to date"));
	else
		gtk_widget_set_tooltip_text (GTK_WIDGET (tmp_button), _("Show amortization schedule from the beginning"));

    account_number = gsb_gui_navigation_get_current_account ();
    bet_finance_update_amortization_tab (account_number);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_finance_ui_export_tab (GtkWidget *menu_item,
									   GtkTreeView *tree_view)
{
    GtkWidget *dialog;
	GtkWidget *button_cancel;
	GtkWidget *button_save;
    gint resultat;
    gchar *filename;
    gchar *tmp_last_directory;

    dialog = gtk_file_chooser_dialog_new (_("Export the array"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_FILE_CHOOSER_ACTION_SAVE,
										  GTK_BUTTONS_NONE,
										  NULL);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_cancel, GTK_RESPONSE_CANCEL);
	gtk_widget_set_can_default (button_cancel, TRUE);

	button_save = gtk_button_new_with_label (_("Save"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_save, GTK_RESPONSE_OK);
	gtk_widget_set_can_default (button_save, TRUE);

    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),  _("credit.csv"));
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gsb_file_get_last_path ());
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);

	gtk_widget_show_all (dialog);

	resultat = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (resultat)
    {
		case GTK_RESPONSE_OK :
			filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
			tmp_last_directory = utils_files_selection_get_last_directory (GTK_FILE_CHOOSER (dialog), TRUE);
			gsb_file_update_last_path (tmp_last_directory);
			g_free (tmp_last_directory);
			gtk_widget_destroy (GTK_WIDGET (dialog));

			/* vérification que c'est possible est faite par la boite de dialogue */
			if (!gsb_csv_export_tree_view_list (filename, tree_view))
			{
				dialogue_error (_("Cannot save file."));
				return;
			}

			break;

		default :
			gtk_widget_destroy (GTK_WIDGET (dialog));
			return;
    }
}

/**
 * Pop up a menu with several actions to apply to data_list.
 *
 * \param tree_view
 *
 * \return
 **/
static void bet_finance_ui_data_list_context_menu (GtkWidget *tree_view,
												   gint page_num)
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    GtkTreeModel *model;
    GtkTreeSelection *tree_selection;
    GtkTreeIter iter;
    gint origin;

    tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (tree_selection), &model, &iter))
		return;

    menu = gtk_menu_new ();

	origin = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tree_view), "origin"));
    if (origin == SPP_ORIGIN_SIMULATOR)
    {
        if (page_num == 0)
        {
            menu_item = utils_menu_item_new_from_image_label ("gsb-ac-liability-16.png",
															  _("View amortization table"));
            g_signal_connect (G_OBJECT (menu_item),
							  "activate",
							  G_CALLBACK (bet_finance_fill_amortization_array),
							  tree_selection);
        }
        else
        {
            menu_item = utils_menu_item_new_from_image_label ("gsb-ac-liability-16.png",
															  _("View credits simulator"));
            g_signal_connect (G_OBJECT (menu_item),
							  "activate",
							  G_CALLBACK (bet_finance_ui_switch_simulator_page),
							  NULL);
        }
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		/* Separator */
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());

    }
    else if (origin == SPP_ORIGIN_FINANCE)
    {
        gboolean amortization_initial_date;

        amortization_initial_date = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tree_view),
																		"amortization_initial_date"));

        if (amortization_initial_date)
            menu_item = utils_menu_item_new_from_image_label ("gsb-ac-liability-16.png",
															  _("Show amortization schedule to date"));
        else
            menu_item = utils_menu_item_new_from_image_label ("gsb-ac-liability-16.png",
															  _("Show amortization schedule from the beginning"));

        g_signal_connect (G_OBJECT (menu_item),
						  "activate",
						  G_CALLBACK (bet_finance_ui_switch_amortization_initial_date),
						  tree_view);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Separator */
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());

    }
    /* Print list */
    menu_item = utils_menu_item_new_from_image_label ("gtk-print-16.png", _("Print the array"));
    g_signal_connect (G_OBJECT (menu_item),
					  "activate",
					  G_CALLBACK (print_tree_view_list),
					  tree_view);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Export list */
    menu_item = utils_menu_item_new_from_image_label ("gsb-export-16.png", _("Export the array"));
    g_signal_connect (G_OBJECT (menu_item),
					  "activate",
					  G_CALLBACK (bet_finance_ui_export_tab),
					  tree_view);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Finish all. */
    gtk_widget_show_all (menu);

	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
}

/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 * \param page
 *
 * \return FALSE
 **/
static gboolean bet_finance_ui_data_list_button_press (GtkWidget *tree_view,
													   GdkEventButton *ev,
													   GtkWidget *page)
{
    /* show the popup */
    if (ev->button == RIGHT_BUTTON)
    {
        gint page_num;

        page_num = gtk_notebook_page_num (GTK_NOTEBOOK (finance_notebook), page);
        bet_finance_ui_data_list_context_menu (tree_view, page_num);
    }

    return FALSE;
}

/**
 * Create the amortization page
 *
 * \param
 *
 * \return
 **/
static GtkWidget *bet_finance_ui_create_amortization_page (void)
{
    GtkWidget *page;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *label_title;
    GtkWidget *label;
    GtkWidget *tree_view;

    page = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);

    /* frame pour la barre d'outils */
    frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (page), frame, FALSE, FALSE, 0);

    /* titre de la page */
    label_title = gtk_label_new (_("Amortization Table"));
	gtk_widget_set_halign (label_title, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (page), label_title, FALSE, FALSE, MARGIN_BOX);

    /* Choix des données sources */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
	gtk_widget_set_halign (hbox, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (page), hbox, FALSE, FALSE, 5);

    /* capital */
    label = gtk_label_new (_("Loan amount: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, MARGIN_BOX);

    label = gtk_label_new (NULL);
    g_object_set_data (G_OBJECT (page), "capital", label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, MARGIN_BOX);

    /* taux */
    label = gtk_label_new (_("Annuel rate interest: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, MARGIN_BOX);

    label = gtk_label_new (NULL);
    g_object_set_data (G_OBJECT (page), "bet_finance_taux", label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    label = gtk_label_new (_("%"));
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);

    /* Duration */
    label = gtk_label_new (_("Duration: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, MARGIN_BOX);

    label = gtk_label_new (NULL);
    g_object_set_data (G_OBJECT (page), "duree", label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    /* création de la liste des données */
    tree_view = bet_finance_ui_create_amortization_tree_view (page, SPP_ORIGIN_SIMULATOR);
    g_object_set_data (G_OBJECT (tree_view), "origin", GINT_TO_POINTER (SPP_ORIGIN_SIMULATOR));
    g_object_set_data (G_OBJECT (page), "tree_view", tree_view);
    g_object_set_data (G_OBJECT (tree_view), "label_title", label_title);

    /* set the color of selected row */
	gtk_widget_set_name (tree_view, "tree_view");

    /* on y ajoute la barre d'outils */
    amortization_toolbar = bet_finance_ui_create_simulator_toolbar (page, tree_view, FALSE, TRUE);
    gtk_container_add (GTK_CONTAINER (frame), amortization_toolbar);

    gtk_widget_show_all (page);

    return page;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *bet_finance_ui_create_data_tree_view (GtkWidget *container)
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkWidget *button;
    GtkTreeStore *tree_model;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    gchar *title;

    tree_view = gtk_tree_view_new ();

    /* Create the tree store */
    tree_model = gtk_tree_store_new (BET_FINANCE_NBRE_COLUMNS,
									 G_TYPE_STRING,		/* BET_FINANCE_DURATION_COLUMN        */
									 G_TYPE_INT,		/* BET_FINANCE_NBRE_ECHEANCE_COLUMN   */
									 G_TYPE_INT,		/* BET_FINANCE_DEVISE_COLUMN          */
									 G_TYPE_STRING,		/* BET_FINANCE_CAPITAL_COLUMN         */
									 G_TYPE_DOUBLE,		/* BET_FINANCE_CAPITAL_DOUBLE         */
									 G_TYPE_STRING,		/* BET_FINANCE_TAUX_COLUMN            */
									 G_TYPE_DOUBLE,		/* BET_FINANCE_TAUX_DOUBLE            */
									 G_TYPE_STRING,		/* BET_FINANCE_HORS_FRAIS_COLUMN      */
									 G_TYPE_DOUBLE,		/* BET_FINANCE_HORS_FRAIS_DOUBLE      */
									 G_TYPE_STRING,		/* BET_FINANCE_FRAIS_COLUMN           */
									 G_TYPE_DOUBLE,		/* BET_FINANCE_FRAIS_DOUBLE           */
									 G_TYPE_STRING,		/* BET_FINANCE_ECHEANCE_COLUMN        */
									 G_TYPE_DOUBLE,		/* BET_FINANCE_ECHEANCE_DOUBLE        */
									 G_TYPE_STRING,		/* BET_FINANCE_COUT_COLUMN            */
									 G_TYPE_DOUBLE,		/* BET_FINANCE_COUT_DOUBLE            */
									 GDK_TYPE_RGBA);	/* BET_FINANCE_BACKGROUND_COLOR       */
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (tree_model));
    g_object_unref (G_OBJECT (tree_model));

    /* create columns */

    /* Duration */
    title = g_strdup (_("Duration"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_FINANCE_DURATION_COLUMN,
													   "cell-background-rgba", BET_FINANCE_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, 0.5);
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_FINANCE_DURATION_COLUMN));
    g_free (title);

    /* Number of periods */
    title = g_strdup (_("Number\nof periods"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_FINANCE_NBRE_ECHEANCE_COLUMN,
													   "cell-background-rgba", BET_FINANCE_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_FINANCE_NBRE_ECHEANCE_COLUMN));
    g_free (title);

    /* Loan capital */
    title = g_strdup (_("Loan\ncapital"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_FINANCE_CAPITAL_COLUMN,
													   "cell-background-rgba", BET_FINANCE_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_FINANCE_CAPITAL_COLUMN));
    g_free (title);

    /* Annuel rate interest */
    title = g_strdup (_("Annuel\nrate interest"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_FINANCE_TAUX_COLUMN,
													   "cell-background-rgba", BET_FINANCE_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_FINANCE_TAUX_COLUMN));
    g_free (title);

    /* Amount without fees */
    title = g_strdup (_("Amount\nwithout fees"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_FINANCE_HORS_FRAIS_COLUMN,
													   "cell-background-rgba", BET_FINANCE_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_FINANCE_HORS_FRAIS_COLUMN));
    g_free (title);

    /* Fees */
    title = g_strdup (_("Fees"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_FINANCE_FRAIS_COLUMN,
													   "cell-background-rgba", BET_FINANCE_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_FINANCE_FRAIS_COLUMN));
    g_free (title);

    /* Monthly paid */
    title = g_strdup (_("Monthly paid"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_FINANCE_ECHEANCE_COLUMN,
													   "cell-background-rgba", BET_FINANCE_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_FINANCE_ECHEANCE_COLUMN));
    g_free (title);

    /* Total cost */
    title = g_strdup (_("Total cost"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_FINANCE_COUT_COLUMN,
													   "cell-background-rgba", BET_FINANCE_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));

    button = gtk_tree_view_column_get_button  (column);
    gtk_widget_set_tooltip_text (GTK_WIDGET (button), _("Approximate cost"));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_FINANCE_COUT_COLUMN));
    g_free (title);

    g_signal_connect (G_OBJECT (tree_view),
					  "button-press-event",
					  G_CALLBACK (bet_finance_ui_data_list_button_press),
					  container);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
									GTK_POLICY_NEVER,
									GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request (scrolled_window, -1, 250);
    gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
    gtk_box_pack_start (GTK_BOX (container), scrolled_window, TRUE, TRUE, 15);

    gtk_widget_show_all (scrolled_window);

    return tree_view;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean bet_finance_ui_duration_button_changed (GtkWidget *combobox,
														GtkWidget *widget)
{
    bet_finance_ui_calculer_clicked (NULL, widget);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *bet_finance_ui_create_duration_widget (GtkWidget *parent)
{
    GtkWidget *combobox;
    const gchar *text_duration [] = {N_("Between 3 and 18 months"),
									 N_("Between 1 and 15 years"),
    								 N_("Between 15 and 30 years"),
    								 NULL};

    combobox = gsb_combo_box_new_with_index (text_duration,
											 G_CALLBACK (bet_finance_ui_duration_button_changed),
											 parent);

    g_signal_handlers_block_by_func (G_OBJECT (combobox),
									 G_CALLBACK (bet_finance_ui_duration_button_changed),
									 parent);
    gsb_combo_box_set_index (combobox, etat.bet_index_duree);
    g_signal_handlers_unblock_by_func (G_OBJECT (combobox),
									   G_CALLBACK (bet_finance_ui_duration_button_changed),
									   parent);

    return combobox;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_finance_ui_spin_button_fees_changed (GtkSpinButton *spinbutton,
													 GtkWidget *page)
{
    etat.bet_frais = gtk_spin_button_get_value (GTK_SPIN_BUTTON (spinbutton));
    bet_finance_ui_calculer_clicked (NULL, page);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_finance_ui_spin_button_taux_changed (GtkSpinButton *spinbutton,
													 GtkWidget *page)
{
    etat.bet_taux_annuel = gtk_spin_button_get_value (GTK_SPIN_BUTTON (spinbutton));
    bet_finance_ui_calculer_clicked (NULL, page);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void bet_finance_ui_type_taux_changed (GtkWidget *togglebutton,
											  GdkEventButton *event,
											  GtkWidget *widget)
{
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);
    etat.bet_type_taux = !etat.bet_type_taux;
    bet_finance_ui_calculer_clicked (NULL, widget);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void bet_finance_ui_create_saisie_widget (GtkWidget *page,
												 GtkWidget *grid)
{
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *spin_button = NULL;
    GtkWidget *button_1, *button_2;
    gchar *tmp_str;

    /* taux */
    label = gtk_label_new (_("Annual interest: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1 ,1);

    spin_button = gtk_spin_button_new_with_range (0.0, 100,
                        bet_data_finance_get_bet_taux_step (BET_TAUX_DIGITS));
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin_button), etat.bet_taux_annuel);
    g_object_set_data (G_OBJECT (page), "taux", spin_button);
	gtk_grid_attach (GTK_GRID (grid), spin_button, 1, 1, 1 ,1);
    g_signal_connect (spin_button,
					  "value-changed",
					  G_CALLBACK (bet_finance_ui_spin_button_taux_changed),
					  page);

    label = gtk_label_new (_("%"));
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 2, 1, 1 ,1);

    /* Frais */
    label = gtk_label_new (_("Fees: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_grid_attach (GTK_GRID (grid), label, 3, 1, 1 ,1);

    spin_button = gtk_spin_button_new_with_range (0.0,
												  100,
												  bet_data_finance_get_bet_taux_step (BET_TAUX_DIGITS));
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin_button), etat.bet_frais);
    g_object_set_data (G_OBJECT (page), "frais", spin_button);
	gtk_grid_attach (GTK_GRID (grid), spin_button, 4, 1, 1 ,1);
    g_signal_connect (spin_button,
					  "value-changed",
					  G_CALLBACK (bet_finance_ui_spin_button_fees_changed),
					  page);

    tmp_str = g_strconcat (_("%"), _(" of capital"), NULL);
    label = gtk_label_new (tmp_str);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_grid_attach (GTK_GRID (grid), label, 5, 1, 1 ,1);
    g_free (tmp_str);

    /* Type de taux */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
	gtk_widget_set_halign (hbox, GTK_ALIGN_CENTER);
    gtk_grid_attach (GTK_GRID (grid), hbox, 0, 2, 6 ,1);

    label = gtk_label_new (_("Rate Type: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, MARGIN_BOX);
    button_1 = gtk_radio_button_new_with_label (NULL, _("CAGR"));

    button_2 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (button_1),
															_("Proportional rate"));
    g_object_set_data (G_OBJECT (page), "type_taux", button_2);

    if (etat.bet_type_taux)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_2), TRUE);
    else
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_1), TRUE);

    gtk_box_pack_start (GTK_BOX (hbox), button_1, FALSE, FALSE, MARGIN_BOX) ;
    gtk_box_pack_start (GTK_BOX (hbox), button_2, FALSE, FALSE, MARGIN_BOX) ;
    g_signal_connect (button_1,
					  "button-release-event",
					  G_CALLBACK (bet_finance_ui_type_taux_changed),
					  page);
    g_signal_connect (button_2,
					  "button-release-event",
					  G_CALLBACK (bet_finance_ui_type_taux_changed),
					  page);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_finance_ui_currency_changed (GtkComboBox *combo_box,
											 GtkWidget *page)
{
    etat.bet_currency = gsb_currency_get_currency_from_combobox (GTK_WIDGET (combo_box));
    bet_finance_ui_calculer_clicked (NULL, page);
}

/**
 * Create the simulator page
 *
 * \param
 *
 * \return
 **/
static GtkWidget *bet_finance_ui_create_simulator_page (void)
{
    GtkWidget *page;
    GtkWidget *frame;
    GtkWidget *widget;
    GtkWidget *grid;
    GtkWidget *label_title;
    GtkWidget *label;
    GtkWidget *tree_view;
    gchar *str_capital;

    page = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);

    /* frame pour la barre d'outils */
    frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (page), frame, FALSE, FALSE, 0);

    /* titre de la page */
    label_title = gtk_label_new (_("Credits simulator"));
	gtk_widget_set_halign (label_title, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (page), label_title, FALSE, FALSE, MARGIN_BOX);

    /* Choix des données sources */
	grid = gtk_grid_new ();
	gtk_grid_set_column_spacing (GTK_GRID (grid), MARGIN_BOX);
	gtk_grid_set_row_spacing (GTK_GRID (grid), MARGIN_BOX);
	gtk_widget_set_margin_start (grid, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (page), grid, FALSE, FALSE, MARGIN_BOX);

    /* capital */
    label = gtk_label_new (_("Loan capital: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1 ,1);

    str_capital = utils_real_get_string_with_currency (gsb_real_double_to_real (etat.bet_capital),
													   etat.bet_currency,
													   FALSE);

    widget = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (widget), str_capital);
    g_object_set_data (G_OBJECT (page), "capital", widget);
	gtk_grid_attach (GTK_GRID (grid), widget, 1, 0, 1 ,1);
    g_signal_connect (G_OBJECT (widget),
					  "changed",
					  G_CALLBACK (bet_finance_ui_capital_entry_changed),
					  page);
    g_signal_connect (G_OBJECT (widget),
					  "key-press-event",
					  G_CALLBACK (bet_finance_ui_capital_entry_key_press_event),
					  page);

    g_free (str_capital);

    /* Set the devises */
    widget = gsb_currency_make_combobox (FALSE);
    gsb_currency_set_combobox_history (widget, etat.bet_currency);
    g_object_set_data (G_OBJECT (page), "devise", widget);
	gtk_grid_attach (GTK_GRID (grid), widget, 2, 0, 1 ,1);
    g_signal_connect (G_OBJECT ( widget),
					  "changed",
					  G_CALLBACK (bet_finance_ui_currency_changed),
					  page);

    /* Duration */
    label = gtk_label_new (_("Duration: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_CENTER);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_grid_attach (GTK_GRID (grid), label, 3, 0, 1 ,1);

    widget = bet_finance_ui_create_duration_widget (page);
    g_object_set_data (G_OBJECT (page), "duree", widget);
	gtk_grid_attach (GTK_GRID (grid), widget, 4, 0, 1 ,1);

    /* création du widget saisie détaillée */
	bet_finance_ui_create_saisie_widget (page, grid);

    /* création de la liste des données */
    tree_view = bet_finance_ui_create_data_tree_view (page);
    g_object_set_data (G_OBJECT (tree_view), "origin", GINT_TO_POINTER (SPP_ORIGIN_SIMULATOR));
    g_object_set_data (G_OBJECT (page), "tree_view", tree_view);
    g_object_set_data (G_OBJECT (tree_view), "label_title", label_title);

    /* set the color of selected row */
	gtk_widget_set_name (tree_view, "colorized_tree_view");

    /* on y ajoute la barre d'outils */
    simulator_toolbar = bet_finance_ui_create_simulator_toolbar (page, tree_view, TRUE, FALSE);
    gtk_container_add (GTK_CONTAINER (frame), simulator_toolbar);

    gtk_widget_show_all (page);

    return page;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static GtkTreeViewColumn *bet_finance_ui_get_column_by_name (GtkWidget *tree_view,
															 const gchar *name)
{
	GList *list;

	list = gtk_tree_view_get_columns (GTK_TREE_VIEW (tree_view));
	while (list)
	{
		GtkTreeViewColumn *col;
		gchar *tmp_str;

		col = list->data;
		tmp_str = g_object_get_data (G_OBJECT (col), "name");
		if (g_strcmp0 (tmp_str, name) == 0)
		{
			g_list_free (list);

			return col;
		}

		list = list->next;
	}
	g_list_free (list);

	return NULL;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_finance_ui_invers_cols_cap_ech_amortization (GtkWidget *tree_view,
															 gboolean invers_cols_cap_ech)
{
	GtkTreeViewColumn *capital_du_col;
	GtkTreeViewColumn *date_col;
	GtkTreeViewColumn *echeance_col;
	GtkTreeViewColumn *frais_col;

	date_col = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), 1);
	frais_col = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), 5);
	capital_du_col = bet_finance_ui_get_column_by_name (tree_view, "CapitalDu");
	echeance_col = bet_finance_ui_get_column_by_name (tree_view, "Echeance");

	if (invers_cols_cap_ech)
	{
		gtk_tree_view_move_column_after (GTK_TREE_VIEW (tree_view),
										 echeance_col,
										 date_col);
		gtk_tree_view_move_column_after (GTK_TREE_VIEW (tree_view),
										 capital_du_col,
										 frais_col);
	}
	else
	{
		gtk_tree_view_move_column_after (GTK_TREE_VIEW (tree_view),
										 capital_du_col,
										 date_col);
		gtk_tree_view_move_column_after (GTK_TREE_VIEW (tree_view),
										 echeance_col,
										 frais_col);
	}
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * Create the loan simulator page
 *
 * \param
 *
 * \return
 **/
GtkWidget *bet_finance_ui_create_loan_simulator (void)
{
    GtkWidget *page;

    devel_debug (NULL);

    /* create a notebook for simulator and array of Amortization */
    finance_notebook = gtk_notebook_new ();
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (finance_notebook), FALSE);
    gtk_widget_show (finance_notebook);

    /* create the simulator page */
    page = bet_finance_ui_create_simulator_page ();
    gtk_notebook_append_page (GTK_NOTEBOOK (finance_notebook), page, NULL);

    /* create the array page */
    page = bet_finance_ui_create_amortization_page ();
    gtk_notebook_append_page (GTK_NOTEBOOK (finance_notebook), page, NULL);

    return finance_notebook;
}

/**
 * Switch to the simulator page
 *
 * \param
 *
 * \return
 **/
void bet_finance_ui_switch_simulator_page (void)
{
    GtkWidget *page;

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (finance_notebook), 0);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (finance_notebook), 0);
    bet_finance_ui_calculer_clicked (NULL, page);
}

/**
 * Create the account amortization page
 *
 * \param
 *
 * \return
 **/
GtkWidget *bet_finance_ui_create_account_amortization_page (void)
{
    GtkWidget *page;
    GtkWidget *frame;
    GtkWidget *label_title;
    GtkWidget *tree_view;
    GtkWidget *account_page;

    devel_debug (NULL);
    account_page = grisbi_win_get_account_page ();

    /* frame pour la barre d'outils */
	page = bet_finance_ui_create_amortization_heading (account_page);

    /* titre de la page */
	label_title = g_object_get_data (G_OBJECT (account_page), "label_title");

    /* création de la liste des données */
    tree_view = bet_finance_ui_create_amortization_tree_view (page, SPP_ORIGIN_FINANCE);
    g_object_set_data (G_OBJECT (tree_view), "origin", GINT_TO_POINTER (SPP_ORIGIN_FINANCE));
    g_object_set_data (G_OBJECT (account_page), "bet_finance_tree_view", tree_view);
    g_object_set_data (G_OBJECT (tree_view), "label_title", label_title);

    /* set the color of selected row */
	gtk_widget_set_name (tree_view, "colorized_tree_view");

    /* on y ajoute la barre d'outils */
	frame = g_object_get_data (G_OBJECT (account_page), "frame");
    account_toolbar = bet_finance_ui_create_simulator_toolbar (page, tree_view, FALSE, FALSE);
    gtk_container_add (GTK_CONTAINER (frame), account_toolbar);

    gtk_widget_show_all (page);

    return page;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *bet_finance_ui_create_amortization_heading (GtkWidget *parent)
{
    GtkWidget *heading;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *label_title;
    GtkWidget *label;

    heading = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);

    /* frame pour la barre d'outils */
    frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (heading), frame, FALSE, FALSE, 0);

    /* titre de la heading */
    label_title = gtk_label_new (_("Amortization Table"));
	gtk_widget_set_halign (label_title, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (heading), label_title, FALSE, FALSE, 5);
    g_object_set_data (G_OBJECT (parent),
					   "bet_finance_amortization_title",
					   label_title);

    /* Choix des données sources */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
	gtk_widget_set_halign (hbox, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (heading), hbox, FALSE, FALSE, 5);

    /* capital */
    label = gtk_label_new (_("Loan amount: "));
    utils_labels_set_alignment (GTK_LABEL (label), 0, 0.5);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);

    label = gtk_label_new (NULL);
    g_object_set_data (G_OBJECT (parent), "bet_finance_capital", label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);

    /* taux */
    label = gtk_label_new (_("Annuel rate interest: "));
    utils_labels_set_alignment (GTK_LABEL (label), 0, 0.5);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);

    label = gtk_label_new (NULL);
    g_object_set_data (G_OBJECT (parent), "bet_finance_taux", label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    label = gtk_label_new (_("%"));
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);

    /* Duration */
    label = gtk_label_new (_("Duration: "));
    utils_labels_set_alignment (GTK_LABEL (label), 0, 0.5);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);

    label = gtk_label_new (NULL);
    g_object_set_data (G_OBJECT (parent), "bet_finance_duree", label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	g_object_set_data (G_OBJECT (parent), "frame", frame);
	g_object_set_data (G_OBJECT (parent), "label_title", label_title);

	gtk_widget_show_all (heading);

	return heading;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
GtkWidget *bet_finance_ui_create_amortization_tree_view (GtkWidget *container,
														 gint origin)
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkTreeStore *tree_model;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    gchar *title;

    tree_view = gtk_tree_view_new ();
    g_object_set_data (G_OBJECT (tree_view), "amortization_initial_date", GINT_TO_POINTER (FALSE));

    /* Create the tree store */
    tree_model = gtk_tree_store_new (BET_AMORTIZATION_NBRE_COLUMNS,
									 G_TYPE_INT,         /* BET_AMORTIZATION_NUMBER_COLUMN       */
									 G_TYPE_STRING,      /* BET_AMORTIZATION_DATE_COLUMN         */
									 G_TYPE_STRING,      /* BET_AMORTIZATION_CAPITAL_DU_COLUMN,  */
									 G_TYPE_STRING,      /* BET_AMORTIZATION_INTERETS_COLUMN     */
									 G_TYPE_STRING,      /* BET_AMORTIZATION_PRINCIPAL_COLUMN    */
									 G_TYPE_STRING,      /* BET_AMORTIZATION_FRAIS_COLUMN        */
									 G_TYPE_STRING,      /* BET_AMORTIZATION_ECHEANCE_COLUMN     */
									 GDK_TYPE_RGBA);    /* BET_AMORTIZATION_BACKGROUND_COLOR    */
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (tree_model));
    g_object_unref (G_OBJECT (tree_model));

    /* create columns */
    /* numéro ou date de l'échéance */
	title = g_strdup (_("Number"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_AMORTIZATION_NUMBER_COLUMN,
													   "cell-background-rgba", BET_AMORTIZATION_BACKGROUND_COLOR,
													   NULL);
	g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_AMORTIZATION_NUMBER_COLUMN));
	g_object_set_data (G_OBJECT (column), "name", (gchar*)"NumEch");
    g_free (title);

    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);

    /* date de l'échéance si nécessaire */
	if ( origin == SPP_ORIGIN_FINANCE )
    {
		title = g_strdup (_("Date"));
		cell = gtk_cell_renderer_text_new ();
		g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

		column = gtk_tree_view_column_new_with_attributes (title,
														   cell,
														   "text", BET_AMORTIZATION_DATE_COLUMN,
														   "cell-background-rgba", BET_AMORTIZATION_BACKGROUND_COLOR,
														   NULL);
		g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_AMORTIZATION_DATE_COLUMN));
		g_object_set_data (G_OBJECT (column), "name", (gchar*)"Date");
		g_free (title);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
		gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
		gtk_tree_view_column_set_resizable (column, TRUE);
		gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
	}

	/* Inversion des colonnes Capital_du et echéance fix Gtk-WARNING **: 13:32:11.757: Negative content height -1 */
	if (origin == SPP_ORIGIN_INVERSE_FINANCE)
	{
		/* Monthly paid */
		title = g_strdup (_("Monthly paid"));
		cell = gtk_cell_renderer_text_new ();
		g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

		column = gtk_tree_view_column_new_with_attributes (title,
														   cell,
														   "text", BET_AMORTIZATION_ECHEANCE_COLUMN,
														   "cell-background-rgba", BET_AMORTIZATION_BACKGROUND_COLOR,
														   NULL);
		g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_AMORTIZATION_ECHEANCE_COLUMN));
		g_object_set_data (G_OBJECT (column), "name", (gchar*)"Echeance");
		g_free (title);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
		gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
		gtk_tree_view_column_set_resizable (column, TRUE);
		gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
	}
	else
	{
		/* Capital restant dû */
		title = g_strdup (_("Capital remaining"));
		cell = gtk_cell_renderer_text_new ();
		g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

		column = gtk_tree_view_column_new_with_attributes (title,
															   cell,
															   "text", BET_AMORTIZATION_CAPITAL_DU_COLUMN,
															   "cell-background-rgba", BET_AMORTIZATION_BACKGROUND_COLOR,
															   NULL);
		g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_AMORTIZATION_CAPITAL_DU_COLUMN));
		g_object_set_data (G_OBJECT (column), "name", (gchar*)"CapitalDu");
		g_free (title);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
		gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
		gtk_tree_view_column_set_resizable (column, TRUE);
		gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
	}

    /* Interests */
    title = g_strdup (_("Capital repaid"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_AMORTIZATION_PRINCIPAL_COLUMN,
													   "cell-background-rgba", BET_AMORTIZATION_BACKGROUND_COLOR,
													   NULL);
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_AMORTIZATION_PRINCIPAL_COLUMN));
	g_object_set_data (G_OBJECT (column), "name", (gchar*)"Principal");
    g_free (title);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);

    /* Capital repaid */
    title = g_strdup (_("Interests"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_AMORTIZATION_INTERETS_COLUMN,
													   "cell-background-rgba", BET_AMORTIZATION_BACKGROUND_COLOR,
													   NULL);
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_AMORTIZATION_INTERETS_COLUMN));
	g_object_set_data (G_OBJECT (column), "name", (gchar*)"Interests");
    g_free (title);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);

    /* Fees*/
    title = g_strdup (_("Fees"));
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

    column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", BET_AMORTIZATION_FRAIS_COLUMN,
													   "cell-background-rgba", BET_AMORTIZATION_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_AMORTIZATION_FRAIS_COLUMN));
	g_object_set_data (G_OBJECT (column), "name", (gchar*)"Fees");
    g_free (title);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);

	if (origin == SPP_ORIGIN_INVERSE_FINANCE)
	{
		/* Capital restant dû */
		title = g_strdup (_("Capital remaining"));
		cell = gtk_cell_renderer_text_new ();
		g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

		column = gtk_tree_view_column_new_with_attributes (title,
															   cell,
															   "text", BET_AMORTIZATION_CAPITAL_DU_COLUMN,
															   "cell-background-rgba", BET_AMORTIZATION_BACKGROUND_COLOR,
															   NULL);
		g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_AMORTIZATION_CAPITAL_DU_COLUMN));
		g_object_set_data (G_OBJECT (column), "name", (gchar*)"CapitalDu");
		g_free (title);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
		gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
		gtk_tree_view_column_set_resizable (column, TRUE);
		gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
	}
	else
	{
		/* Monthly paid */
		title = g_strdup (_("Monthly paid"));
		cell = gtk_cell_renderer_text_new ();
		g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);

		column = gtk_tree_view_column_new_with_attributes (title,
														   cell,
														   "text", BET_AMORTIZATION_ECHEANCE_COLUMN,
														   "cell-background-rgba", BET_AMORTIZATION_BACKGROUND_COLOR,
														   NULL);
		g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (BET_AMORTIZATION_ECHEANCE_COLUMN));
		g_object_set_data (G_OBJECT (column), "name", (gchar*)"Echeance");
		g_free (title);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
		gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
		gtk_tree_view_column_set_resizable (column, TRUE);
		gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
	}

    g_signal_connect (G_OBJECT (tree_view),
					  "button-press-event",
					  G_CALLBACK (bet_finance_ui_data_list_button_press),
					  container);

    /* create the scrolled window */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
									GTK_POLICY_NEVER,
									GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request (scrolled_window, -1, 250);
    gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
    gtk_box_pack_start (GTK_BOX (container), scrolled_window, TRUE, TRUE, MARGIN_PADDING_BOX);

    gtk_widget_show_all (scrolled_window);

    return tree_view;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void bet_finance_update_amortization_tab (gint account_number)
{
    GtkWidget *account_page;
	LoanStruct *s_loan = NULL;

/*     devel_debug (NULL);  */
	if (gsb_gui_navigation_get_current_account () != account_number)
		return;

	s_loan = (LoanStruct *) bet_data_loan_get_last_loan_struct_by_account (account_number);
	if (!s_loan)
		return;

    /* récupère le tableau d'amortissement */
	account_page = grisbi_win_get_account_page ();
	bet_finance_update_amortization_tab_with_data (account_number, account_page, s_loan);
}

/**
 * Création de la barre d'outils pour les crédits
 *
 * \param parent
 * \param toolbar
 * \param tree_view
 * \param simulator     TRUE si page de simulation
 * \param               TRUE si tableau d'amortissement
 *
 * \return
 **/
GtkWidget *bet_finance_ui_create_simulator_toolbar (GtkWidget *parent,
													GtkWidget *tree_view,
													gboolean simulator,
													gboolean amortization)
{
    GtkWidget *toolbar;
    GtkTreeSelection *selection;
    GtkToolItem *item;

    toolbar = gtk_toolbar_new ();
    g_object_set_data (G_OBJECT (toolbar), "tree_view", tree_view);
    g_object_set_data (G_OBJECT (toolbar), "page", parent);

    if (simulator)
    {
        /* création du bouton afficher le tableau d'amortissement */
        item = utils_buttons_tool_button_new_from_image_label ("gsb-ac-liability-24.png", _("Amortization"));
        gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("View amortization table"));
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
        g_signal_connect (G_OBJECT (item),
						  "clicked",
						  G_CALLBACK (bet_finance_fill_amortization_array),
						  selection);
        gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
    }
    else if (amortization)
    {
        /* création du bouton afficher le simulateur de crédits */
        item = utils_buttons_tool_button_new_from_image_label ("gsb-ac-liability-24.png", _("Credits"));
        gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("View credits simulator"));
        g_signal_connect (G_OBJECT (item),
						  "clicked",
						  G_CALLBACK (bet_finance_ui_switch_simulator_page),
						  NULL);
        gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
    }
    else
    {
        gboolean amortization_initial_date = FALSE;

        /* création du bouton afficher le simulateur de crédits */
        item = utils_buttons_tool_button_new_from_image_label ("gsb-ac-liability-24.png", _("Start date"));
        gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Show amortization schedule from the beginning"));
        g_object_set_data (G_OBJECT (tree_view), "amortization_initial_date_button", item);
        g_signal_connect (G_OBJECT (item),
						  "clicked",
						  G_CALLBACK (bet_finance_ui_switch_amortization_initial_date),
						  tree_view);
        gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
        g_object_set_data (G_OBJECT (tree_view),
						   "amortization_initial_date",
						   GINT_TO_POINTER (amortization_initial_date));
    }

    /* création du bouton print */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-print-24.png", _("Print"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Print the array"));
    g_signal_connect (G_OBJECT (item),
					  "clicked",
					  G_CALLBACK (print_tree_view_list),
					  tree_view);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* Export button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-export-24.png", _("Export"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Export the array"));
    g_signal_connect (G_OBJECT (item),
					  "clicked",
					  G_CALLBACK (bet_finance_ui_export_tab),
					  tree_view);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* return value */
    return toolbar;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *bet_finance_ui_get_capital_entry (void)
{
    GtkWidget *page;
    GtkWidget *entry;

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (finance_notebook), 0);
    entry = g_object_get_data (G_OBJECT (page), "capital");

    return entry;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void bet_finance_ui_update_all_finance_toolbars (gint toolbar_style)
{
    gtk_toolbar_set_style (GTK_TOOLBAR (account_toolbar), toolbar_style);
    gtk_toolbar_set_style (GTK_TOOLBAR (amortization_toolbar), toolbar_style);
    gtk_toolbar_set_style (GTK_TOOLBAR (simulator_toolbar), toolbar_style);
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
GsbReal bet_finance_get_loan_amount_at_date (gint scheduled_number,
											 gint transfer_account,
											 const GDate *date,
											 gboolean maj_s_loan_capital_du)
{
	gint transaction_mother = 0;
	static gint mother_number = 0;
	static AmortissementStruct *s_amortissement;
	GsbReal amount = {0, 0};

	devel_debug_int (scheduled_number);
	if (gsb_data_scheduled_get_split_of_scheduled (scheduled_number))
	{
		LoanStruct *s_loan;

		s_loan = bet_data_loan_get_last_loan_struct_by_account (transfer_account);
		if (!s_loan)
			return amount;

		s_amortissement = bet_finance_get_echeance_at_date (s_loan, date, maj_s_loan_capital_du);
		if (!s_amortissement)
			return amount;

		mother_number = scheduled_number;
		amount = gsb_real_opposite (gsb_real_double_to_real (s_amortissement->echeance));
	}
	else
	{
		transaction_mother = gsb_data_scheduled_get_mother_scheduled_number (scheduled_number);
//~ printf ("transaction_mother = %d mother_number = %d\n", transaction_mother, mother_number);
		if (transaction_mother == mother_number)
		{
			if (scheduled_number == mother_number+1 && (s_amortissement && s_amortissement->principal))
				amount = gsb_real_opposite (gsb_real_double_to_real (s_amortissement->principal));
			else if (scheduled_number == mother_number+2  && (s_amortissement && s_amortissement->interets))
				amount = gsb_real_opposite (gsb_real_double_to_real (s_amortissement->interets));
			else if (scheduled_number == mother_number+3  && (s_amortissement && s_amortissement->frais))
				amount = gsb_real_opposite (gsb_real_double_to_real (s_amortissement->frais));
//~ printf ("amount = %s\n", utils_real_get_string (amount));
		}
	}

	return amount;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
AmortissementStruct *bet_finance_get_echeance_first (LoanStruct *s_loan,
													 GDate *first_date)
{
	gdouble taux_periodique;
	AmortissementStruct *s_amortissement = NULL;
	gchar *tmp_str;

	tmp_str = gsb_format_gdate (first_date);
	devel_debug (tmp_str);
	g_free(tmp_str);
	taux_periodique = bet_data_finance_get_taux_periodique (s_loan->annual_rate, s_loan->type_taux);
	s_amortissement = bet_data_finance_structure_amortissement_init ();
	if (s_loan->first_is_different)
	{
		s_amortissement->echeance = s_loan->first_capital;
		s_amortissement->echeance += s_loan->amount_fees + s_loan->first_interests;
		s_amortissement->interets = s_loan->first_interests;
		s_amortissement->frais = s_loan->amount_fees;
		s_amortissement->principal = s_loan->first_capital;
		s_loan->other_echeance_amount = bet_data_finance_get_echeance (s_loan->capital,
																	   taux_periodique,
																	   s_loan->duree);
		s_loan->other_echeance_amount += s_loan->amount_fees;
	}
	else
	{
		s_amortissement->echeance = bet_data_finance_get_echeance (s_loan->capital,
																   taux_periodique,
																   s_loan->duree);
		s_amortissement->echeance += s_loan->amount_fees;
		s_amortissement->interets = bet_data_finance_get_interets (s_loan->capital_du,
																   taux_periodique);
		s_amortissement->frais = s_loan->amount_fees;
		s_amortissement->principal = bet_data_finance_get_principal (s_amortissement->echeance,
																	 s_amortissement->interets,
																	 s_amortissement->frais);
		s_loan->other_echeance_amount = s_amortissement->echeance;
	}

	return s_amortissement;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gdouble bet_finance_get_number_from_string (GtkWidget *parent,
											const gchar *name)
{
    GtkWidget *widget;
    gdouble number = 0;

    widget = g_object_get_data (G_OBJECT (parent), name);

    if (GTK_IS_SPIN_BUTTON  (widget))
    {
        number = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
    }
    else if (GTK_IS_ENTRY (widget))
    {
        const gchar *entry;
        gchar *tmp_str;

        entry = gtk_entry_get_text (GTK_ENTRY (widget));

        if (entry && strlen (entry) > 0)
        {
            number = utils_str_strtod (entry, NULL);

            tmp_str = utils_real_get_string_with_currency (gsb_real_double_to_real (number),
														   etat.bet_currency,
														   FALSE);
            gtk_entry_set_text (GTK_ENTRY (widget), tmp_str);
            g_free (tmp_str);
        }
    }

    return number;
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void bet_finance_update_amortization_tab_with_data (gint account_number,
													GtkWidget *parent,
													LoanStruct *s_loan)
{
    GtkWidget *label;
    GtkWidget *tree_view;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreePath *sel_path;
    GDate *date;
    GDate *last_paid_date = NULL;
	GDate *last_installment_paid = NULL;
	gchar *str_capital_du = NULL;
	gchar *str_date = NULL;
    gchar *tmp_str;
	gchar *tmp_str2;
    gint index = 0;
	gint index_last_installment_paid = 0;
    gint nbre_echeances;
	gint origin;
    gint type_taux;
    gdouble taux;
    gdouble taux_periodique;
    gboolean amortization_initial_date;
	gboolean first_echeance = FALSE;
    AmortissementStruct *s_amortissement;

    devel_debug (NULL);

    /* récupère le tableau d'amortissement */
    tree_view = g_object_get_data (G_OBJECT (parent), "bet_finance_tree_view");
	origin = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tree_view), "origin"));
	if (origin != SPP_ORIGIN_CONFIG)
		bet_finance_ui_invers_cols_cap_ech_amortization (tree_view, s_loan->invers_cols_cap_ech);
	amortization_initial_date = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tree_view),
																	"amortization_initial_date"));

    /* remplit le tableau d'amortissement */
    store = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    gtk_tree_store_clear (GTK_TREE_STORE (store));

    /* récupère les paramètres du crédit */
    nbre_echeances = s_loan->duree;

	date = gsb_date_copy (s_loan->first_date);
	if (!date || !g_date_valid (date))
	{
		return;
	}

	str_date = gsb_format_gdate (date);
	if (strlen (str_date) == 0)
	{
		g_free(str_date);
		return;
	}

	/* A ce stade on peut commencer à remplir la structure s_amortissement */
	s_amortissement = bet_data_finance_structure_amortissement_init ();
	s_amortissement->origin = SPP_ORIGIN_FINANCE;

	/* set currency */
	s_amortissement->devise = gsb_data_account_get_currency (account_number);

	/* set date s_amortissement->str_date */
	s_amortissement->str_date = str_date;

	if (amortization_initial_date == FALSE)
	{
		last_paid_date = bet_data_finance_get_date_last_installment_paid (date);

		/* set first line si necessaire */
		if (s_loan->invers_cols_cap_ech)
		{
			str_capital_du = utils_real_get_string_with_currency (gsb_real_double_to_real (s_loan->capital),
																  s_amortissement->devise,
																  TRUE);

			gtk_tree_store_append (GTK_TREE_STORE (store), &iter, NULL);
			gtk_tree_store_set (GTK_TREE_STORE (store),
								&iter,
								BET_AMORTIZATION_NUMBER_COLUMN, 0,
								BET_AMORTIZATION_DATE_COLUMN, "",
								BET_AMORTIZATION_CAPITAL_DU_COLUMN, str_capital_du,
								BET_AMORTIZATION_INTERETS_COLUMN, "",
								BET_AMORTIZATION_PRINCIPAL_COLUMN, "",
								BET_AMORTIZATION_FRAIS_COLUMN, "",
								BET_AMORTIZATION_ECHEANCE_COLUMN, "",
								- 1);
			g_free (str_capital_du);
		}
	}
	else
	{
		last_paid_date = gsb_date_copy (date);
		last_installment_paid = bet_data_finance_get_date_last_installment_paid (date);

		/* set first line si necessaire */
		if (s_loan->invers_cols_cap_ech)
		{
			str_capital_du = utils_real_get_string_with_currency (gsb_real_double_to_real (s_loan->capital),
																  s_amortissement->devise,
																  TRUE);

			gtk_tree_store_append (GTK_TREE_STORE (store), &iter, NULL);
			gtk_tree_store_set (GTK_TREE_STORE (store),
								&iter,
								BET_AMORTIZATION_NUMBER_COLUMN, 0,
								BET_AMORTIZATION_DATE_COLUMN, "",
								BET_AMORTIZATION_CAPITAL_DU_COLUMN, str_capital_du,
								BET_AMORTIZATION_INTERETS_COLUMN, "",
								BET_AMORTIZATION_PRINCIPAL_COLUMN, "",
								BET_AMORTIZATION_FRAIS_COLUMN, "",
								BET_AMORTIZATION_ECHEANCE_COLUMN, "",
								- 1);
			g_free (str_capital_du);
		}
	}

	/* met à jour le titre du tableau */
    label = g_object_get_data (G_OBJECT (parent), "bet_finance_amortization_title");
	tmp_str2 = gsb_format_gdate (last_paid_date);
    tmp_str = g_strconcat (_("Amortization Table"), _(" at "), tmp_str2, NULL);
	g_free(tmp_str2);
    gtk_label_set_label (GTK_LABEL (label), tmp_str);
    g_free (tmp_str);

    /* set capital */
    s_amortissement->capital_du = s_loan->capital;
    label = g_object_get_data (G_OBJECT (parent), "bet_finance_capital");
    tmp_str = utils_real_get_string_with_currency (gsb_real_double_to_real (s_amortissement->capital_du),
												   s_amortissement->devise, TRUE);
    gtk_label_set_label (GTK_LABEL (label), tmp_str);
    g_free (tmp_str);

    /* set taux */
    label = g_object_get_data (G_OBJECT (parent), "bet_finance_taux");
    taux = s_loan->annual_rate;
    type_taux = s_loan->type_taux;
    taux_periodique = bet_data_finance_get_taux_periodique (taux, type_taux);
    tmp_str = utils_str_dtostr (taux, BET_TAUX_DIGITS, FALSE);
    gtk_label_set_label (GTK_LABEL (label), tmp_str);
    g_free (tmp_str);

    /* set duration */
    label = g_object_get_data (G_OBJECT (parent), "bet_finance_duree");
	tmp_str2 = utils_str_itoa (s_loan->duree);
    tmp_str = g_strconcat (tmp_str2, _(" months "), NULL);
	g_free(tmp_str2);
    gtk_label_set_label (GTK_LABEL (label), tmp_str);
    g_free (tmp_str);

    /* set frais */
	if (s_loan->type_taux < 2)
	{
		s_amortissement->frais = s_loan->amount_fees;
		s_amortissement->str_frais = utils_real_get_string_with_currency (gsb_real_double_to_real
																		  (s_amortissement->frais),
																		  s_amortissement->devise,
																		  TRUE);
	}

	/* set first echeance */
	s_amortissement->num_echeance = 1;
	if (s_loan->first_is_different)
	{
		if (s_loan->type_taux == 2)
		{
			s_amortissement->echeance = s_loan->first_capital;
			s_amortissement->interets = s_loan->first_interests;
			s_amortissement->frais = s_loan->first_fees;
			s_amortissement->str_frais = utils_real_get_string_with_currency (gsb_real_double_to_real
																			  (s_amortissement->frais),
																			  s_amortissement->devise,
																			  TRUE);
			s_amortissement->echeance += s_amortissement->frais + s_amortissement->interets;
			s_amortissement->str_echeance = utils_real_get_string_with_currency (gsb_real_double_to_real
																				 (s_amortissement->echeance),
																				 s_amortissement->devise,
																				 TRUE);
			s_amortissement->principal = s_loan->first_capital;
		}
		else
		{
			s_amortissement->echeance = s_loan->first_capital;
			s_amortissement->interets = s_loan->first_interests;
			s_amortissement->echeance += s_amortissement->frais + s_amortissement->interets;
			s_amortissement->str_echeance = utils_real_get_string_with_currency (gsb_real_double_to_real (s_amortissement->echeance),
																				 s_amortissement->devise, TRUE);
			s_amortissement->principal = bet_data_finance_get_principal (s_amortissement->echeance,
																		 s_amortissement->interets,
																		 s_amortissement->frais);
		}

			if (s_loan->invers_cols_cap_ech)
			{
				s_amortissement->capital_du -= s_amortissement->principal;
				if (g_date_compare (date, last_paid_date) >= 0)
					bet_finance_fill_amortization_ligne (store, s_amortissement);
			}
			else
			{
				if (g_date_compare (date, last_paid_date) >= 0)
					bet_finance_fill_amortization_ligne (store, s_amortissement);
				s_amortissement->capital_du -= s_amortissement->principal;
			}
	}
	else
	{
		if (s_loan->type_taux == 2)
		{
			s_amortissement->echeance = s_loan->fixed_due_amount;
			s_amortissement->str_echeance = utils_real_get_string_with_currency (gsb_real_double_to_real
																				 (s_amortissement->echeance),
																				 s_amortissement->devise, TRUE);
			s_amortissement->frais = bet_data_finance_get_fees_for_type_taux_2 (s_amortissement->capital_du,
																				s_loan->percentage_fees);
			s_amortissement->str_frais = utils_real_get_string_with_currency (gsb_real_double_to_real
																			  (s_amortissement->frais),
																			  s_amortissement->devise,
																			  TRUE);
			s_amortissement->interets = bet_data_finance_get_interets (s_amortissement->capital_du,
																	   taux_periodique);
			s_amortissement->principal = bet_data_finance_get_principal (s_amortissement->echeance,
																		 s_amortissement->interets,
																		 s_amortissement->frais);

			if (g_date_compare (date, last_paid_date) >= 0)
				bet_finance_fill_amortization_ligne (store, s_amortissement);
			s_amortissement->capital_du -= s_amortissement->principal;
		}
		else
		{
			s_amortissement->echeance = bet_data_finance_get_echeance (s_amortissement->capital_du,
																	   taux_periodique, nbre_echeances);
			s_amortissement->echeance += s_amortissement->frais;
			s_amortissement->str_echeance = utils_real_get_string_with_currency (gsb_real_double_to_real (s_amortissement->echeance),
																				 s_amortissement->devise, TRUE);
			s_amortissement->interets = bet_data_finance_get_interets (s_amortissement->capital_du,
																	   taux_periodique);
			s_amortissement->principal = bet_data_finance_get_principal (s_amortissement->echeance,
																		 s_amortissement->interets,
																		 s_amortissement->frais);
			if (s_loan->invers_cols_cap_ech)
			{
				s_amortissement->capital_du -= s_amortissement->principal;
				if (g_date_compare (date, last_paid_date) >= 0)
					bet_finance_fill_amortization_ligne (store, s_amortissement);
			}
			else
			{
				if (g_date_compare (date, last_paid_date) >= 0)
					bet_finance_fill_amortization_ligne (store, s_amortissement);
				s_amortissement->capital_du -= s_amortissement->principal;
			}
		}
	}

	date = gsb_date_add_one_month (date, TRUE);
	s_amortissement->str_date = gsb_format_gdate (date);

    /* set the other echeances */
	for (index = 2; index <= nbre_echeances; index++)
	{
		s_amortissement->num_echeance = index;
		s_amortissement->interets = bet_data_finance_get_interets (s_amortissement->capital_du,
																   taux_periodique);
		if (s_loan->type_taux == 2)
		{
			s_amortissement->echeance = s_loan->fixed_due_amount;
			s_amortissement->str_echeance = utils_real_get_string_with_currency (gsb_real_double_to_real
																				 (s_amortissement->echeance),
																				 s_amortissement->devise,
																				 TRUE);
			s_amortissement->frais = bet_data_finance_get_fees_for_type_taux_2 (s_amortissement->capital_du,
																				s_loan->percentage_fees);
			s_amortissement->str_frais = utils_real_get_string_with_currency (gsb_real_double_to_real
																			  (s_amortissement->frais),
																			  s_amortissement->devise,
																			  TRUE);
			if (index == nbre_echeances)
			{
				s_amortissement->echeance = bet_data_finance_get_last_echeance (
							s_amortissement->capital_du,
							s_amortissement->interets,
							s_amortissement->frais);
				g_free (s_amortissement->str_echeance);
				s_amortissement->str_echeance = utils_real_get_string_with_currency (
							gsb_real_double_to_real (s_amortissement->echeance),
							s_amortissement-> devise, TRUE);
				s_amortissement->principal = s_amortissement->capital_du;
			}
			else
			{
				s_amortissement->principal = bet_data_finance_get_principal (s_amortissement->echeance,
																			 s_amortissement->interets,
																			 s_amortissement->frais);
			}

			if (g_date_compare (date, last_paid_date) >= 0)
				bet_finance_fill_amortization_ligne (store, s_amortissement);
			s_amortissement->capital_du -= s_amortissement->principal;
		}
		else
		{
			s_amortissement->num_echeance = index;
			s_amortissement->interets = bet_data_finance_get_interets (s_amortissement->capital_du,
																	   taux_periodique);

			if (index == nbre_echeances)
			{
				s_amortissement->echeance = bet_data_finance_get_last_echeance (
							s_amortissement->capital_du,
							s_amortissement->interets,
							s_amortissement->frais);
				g_free (s_amortissement->str_echeance);
				s_amortissement->str_echeance = utils_real_get_string_with_currency (
							gsb_real_double_to_real (s_amortissement->echeance),
							s_amortissement-> devise, TRUE);
				s_amortissement->principal = s_amortissement->capital_du;
			}
			else
				s_amortissement->principal = bet_data_finance_get_principal (s_amortissement->echeance,
																			 s_amortissement->interets,
																			 s_amortissement->frais);

			if (s_loan->invers_cols_cap_ech)
			{
				s_amortissement->capital_du -= s_amortissement->principal;
				if (g_date_compare (date, last_paid_date) >= 0)
					bet_finance_fill_amortization_ligne (store, s_amortissement);
			}
			else
			{
				if (g_date_compare (date, last_paid_date) >= 0)
					bet_finance_fill_amortization_ligne (store, s_amortissement);
				s_amortissement->capital_du -= s_amortissement->principal;
			}
		}
		if (amortization_initial_date && !first_echeance)
		{
			if (g_date_compare (date, last_installment_paid) >= 0)
			{
				first_echeance = TRUE;
				index_last_installment_paid = index;
			}
		}

        date = gsb_date_add_one_month (date, TRUE);
        s_amortissement->str_date = gsb_format_gdate (date);

//~ if (index < 11)
	//~ printf ("num_echeance = %d capital_du = %f, principal = %f interets = %f\n",
			//~ s_amortissement->num_echeance, s_amortissement->capital_du, s_amortissement->principal, s_amortissement->interets);
	}

    bet_finance_struct_amortization_free (s_amortissement);
    g_date_free (date);
    g_date_free (last_paid_date);

    utils_set_tree_store_background_color (tree_view, BET_AMORTIZATION_BACKGROUND_COLOR);
	path = gtk_tree_path_new_first ();
	if (first_echeance)
	{
		tmp_str = g_strdup_printf ("%d", index_last_installment_paid-1);
		sel_path = gtk_tree_path_new_from_string (tmp_str);
		g_free (tmp_str);
	}
	else
	{
		sel_path = gtk_tree_path_new_first ();
	}

    gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, TRUE, 0.0, 0.0);
	gtk_tree_selection_select_path (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)), sel_path);

    gtk_tree_path_free (path);
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
