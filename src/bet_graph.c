/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2011-2022 Pierre Biava (grisbi@pierre.biava.name)        */
/*                   2011 Guillaume Verger (guillaume.verger@laposte.net)     */
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

#include <gdk/gdk.h>
#include <glib/gi18n.h>
#include <strings.h>

/*START_INCLUDE*/
#include "bet_graph.h"
#include "bet_data.h"
#include "bet_hist.h"
#include "bet_tab.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "widget_bet_graph_options.h"
#include "widget_bet_graph_others.h"
#include "widget_bet_graph_pie.h"
#include "erreur.h"
#include "utils_widgets.h"
/*END_INCLUDE*/

/*START_STATIC*/
/* variables statiques pour les différents types de graphiques */
static BetGraphPrefsStruct *	prefs_prev = NULL;		/* for forecast graph */
static BetGraphPrefsStruct *	prefs_hist = NULL;		/* for monthly graph */

/* mois sous la forme abrégée */
static const gchar *			short_str_months[] =
{
	N_("Jan"), N_("Feb"), N_("Mar"), N_("Apr"),
	N_("May"), N_("Jun"), N_("Jul"), N_("Aug"),
	N_("Sep"), N_("Oct"), N_("Nov"), N_("Dec")
};

/* mois sous la forme longue */
static const gchar *			long_str_months[] =
{
	N_("January"), N_("February"), N_("March"), N_("April"),
	N_("May"), N_("June"), N_("July"), N_("August"),
	N_("September"), N_("October"), N_("November"), N_("December")
};


/*END_STATIC*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
static BetGraphButtonStruct *struct_initialise_bet_graph_button (void)
{
    BetGraphButtonStruct *self;

    self = g_new0 (BetGraphButtonStruct, 1);

   return self;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void struct_free_bet_graph_button (BetGraphButtonStruct *self)
{
	g_free (self->name);
	g_free (self->filename);
	g_free (self->service_id);

	g_free (self);
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
static BetGraphPrefsStruct *struct_initialise_bet_graph_prefs (void)
{
    BetGraphPrefsStruct *self;

    self = g_new0 (BetGraphPrefsStruct, 1);

    self->major_tick_out = TRUE;
    self->major_tick_labeled = TRUE;
    self->position = 2;
    self->new_axis_line = FALSE;
    self->degrees = 90;
    self->gap_spinner = 50;
    self->before_grid = TRUE;

   return self;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_graph_popup_menu_activate (GtkMenuItem *menuitem,
										   BetGraphButtonStruct *self)
{
	GtkWidget *toolbar;
	GtkWidget *tree_view;
	GCallback callback;
	GtkToolItem *item;
	gchar *tmp_str;
	gint nbre_elemnts;
	BetGraphPrefsStruct *prefs;

	prefs = self->prefs;

	/* on définit l'origine du bouton */
	if (self->origin_tab == BET_ONGLETS_PREV)
	{
		toolbar = bet_array_get_toolbar ();
		tmp_str = g_strdup ("forecast_graph");
		nbre_elemnts = gtk_toolbar_get_n_items (GTK_TOOLBAR (toolbar)) -1;
	}
	else
	{
		toolbar = bet_hist_get_toolbar ();
		tmp_str = g_strdup ("hist_graph");
		nbre_elemnts = gtk_toolbar_get_n_items (GTK_TOOLBAR (toolbar)) -1;
	}

	/* on memorise le tree_view  et la fonction callback */
	tree_view = self->tree_view;
	callback = self->callback;

	/* on change le type de graphique type 0 > type 1 ou type 1 > type 0 */
	prefs->type_graph = !prefs->type_graph;

	/* on supprime le bouton existant */
	item = gtk_toolbar_get_nth_item (GTK_TOOLBAR (toolbar), nbre_elemnts);
	if (item)
		gtk_container_remove (GTK_CONTAINER (toolbar), GTK_WIDGET (item));

	/* on recrée le bouton */
	item = bet_graph_button_menu_new (toolbar, tmp_str, G_CALLBACK (callback), tree_view);
	g_free (tmp_str);

	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
	gtk_widget_show_all (toolbar);

	/* on memorise le changement */
	gsb_file_set_modified (TRUE);

	/* on lance le graphique */
	g_signal_emit_by_name (item, "clicked", tree_view, NULL);
}

/**
 * construit le menu popup du bouton
 *
 * \param button
 * \param
 *
 * \return FALSE
 **/
static void bet_graph_popup_show_menu (GtkWidget *button,
									   GList *liste)
{
	GtkWidget *menu;
	GtkWidget *menu_item;
	GList *tmp_list;

	menu = gtk_menu_new ();

	tmp_list = liste;
	while (tmp_list)
	{
		BetGraphButtonStruct *self;

		self = tmp_list->data;

		menu_item = gtk_menu_item_new_with_label (self->name);

		g_signal_connect (G_OBJECT (menu_item),
						  "activate",
						  G_CALLBACK (bet_graph_popup_menu_activate),
						  self);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		/* on rend insensible le bouton visible */
		gtk_widget_set_sensitive (menu_item, !self->is_visible);

		tmp_list = tmp_list->next;
	}

	gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (button), menu);
	gtk_widget_show_all (menu);
}

/**
 * retourne la date de début pour les graphiques mensuels
 *
 * \param   none
 *
 * \return  date_debut_periode
 **/
static GDate *bet_graph_get_date_debut_periode (void)
{
    GDate *date_debut_periode;
    GDate *date_jour;
    gint fyear_number;

    date_jour = gdate_today ();
    fyear_number = gsb_data_fyear_get_from_date (date_jour);
    if (!fyear_number)
        date_debut_periode = g_date_new_dmy (1, 1, g_date_get_year (date_jour));
    else
        date_debut_periode = gsb_date_copy (gsb_data_fyear_get_beginning_date (fyear_number));

    g_date_free (date_jour);

    return date_debut_periode;
}

static void bet_graph_button_menu_destroy (GtkWidget *button,
										   GList *liste)
{
    GList *tmp_list;

	devel_debug (NULL);
	tmp_list = liste;
	while (tmp_list)
	{
		BetGraphButtonStruct *self;

		self = tmp_list->data;
		struct_free_bet_graph_button (self);

		tmp_list = tmp_list->next;
	}
	g_list_free (liste);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * retourne un bouton pour choisir entre un graph ligne ou colonne
 *
 * \param type_graph    forecast_graph or historical_graph
 * \param callback
 * \paramtree_view
 *
 * \return item
 **/
GtkToolItem *bet_graph_button_menu_new (GtkWidget *toolbar,
										const gchar *type_graph,
										GCallback callback,
										GtkWidget *tree_view)
{
    GtkToolItem *item = NULL;
    GList *liste = NULL;
    gchar *tooltip = NULL;
    gint origin_tab = 0;
    BetGraphButtonStruct *self;
    BetGraphPrefsStruct *prefs = NULL;

    /* initialisation des préférences */
    if (strcmp (type_graph, "forecast_graph") == 0)
    {
        if (prefs_prev == NULL)
            prefs_prev = struct_initialise_bet_graph_prefs ();
        prefs = prefs_prev;
        origin_tab = BET_ONGLETS_PREV;
        tooltip = g_strdup (_("Display the graph of forecast"));
    }
    else if (strcmp (type_graph, "hist_graph") == 0)
    {
        if (prefs_hist == NULL)
            prefs_hist = struct_initialise_bet_graph_prefs ();
        prefs = prefs_hist;
        origin_tab = BET_ONGLETS_HIST;
        tooltip = g_strdup (_("Display the monthly graph"));
    }

    /* initialisation des données du premier bouton */
    self = struct_initialise_bet_graph_button ();

    self->name = g_strdup (_("Column"));
    self->filename = g_strdup ("gsb-graph-histo-24.png");
    self->service_id = g_strdup ("GogBarColPlot");
    self->callback = callback;
    self->tree_view = tree_view;
    self->origin_tab = origin_tab;
    self->prefs = prefs;

    /* si ce boutton est celui par défaut on l'affiche */
    if (prefs && prefs->type_graph == 0)
    {
        self->is_visible = TRUE;
        item = utils_buttons_tool_menu_new_from_image_label (self->filename, self->name);
        self->button = item;
        g_object_set_data (G_OBJECT (self->button), "service_id", self->service_id);
        g_object_set_data (G_OBJECT (self->button), "origin_tab", GINT_TO_POINTER (origin_tab));
        g_signal_connect (G_OBJECT (self->button), "clicked", self->callback, self->tree_view);
    }
    liste = g_list_append (liste, self);

    /* initialisation des données du deuxième bouton */
    self = struct_initialise_bet_graph_button ();

    self->name = g_strdup (_("Line"));
    self->filename = g_strdup ("gsb-graph-line-24.png");
    self->service_id = g_strdup ("GogLinePlot");
    self->callback = callback;
    self->tree_view = tree_view;
    self->origin_tab = origin_tab;
    self->prefs = prefs;

    /* si ce boutton est celui par défaut on l'affiche */
    if (prefs && prefs->type_graph == 1)
    {
        self->is_visible = TRUE;
        item = utils_buttons_tool_menu_new_from_image_label (self->filename, self->name);
        self->button = item;
        g_object_set_data (G_OBJECT (self->button), "service_id", self->service_id);
        g_object_set_data (G_OBJECT (self->button), "origin_tab", GINT_TO_POINTER (origin_tab));
        g_signal_connect (G_OBJECT (self->button), "clicked", self->callback, self->tree_view);
    }
    liste = g_list_append (liste, self);

    if (tooltip)
    {
        gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item), tooltip);
        g_free (tooltip);
    }

	/* set menu */
	gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (item), gtk_menu_new ());

	g_signal_connect (G_OBJECT (item),
					  "show-menu",
					  G_CALLBACK (bet_graph_popup_show_menu),
					  liste);

	g_signal_connect (G_OBJECT (item),
					  "destroy",
					  G_CALLBACK (bet_graph_button_menu_destroy),
					  liste);

    return item;
}

/**
 *
 *
 * \param
 *
 * \return TRUE if OK FALSE otherwise
 **/
gboolean bet_graph_populate_lines_by_forecast_data (BetGraphDataStruct *self)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;
    gdouble prev_montant = 0.0;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->tree_view));
    if (model == NULL)
        return FALSE;

    if (gtk_tree_model_get_iter_first (model, &iter))
    {
        gchar *libelle_axe_x = self->tab_libelle[0];
        gdouble *tab_Y = self->tab_Y;
        gdouble montant = 0.;
        GDate *first_date;
        GDate *last_date;
        GDate *date_courante = NULL;
        GDateDay day_courant = G_DATE_BAD_DAY;
        GDateMonth month_courant = G_DATE_BAD_MONTH;
        gint nbre_iterations = 0;

        do
        {
            gchar *amount;
            gchar *str_date;
            GValue date_value = G_VALUE_INIT;
            GDate *date;
            GDateDay day;
            GDateMonth month;
            gint diff_jours;
            gint i;

			if (G_IS_VALUE (&date_value))
				g_value_init (&date_value, G_TYPE_DATE);

            gtk_tree_model_get_value (model,
									  &iter,
									  SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, &date_value);

            gtk_tree_model_get (GTK_TREE_MODEL(model),
								&iter,
								SPP_ESTIMATE_TREE_AMOUNT_COLUMN, &amount,
								-1);
            date = g_value_get_boxed (&date_value);

            montant += (gdouble) utils_str_strtod ((amount == NULL) ? "0" : amount, NULL);

            if (self->nbre_elemnts == 0)
            {
                /* on ajoute 1 jour pour passer au 1er du mois */
                g_date_add_days (date, 1);

                /* on calcule le nombre maxi d'itération pour une année */
                first_date = gsb_date_copy (date);
                last_date = gsb_date_copy (date);
                g_date_add_years (last_date, 1);
                nbre_iterations = g_date_days_between (first_date, last_date);

                date_courante = gsb_date_copy (date);
                day_courant = g_date_get_day (date);
                month_courant = g_date_get_month (date);

                str_date = gsb_format_gdate (date_courante);
                strncpy (&libelle_axe_x[self->nbre_elemnts * TAILLE_MAX_LIBELLE], str_date, TAILLE_MAX_LIBELLE-1);

                self->nbre_elemnts++;
                g_free (str_date);
                g_date_free (first_date);
                g_date_free (last_date);
            }
            else
            {
                day = g_date_get_day (date);
                month = g_date_get_month (date);
                if (day != day_courant || month != month_courant)
                {
                    /* nombre de jours manquants */
                    diff_jours = g_date_days_between (date_courante, date);
                    for (i = diff_jours; i > 0; i--)
                    {
                        g_date_add_days (date_courante, 1);
                        str_date = gsb_format_gdate (date_courante);

                        strncpy (&libelle_axe_x[self->nbre_elemnts * TAILLE_MAX_LIBELLE], str_date, TAILLE_MAX_LIBELLE-1);
						g_free (str_date);
                        tab_Y[self->nbre_elemnts-1] = prev_montant;
                        self->nbre_elemnts++;

                        /* on dépasse d'un jour pour obtenir le solde du dernier jour */
                        if (self->nbre_elemnts > nbre_iterations)
                        {
                            self->nbre_elemnts = nbre_iterations + 1;
							g_value_unset (&date_value);

							break;
                        }
                    }
                    day_courant = day;
                    if (g_date_is_first_of_month (date))
                        month_courant = g_date_get_month (date);
                }
            }
            prev_montant = montant;

            if (self->nbre_elemnts > nbre_iterations)
            {
                self->nbre_elemnts = nbre_iterations;
                dialogue_hint (_("You can not exceed one year of visualization"), _("Overflow"));
				g_value_unset (&date_value);

                break;
            }
			g_value_unset (&date_value);
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

        tab_Y[self->nbre_elemnts-1] = prev_montant;

        g_date_free (date_courante);

        return TRUE;
    }

    return FALSE;
}

/**
 * récupère les données historiques sélectionnées
 *
 * \param BetGraphDataStruct
 *
 * \return TRUE or FALSE
 **/
gboolean bet_graph_populate_lines_by_hist_line (BetGraphDataStruct *self)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;
    GHashTable *list_transactions;
    GHashTableIter hash_iter;
    gpointer key;
    gpointer value;
    GDate *start_current_fyear;
    GDateMonth date_month = G_DATE_BAD_MONTH;
    GDateMonth today_month = G_DATE_BAD_MONTH;
    gchar *desc;
    gchar *str_amount = NULL;
    gchar *libelle_axe_x = self->tab_libelle[0];
    gdouble *tab_Y = self->tab_Y;
    gdouble *tab_Y2 = self->tab_Y2;
    gint div_number;
    gint sub_div_nb;
    gint fyear_number;
    gint i;
    GsbReal tab[12];
    GsbReal tab2[12];

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->tree_view));

    if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
        return FALSE;

    gtk_tree_model_get (GTK_TREE_MODEL (model),
                        &iter,
                        SPP_HISTORICAL_CURRENT_COLUMN, &str_amount,
                        SPP_HISTORICAL_DIV_NUMBER, &div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
                        -1);

    fyear_number = gsb_data_account_get_bet_hist_fyear (self->account_number);

    /* on calcule les montants par mois en premier */
    list_transactions = bet_hist_get_list_trans_current_fyear ();
    if (g_hash_table_size (list_transactions) == 0)
        return FALSE;

    /* on initialise les tableaux des montants */
    for (i = 0; i < 12; i++)
    {
        tab[i] = null_real;
        if (fyear_number > 0)
            tab2[i] = null_real;
    }

    g_hash_table_iter_init (&hash_iter, list_transactions);
    while (g_hash_table_iter_next (&hash_iter, &key, &value))
    {
        TransactionCurrentFyear *tcf = (TransactionCurrentFyear*) value;

        if (tcf->div_nb == div_number)
        {
            date_month = g_date_get_month (tcf->date);
            if (sub_div_nb > 0)
            {
                if (tcf->sub_div_nb == sub_div_nb)
                {
                    switch (tcf->type_de_transaction)
                    {
                        case 0:
                            if (fyear_number > 0)
                                tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                            break;
                        case 1:
                            if (fyear_number > 0)
                                tab2[date_month-1] = gsb_real_add (tab2[date_month-1], tcf->amount);
                            else
                                tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                            break;
                        case 2:
                            tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                            if (fyear_number > 0)
                                tab2[date_month-1] = gsb_real_add (tab2[date_month-1], tcf->amount);
                            break;
                    }
                }
            }
            else
            {
                switch (tcf->type_de_transaction)
                {
                    case 0:
                        if (fyear_number > 0)
                            tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                        break;
                    case 1:
                        if (fyear_number > 0)
                            tab2[date_month-1] = gsb_real_add (tab2[date_month-1], tcf->amount);
                        else
                            tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                        break;
                    case 2:
                        tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                        if (fyear_number > 0)
                            tab2[date_month-1] = gsb_real_add (tab2[date_month-1], tcf->amount);
                        break;
                }
            }
        }
    }

    /* On commence par le début de l'exercice courant puis on balaie les douze mois */
    start_current_fyear = bet_hist_get_start_date_current_fyear ();
    date_month = g_date_get_month (start_current_fyear);
    today_month = g_date_get_month (gdate_today ());

    for (i = 0; i < 12; i++)
    {
        if (fyear_number > 0)
            desc = g_strdup_printf ("%s", gettext (long_str_months[date_month-1]));
        else
            desc = g_strdup_printf ("%s %d", gettext (short_str_months[date_month-1]),
									g_date_get_year (start_current_fyear));
        strncpy (&libelle_axe_x[self->nbre_elemnts * TAILLE_MAX_LIBELLE], desc, TAILLE_MAX_LIBELLE-1);
		g_free(desc);

        /* Pour un graphique line on n'affiche pas 0 comme donnée des mois futurs */
        if (strcmp (self->service_id, "GogLinePlot") == 0)
        {
            if (i < (gint) today_month)
            {
                tab_Y[self->nbre_elemnts] = gsb_real_real_to_double (tab[date_month-1]);
                if (fyear_number > 0)
                    tab_Y2[self->nbre_elemnts] = gsb_real_real_to_double (tab2[date_month-1]);
            }
            else
            {
                if (fyear_number > 0)
                {
                    tab_Y[self->nbre_elemnts] = gsb_real_real_to_double (tab[date_month-1]);
                    tab_Y2[self->nbre_elemnts] = go_nan;
                }
                else
                    tab_Y[self->nbre_elemnts] = go_nan;
            }
        }
        else
        {
            tab_Y[self->nbre_elemnts] = gsb_real_real_to_double (tab[date_month-1]);
            if (fyear_number > 0)
                tab_Y2[self->nbre_elemnts] = gsb_real_real_to_double (tab2[date_month-1]);
        }

        self->nbre_elemnts++;
        g_date_add_months (start_current_fyear, 1);
        date_month = g_date_get_month (start_current_fyear);
    }

    /* on met la division sous division comme titre */
    self->title = g_strconcat (bet_data_get_div_name (div_number, sub_div_nb, NULL), " = ", str_amount, NULL);

    g_date_free (start_current_fyear);

    /* return value */
    return TRUE;
}

/**
 * remplit les structures avec les données des sous divisions.
 *
 * \param BetGraphDataStruct     structure data du parent
 * \param gint                      numéro de division
 *
 * \return FALSE
 **/
gboolean bet_graph_populate_sectors_by_sub_divisions (BetGraphDataStruct *self,
													  gint div_number)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->tree_view));
    if (model == NULL)
        return FALSE;

    if (gtk_tree_model_get_iter_first (model, &iter))
    {
        gint account_number;
        gchar *libelle_division = self->tab_libelle[0];
        gdouble *tab_montant_division = self->tab_Y;

        /* test du numero de compte */
        gtk_tree_model_get (GTK_TREE_MODEL (model),
							&iter,
							SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
							-1);
        if (account_number != self->account_number)
		{
			if (gsb_data_account_get_bet_hist_use_data_in_account (account_number))
			{
				gint main_account_number;

				main_account_number = bet_data_transfert_get_main_account_from_card (account_number);
				if (main_account_number != self->account_number)
					return FALSE;
			}
			else
				return FALSE;
		}

        do
        {
            gint div;

            gtk_tree_model_get (model, &iter, SPP_HISTORICAL_DIV_NUMBER, &div, -1);

            if (div == div_number)
            {
                GtkTreeIter child_iter;
                gchar *desc;
                gchar *amount;
                gint nbre_elemnts;
                gint i;

                nbre_elemnts = gtk_tree_model_iter_n_children (model, &iter);
                if (nbre_elemnts > MAX_SEGMENT_CAMEMBERT)
                    nbre_elemnts = MAX_SEGMENT_CAMEMBERT;

                for (i = 0; i < nbre_elemnts; i++)
                {
                    if (gtk_tree_model_iter_nth_child (model, &child_iter, &iter, i))
                    {
                        gtk_tree_model_get (model,
											&child_iter,
											SPP_HISTORICAL_DESC_COLUMN, &desc,
											SPP_HISTORICAL_BALANCE_AMOUNT, &amount,
											SPP_HISTORICAL_SUB_DIV_NUMBER, &div,
											-1);
                        strncpy (&libelle_division[self->nbre_elemnts * TAILLE_MAX_LIBELLE], desc, TAILLE_MAX_LIBELLE-1);
                        tab_montant_division[self->nbre_elemnts] = utils_str_strtod ((amount == NULL) ? "0" : amount, NULL);

                        if (tab_montant_division[self->nbre_elemnts] < 0)
                            self->montant += -tab_montant_division[self->nbre_elemnts];
                        else
                            self->montant += tab_montant_division[self->nbre_elemnts];

                        self->nbre_elemnts++;
                        g_free (desc);
                        g_free (amount);
                    }
                }

                break;
            }
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

        if (self->nbre_elemnts)
            return TRUE;
    }

    /* return */
    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return TRUE if OK FALSE otherwise
 **/
gboolean bet_graph_populate_sectors_by_hist_data (BetGraphDataStruct *self)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->tree_view));
    if (model == NULL)
        return FALSE;

    if (gtk_tree_model_get_iter_first (model, &iter))
    {
        gint account_number;
        gchar *libelle_division = self->tab_libelle[0];
        gdouble *tab_montant_division = self->tab_Y;

        /* test du numero de compte */
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
							SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
							-1);
        if (account_number != self->account_number)
		{
			if (gsb_data_account_get_bet_hist_use_data_in_account (account_number))
			{
				gint main_account_number;

				main_account_number = bet_data_transfert_get_main_account_from_card (account_number);
				if (main_account_number != self->account_number)
					return FALSE;
			}
			else
				return FALSE;
		}

        do
        {
            gchar *desc = NULL;
            gchar *amount = NULL;
            gint div;
            gint type_infos;

            gtk_tree_model_get (GTK_TREE_MODEL(model),
								&iter,
								SPP_HISTORICAL_DESC_COLUMN, &desc,
								SPP_HISTORICAL_BALANCE_AMOUNT, &amount,
								SPP_HISTORICAL_DIV_NUMBER, &div,
								-1);

            type_infos = bet_data_get_div_type (div);
            if (desc && (self->type_infos == -1 || type_infos == self->type_infos))
            {
                strncpy (&libelle_division[self->nbre_elemnts * TAILLE_MAX_LIBELLE], desc, TAILLE_MAX_LIBELLE-1);
                tab_montant_division[self->nbre_elemnts] = utils_str_strtod ((amount == NULL) ? "0" : amount, NULL);

                if (tab_montant_division[self->nbre_elemnts] < 0)
                    self->montant += -tab_montant_division[self->nbre_elemnts];
                else
                    self->montant += tab_montant_division[self->nbre_elemnts];

                self->nbre_elemnts++;
            }
            g_free (desc);
            g_free (amount);

            if (self->nbre_elemnts >= MAX_SEGMENT_CAMEMBERT)
                break;
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

        if (self->nbre_elemnts)
            return TRUE;
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
void bet_graph_forecast_graph_new (GtkWidget *button,
								   GtkTreeView *tree_view)
{
    GtkWidget *dialog;
    GtkWidget *label;
    gchar *service_id;
    gint account_number;
    gint currency_number;
    gint origin_tab;
    BetGraphDataStruct *self = NULL;
    BetGraphPrefsStruct *prefs = NULL;

    devel_debug (NULL);

    account_number = gsb_gui_navigation_get_current_account ();
    currency_number = gsb_data_account_get_currency (account_number);
    service_id = g_object_get_data (G_OBJECT (button), "service_id");

    /* initialisation des préférences */
    origin_tab = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "origin_tab"));
    if (origin_tab == BET_ONGLETS_HIST)
        prefs = prefs_hist;
    else
        prefs = prefs_prev;

    /* Initialisations des données */
    self = struct_initialise_bet_graph_data ();
    self->tree_view = tree_view;
    self->account_number = account_number;
    self->currency_number = currency_number;
    self->service_id = g_strdup (service_id);
    self->prefs = prefs;

    /* set the title */
    label = GTK_WIDGET (g_object_get_data (G_OBJECT (tree_view), "label_title"));

    /* initialise les pages pour les graphiques ligne et barre */
    dialog = GTK_WIDGET (widget_bet_graph_others_new (self, gtk_label_get_text (GTK_LABEL (label)), origin_tab));

    gtk_widget_show_all (dialog);
    gtk_notebook_set_current_page (self->notebook, 0);

    gtk_dialog_run (GTK_DIALOG (dialog));

    /* free the data */
    struct_free_bet_graph_data (self);

    gtk_widget_destroy (dialog);
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
void bet_graph_hist_graph_new (GtkWidget *button,
							   GtkTreeView *tree_view)
{
    GtkWidget *dialog;
    gchar *title = NULL;
    gchar *tmp_str;
    gint origin_tab;
    gint fyear_number;
    BetGraphDataStruct *self;

    devel_debug (NULL);

    /* Initialisations des données */
    self = struct_initialise_bet_graph_data ();
    self->tree_view = tree_view;
    self->account_number = gsb_gui_navigation_get_current_account ();
    self->currency_number = gsb_data_account_get_currency (self->account_number);
    self->service_id = g_strdup (g_object_get_data (G_OBJECT (button), "service_id"));

    origin_tab = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "origin_tab"));
	if (origin_tab == BET_ONGLETS_HIST)
	{
		self->prefs = prefs_hist;
		fyear_number = gsb_data_account_get_bet_hist_fyear (self->account_number);
		if (fyear_number > 0)
		{
			GDate *start_current_fyear;

			self->double_axe = TRUE;
			self->is_legend = TRUE;

			/* set the titles */
			start_current_fyear = bet_hist_get_start_date_current_fyear ();
			self->title_Y = g_strdup (gsb_data_fyear_get_name (fyear_number));
			self->title_Y2 = g_strdup_printf ("%d", g_date_get_year (start_current_fyear));
			title = g_strdup_printf (_("Amounts %s - %s for the account: '%s'"),
							self->title_Y,
							self->title_Y2,
							gsb_data_account_get_name (self->account_number));
			g_date_free (start_current_fyear);
		}
		else
		{
			GDate *date_debut_periode;

			/* set the title */
			date_debut_periode = bet_graph_get_date_debut_periode ();
			tmp_str = gsb_format_gdate (date_debut_periode);
			title = g_strdup_printf (_("Monthly amounts since %s for the account: '%s'"),
							tmp_str,
							gsb_data_account_get_name (self->account_number));
			g_date_free (date_debut_periode);
			g_free (tmp_str);
		}
	}
	else
		self->prefs = prefs_prev;

	/* initialise les pages pour les graphiques ligne et barre */
	dialog = GTK_WIDGET (widget_bet_graph_others_new (self, title, origin_tab));
	g_free (title);

    gtk_widget_show_all (dialog);
    gtk_notebook_set_current_page (self->notebook, 0);

    gtk_dialog_run (GTK_DIALOG (dialog));

    /* free the data */
    struct_free_bet_graph_data (self);

    gtk_widget_destroy (dialog);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void bet_graph_sectors_graph_new (GtkWidget *button,
								  GtkTreeView *tree_view)
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *notebook;
    gint account_number;
    gint currency_number;
    BetGraphDataStruct *self_credit;
    BetGraphDataStruct *self_debit;
	GrisbiWinEtat *w_etat;

    devel_debug (NULL);
	w_etat = grisbi_win_get_w_etat ();

	/* set the title */
    label = GTK_WIDGET (g_object_get_data (G_OBJECT (tree_view), "label_title"));

	/* Création de la fenêtre de dialogue pour le graph */
	dialog = GTK_WIDGET (widget_bet_graph_pie_new (NULL, gtk_label_get_text (GTK_LABEL (label))));

    /* on recupere le notebook pour plus tard */
	notebook = widget_bet_graph_pie_get_notebook (dialog);

	/* initialisation des structures de données */
    account_number = gsb_gui_navigation_get_current_account ();
    currency_number = gsb_data_account_get_currency (account_number);

    /* Set the graph for debit */
    self_debit = struct_initialise_bet_graph_data ();
    self_debit->notebook = GTK_NOTEBOOK (notebook);
    self_debit->tree_view = tree_view;
    self_debit->account_number = account_number;
    self_debit->currency_number = currency_number;
    self_debit->type_infos = 1;
	if (w_etat->metatree_assoc_mode)
		self_debit->title = g_strdup ("Charges");
	else
		self_debit->title = g_strdup (_("Expenses"));
    self_debit->service_id = g_strdup ("GogPiePlot");
    self_debit->is_legend = TRUE;

    /* create and display graph for debit */
	widget_bet_graph_pie_display_graph (self_debit, FALSE, 0);

    /* Set the graph for credit */
    self_credit = struct_initialise_bet_graph_data ();
    self_credit->notebook = GTK_NOTEBOOK (notebook);
    self_credit->tree_view = tree_view;
    self_credit->account_number = account_number;
    self_credit->currency_number = currency_number;
    self_credit->type_infos = 0;
	if (w_etat->metatree_assoc_mode)
		self_credit->title = g_strdup ("Produits");
	else
		self_credit->title = g_strdup (_("Incomes"));

    self_credit->service_id = g_strdup ("GogPiePlot");
    self_credit->is_legend = TRUE;

    /* create and display graph for credit */
	widget_bet_graph_pie_display_graph (self_credit, FALSE, 0);

    /* show or hide widgets */
    gtk_widget_show_all (dialog);

    gtk_dialog_run (GTK_DIALOG (dialog));

    /* free the data */
    struct_free_bet_graph_data (self_debit);
    struct_free_bet_graph_data (self_credit);
    gtk_widget_destroy (dialog);
}

/**
 *  sérialise les préférences des graphes
 *
 * \param   none
 *
 * \return a newly allocated string which must be freed when no more used.
 **/
gchar *bet_graph_get_options_string (gint origin_tab)
{
    gchar *new_str = NULL;
    const gchar *tmp_str = NULL;
    BetGraphPrefsStruct *prefs = NULL;

    if (origin_tab == BET_ONGLETS_PREV)
    {
        prefs = prefs_prev;
        tmp_str = "forecast_prefs";
    }
    else
    {
        prefs = prefs_hist;
        tmp_str = "hist_prefs";
    }

	if (NULL == prefs)
		return NULL;

    new_str = g_markup_printf_escaped ("\t<Bet_graph prefs=\"%s:%d:%d:%d:%d:%d:%d:%d:%.0f:%d:%d:%d:%d\" />\n",
                        tmp_str,
                        prefs->type_graph,
                        prefs->major_tick_out,
                        prefs->major_tick_in,
                        prefs->major_tick_labeled,
                        prefs->position,
                        prefs->new_axis_line,
                        prefs->cross_entry,
                        prefs->degrees,
                        prefs->gap_spinner,
                        prefs->before_grid,
                        prefs->major_grid_y,
                        prefs->minor_grid_y);

    return new_str;
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
void bet_graph_set_options_variables (const gchar *string)
{
	gchar **tab;
	BetGraphPrefsStruct *prefs = NULL;

	if (string == NULL)
	{
		if (prefs_prev == NULL)
		{
			prefs_prev = struct_initialise_bet_graph_prefs ();
		}
		else
		{
			g_free (prefs_prev);
			prefs_prev =struct_initialise_bet_graph_prefs ();
		}

		if (prefs_hist == NULL)
		{
			prefs_hist = struct_initialise_bet_graph_prefs ();
		}
		else
		{
			g_free (prefs_hist);
			prefs_hist = struct_initialise_bet_graph_prefs ();
		}

		return;
	}

	tab = g_strsplit (string, ":", 0);

	if (strcmp (tab[0], "forecast_prefs") == 0)
	{
		if (prefs_prev == NULL)
			prefs_prev = struct_initialise_bet_graph_prefs ();
		prefs = prefs_prev;
	}
	else
	{
		if (prefs_hist == NULL)
			prefs_hist = struct_initialise_bet_graph_prefs ();
		prefs = prefs_hist;
	}

	/* initialisation des variables */
	prefs->type_graph = utils_str_atoi (tab[1]);
	if (prefs->type_graph > 1)
		prefs->type_graph = 1;
	prefs->major_tick_out = utils_str_atoi (tab[2]);
	if (prefs->major_tick_out > 1)
		prefs->major_tick_out = TRUE;
	prefs->major_tick_in = utils_str_atoi (tab[3]);
	if (prefs->major_tick_in > 1)
		prefs->major_tick_in = FALSE;
	prefs->major_tick_labeled = utils_str_atoi (tab[4]);
	if (prefs->major_tick_labeled > 1)
		prefs->major_tick_labeled = TRUE;
	prefs->position = utils_str_atoi (tab[5]);
	if (prefs->position > 2)
		prefs->position = 2;
	prefs->new_axis_line = utils_str_atoi (tab[6]);
	if (prefs->new_axis_line > 1)
		prefs->new_axis_line = FALSE;
	prefs->cross_entry = utils_str_atoi (tab[7]);
	if (prefs->cross_entry > 1)
		prefs->cross_entry = 0;
	prefs->degrees = utils_str_atoi (tab[8]);
	if (prefs->degrees > 180)
		prefs->degrees = 90;
	prefs->gap_spinner = utils_str_atoi (tab[9]);
	if (prefs->gap_spinner > 100)
		prefs->gap_spinner = 50;
	prefs->before_grid = utils_str_atoi (tab[10]);
	if (prefs->before_grid > 1)
		prefs->before_grid = TRUE;
	prefs->major_grid_y = utils_str_atoi (tab[11]);
	if (prefs->major_grid_y > 1)
		prefs->major_grid_y = FALSE;
	prefs->minor_grid_y = utils_str_atoi (tab[12]);
	if (prefs->minor_grid_y > 1)
		prefs->minor_grid_y = FALSE;

	g_strfreev (tab);
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
void bet_graph_free_options_variables (void)
{
    if (prefs_prev)
    {
        g_free (prefs_prev);
        prefs_prev = NULL;
    }
    if (prefs_hist)
    {
        g_free (prefs_hist);
        prefs_hist = NULL;
    }
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
BetGraphDataStruct *struct_initialise_bet_graph_data (void)
{
    BetGraphDataStruct *self;
    gint i;

    self = g_new0 (BetGraphDataStruct, 1);

    self->service_id = NULL;
    self->title = NULL;

    self->tab_vue_libelle = g_malloc (MAX_POINTS_GRAPHIQUE * sizeof (gchar*));

    for (i = 0; i < MAX_POINTS_GRAPHIQUE; i++)
    {
        self->tab_vue_libelle[i] = self->tab_libelle[i];
    }

    self->title_Y = NULL;
    self->title_Y2 = NULL;

   return self;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void struct_free_bet_graph_data (BetGraphDataStruct *self)
{
    g_free (self->title);
    g_free (self->service_id);
    g_free (self->tab_vue_libelle);
    g_free (self->title_Y);
    g_free (self->title_Y2);

    g_free (self);
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
