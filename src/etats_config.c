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


/*START_INCLUDE*/
#include "etats_config.h"
#include "dialog.h"
#include "etats_calculs.h"
#include "etats_config.h"
#include "etats_prefs.h"
#include "etats_onglet.h"
#include "etats_page_accounts.h"
#include "etats_page_period.h"
#include "etats_page_transfer.h"
#include "grisbi_app.h"
#include "gsb_calendar_entry.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_report.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report_text_comparison.h"
#include "gsb_file.h"
#include "gsb_form_widget.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_dates.h"
#include "utils_gtkbuilder.h"
#include "utils_prefs.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static void etats_config_onglet_montants_get_buttons_add_remove ( GtkWidget *parent,
                        gint text_comparison_number );
static void etats_config_onglet_texte_get_buttons_add_remove ( GtkWidget *parent,
                        gint text_comparison_number );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/

/* last_report */
static gint last_report = -1;

static gboolean payee_last_state;

/* the def of the columns in the categ and budget list to filter by categ and budget */
enum
{
    GSB_ETAT_CATEG_BUDGET_LIST_NAME = 0,
    GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE,
    GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE,
    GSB_ETAT_CATEG_BUDGET_LIST_NUMBER,
    GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER,
    GSB_ETAT_CATEG_BUDGET_LIST_NB
};

static const gchar *champs_type_recherche_texte[] =
{
    N_("payee"),
    N_("payee information"),
    N_("category"),
    N_("sub-category"),
    N_("budgetary line"),
    N_("sub-budgetary line"),
    N_("note"),
    N_("bank reference"),
    N_("voucher"),
    N_("cheque number"),
    N_("reconciliation reference"),
    NULL
};

static const gchar *champs_operateur_recherche_texte[] =
{
    N_("contains"),
    N_("doesn't contain"),
    N_("begins with"),
    N_("ends with"),
    N_("is empty"),
    N_("isn't empty"),
    NULL
};

static const gchar *champs_lien_lignes_comparaison[] =
{
    N_("and"),
    N_("or"),
    N_("except"),
    NULL
};

static const gchar *champs_comparateur_nombre[] =
{
    N_("equal"),
    N_("less than"),
    N_("less than or equal"),
    N_("greater than"),
    N_("greater than or equal"),
    N_("different from"),
    N_("the biggest"),
    NULL
};

static const gchar *champs_lien_nombre_2[] =
{
    N_("and"),
    N_("or"),
    N_("except"),
    N_("stop"),
    NULL
};

static const gchar *champs_comparateur_montant[] =
{
    N_("equal"),
    N_("less than"),
    N_("less than or equal"),
    N_("greater than"),
    N_("greater than or equal"),
    N_("different from"),
    N_("null"),
    N_("not null"),
    N_("positive"),
    N_("negative"),
    NULL
};

/*END*/

/*COMMON_FUNCTIONS*/
/**
 * Rend sensitif la suite de la comparaison de numéro
 *
 * \param combo_box
 * \param widget        widget à rendre sensitif
 *
 * \return
 */
static void etats_config_combo_lien_nombre_2_changed ( GtkComboBox *combo,
                        GtkWidget *widget )
{
    gint index;

    index = gtk_combo_box_get_active ( combo );
    if ( index == 3 )
        gtk_widget_set_sensitive ( widget, FALSE );
    else
        gtk_widget_set_sensitive ( widget, TRUE );
}

/*ONGLET_TIERS*/
/**
 * Initialise les informations de l'onglet tiers
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_initialise_onglet_tiers ( gint report_number )
{
    gint active;

    active = gsb_data_report_get_payee_detail_used ( report_number );
    etats_prefs_button_toggle_set_actif ( "bouton_detaille_tiers_etat", active );

    if ( active )
    {
        etats_prefs_tree_view_select_rows_from_list (
                                gsb_data_report_get_payee_numbers_list ( report_number ),
                                "treeview_tiers",
                                1 );

        if ( g_slist_length ( gsb_data_report_get_payee_numbers_list ( report_number ) ) )
            utils_togglebutton_set_label_position_unselect (
                                etats_prefs_widget_get_widget_by_name (
                                "togglebutton_select_all_tiers", NULL ),
                                NULL,
                                etats_prefs_widget_get_widget_by_name ( "treeview_tiers", NULL ) );
    }
}


/**
 * retourne la liste des tiers
 *
 * \param
 *
 * \return model
 * */
GtkTreeModel *etats_config_onglet_get_liste_tiers ( void )
{
    GtkListStore *list_store;
    GSList *list_tmp;

    list_store = gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_INT );

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( list_store ),
                        0, GTK_SORT_ASCENDING );

    /* on remplit la liste des tiers */
    list_tmp = gsb_data_payee_get_payees_list ( );

    while ( list_tmp )
    {
        GtkTreeIter iter;
        gchar *name;
        gint payee_number;

        payee_number = gsb_data_payee_get_no_payee ( list_tmp -> data );

        name = my_strdup ( gsb_data_payee_get_name ( payee_number, FALSE ) );

        gtk_list_store_append ( list_store, &iter );
        gtk_list_store_set ( list_store, &iter, 0, name, 1, payee_number, -1 );

        if ( name )
            g_free ( name );

        list_tmp = list_tmp -> next;
    }

    return GTK_TREE_MODEL ( list_store );
}


/**
 * Récupère les informations de l'onglet tiers
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_tiers ( gint report_number )
{
    gint active;

    active = etats_prefs_button_toggle_get_actif ( "bouton_detaille_tiers_etat" );
    gsb_data_report_set_payee_detail_used ( report_number, active );
    if ( active )
    {
        gsb_data_report_free_payee_numbers_list ( report_number );

        if ( utils_tree_view_all_rows_are_selected ( GTK_TREE_VIEW (
         etats_prefs_widget_get_widget_by_name ( "treeview_tiers", NULL ) ) ) )
        {
            gchar *text;
            gchar *hint;

            hint = g_strdup ( _("Performance issue.") );
            text = g_strdup ( _("All payees have been selected.  Grisbi will run "
                            "faster without the \"Detail payees used\" option activated.") );

            dialogue_hint ( text, hint );
            etats_prefs_button_toggle_set_actif ( "togglebutton_select_all_tiers", FALSE );
            gsb_data_report_set_payee_detail_used ( report_number, FALSE );

            g_free ( text );
            g_free ( hint );
        }
        else
            gsb_data_report_set_payee_numbers_list ( report_number,
                            etats_prefs_tree_view_get_list_rows_selected ( "treeview_tiers" ) );
    }
}


/*ONGLET_CATEGORIES BUDGETS*/
/**
 * fill the categories selection list for report
 *
 * \param model
 * \param is_categ  TRUE for category, FALSE for budget
 *
 * \return FALSE
 * */
static gboolean etats_config_onglet_categ_budget_fill_model ( GtkTreeModel *model,
                        gboolean is_categ )
{
    GtkTreeIter parent_iter;
    GtkTreeIter child_iter;
    GSList *list_tmp;
    gchar *name;
    gchar *without_name;
    gchar *without_sub_name;

    if ( is_categ )
    {
        list_tmp = gsb_data_category_get_categories_list ( );

        gtk_tree_store_clear ( GTK_TREE_STORE ( model ) );

        without_name = _("No category");
        without_sub_name = _("No subcategory");
    }
    else
    {
        list_tmp = gsb_data_budget_get_budgets_list ( );
        gtk_tree_store_clear ( GTK_TREE_STORE ( model ) );

        without_name = _("No budgetary line");
        without_sub_name = _("No sub-budgetary line");
    }

    while ( list_tmp )
    {
        gint div_number;
        GSList *tmp_list_sub_div;

        /* get the category to append */
        if ( is_categ )
        {
            div_number = gsb_data_category_get_no_category ( list_tmp -> data );
            name = gsb_data_category_get_name (div_number, 0, NULL );
            tmp_list_sub_div = gsb_data_category_get_sub_category_list ( div_number );
        }
        else
        {
            div_number = gsb_data_budget_get_no_budget ( list_tmp -> data );
            name = gsb_data_budget_get_name ( div_number, 0, NULL );
            tmp_list_sub_div = gsb_data_budget_get_sub_budget_list ( div_number );
        }
		if (name)
		{
			/* append to the model */
				gtk_tree_store_append ( GTK_TREE_STORE ( model ), &parent_iter, NULL );
				gtk_tree_store_set (GTK_TREE_STORE ( model ),
									&parent_iter,
									GSB_ETAT_CATEG_BUDGET_LIST_NAME, name,
									GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, div_number,
									GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, -1,
									GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
									-1 );

			g_free (name);

			/* append the sub categories */
			while (tmp_list_sub_div)
			{
				gint sub_div_number;

				if ( is_categ )
				{
					sub_div_number = gsb_data_category_get_no_sub_category ( tmp_list_sub_div -> data );
					name = gsb_data_category_get_sub_category_name ( div_number, sub_div_number, NULL );
				}
				else
				{
					sub_div_number = gsb_data_budget_get_no_sub_budget ( tmp_list_sub_div -> data );
					name = gsb_data_budget_get_sub_budget_name ( div_number, sub_div_number, NULL );
				}
				if (!name)
					name = g_strdup (_("Not available"));

				/* append to the model */
				gtk_tree_store_append ( GTK_TREE_STORE ( model ), &child_iter, &parent_iter );
				gtk_tree_store_set ( GTK_TREE_STORE ( model ),
									&child_iter,
									GSB_ETAT_CATEG_BUDGET_LIST_NAME, name,
									GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, -1,
									GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, sub_div_number,
									GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
									-1 );

				g_free (name);

				tmp_list_sub_div = tmp_list_sub_div -> next;
			}
		}
        /* append without sub-div */
            gtk_tree_store_append ( GTK_TREE_STORE ( model ), &child_iter, &parent_iter );
            gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                                &child_iter,
                                GSB_ETAT_CATEG_BUDGET_LIST_NAME, without_sub_name,
                                GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, -1,
                                GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, 0,
                                GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
                                -1 );

        list_tmp = list_tmp -> next;
    }

    /* append without div and sub-div*/
    gtk_tree_store_append ( GTK_TREE_STORE ( model ), &parent_iter, NULL );
    gtk_tree_store_set (GTK_TREE_STORE ( model ),
                        &parent_iter,
                        GSB_ETAT_CATEG_BUDGET_LIST_NAME, without_name,
                        GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, 0,
                        GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, -1,
                        GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
                        -1 );

    gtk_tree_store_append ( GTK_TREE_STORE ( model ), &child_iter, &parent_iter );
    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &child_iter,
                        GSB_ETAT_CATEG_BUDGET_LIST_NAME, without_sub_name,
                        GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, -1,
                        GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, 0,
                        GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
                        -1 );

    return FALSE;
}


/**
 * retourne la liste des lignes sélectionnées
 *
 * \param treeview_name nom du tree_view
 *
 * \return a GSList of CategBudgetSel or NULL if all the categories/sub-categ were selected
 *          to avoid to filter by categ, to improve speed
 * */
static GSList *etats_config_onglet_categ_budget_get_selected ( const gchar *treeview_name )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeIter parent_iter;
    gboolean all_selected = TRUE;
    GSList *tmp_list = NULL;

    /* on récupère le modèle */
    tree_view = etats_prefs_widget_get_widget_by_name ( treeview_name, NULL );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    if ( !gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &parent_iter ) )
        return NULL;

    do
    {
        gint div_number;
        gboolean active;
        CategBudgetSel *categ_budget_struct;
        GtkTreeIter iter_children;

        gtk_tree_model_get (GTK_TREE_MODEL (model),
                        &parent_iter,
                        GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, &div_number,
                        GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, &active,
                        -1 );
        if (!active)
        {
            all_selected = FALSE;
            continue;
        }

        /* ok, we are on a selected category/budget, create and fill the structure */
        categ_budget_struct = g_malloc0 ( sizeof ( CategBudgetSel ) );

        categ_budget_struct -> div_number = div_number;
        tmp_list = g_slist_append ( tmp_list, categ_budget_struct );

        /* check the children */
        if (gtk_tree_model_iter_children ( GTK_TREE_MODEL (model), &iter_children, &parent_iter))
        {
            /* we are on the children */
            do
            {
                gint sub_div_number;

                gtk_tree_model_get (GTK_TREE_MODEL (model),
                                &iter_children,
                                GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, &sub_div_number,
                                GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, &active,
                                -1 );
                if (active)
                    categ_budget_struct -> sub_div_numbers = g_slist_append (
                                                                categ_budget_struct -> sub_div_numbers,
                                                                GINT_TO_POINTER ( sub_div_number ) );
                else
                    all_selected = FALSE;
            }
            while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter_children ) );
        }
    }
    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &parent_iter ) );

    /* if all is selected, erase the new list and set an info message */
    if ( all_selected )
    {
        if ( strcmp ( treeview_name, "treeview_categ" ) == 0 )
            dialogue_hint (
                        _("All categories have been selected.  Grisbi will run faster without "
                        "the \"Detail categories used\" option activated."),
                        _("Performance issue.") );
        else
            dialogue_hint (
                        _("All budgets have been selected.  Grisbi will run faster without the "
                        "\"Detail budgets used\" option activated."),
                        _("Performance issue.") );

        etats_config_onglet_categ_budget_tree_model_check_uncheck_all ( model, FALSE );
        gsb_data_report_free_categ_budget_struct_list ( tmp_list );

        tmp_list = NULL;
    }
    return tmp_list;
}


/**
 * Initialisation du tree_view
 *
 * \param treeview_name
 * \param tmp_list
 *
 * \return
 * */
static void etats_config_onglet_categ_budget_init_treeview ( const gchar *treeview_name,
                            GSList *tmp_list )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeIter parent_iter;

    /* on récupère le modèle */
    tree_view = etats_prefs_widget_get_widget_by_name ( treeview_name, NULL );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    etats_config_onglet_categ_budget_tree_model_check_uncheck_all ( model, FALSE );

    while ( tmp_list )
    {
        CategBudgetSel *categ_budget_struct = tmp_list -> data;

		if ( !gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &parent_iter ) )
			return;
        do
        {
            gint div_number;

            gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
                                &parent_iter,
                                GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, &div_number,
                                -1 );

            /* we check the children only if the category is selected */
            if ( div_number == categ_budget_struct -> div_number )
            {
                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                                &parent_iter,
                                GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
                                -1 );

                if ( categ_budget_struct -> sub_div_numbers )
                {
                    GtkTreeIter iter_child;

                    if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL ( model ), &iter_child, &parent_iter ) )
                    {
                        do
                        {
                            gint sub_div_number;

                            gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
                                                &iter_child,
                                                GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, &sub_div_number,
                                                -1 );

                            if  ( g_slist_find ( categ_budget_struct -> sub_div_numbers,
                             GINT_TO_POINTER ( sub_div_number ) ) )
                                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                                                &iter_child,
                                                GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
                                                -1 );
                        }
                        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter_child ) );
                    }
                }
                /* we have found the category, can stop here */
                break;
            }
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &parent_iter ) );

        tmp_list = tmp_list -> next;
    }
}


/**
 * Initialise les informations des onglets catégories ou budgets
 *
 * \param report_number
 * \param is_categ      TRUE = Categories FALSE = Budgets
 *
 * \return
 */
static void etats_config_initialise_onglet_categ_budget ( gint report_number,
                        gboolean is_categ )
{
    gint active;
    gchar *checkbutton_name;
    gchar *button_name;
    gchar *treeview_name;
    GSList *tmp_list;

    if ( is_categ )
    {
        active = gsb_data_report_get_category_detail_used ( report_number );
        checkbutton_name = g_strdup ( "bouton_detaille_categ_etat" );
        treeview_name = g_strdup ( "treeview_categ" );
        button_name = g_strdup ( "togglebutton_select_all_categ" );
        tmp_list = gsb_data_report_get_category_struct_list ( report_number );
    }
    else
    {
        active = gsb_data_report_get_budget_detail_used ( report_number );
        checkbutton_name = g_strdup ( "bouton_detaille_budget_etat" );
        treeview_name = g_strdup ( "treeview_budget" );
        button_name = g_strdup ( "togglebutton_select_all_budget" );
        tmp_list = gsb_data_report_get_budget_struct_list ( report_number );
    }

    etats_prefs_button_toggle_set_actif ( checkbutton_name, active );

    if ( active )
    {
        etats_config_onglet_categ_budget_init_treeview ( treeview_name, tmp_list );

        if ( g_slist_length ( tmp_list ) )
            utils_togglebutton_set_label_position_unselect (
                                etats_prefs_widget_get_widget_by_name (
                                button_name, NULL ),
                                G_CALLBACK ( etats_prefs_onglet_categ_budget_check_uncheck_all ),
                                etats_prefs_widget_get_widget_by_name ( treeview_name, NULL ) );
    }

    g_free ( checkbutton_name );
    g_free ( treeview_name );
    g_free ( button_name );
}


/**
 *
 *
 * \param
 *
 * \return
 * */
static gint etats_config_onglet_categ_budget_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gpointer ptr )
{
    gchar *name_1;
    gchar *name_2;
    gint number_1;
    gint number_2;
    gint sub_number_1;
    gint sub_number_2;
    gint return_value = 0;

    /* first, we sort by date (col 0) */
    gtk_tree_model_get ( model,
                        iter_1,
                        GSB_ETAT_CATEG_BUDGET_LIST_NAME, &name_1,
                        GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, &number_1,
                        GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, &sub_number_1,
                        -1 );

    gtk_tree_model_get ( model,
                        iter_2,
                        GSB_ETAT_CATEG_BUDGET_LIST_NAME, &name_2,
                        GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, &number_2,
                        GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, &sub_number_2,
                        -1 );


    if ( number_1 != -1 && number_2 != -1 && number_1 - number_2 )
    {
        if ( number_1 == 0 )
            return_value = -1;
        else if ( number_2 == 0 )
            return_value = 1;
        else
            return_value =  strcmp ( g_utf8_collate_key ( name_1, -1 ),
                        g_utf8_collate_key ( name_2, -1 ) );

        if ( name_1)
		{
			g_free ( name_1);
			name_1 = NULL;
		}
        if ( name_2)
		{
			g_free ( name_2);
			name_2 = NULL;
		}
    }

    if ( return_value )
        return return_value;

    if ( sub_number_1 == 0 )
            return_value = -1;
    else if ( sub_number_2 == 0 )
            return_value = 1;
    else
        return_value =  strcmp ( g_utf8_collate_key ( name_1, -1 ),
                        g_utf8_collate_key ( name_2, -1 ) );

    if ( name_1) g_free ( name_1);
    if ( name_2) g_free ( name_2);

    return return_value;
}


/**
 * Récupère les informations des onglets catégories ou budgets
 *
 * \param report_number
 * \param is_categ      TRUE = Categories FALSE = Budgets
 *
 * \return
 */
static void etats_config_recupere_info_onglet_categ_budget ( gint report_number,
                        gboolean is_categ )
{
    gint active;
    gchar *treeview_name;

    if ( is_categ )
    {
        active = etats_prefs_button_toggle_get_actif ( "bouton_detaille_categ_etat" );
        treeview_name = g_strdup ( "treeview_categ" );

        gsb_data_report_set_category_detail_used ( report_number, active );
        if ( active )
        {
            gsb_data_report_set_category_struct_list ( report_number,
                                etats_config_onglet_categ_budget_get_selected ( treeview_name ) );
        }
    }
    else
    {
        active = etats_prefs_button_toggle_get_actif ( "bouton_detaille_budget_etat" );
        treeview_name = g_strdup ( "treeview_budget" );

        gsb_data_report_set_budget_detail_used ( report_number, active );
        if ( active )
        {
            gsb_data_report_set_budget_struct_list ( report_number,
                                etats_config_onglet_categ_budget_get_selected ( treeview_name ) );
        }
    }

    g_free ( treeview_name );
}


/**
 * création du model
 *
 * \param is_categ
 *
 * \return a GtkWidget : the GtkTreeView
 * */
GtkTreeModel *etats_config_onglet_categ_budget_get_model ( gboolean is_categ )
{
    GtkTreeStore *store;

    store = gtk_tree_store_new ( GSB_ETAT_CATEG_BUDGET_LIST_NB,
                        G_TYPE_STRING,          /* GSB_ETAT_CATEG_BUDGET_LIST_NAME */
                        G_TYPE_BOOLEAN,         /* GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE */
                        G_TYPE_BOOLEAN,         /* GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE */
                        G_TYPE_INT,             /* GSB_ETAT_CATEG_BUDGET_LIST_NUMBER */
                        G_TYPE_INT );           /* GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER */

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( store ),
                        GSB_ETAT_CATEG_BUDGET_LIST_NAME, GTK_SORT_ASCENDING );

    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( store ),
                        GSB_ETAT_CATEG_BUDGET_LIST_NAME,
                        (GtkTreeIterCompareFunc) etats_config_onglet_categ_budget_sort_function,
                        GINT_TO_POINTER ( is_categ ),
                        NULL );

    etats_config_onglet_categ_budget_fill_model ( GTK_TREE_MODEL ( store ), is_categ );

    /* return */
    return GTK_TREE_MODEL ( store );
}


/**
 * callback if we toggle a checkbox in the category/budget list
 * if we toggle a div, toggle all the sub-div
 * if we toggle a sub-div, toggle also the div
 *
 * \param radio_renderer
 * \param path          the string of path
 * \param store         the GtkTreeStore of categ/budget
 *
 * \return FALSE
 * */
gboolean etats_config_onglet_categ_budget_toggled ( GtkCellRendererToggle *radio_renderer,
                        gchar *path_str,
                        GtkTreeStore *store )
{
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeIter iter_children;
    gboolean toggle_value;

    g_return_val_if_fail (path_str != NULL, FALSE);
    g_return_val_if_fail (store != NULL && GTK_IS_TREE_STORE (store), FALSE);

    /* first get the iter and the value of the checkbutton */
    path = gtk_tree_path_new_from_string (path_str);
    gtk_tree_model_get_iter ( GTK_TREE_MODEL ( store ), &iter, path );
    gtk_tree_model_get ( GTK_TREE_MODEL ( store ),
                        &iter,
                        GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, &toggle_value,
                        -1 );

    /* ok, we invert the button */
    toggle_value = !toggle_value;

    gtk_tree_store_set ( GTK_TREE_STORE ( store ),
                        &iter,
                        GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
                        -1 );

    /* if we are on a mother, we set the same value to all the children */
    if (gtk_tree_model_iter_children ( GTK_TREE_MODEL ( store ), &iter_children, &iter ) )
    {
        /* we are on the children */
        do
            gtk_tree_store_set ( GTK_TREE_STORE (store),
                        &iter_children,
                        GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
                        -1 );
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( store ), &iter_children ) );
    }

    /* if we are activating a child, activate the mother */
    if (toggle_value
     &&
     gtk_tree_model_iter_parent ( GTK_TREE_MODEL ( store ), &iter_children, &iter ) )
        gtk_tree_store_set ( GTK_TREE_STORE ( store ),
                        &iter_children,
                        GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
                        -1 );

    gtk_tree_path_free ( path );

    return FALSE;
}


/**
 * check or uncheck all the budgets or categories
 *
 * \param model         the model to fill (is model_categ or model_budget
 * \param select_ptr    TRUE or FALSE to select/unselect all
 *
 * \return FALSE
 * */
void etats_config_onglet_categ_budget_tree_model_check_uncheck_all ( GtkTreeModel *model,
                        gboolean toggle_value )
{
    GtkTreeIter parent_iter;

    if ( !gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &parent_iter ) )
        return;

    do
    {
        GtkTreeIter iter_children;

        gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &parent_iter,
                        GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
                        -1 );

        if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL ( model ), &iter_children, &parent_iter ) )
        {
            /* we are on the children */
            do
                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter_children,
                        GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
                        -1 );
            while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter_children ) );
        }
    }
    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &parent_iter ) );
}


/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 * */
gboolean etats_config_onglet_categ_budget_select_partie_liste ( GtkWidget *button,
                        GdkEventButton *event,
                        GtkWidget *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean is_categ;
    gboolean type_div;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    if ( !gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
        return FALSE;

    etats_config_onglet_categ_budget_tree_model_check_uncheck_all ( model, FALSE );

    is_categ = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( button ), "is_categ" ) );
    type_div = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( button ), "type_div" ) );

    do
    {
        gint div_number;

        gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
                        &iter,
                        GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, &div_number,
                        -1 );

        /* we always select without categories/budget ?? set yes by default */
        if ( !div_number
         ||
         (is_categ && ( gsb_data_category_get_type ( div_number ) == type_div ) )
         ||
         (!is_categ && ( gsb_data_budget_get_type ( div_number ) == type_div ) ) )
        {
            GtkTreeIter iter_children;

            gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
                        -1 );

            if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL ( model ), &iter_children, &iter ) )
            {
                /* we are on the children */
                do
                    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                                &iter_children,
                                GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
                                -1 );

                while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter_children ) );
            }
        }
    }
    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );

    return FALSE;
}


/*ONGLET_TEXTES*/
/**
 * Rend sensitif la ligne en fonction du choix du combo_box
 *
 * \param combo_box
 * \param number        gpointer sur text_comparison_number
 *
 * \return
 */
static void etats_config_onglet_texte_combo_texte_changed ( GtkComboBox *combo,
                        gpointer data )
{
    gint index;
    gint text_comparison_number;

    text_comparison_number = GPOINTER_TO_INT ( data );

    index = gtk_combo_box_get_active ( combo );
    if ( index == 8 || index == 9 || index == 10 )
    {
        gtk_toggle_button_set_active ( gsb_data_report_text_comparison_get_button_use_text (
                                text_comparison_number ), FALSE );
        gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_button_use_text (
                                text_comparison_number ), FALSE );
        gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_hbox_text (
                                text_comparison_number ), FALSE );

        gtk_toggle_button_set_active ( gsb_data_report_text_comparison_get_button_use_number (
                                text_comparison_number ), TRUE );
        gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_button_use_number (
                                text_comparison_number ), TRUE );
        gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_hbox_cheque (
                                text_comparison_number ), TRUE );
    }
    else
    {
        gtk_toggle_button_set_active ( gsb_data_report_text_comparison_get_button_use_text (
                                text_comparison_number ), TRUE );
        gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_button_use_text (
                                text_comparison_number ), TRUE );
        gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_hbox_text (
                                text_comparison_number ), TRUE );

        gtk_toggle_button_set_active ( gsb_data_report_text_comparison_get_button_use_number (
                                text_comparison_number ), FALSE );
        gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_button_use_number (
                                text_comparison_number ), FALSE );
        gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_hbox_cheque (
                                text_comparison_number ), FALSE );
    }
}


/**
 * crée une ligne de recherche de texte
 *
 * \parent  vbox qui contiendra toutes les lignes
 * \param   première ligne ou ligne supplémentaire
 * \param   avec lien
 *
 * \return la nouvelle ligne
 */
static GtkWidget *etats_config_onglet_texte_new_comparison_line ( GtkWidget *parent,
                        gint text_comparison_number,
                        gint with_link )
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *hbox_1;
    GtkWidget *hbox_2;
    GtkWidget *combo;
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *radio_1;
    GtkWidget *radio_2;

    /* la vbox qui contient la ligne complète */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );

    /* la première hbox pour le type de donnée concernée */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

    /* on crée le premier lien ne sert pas si c'est la première ligne */
    if ( with_link )
    {
        combo = utils_combo_box_make_from_string_array ( champs_lien_lignes_comparaison );
        gsb_data_report_text_comparison_set_button_link ( text_comparison_number, combo );
        gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 5 );
    }

    label = gtk_label_new ( _("Transactions whose ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* avant de créer le bouton des champs, on doit créer hbox_txt, hbox_chq et les 2 check button */
    gsb_data_report_text_comparison_set_hbox_text ( text_comparison_number,
                        gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX ) );
    gsb_data_report_text_comparison_set_hbox_cheque ( text_comparison_number,
                        gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX ) );

    /* on crée le radio bouton de sélection entre les deux types de recherche caché par défaut */
    radio_1 = gtk_radio_button_new ( NULL );
    gsb_data_report_text_comparison_set_button_use_text ( text_comparison_number,radio_1 );

    radio_2 = gtk_radio_button_new_from_widget ( GTK_RADIO_BUTTON ( radio_1 ) );
    gsb_data_report_text_comparison_set_button_use_number ( text_comparison_number, radio_2 );

    /* on crée et initialise le combobox du type de choix pour la recherche de texte */
    combo = utils_combo_box_make_from_string_array ( champs_type_recherche_texte );
    gsb_data_report_text_comparison_set_button_field ( text_comparison_number, combo );
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 0 );

    /* on définit l'action a faire lorsque l'on change le choix du combobox */
    g_signal_connect ( G_OBJECT ( combo ),
                        "changed",
                        G_CALLBACK ( etats_config_onglet_texte_combo_texte_changed ),
                        GINT_TO_POINTER ( text_comparison_number ) );

    /* la suite se met dans hbox_txt en 2ème row */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( NULL );
    gtk_widget_set_size_request ( label, 12, -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    /* on met le check button utilisé en cas de champ à no */
    button = gsb_data_report_text_comparison_get_button_use_text ( text_comparison_number );
    hbox_1 = gsb_data_report_text_comparison_get_hbox_text ( text_comparison_number );

    g_signal_connect ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        hbox_1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    /* on met maintenant le comparateur txt */
    gtk_box_pack_start ( GTK_BOX ( hbox ), hbox_1, FALSE, FALSE, 0 );

    /* avant de créer l'opérateur, on doit créer l'entrée de txt */
    entry = gtk_entry_new ( );
    gsb_data_report_text_comparison_set_entry_text ( text_comparison_number, entry );

    combo = utils_combo_box_make_from_string_array ( champs_operateur_recherche_texte );
    gsb_data_report_text_comparison_set_button_operator ( text_comparison_number, combo );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), combo, FALSE, FALSE, 0 );
    g_signal_connect ( G_OBJECT ( combo ),
                        "changed",
                        G_CALLBACK ( sensitive_widget ),
                        entry );

    /* on peut maintenant mettre l'entrée de txt */
    gtk_widget_set_size_request ( entry, 150, -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), entry, FALSE, FALSE, 0 );

    /* on crée maintenant la 2ème row qui concerne les tests de chq */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( NULL );
    gtk_widget_set_size_request ( label, 12, -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    /* on met le check button utilisé en cas de champ à no */
    button = gsb_data_report_text_comparison_get_button_use_number ( text_comparison_number );
    hbox_1 = gsb_data_report_text_comparison_get_hbox_cheque ( text_comparison_number );

    g_signal_connect ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        hbox_1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    /* mise en place de la hbox des montants de chq */
    gtk_box_pack_start ( GTK_BOX ( hbox ), hbox_1, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("is ") );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), label, FALSE, FALSE, 0 );

    /* on crée et initialise le combobox pour la première comparaison de nombre */
    combo = utils_combo_box_make_from_string_array ( champs_comparateur_nombre );
    gsb_data_report_text_comparison_set_button_first_comparison ( text_comparison_number, combo );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), combo, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("to") );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), label, FALSE, FALSE, 0 );

    /* on crée le champs texte pour entrer la première comparaison */
    entry = gtk_entry_new ( );
    gtk_widget_set_size_request ( entry, 100, -1 );
    gsb_data_report_text_comparison_set_entry_first_amount ( text_comparison_number, entry );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), entry, FALSE, FALSE, 0 );

    /* on crée et initialise le combobox pour autoriser la seconde comparaison de nombre */
    combo = utils_combo_box_make_from_string_array ( champs_lien_nombre_2 );
    gsb_data_report_text_comparison_set_button_link_first_to_second_part ( text_comparison_number, combo );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), combo, FALSE, FALSE, 0 );

    /* on crée la hbox pour la deuxième partie de la comparaison*/
    hbox_2 = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gsb_data_report_text_comparison_set_hbox_second_part ( text_comparison_number, hbox_2 );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), hbox_2, FALSE, FALSE, 0 );

    /* maintenant que hbox_2 est définie on met le signal lorsque l'on change le choix du combobox */
    g_signal_connect ( G_OBJECT ( combo ),
                        "changed",
                        G_CALLBACK ( etats_config_combo_lien_nombre_2_changed ),
                        hbox_2 );

    /* on peut maintenant ajouter dans hbox_partie_2 */
    /* on crée et initialise le combobox pour la seconde comparaison de nombre */
    combo = utils_combo_box_make_from_string_array ( champs_comparateur_nombre );
    gtk_widget_set_size_request ( entry, 100, -1 );
    gsb_data_report_text_comparison_set_button_second_comparison ( text_comparison_number, combo );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), combo, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("at") );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), label, FALSE, FALSE, 0 );

    /* on crée le champs texte pour entrer la seconde comparaison */
    entry = gtk_entry_new ( );
    gsb_data_report_text_comparison_set_entry_second_amount ( text_comparison_number, entry );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), entry, FALSE, FALSE, 0 );

    /* on ajoute le bouton ajouter une nouvelle ligne */
    etats_config_onglet_texte_get_buttons_add_remove ( hbox, text_comparison_number );

    /* on met la ligne complète (vbox) dans son parent */
    gtk_box_pack_start ( GTK_BOX ( parent ), vbox, FALSE, FALSE, 0 );
    gtk_widget_show_all ( vbox );

    /* return */
    return vbox;
}


/**
 * ajoute une ligne de recherche de texte à la liste des lignes de recherche de texte
 *
 *\param last_text_comparison_number
 *
 *\return
 */
static void etats_config_onglet_texte_ajoute_ligne_liste_comparaisons ( gint last_text_comparison_number )
{
    GtkWidget *lignes;
    GtkWidget *widget;
    gint text_comparison_number;
    gint position;
    gint current_report_number;

    lignes = etats_prefs_widget_get_widget_by_name ( "liste_textes_etat", NULL );
    current_report_number = gsb_gui_navigation_get_current_report ();

    /* on récupère tout de suite la position à laquelle il faut insérer la row */
    if ( last_text_comparison_number )
        position = g_slist_index ( gsb_data_report_get_text_comparison_list ( current_report_number ),
                                GINT_TO_POINTER ( last_text_comparison_number ) ) + 1;
    else
        position = 0;

    /* on commence par créer une structure vide */
    text_comparison_number = gsb_data_report_text_comparison_new ( 0 );
    gsb_data_report_text_comparison_set_report_number ( text_comparison_number,
                        current_report_number );

    /* on crée la row et remplit les widget de la structure */
    widget = etats_config_onglet_texte_new_comparison_line ( lignes, text_comparison_number, TRUE );
    gsb_data_report_text_comparison_set_vbox_line ( text_comparison_number, widget );

    /* on vire le lien de la row s'il n'y a pas encore de liste (cad si c'est la 1ère row) */
    if ( !gsb_data_report_get_text_comparison_list ( current_report_number ) )
    {
        gtk_widget_destroy ( gsb_data_report_text_comparison_get_button_link ( text_comparison_number ) );
        gsb_data_report_text_comparison_set_button_link ( text_comparison_number, NULL );
    }

    /* par défaut, le bouton bouton_lien_1_2 est sur stop */
    widget = gsb_data_report_text_comparison_get_button_link_first_to_second_part ( text_comparison_number );
    gtk_combo_box_set_active ( GTK_COMBO_BOX ( widget ), 3 );
    gsb_data_report_text_comparison_set_link_first_to_second_part ( text_comparison_number, 3 );
    gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_hbox_second_part (
                        text_comparison_number ),
                        FALSE );

    /* par défaut, la row de chq est non sensitive */
    gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_button_use_text (
                        text_comparison_number ),
                        FALSE );
    gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_button_use_number (
                        text_comparison_number ),
                        FALSE );
    gtk_widget_set_sensitive ( gsb_data_report_text_comparison_get_hbox_cheque (
                        text_comparison_number ),
                        FALSE );

    /* on met la structure dans la liste à la position demandée */
    gsb_data_report_set_text_comparison_list ( current_report_number,
                        g_slist_insert ( gsb_data_report_get_text_comparison_list ( current_report_number ),
                        GINT_TO_POINTER ( text_comparison_number ),
                        position ) );

    /* on met la row à sa place dans la liste */
    gtk_box_reorder_child ( GTK_BOX ( lignes ),
                        gsb_data_report_text_comparison_get_vbox_line ( text_comparison_number ),
                        position );
}


/**
 *
 *
 * \param
 *
 * \return
 * */
static void etats_config_onglet_texte_sensitive_hbox_fonction_bouton_txt ( gint text_comparison_number )
{
    sens_desensitive_pointeur ( gsb_data_report_text_comparison_get_button_use_text ( text_comparison_number ),
                        gsb_data_report_text_comparison_get_hbox_text ( text_comparison_number ) );
    sens_desensitive_pointeur ( gsb_data_report_text_comparison_get_button_use_number ( text_comparison_number ),
                        gsb_data_report_text_comparison_get_hbox_cheque ( text_comparison_number ) );
}

/**
 * fixes error [-Werror=cast-function-type]
 *
 * \param
 * \param
 *
 * \return
 **/
static void etats_config_onglet_texte_montant_gtk_callback (GtkWidget *widget,
													gpointer null)
{
	gtk_widget_destroy (widget);
}

/**
 * remplit la liste des comparaisons de texte
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_onglet_texte_remplit_liste_comparaisons ( gint report_number )
{
    GtkWidget *lignes;
    GSList *tmp_list;
    gchar *tmp_str;
    gboolean multi_lignes = FALSE;

    /* on commence par effacer les anciennes lignes */
    lignes = etats_prefs_widget_get_widget_by_name ( "liste_textes_etat", NULL );
    gtk_container_foreach (GTK_CONTAINER (lignes), (GtkCallback) etats_config_onglet_texte_montant_gtk_callback, NULL);

    tmp_list = gsb_data_report_get_text_comparison_list ( report_number );
    /*   s'il n'y a rien dans la liste, on met juste une row vide */
    if ( !tmp_list )
    {
        etats_config_onglet_texte_ajoute_ligne_liste_comparaisons ( 0 );
        return;
    }

    /* on fait le tour de la liste des comparaisons de texte, ajoute une row
     * et la remplit à chaque fois */
    while ( tmp_list )
    {
        gint text_comparison_number;
        GtkWidget *widget;

        text_comparison_number = GPOINTER_TO_INT ( tmp_list -> data );

        /* on crée la row et remplit les widget de la structure */
        widget = etats_config_onglet_texte_new_comparison_line ( lignes, text_comparison_number, multi_lignes );
        gsb_data_report_text_comparison_set_vbox_line ( text_comparison_number, widget );

        /* on remplit maintenant les widget avec les valeurs de la stucture */
        /* on rajoute le && si plusieurs lignes */
        if ( gsb_data_report_text_comparison_get_link_to_last_text_comparison ( text_comparison_number ) != -1
         &&
         tmp_list != gsb_data_report_get_text_comparison_list ( report_number ) )
        {
            gtk_combo_box_set_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_link ( text_comparison_number ) ),
                                gsb_data_report_text_comparison_get_link_to_last_text_comparison (
                                text_comparison_number ) );
        }

        gtk_combo_box_set_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_field ( text_comparison_number ) ),
                                gsb_data_report_text_comparison_get_field ( text_comparison_number ) );
        gtk_combo_box_set_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_operator ( text_comparison_number ) ),
                                gsb_data_report_text_comparison_get_operator ( text_comparison_number ) );

        if ( gsb_data_report_text_comparison_get_text ( text_comparison_number ) )
            gtk_entry_set_text ( GTK_ENTRY (
                                gsb_data_report_text_comparison_get_entry_text ( text_comparison_number ) ),
                                gsb_data_report_text_comparison_get_text ( text_comparison_number ) );

        gtk_combo_box_set_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_first_comparison (
                                text_comparison_number ) ),
                                gsb_data_report_text_comparison_get_first_comparison ( text_comparison_number ) );
        gtk_combo_box_set_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_link_first_to_second_part (
                                text_comparison_number ) ),
                                gsb_data_report_text_comparison_get_link_first_to_second_part (
                                text_comparison_number ) );
        gtk_combo_box_set_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_second_comparison (
                                text_comparison_number ) ),
                                gsb_data_report_text_comparison_get_second_comparison ( text_comparison_number ) );

        tmp_str = utils_str_itoa ( gsb_data_report_text_comparison_get_first_amount ( text_comparison_number ) );
        gtk_entry_set_text ( GTK_ENTRY (
                                gsb_data_report_text_comparison_get_entry_first_amount (
                                text_comparison_number ) ),
                                tmp_str );
        g_free ( tmp_str );

        tmp_str = utils_str_itoa ( gsb_data_report_text_comparison_get_second_amount ( text_comparison_number ) );
        gtk_entry_set_text ( GTK_ENTRY (
                                gsb_data_report_text_comparison_get_entry_second_amount (
                                text_comparison_number ) ),
                                tmp_str );
        g_free ( tmp_str );

        if ( gsb_data_report_text_comparison_get_use_text ( text_comparison_number ) )
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (
                                gsb_data_report_text_comparison_get_button_use_text ( text_comparison_number ) ),
                                TRUE );
        else
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (
                                gsb_data_report_text_comparison_get_button_use_number ( text_comparison_number ) ),
                                TRUE );

        /* on désensitive tous ce qui est nécessaire */
        if ( gsb_data_report_text_comparison_get_field ( text_comparison_number ) == 8
             ||
             gsb_data_report_text_comparison_get_field ( text_comparison_number ) == 9
             ||
             gsb_data_report_text_comparison_get_field ( text_comparison_number ) == 10 )
        {
            /* 	  on est sur un chq ou une pc */
            /* on rend sensitif les check button et la hbox correspondante */
            sensitive_widget ( NULL, gsb_data_report_text_comparison_get_button_use_text (
                                text_comparison_number ) );
            sensitive_widget ( NULL, gsb_data_report_text_comparison_get_button_use_number (
                                text_comparison_number ) );
            etats_config_onglet_texte_sensitive_hbox_fonction_bouton_txt (
                                text_comparison_number );
        }
        else
        {
            desensitive_widget ( NULL, gsb_data_report_text_comparison_get_button_use_text (
                                text_comparison_number ) );
            desensitive_widget (NULL, gsb_data_report_text_comparison_get_button_use_number (
                                text_comparison_number ) );
            desensitive_widget (NULL, gsb_data_report_text_comparison_get_hbox_cheque (
                                text_comparison_number ) );
            sensitive_widget (NULL, gsb_data_report_text_comparison_get_hbox_text (
                                text_comparison_number ) );
        }

        /* on sensitive/désensitive l'entrée txt*/
        if ( gsb_data_report_text_comparison_get_operator ( text_comparison_number ) >= 4 )
            desensitive_widget (NULL, gsb_data_report_text_comparison_get_entry_text (
                                text_comparison_number ) );

        /* on sensitive/désensitive les entrées de montant si nécessaire */
        if ( gsb_data_report_text_comparison_get_first_comparison ( text_comparison_number ) == 6 )
            desensitive_widget (NULL, gsb_data_report_text_comparison_get_entry_first_amount (
                                text_comparison_number ) );

        if ( gsb_data_report_text_comparison_get_second_comparison ( text_comparison_number ) == 6 )
            desensitive_widget (NULL, gsb_data_report_text_comparison_get_entry_second_amount (
                                text_comparison_number ) );

        /* on sensitive/désensitive la hbox_2 si nécessaire */
        if ( gsb_data_report_text_comparison_get_link_first_to_second_part ( text_comparison_number ) == 3 )
            desensitive_widget (NULL, gsb_data_report_text_comparison_get_hbox_second_part (
                                text_comparison_number ) );

        gtk_widget_show ( widget );

        multi_lignes = TRUE;
        tmp_list = tmp_list -> next;
    }
}


/**
 * Supprime une ligne de comparaison de texte
 *
 * \param last_text_comparison_number
 *
 * \return
 */
static void etats_config_onglet_texte_retire_ligne_liste_comparaisons ( gint last_text_comparison_number )
{
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ( );

    /* il faut qu'il y ai plus d'une row affichée */
    if ( g_slist_length ( gsb_data_report_get_text_comparison_list ( current_report_number ) ) < 2 )
        return;

    /* on commence par supprimer la row dans la liste */
    gtk_widget_destroy ( gsb_data_report_text_comparison_get_vbox_line ( last_text_comparison_number ) );

    /* si la structure qu'on retire est la 1ère, on vire le widget de lient */
    if ( !g_slist_index ( gsb_data_report_get_text_comparison_list ( current_report_number ),
     GINT_TO_POINTER ( last_text_comparison_number ) ) )
    {
        gint text_comparison_number;

        text_comparison_number = GPOINTER_TO_INT ( gsb_data_report_get_text_comparison_list (
                                    current_report_number)-> next -> data ) ;
        gtk_widget_destroy ( gsb_data_report_text_comparison_get_button_link ( text_comparison_number ) );
        gsb_data_report_text_comparison_set_button_link ( text_comparison_number, NULL );
    }

    /* et on retire la struct de la sliste */
    gsb_data_report_set_text_comparison_list ( current_report_number,
                        g_slist_remove ( gsb_data_report_get_text_comparison_list ( current_report_number ),
                        GINT_TO_POINTER ( last_text_comparison_number ) ) );
}


/**
 * création des boutons add et remove
 *
 * \param parent
 * \param text_comparison_number numéro à transmettre à la fonction de callback
 *
 * \return
 * */
static void etats_config_onglet_texte_get_buttons_add_remove ( GtkWidget *parent,
                        gint text_comparison_number )
{
    GtkWidget *button;

    button = gtk_button_new_with_label ( _("Add") );
    gtk_widget_show ( button );
    gtk_button_set_relief ( GTK_BUTTON ( button ), GTK_RELIEF_NORMAL );

    g_signal_connect_swapped ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( etats_config_onglet_texte_ajoute_ligne_liste_comparaisons ),
                        GINT_TO_POINTER ( text_comparison_number ) );

    gtk_box_pack_start ( GTK_BOX ( parent ), button, TRUE, TRUE, MARGIN_BOX );

    button = gtk_button_new_with_label ( _("Remove") );
    gtk_button_set_relief ( GTK_BUTTON ( button ), GTK_RELIEF_NORMAL );

    g_signal_connect_swapped ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( etats_config_onglet_texte_retire_ligne_liste_comparaisons ),
                        GINT_TO_POINTER ( text_comparison_number ) );

    gtk_box_pack_start ( GTK_BOX ( parent ), button, TRUE, TRUE, MARGIN_BOX );
}


/**
 * Initialise les informations de l'onglet textes
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_initialise_onglet_textes ( gint report_number )
{
    /* on affiche ou pas le choix des textes */
    etats_prefs_button_toggle_set_actif ( "bouton_utilise_texte",
                        gsb_data_report_get_text_comparison_used ( report_number ) );

    /* on remplit la liste des lignes de recherche */
    etats_config_onglet_texte_remplit_liste_comparaisons ( report_number );
}


/**
 * Récupère les informations de l'onglet textes
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_textes ( gint report_number )
{
    GSList *comparison_list;

    /* on récupère le choix des textes */
    gsb_data_report_set_text_comparison_used ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_utilise_texte" ) );

    /* récupération de la liste des comparaisons de texte */
    /* on a rentré au moins une comparaison */
    /* on rempli les champs de la structure */
    comparison_list = gsb_data_report_get_text_comparison_list ( report_number );

    while ( comparison_list )
    {
        const gchar *string;
        gint text_comparison_number;

        text_comparison_number = GPOINTER_TO_INT ( comparison_list -> data );

        if ( gsb_data_report_text_comparison_get_button_link ( text_comparison_number ) )
            gsb_data_report_text_comparison_set_link_to_last_text_comparison ( text_comparison_number,
                        gtk_combo_box_get_active ( GTK_COMBO_BOX (
                        gsb_data_report_text_comparison_get_button_link ( text_comparison_number ) ) ) );
        else
            gsb_data_report_text_comparison_set_link_to_last_text_comparison ( text_comparison_number, -1 );

        gsb_data_report_text_comparison_set_field ( text_comparison_number,
                                gtk_combo_box_get_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_field (
                                text_comparison_number ) ) ) );

        gsb_data_report_text_comparison_set_use_text ( text_comparison_number,
                                gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (
                                gsb_data_report_text_comparison_get_button_use_text (
                                text_comparison_number ) ) ) );
        gsb_data_report_text_comparison_set_operator ( text_comparison_number,
                                gtk_combo_box_get_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_operator (
                                text_comparison_number ) ) ) );

        string = gtk_entry_get_text ( GTK_ENTRY (
                                gsb_data_report_text_comparison_get_entry_text ( text_comparison_number ) ) );
        gsb_data_report_text_comparison_set_text ( text_comparison_number, string );

        gsb_data_report_text_comparison_set_first_comparison ( text_comparison_number,
                                gtk_combo_box_get_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_first_comparison (
                                text_comparison_number ) ) ) );

        gsb_data_report_text_comparison_set_link_first_to_second_part ( text_comparison_number,
                                gtk_combo_box_get_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_link_first_to_second_part (
                                text_comparison_number ) ) ) );
        gsb_data_report_text_comparison_set_second_comparison ( text_comparison_number,
                                gtk_combo_box_get_active ( GTK_COMBO_BOX (
                                gsb_data_report_text_comparison_get_button_second_comparison (
                                text_comparison_number ) ) ) );
        gsb_data_report_text_comparison_set_first_amount ( text_comparison_number,
                                utils_str_atoi ( gtk_entry_get_text ( GTK_ENTRY (
                                gsb_data_report_text_comparison_get_entry_first_amount (
                                text_comparison_number ) ) ) ) );
        gsb_data_report_text_comparison_set_second_amount ( text_comparison_number,
                                utils_str_atoi (gtk_entry_get_text ( GTK_ENTRY (
                                gsb_data_report_text_comparison_get_entry_second_amount (
                                text_comparison_number ) ) ) ) );

        comparison_list = comparison_list -> next;
    }
}


/*ONGLET_MONTANTS*/
/**
 * crée une ligne de recherche de montant
 *
 * \parent  vbox qui contiendra toutes les lignes
 * \param   première ligne ou ligne supplémentaire
 * \param   avec lien
 *
 * \return la nouvelle ligne
 */
static GtkWidget *etats_config_onglet_montants_new_comparison_line ( GtkWidget *parent,
                        gint amount_comparison_number,
                        gint with_link )
{
    GtkWidget *hbox;
    GtkWidget *hbox_2;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *combo;

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );

    /* on crée le premier lien ne sert pas si c'est la première ligne */
    if ( with_link )
    {
        combo = utils_combo_box_make_from_string_array ( champs_lien_lignes_comparaison );
        gsb_data_report_amount_comparison_set_button_link ( amount_comparison_number, combo );
        gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 5 );
    }

    label = gtk_label_new ( _("Transactions with an amount of") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    /* on crée et initialise le premier combobox de comparaison de montant */
    combo = utils_combo_box_make_from_string_array ( champs_comparateur_montant );
    gsb_data_report_amount_comparison_set_button_first_comparison ( amount_comparison_number, combo );
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("to") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    /* on ajoute l'entry pour la saisie du premier montant */
    entry = gtk_entry_new ( );
    gtk_widget_set_size_request ( entry, 100, -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), entry, FALSE, FALSE, 0 );
    gsb_data_report_amount_comparison_set_entry_first_amount ( amount_comparison_number, entry );

    /* on crée le bouton de lien pour la deuxième partie de la comparaison*/
    combo = utils_combo_box_make_from_string_array ( champs_lien_nombre_2 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 0 );
    gsb_data_report_amount_comparison_set_button_link_first_to_second_part ( amount_comparison_number, combo );

    /* the hbox is created in hbox_2 and is used later */
    hbox_2 = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gsb_data_report_amount_comparison_set_hbox_second_part ( amount_comparison_number, hbox_2 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), hbox_2, FALSE, FALSE, 0 );

    /* maintenant que hbox_2 est définie on met le signal lorsque l'on change le choix du combobox */
    g_signal_connect ( G_OBJECT ( combo ),
                        "changed",
                        G_CALLBACK ( etats_config_combo_lien_nombre_2_changed ),
                        hbox_2 );

    /* on peut maintenant ajouter dans comp_montants -> hbox_partie_2 */
    combo = utils_combo_box_make_from_string_array ( champs_comparateur_montant );
    gsb_data_report_amount_comparison_set_button_second_comparison ( amount_comparison_number, combo );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), combo, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("to") );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), label, FALSE, FALSE, 0 );

    /* on ajoute l'entry pour la saisie du premier montant */
    entry = gtk_entry_new ( );
    gtk_widget_set_size_request ( entry, 100, -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), entry, FALSE, FALSE, 0 );
    gsb_data_report_amount_comparison_set_entry_second_amount ( amount_comparison_number, entry );

    /* on ajoute le bouton ajouter une nouvelle ligne */
    etats_config_onglet_montants_get_buttons_add_remove ( hbox, amount_comparison_number );

    /* on met la ligne complète (vbox) dans son parent */
    gtk_box_pack_start ( GTK_BOX ( parent ), hbox, FALSE, FALSE, 0 );
    gtk_widget_show_all ( hbox );

    /* return */
    return hbox;
}


/**
 * ajoute une ligne de recherche de montant à la liste des lignes de recherche de montant
 *
 *\param last_amount_comparison_number
 *
 *\return
 */
static void etats_config_onglet_montants_ajoute_ligne_liste_comparaisons ( gint last_amount_comparison_number )
{
    GtkWidget *lignes;
    GtkWidget *widget;
    gint amount_comparison_number;
    gint position;
    gint current_report_number;

    lignes = etats_prefs_widget_get_widget_by_name ( "liste_montants_etat", NULL );
    current_report_number = gsb_gui_navigation_get_current_report ();

    /* on récupère tout de suite la position à laquelle il faut insérer la row */
    if ( last_amount_comparison_number )
        position = g_slist_index ( gsb_data_report_get_amount_comparison_list ( current_report_number ),
                                GINT_TO_POINTER ( last_amount_comparison_number ) ) + 1;
    else
        position = 0;

    /* on commence par créer une structure vide */
    amount_comparison_number = gsb_data_report_amount_comparison_new ( 0 );
    gsb_data_report_amount_comparison_set_report_number ( amount_comparison_number,
                        current_report_number);

    /* on crée la row et remplit les widget de la structure */
    widget = etats_config_onglet_montants_new_comparison_line ( lignes, amount_comparison_number, TRUE );
    gsb_data_report_amount_comparison_set_hbox_line ( amount_comparison_number, widget );

    /* on vire le lien de la row s'il n'y a pas encore de liste (cad si c'est la 1ère row) */
    if ( !gsb_data_report_get_amount_comparison_list ( current_report_number ) )
    {
        gtk_widget_destroy ( gsb_data_report_amount_comparison_get_button_link ( amount_comparison_number ) );
        gsb_data_report_amount_comparison_set_button_link ( amount_comparison_number, NULL );
    }

    /*   par défaut, le bouton bouton_lien_1_2 est sur stop */
    gtk_combo_box_set_active ( GTK_COMBO_BOX (
                        gsb_data_report_amount_comparison_get_button_link_first_to_second_part (
                        amount_comparison_number ) ),
                        3 );
    gsb_data_report_amount_comparison_set_link_first_to_second_part ( amount_comparison_number, 3 );
    gtk_widget_set_sensitive ( gsb_data_report_amount_comparison_get_hbox_second_part (
                        amount_comparison_number ),
                        FALSE );

    /* on met la structure dans la liste à la position demandée */
    gsb_data_report_set_amount_comparison_list ( current_report_number,
                        g_slist_insert ( gsb_data_report_get_amount_comparison_list ( current_report_number ),
                        GINT_TO_POINTER ( amount_comparison_number ),
                        position ) );


    /* on met la row à sa place dans la liste */
    gtk_box_reorder_child ( GTK_BOX ( lignes ),
                        gsb_data_report_amount_comparison_get_hbox_line ( amount_comparison_number ),
                        position );
}


/**
 * remplit la liste des comparaisons de montants
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_onglet_montants_remplit_liste_comparaisons ( gint report_number )
{
    GtkWidget *lignes;
    GSList *list_tmp;
    gchar* tmp_str;
    gboolean multi_lignes = FALSE;

    /* on commence par effacer les anciennes lignes */
    lignes = etats_prefs_widget_get_widget_by_name ( "liste_montants_etat", NULL );
    gtk_container_foreach (GTK_CONTAINER (lignes), (GtkCallback) etats_config_onglet_texte_montant_gtk_callback, NULL);

    list_tmp = gsb_data_report_get_amount_comparison_list ( report_number );
    /*   s'il n'y a rien dans la liste, on met juste une row vide */
    if ( !list_tmp )
    {
        etats_config_onglet_montants_ajoute_ligne_liste_comparaisons ( 0 );
        return;
    }

    /* on fait le tour de la liste des comparaisons de montant, ajoute une row
     * et la remplit à chaque fois */
    while ( list_tmp )
    {
        gint amount_comparison_number;
        GtkWidget *hbox;

        amount_comparison_number = GPOINTER_TO_INT ( list_tmp -> data );
        hbox = etats_config_onglet_montants_new_comparison_line ( lignes, amount_comparison_number, multi_lignes );

        /* on crée la row et remplit les widget de la structure */
        gsb_data_report_amount_comparison_set_hbox_line ( amount_comparison_number, hbox );

        /* on remplit maintenant les widget avec les valeurs de la stucture */
        /* s'il n'y a pas de lien avec la struct précédente, on le vire
         * on rajoute le && car parfois le bouton de lien se met quand même en 1ère row */

        if ( gsb_data_report_amount_comparison_get_link_to_last_amount_comparison ( amount_comparison_number ) != -1
         &&
         list_tmp != gsb_data_report_get_amount_comparison_list ( report_number ) )
        {
            gtk_combo_box_set_active ( GTK_COMBO_BOX (
                        gsb_data_report_amount_comparison_get_button_link ( amount_comparison_number ) ),
                        gsb_data_report_amount_comparison_get_link_to_last_amount_comparison (
                        amount_comparison_number ) );
        }

        gtk_combo_box_set_active ( GTK_COMBO_BOX (
                        gsb_data_report_amount_comparison_get_button_first_comparison (
                        amount_comparison_number ) ),
                        gsb_data_report_amount_comparison_get_first_comparison ( amount_comparison_number ) );
        gtk_combo_box_set_active ( GTK_COMBO_BOX (
                        gsb_data_report_amount_comparison_get_button_link_first_to_second_part (
                        amount_comparison_number ) ),
                        gsb_data_report_amount_comparison_get_link_first_to_second_part (
                        amount_comparison_number ) );
        gtk_combo_box_set_active ( GTK_COMBO_BOX (
                        gsb_data_report_amount_comparison_get_button_second_comparison (
                        amount_comparison_number ) ),
                        gsb_data_report_amount_comparison_get_second_comparison (
                        amount_comparison_number ) );

        tmp_str = utils_real_get_string ( gsb_data_report_amount_comparison_get_first_amount (
                        amount_comparison_number ) );
        gtk_entry_set_text ( GTK_ENTRY ( gsb_data_report_amount_comparison_get_entry_first_amount (
                        amount_comparison_number ) ),
                        tmp_str );
        g_free ( tmp_str );

        tmp_str = utils_real_get_string ( gsb_data_report_amount_comparison_get_second_amount (
                        amount_comparison_number ) );
        gtk_entry_set_text ( GTK_ENTRY ( gsb_data_report_amount_comparison_get_entry_second_amount (
                        amount_comparison_number ) ),
                        tmp_str );
        g_free ( tmp_str );

        /* on sensitive/désensitive les entrées si nécessaire */
        if ( gsb_data_report_amount_comparison_get_first_comparison ( amount_comparison_number ) > 5 )
            desensitive_widget ( NULL, gsb_data_report_amount_comparison_get_entry_first_amount (
                        amount_comparison_number ) );
        else
            sensitive_widget ( NULL, gsb_data_report_amount_comparison_get_entry_first_amount (
                        amount_comparison_number ) );

        if ( gsb_data_report_amount_comparison_get_second_comparison ( amount_comparison_number ) > 5 )
            desensitive_widget ( NULL, gsb_data_report_amount_comparison_get_entry_second_amount (
                        amount_comparison_number ) );
        else
            sensitive_widget ( NULL, gsb_data_report_amount_comparison_get_entry_second_amount (
                        amount_comparison_number ) );

        /* on sensitive/désensitive la hbox_2 si nécessaire */
        if ( gsb_data_report_amount_comparison_get_link_first_to_second_part ( amount_comparison_number ) == 3 )
            desensitive_widget ( NULL, gsb_data_report_amount_comparison_get_hbox_second_part (
                        amount_comparison_number ) );
        else
            sensitive_widget ( NULL, gsb_data_report_amount_comparison_get_hbox_second_part (
                        amount_comparison_number ) );

        list_tmp = list_tmp -> next;
    }
}


/**
 * supprime une liste de comparaison
 *
 * \param
 *
 * \return
 * */
static void etats_config_onglet_montants_retire_ligne_liste_comparaisons ( gint last_amount_comparison_number )
{
     gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    /* il faut qu'il y ai plus d'une row affichée */
    if ( g_slist_length ( gsb_data_report_get_amount_comparison_list ( current_report_number ) ) < 2 )
        return;

    /* on commence par supprimer la row dans la liste */
    gtk_widget_destroy ( gsb_data_report_amount_comparison_get_hbox_line ( last_amount_comparison_number ) );

    /* si la structure qu'on retire est la 1ère, on vire le widget de lient */
    if ( !g_slist_index ( gsb_data_report_get_amount_comparison_list ( current_report_number ),
     GINT_TO_POINTER ( last_amount_comparison_number ) ) )
    {
        gint amount_comparison_number;

        amount_comparison_number = GPOINTER_TO_INT ( gsb_data_report_get_amount_comparison_list (
                        current_report_number )-> next -> data );
        gtk_widget_destroy ( gsb_data_report_amount_comparison_get_button_link ( amount_comparison_number ) );
        gsb_data_report_amount_comparison_set_button_link ( amount_comparison_number, NULL );
    }

    /* et on retire la struct de la sliste */
    gsb_data_report_set_amount_comparison_list ( current_report_number,
                        g_slist_remove ( gsb_data_report_get_amount_comparison_list ( current_report_number ),
                        GINT_TO_POINTER ( last_amount_comparison_number ) ) );
}


/**
 * création des boutons add et remove
 *
 * \param parent
 * \param text_comparison_number numéro à transmettre à la fonction de callback
 *
 * \return
 * */
static void etats_config_onglet_montants_get_buttons_add_remove ( GtkWidget *parent,
                        gint text_comparison_number )
{
    GtkWidget *button;

    button = gtk_button_new_with_label ( _("Add") );
    gtk_widget_show ( button );
    gtk_button_set_relief ( GTK_BUTTON ( button ), GTK_RELIEF_NONE );

    g_signal_connect_swapped ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( etats_config_onglet_montants_ajoute_ligne_liste_comparaisons ),
                        GINT_TO_POINTER ( text_comparison_number ) );

    gtk_box_pack_start ( GTK_BOX ( parent ), button, TRUE, TRUE, 0 );

    button = gtk_button_new_with_label ( _("Remove") );
    gtk_button_set_relief ( GTK_BUTTON ( button ), GTK_RELIEF_NONE );

    g_signal_connect_swapped ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( etats_config_onglet_montants_retire_ligne_liste_comparaisons ),
                        GINT_TO_POINTER ( text_comparison_number ) );

    gtk_box_pack_start ( GTK_BOX ( parent ), button, TRUE, TRUE, 0 );
}


/**
 * Initialise les informations de l'onglet montants
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_initialise_onglet_montants ( gint report_number )
{
    /* on affiche ou pas le choix des montants */
    etats_prefs_button_toggle_set_actif ( "bouton_utilise_montant",
                        gsb_data_report_get_amount_comparison_used ( report_number ) );

    /* on remplit la liste des lignes de recherche */
    etats_config_onglet_montants_remplit_liste_comparaisons ( report_number );
}


/**
 * Récupère les informations de l'onglet montants
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_montants ( gint report_number )
{
    gint amount_comparison_number;

    /* on récupère le choix des montants */
    gsb_data_report_set_amount_comparison_used ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_utilise_montant" ) );

    /* récupération de la liste des comparaisons de montant */
    /*   il y a au moins une structure de créé, si celle si a les 2 montants vides, */
    /* c'est qu'il n'y a aucune liste */
    amount_comparison_number = GPOINTER_TO_INT ( gsb_data_report_get_amount_comparison_list (
                                report_number )-> data );

    if ( g_slist_length ( gsb_data_report_get_amount_comparison_list ( report_number ) ) == 1
     &&
     !strlen ( gtk_entry_get_text ( GTK_ENTRY ( gsb_data_report_amount_comparison_get_entry_first_amount (
     amount_comparison_number ) ) ) )
     &&
     !strlen ( gtk_entry_get_text ( GTK_ENTRY ( gsb_data_report_amount_comparison_get_entry_second_amount (
     amount_comparison_number ) ) ) )
     &&
     gtk_widget_get_sensitive ( gsb_data_report_amount_comparison_get_entry_first_amount (
     amount_comparison_number ) )
     &&
     gtk_widget_get_sensitive ( gsb_data_report_amount_comparison_get_entry_second_amount (
     amount_comparison_number ) ) )
    {
        g_slist_free ( gsb_data_report_get_amount_comparison_list ( report_number ) );
        gsb_data_report_set_amount_comparison_list ( report_number, NULL );
    }
    else
    {
        /* on a rentré au moins une comparaison */
        /* on rempli les champs de la structure */
        GSList *comparison_list;

        comparison_list = gsb_data_report_get_amount_comparison_list ( report_number );

        while ( comparison_list )
        {
            amount_comparison_number = GPOINTER_TO_INT ( comparison_list -> data );

            if ( gsb_data_report_amount_comparison_get_button_link ( amount_comparison_number ) )
                gsb_data_report_amount_comparison_set_link_to_last_amount_comparison ( amount_comparison_number,
                                gtk_combo_box_get_active ( GTK_COMBO_BOX (
                                gsb_data_report_amount_comparison_get_button_link (
                                amount_comparison_number ) ) ) );
            else
                gsb_data_report_amount_comparison_set_link_to_last_amount_comparison (
                                amount_comparison_number,
                                -1 );

            gsb_data_report_amount_comparison_set_first_comparison ( amount_comparison_number,
                        gtk_combo_box_get_active ( GTK_COMBO_BOX (
                        gsb_data_report_amount_comparison_get_button_first_comparison (
                        amount_comparison_number ) ) ) );

            gsb_data_report_amount_comparison_set_link_first_to_second_part ( amount_comparison_number,
                        gtk_combo_box_get_active ( GTK_COMBO_BOX (
                        gsb_data_report_amount_comparison_get_button_link_first_to_second_part (
                        amount_comparison_number ) ) ) );
            gsb_data_report_amount_comparison_set_second_comparison ( amount_comparison_number,
                        gtk_combo_box_get_active ( GTK_COMBO_BOX (
                        gsb_data_report_amount_comparison_get_button_second_comparison (
                        amount_comparison_number ) ) ) );

            gsb_data_report_amount_comparison_set_first_amount ( amount_comparison_number,
                        utils_real_get_from_string ( gtk_entry_get_text ( GTK_ENTRY (
                        gsb_data_report_amount_comparison_get_entry_first_amount (
                        amount_comparison_number ) ) ) ) );
            gsb_data_report_amount_comparison_set_second_amount ( amount_comparison_number,
                        utils_real_get_from_string ( gtk_entry_get_text ( GTK_ENTRY (
                        gsb_data_report_amount_comparison_get_entry_second_amount (
                        amount_comparison_number ) ) ) ) );

            comparison_list = comparison_list -> next;
        }
    }

    gsb_data_report_set_amount_comparison_only_report_non_null ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_exclure_ope_nulles_etat" ) );
}


/*ONGLET_MODE_PAIEMENT*/
/**
 * Initialise les informations de l'onglet modes de paiement
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_initialise_onglet_mode_paiement ( gint report_number )
{
    gint active;

    active = gsb_data_report_get_method_of_payment_used ( report_number );
    etats_prefs_button_toggle_set_actif ( "bouton_detaille_mode_paiement_etat", active );

    if ( active )
    {
        etats_prefs_onglet_mode_paiement_select_rows_from_list (
                                gsb_data_report_get_method_of_payment_list ( report_number ),
                                "treeview_mode_paiement" );

        if ( g_slist_length ( gsb_data_report_get_method_of_payment_list ( report_number ) ) )
            utils_togglebutton_set_label_position_unselect (
                                etats_prefs_widget_get_widget_by_name (
                                "togglebutton_select_all_mode_paiement", NULL ),
                                NULL,
                                etats_prefs_widget_get_widget_by_name ( "treeview_mode_paiement", NULL ) );
    }
}


/**
 * Récupère les informations de l'onglet mode de paiement
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_mode_paiement ( gint report_number )
{
    gint active;

    active = etats_prefs_button_toggle_get_actif ( "bouton_detaille_mode_paiement_etat" );
    gsb_data_report_set_method_of_payment_used ( report_number, active );
    if ( active )
    {
        gsb_data_report_free_method_of_payment_list ( report_number );

        if ( utils_tree_view_all_rows_are_selected ( GTK_TREE_VIEW (
         etats_prefs_widget_get_widget_by_name ( "treeview_mode_paiement", NULL ) ) ) )
        {
            gchar *text;
            gchar *hint;

            hint = g_strdup ( _("Performance issue.") );
            text = g_strdup ( _("All methods of payment have been selected.  Grisbi will run "
                            "faster without the \"Detail methods of payment used\" option activated.") );

            dialogue_hint ( text, hint );
            etats_prefs_button_toggle_set_actif ( "togglebutton_select_all_mode_paiement", FALSE );
            gsb_data_report_set_method_of_payment_used ( report_number, 0 );

            g_free ( text );
            g_free ( hint );
        }
        else
            gsb_data_report_set_method_of_payment_list ( report_number,
                            etats_prefs_onglet_mode_paiement_get_list_rows_selected (
                            "treeview_mode_paiement" ) );

    }
}


/**
 *
 *
 * \param
 *
 * \return
 * */
GtkTreeModel *etats_config_onglet_mode_paiement_get_model ( void )
{
    GtkListStore *list_store;
    GSList *liste_nom_types = NULL;
    GSList *list_tmp;

    list_store = gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_INT );

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( list_store ),
                        0, GTK_SORT_ASCENDING );

    /* create a list of unique names */
    list_tmp = gsb_data_payment_get_payments_list ( );

    while ( list_tmp )
    {
        GtkTreeIter iter;
        gchar *name;
        gint payment_number;

        payment_number = gsb_data_payment_get_number (list_tmp -> data);
        name = my_strdup ( gsb_data_payment_get_name ( payment_number ) );

        if ( !g_slist_find_custom ( liste_nom_types,
                        name,
                        ( GCompareFunc ) cherche_string_equivalente_dans_slist ) )
        {
            liste_nom_types = g_slist_append ( liste_nom_types, name );
            gtk_list_store_append ( list_store, &iter );
            gtk_list_store_set ( list_store, &iter, 0, name, 1, payment_number, -1 );
        }
        else
            g_free ( name );

        list_tmp = list_tmp -> next;
    }

    /* on libère la mémoire utilisée par liste_nom_types */
    g_slist_free_full ( liste_nom_types, ( GDestroyNotify ) g_free );

    /* return */
    return GTK_TREE_MODEL ( list_store );
}


/*ONGLET_DIVERS*/
/**
 * Initialise les informations de l'onglet divers
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_initialise_onglet_divers ( gint report_number )
{
    gint index;

    index = gsb_data_report_get_show_m ( report_number );
    etats_prefs_buttons_radio_set_active_index ( "radiobutton_marked_all", index );

    etats_prefs_button_toggle_set_actif ( "checkbutton_marked_P",
                        gsb_data_report_get_show_p ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "checkbutton_marked_R",
                        gsb_data_report_get_show_r ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "checkbutton_marked_T",
                        gsb_data_report_get_show_t ( report_number ) );

    etats_prefs_button_toggle_set_actif ( "bouton_pas_detailler_ventilation",
                        gsb_data_report_get_not_detail_split ( report_number ) );
}


/**
 * Récupère les informations de l'onglet divers
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_divers ( gint report_number )
{
    gint index;

    index = etats_prefs_buttons_radio_get_active_index ( "radiobutton_marked_all" );
    gsb_data_report_set_show_m ( report_number, index );

    gsb_data_report_set_show_p ( report_number,
                        etats_prefs_button_toggle_get_actif ( "checkbutton_marked_P" ) );
    gsb_data_report_set_show_r ( report_number,
                        etats_prefs_button_toggle_get_actif ( "checkbutton_marked_R" ) );
    gsb_data_report_set_show_t ( report_number,
                        etats_prefs_button_toggle_get_actif ( "checkbutton_marked_T" ) );

    gsb_data_report_set_not_detail_split ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_pas_detailler_ventilation" ) );
}


/*ONGLET_DATA_GROUPING*/
/**
 * return la liste des données par ordre de tri
 *
 * \param report_number
 *
 * \return a GSList
 * */
static GSList *etats_config_onglet_data_grouping_get_list ( gint report_number )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GSList *tmp_list = NULL;

    tree_view = etats_prefs_widget_get_widget_by_name ( "treeview_data_grouping", NULL );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    if ( gtk_tree_model_get_iter_first ( model, &iter ) )
    {
        do
        {
            gint type_data;

            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter, 2, &type_data, -1 );
            tmp_list = g_slist_append ( tmp_list, GINT_TO_POINTER ( type_data ) );

            /* on ajoute les sous catégories ici */
            if ( type_data == 1 )
            {
                tmp_list = g_slist_append ( tmp_list, GINT_TO_POINTER ( 2 ) );
            }
            /* et les sous imputations ici */
            else if ( type_data == 3 )
            {
                tmp_list = g_slist_append ( tmp_list, GINT_TO_POINTER ( 4 ) );
            }
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );

        return tmp_list;
    }

    /* return */
    return NULL;
}

/**
 * Update le modèle avec les données sauvegardées
 *
 * \param report_number
 *
 * \return TRUE
 */
static gboolean etats_config_onglet_data_grouping_update_model ( gint report_number )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GSList *tmp_list;
    gint i = 0;

    tree_view = etats_prefs_widget_get_widget_by_name ( "treeview_data_grouping", NULL );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    /* on reset le model */
    gtk_list_store_clear ( GTK_LIST_STORE ( model ) );

    /* on remplit la liste des données */
    tmp_list = gsb_data_report_get_sorting_type_list ( report_number );

    while ( tmp_list )
    {
        GtkTreeIter iter;
        gchar *string = NULL;
        gint type_data;

        type_data = GPOINTER_TO_INT ( tmp_list->data );

        string = etats_config_onglet_data_grouping_get_string ( type_data, i );
        if ( !string )
        {
            tmp_list = tmp_list->next;
            continue;
        }

        gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
        gtk_list_store_set ( GTK_LIST_STORE ( model ), &iter, 0, string, 1, i, 2, type_data, -1 );

        g_free ( string );

        i++;
        tmp_list = tmp_list->next;
    }

    /* return */
    return TRUE;
}


/**
 * Initialise les informations de l'onglet groupement des données
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_initialise_onglet_data_grouping ( gint report_number )
{
    etats_prefs_button_toggle_set_actif ( "bouton_regroupe_ope_compte_etat",
                        gsb_data_report_get_account_group_reports ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_utilise_tiers_etat",
                        gsb_data_report_get_payee_used ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_group_by_categ",
                        gsb_data_report_get_category_used ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_utilise_ib_etat",
                        gsb_data_report_get_budget_used ( report_number ) );

    etats_config_onglet_data_grouping_update_model ( report_number );
}


/**
 * Récupère les informations de l'onglet groupement des données
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_data_grouping ( gint report_number )
{
    gsb_data_report_set_account_group_reports ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_regroupe_ope_compte_etat" ) );
    gsb_data_report_set_payee_used ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_utilise_tiers_etat" ) );
    gsb_data_report_set_category_used ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_group_by_categ" ) );
    gsb_data_report_set_budget_used ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_utilise_ib_etat" ) );

    /* on récupère les données servant au tri des données pour l'affichage */
    gsb_data_report_free_sorting_type_list ( report_number );
    gsb_data_report_set_sorting_type_list ( report_number,
                        etats_config_onglet_data_grouping_get_list ( report_number ) );
}


/**
 * retourne une chaine de caractère formatée en fonction du type de donnée
 * et de la position dans la liste
 *
 * \param type_data     type de donnée : 1 Categ, 3 IB, 5 Account, 6 Payee.
 * \param pos           numéro de ligne dans le modèle
 *
 * \return NULL or a formatted string
 */
gchar *etats_config_onglet_data_grouping_get_string ( gint type_data,
                        gint pos )
{
    gchar *string = NULL;
    gchar *text = NULL;

    switch ( type_data )
    {
        case 1:
        text = g_strdup ( _("Category") );
        break;

        case 3:
        text = g_strdup ( _("Budgetary line") );
        break;

        case 5:
        text = g_strdup ( _("Account") );
        break;

        case 6:
        text = g_strdup ( _("Payee") );
        break;
    }

    if ( !text )
        return NULL;

    if ( pos <= 0 )
        string = text;
    else if ( pos == 1 )
    {
        string = g_strconcat ( "\t", text, NULL );
        g_free ( text );
    }
    else if ( pos == 2 )
    {
        string = g_strconcat ( "\t\t", text, NULL );
        g_free ( text );
    }
    else if ( pos == 3 )
    {
        string = g_strconcat ( "\t\t\t", text, NULL );
        g_free ( text );
    }

    return string;
}

/**
 * déplace un item suite à un drag and drop dans le tree_view
 *
 * \param src_pos           position avant
 * \param src_type_data     type de donnée
 * \param dst_pos           position après drag and drop
 *
 * \return
 * */
void etats_config_onglet_data_grouping_move_in_list ( gint src_pos,
                        gint src_type_data,
                        gint dst_pos )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GSList *tmp_list;
    gint report_number;
    gint i = 0;

    report_number = gsb_gui_navigation_get_current_report ( );
    tmp_list = g_slist_copy ( gsb_data_report_get_sorting_type_list ( report_number ) );

    /* on supprime les sous categ et les sous IB */
    tmp_list = g_slist_remove ( tmp_list, GINT_TO_POINTER ( 4 ) );
    tmp_list = g_slist_remove ( tmp_list, GINT_TO_POINTER ( 2 ) );

    /* on supprime la donnée à la position initiale */
    tmp_list = g_slist_remove ( tmp_list, GINT_TO_POINTER ( src_type_data ) );

    /* on insère la donnée à la position finale */
    tmp_list = g_slist_insert ( tmp_list, GINT_TO_POINTER ( src_type_data ), dst_pos );

    tree_view = etats_prefs_widget_get_widget_by_name ( "treeview_data_grouping", NULL );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    /* on reset le model */
    gtk_list_store_clear ( GTK_LIST_STORE ( model ) );

    while ( tmp_list )
    {
        GtkTreeIter iter;
        gchar *string = NULL;
        gint type_data;
        gpointer ptr;

        ptr = tmp_list->data;
        type_data = GPOINTER_TO_INT ( ptr );

        string = etats_config_onglet_data_grouping_get_string ( type_data, i );

        gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
        gtk_list_store_set ( GTK_LIST_STORE ( model ), &iter, 0, string, 1, i, 2, type_data, -1 );

        g_free ( string );

        i++;
        tmp_list = tmp_list->next;
    }
}


/*ONGLET_DATA_SEPARATION*/
/**
 * Initialise les informations de l'onglet separation des données
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_initialise_onglet_data_separation ( gint report_number )
{
    GtkWidget *combo_1;
    GtkWidget *combo_2;

    etats_prefs_button_toggle_set_actif ( "bouton_separer_revenus_depenses",
                        gsb_data_report_get_split_credit_debit ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_separe_exo_etat",
                        gsb_data_report_get_financial_year_split ( report_number ) );

    /* on initialise le combo bouton_type_separe_plages_etat */
    combo_1 = etats_prefs_widget_get_widget_by_name ( "bouton_type_separe_plages_etat", NULL );
    gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo_1 ),
                        gsb_data_report_get_period_split_type ( report_number ) );

    /* on initialise le combo bouton_debut_semaine */
    combo_2 = etats_prefs_widget_get_widget_by_name ( "bouton_debut_semaine", NULL );
    gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo_2 ),
                        gsb_data_report_get_period_split_day ( report_number ) - G_DATE_MONDAY);

    if ( gsb_data_report_get_period_split ( report_number ) )
    {
        GtkWidget *button;

        button = etats_prefs_widget_get_widget_by_name ( "bouton_separe_plages_etat", NULL );
        etats_prefs_button_toggle_set_actif ( "bouton_separe_plages_etat", TRUE );
        sens_desensitive_pointeur ( button,
                        etats_prefs_widget_get_widget_by_name ( "paddingbox_data_separation2", NULL ) );

        if ( gtk_combo_box_get_active ( GTK_COMBO_BOX ( combo_1 ) ) == 1 )
            gtk_widget_set_sensitive ( combo_2, TRUE );
    }
}


/**
 * Récupère les informations de l'onglet separation des données
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_data_separation ( gint report_number )
{
    gsb_data_report_set_split_credit_debit ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_separer_revenus_depenses" ) );
    gsb_data_report_set_financial_year_split ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_separe_exo_etat" ) );
    gsb_data_report_set_period_split ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_separe_plages_etat" ) );

    /* récupère des index des GtkComboBox */
    gsb_data_report_set_period_split_type ( report_number,
                        gtk_combo_box_get_active ( GTK_COMBO_BOX (
                        etats_prefs_widget_get_widget_by_name ( "bouton_type_separe_plages_etat", NULL ) ) ) );

    gsb_data_report_set_period_split_day ( report_number,
                        gtk_combo_box_get_active ( GTK_COMBO_BOX (
                        etats_prefs_widget_get_widget_by_name ( "bouton_debut_semaine", NULL ) ) ) );
}


/**
 * fonction de callback appellée quand on change le type de période
 *
 * \param combo         le GtkComboBox qui change
 * \param widget        le widget qu'on rend sensible ou pas.
 *
 * \return
 */
void etats_config_onglet_data_separation_combo_changed ( GtkComboBox *combo,
                        GtkWidget *widget )
{
    gint report_number;

    report_number = gsb_gui_navigation_get_current_report ( );
    if ( gsb_data_report_get_period_split ( report_number ) )
    {
        if ( gtk_combo_box_get_active ( GTK_COMBO_BOX ( combo ) ) == 1 )
            gtk_widget_set_sensitive ( widget, TRUE );
        else
            gtk_widget_set_sensitive ( widget, FALSE );
    }
}


/*ONGLET_AFFICHAGE_GENERALITES*/
/**
 * pré visualise le titre avec son complément
 *
 * \param report_number
 *
 * \return
 **/
static void etats_config_display_name_with_complement (gint report_number)
{
	gchar **tab;
	gchar *report_name = NULL;
	gchar *compl_str;
	gchar *tmp_str = NULL;
	gint function;
	gint position;

	devel_debug_int (report_number);
	report_name = gsb_data_report_get_report_name (report_number);
	function = gsb_data_report_get_compl_name_function (report_number);
	position = gsb_data_report_get_compl_name_position (report_number);
	tab = gsb_date_get_date_time_now_local ();

	if (function == 1)
	{
		if (position == 2)
		{
			compl_str = g_strconcat (tab[0], " ", _("at"), " ", tab[1], NULL);
		}
		else
		{
			compl_str = g_strconcat (tab[0], " ", tab[1], NULL);
		}
	}
	else
	{
		compl_str = g_strdup (tab[0]);
	}

	switch (position)
	{
		case 1:
			tmp_str = g_strconcat (report_name, " - ", compl_str, NULL);
			break;
		case 2:
			tmp_str = g_strconcat (report_name, "\n(", _("Edited"), " ", compl_str, ")", NULL);
			break;
		default:
			tmp_str = g_strconcat (compl_str, " - ", report_name, NULL);
	}
	g_free (compl_str);
	g_strfreev (tab);

	if (tmp_str)
	{
		GtkWidget *textview;
		GtkTextBuffer *buffer;

		textview = etats_prefs_widget_get_widget_by_name ("textview_compl_name", NULL);
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
		gtk_text_buffer_set_text (buffer, tmp_str, -1);
		g_free (tmp_str);
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
static void etats_config_combo_box_compl_name_changed (GtkComboBox *widget,
													   void function (gint report_number,
																	  gint compl_name_int))
{
	gint report_number;

	report_number = gsb_gui_navigation_get_current_report ();
	function (report_number, gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));
	etats_config_display_name_with_complement (report_number);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean etats_config_check_button_compl_name_toggled (GtkWidget *check_button,
															  GtkWidget *widget)
{
	gboolean activ;

	activ = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button));
    gtk_widget_set_sensitive ( widget, activ);

	if (activ)
		etats_config_display_name_with_complement (gsb_gui_navigation_get_current_report ());
	else
	{
		GtkWidget *textview;
		GtkTextBuffer *buffer;

		textview = etats_prefs_widget_get_widget_by_name ("textview_compl_name", NULL);
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
		gtk_text_buffer_set_text (buffer, "", -1);
	}

	return FALSE;
}

/**
 * Initialise les informations de l'onglet généraités
 *
 * \param report_number
 *
 * \return
 **/
static void etats_config_initialise_onglet_affichage_generalites ( gint report_number )
{
	GtkWidget *checkbutton;
	GtkWidget *widget;
	gchar *report_name = NULL;
	gboolean activ;

	report_name = gsb_data_report_get_report_name (report_number);
    gtk_entry_set_text ( GTK_ENTRY ( etats_prefs_widget_get_widget_by_name ( "entree_nom_etat", NULL ) ),
                        report_name);

    /* on initialise le complément du nom si actif */
	checkbutton = etats_prefs_widget_get_widget_by_name ("check_button_compl_name", NULL);
	widget = etats_prefs_widget_get_widget_by_name ("hbox_combo_compl_name", NULL);
	activ = gsb_data_report_get_compl_name_used (report_number);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), activ);
	if (report_name && activ)
	{
		utils_buttons_sensitive_by_checkbutton (checkbutton, widget);
		gtk_combo_box_set_active (GTK_COMBO_BOX (etats_prefs_widget_get_widget_by_name ("combo_box_compl_name_function", NULL)),
								  gsb_data_report_get_compl_name_function (report_number));
		gtk_combo_box_set_active (GTK_COMBO_BOX (etats_prefs_widget_get_widget_by_name ("combo_box_compl_name_position", NULL)),
								  gsb_data_report_get_compl_name_position (report_number));
		etats_config_display_name_with_complement (report_number);
	}

    /* Connect signal */
    g_signal_connect (checkbutton,
					  "toggled",
					  G_CALLBACK (etats_config_check_button_compl_name_toggled),
					  widget);

	g_signal_connect (etats_prefs_widget_get_widget_by_name ("combo_box_compl_name_function", NULL),
					  "changed",
					  G_CALLBACK (etats_config_combo_box_compl_name_changed),
					  gsb_data_report_set_compl_name_function);
    g_signal_connect (etats_prefs_widget_get_widget_by_name ("combo_box_compl_name_position", NULL),
					  "changed",
					  G_CALLBACK (etats_config_combo_box_compl_name_changed),
					  gsb_data_report_set_compl_name_position);

    /* on initialise le type de date à sélectionner */
    etats_prefs_button_toggle_set_actif ( "button_sel_value_date",
                        gsb_data_report_get_date_select_value ( report_number ) );

    /* on initialise les autres données */
    etats_prefs_button_toggle_set_actif ( "bouton_ignore_archives",
                        gsb_data_report_get_ignore_archives ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_nb_opes",
                        gsb_data_report_get_show_report_transaction_amount ( report_number ) );

	/* mémorisation de l'état avant initialisation */
	payee_last_state = gsb_data_report_get_append_in_payee (report_number);
    etats_prefs_button_toggle_set_actif ("bouton_inclure_dans_tiers", payee_last_state);
}

/**
 * Récupère les informations de l'onglet généralités
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_affichage_generalites ( gint report_number )
{
    const gchar *text;

    /* on récupère le nom de l'état */
    text = gtk_entry_get_text ( GTK_ENTRY (
                        etats_prefs_widget_get_widget_by_name ( "entree_nom_etat", NULL ) ) );

    if ( strlen ( text )
     &&
     strcmp ( text, gsb_data_report_get_report_name ( report_number ) ) )
    {
        gsb_data_report_set_report_name ( report_number, text );
    }

	if (etats_prefs_button_toggle_get_actif ("check_button_compl_name"))
	{
		GtkWidget *widget;

		gsb_data_report_set_compl_name_used (report_number, TRUE);
		widget = etats_prefs_widget_get_widget_by_name ("combo_box_compl_name_function", NULL);
		gsb_data_report_set_compl_name_function (report_number,
												 gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));
		widget = etats_prefs_widget_get_widget_by_name ("combo_box_compl_name_position", NULL);
		gsb_data_report_set_compl_name_position (report_number,
												 gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));
	}
	else
	{
		gsb_data_report_set_compl_name_used (report_number, FALSE);
		gsb_data_report_set_compl_name_function (report_number, 0);
		gsb_data_report_set_compl_name_function (report_number, 0);
	}

    /* on récupère les autres informations */
    gsb_data_report_set_date_select_value ( report_number,
                        etats_prefs_button_toggle_get_actif ( "button_sel_value_date" ) );
    gsb_data_report_set_ignore_archives ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_ignore_archives" ) );
    gsb_data_report_set_show_report_transaction_amount ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_afficher_nb_opes" ) );
    gsb_data_report_set_append_in_payee ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_inclure_dans_tiers" ) );
}


/*ONGLET_AFFICHAGE_TITLES*/
/**
 * Initialise les informations de l'onglet titres
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_initialise_onglet_affichage_titres ( gint report_number )
{
    GtkWidget *button;
    GtkWidget *widget;

    /* données des comptes */
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_noms_comptes",
                        gsb_data_report_get_account_show_name ( report_number ) );

    button = etats_prefs_widget_get_widget_by_name ( "bouton_regroupe_ope_compte_etat", NULL );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_affiche_sous_total_compte", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
                        gsb_data_report_get_account_show_amount ( report_number ) );

    /* données des tiers */
    button = etats_prefs_widget_get_widget_by_name ( "bouton_utilise_tiers_etat", NULL );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_afficher_noms_tiers", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
                        gsb_data_report_get_payee_show_name ( report_number ) );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_affiche_sous_total_tiers", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
                        gsb_data_report_get_payee_show_payee_amount ( report_number ) );

    /* données des catégories */
    button = etats_prefs_widget_get_widget_by_name ( "bouton_group_by_categ", NULL );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_afficher_noms_categ", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
				   gsb_data_report_get_category_show_name ( report_number ) );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_affiche_sous_total_categ", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
				   gsb_data_report_get_category_show_category_amount ( report_number ) );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_afficher_sous_categ", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
				   gsb_data_report_get_category_show_sub_category ( report_number ) );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_affiche_sous_total_sous_categ", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
				   gsb_data_report_get_category_show_sub_category_amount ( report_number ) );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_afficher_pas_de_sous_categ", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
				   gsb_data_report_get_category_show_without_category ( report_number ) );

    /* données des IB */
    button = etats_prefs_widget_get_widget_by_name ( "bouton_utilise_ib_etat", NULL );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_afficher_noms_ib", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
				   gsb_data_report_get_budget_show_name ( report_number ) );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_affiche_sous_total_ib", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
				   gsb_data_report_get_budget_show_budget_amount ( report_number ) );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_afficher_sous_ib", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
				   gsb_data_report_get_budget_show_sub_budget ( report_number ) );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_affiche_sous_total_sous_ib", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
				   gsb_data_report_get_budget_show_sub_budget_amount ( report_number ) );

    widget = etats_prefs_widget_get_widget_by_name ( "bouton_afficher_pas_de_sous_ib", NULL );
    sens_desensitive_pointeur ( button, widget );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ),
				   gsb_data_report_get_budget_show_without_budget ( report_number ) );
}


/**
 * Récupère les informations de l'onglet titres
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_affichage_titres ( gint report_number )
{
    /* données des comptes */
    gsb_data_report_set_account_show_name ( report_number,
    				    etats_prefs_button_toggle_get_actif ( "bouton_afficher_noms_comptes" ) );
    gsb_data_report_set_account_show_amount ( report_number,
    				    etats_prefs_button_toggle_get_actif ( "bouton_affiche_sous_total_compte" ) );

    /* données des tiers */
    gsb_data_report_set_payee_show_name ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_afficher_noms_tiers" ) );
    gsb_data_report_set_payee_show_payee_amount ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_affiche_sous_total_tiers" ) );

    /* données des catégories */
    gsb_data_report_set_category_show_name ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_afficher_noms_categ" ) );
    gsb_data_report_set_category_show_category_amount ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_affiche_sous_total_categ" ) );
    gsb_data_report_set_category_show_sub_category ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_afficher_sous_categ" ) );
    gsb_data_report_set_category_show_sub_category_amount ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_affiche_sous_total_sous_categ" ) );
    gsb_data_report_set_category_show_without_category ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_afficher_pas_de_sous_categ" ) );

    /* données des IB */
    gsb_data_report_set_budget_show_name ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_afficher_noms_ib" ) );
    gsb_data_report_set_budget_show_budget_amount ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_affiche_sous_total_ib" ) );
    gsb_data_report_set_budget_show_sub_budget ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_afficher_sous_ib" ) );
    gsb_data_report_set_budget_show_sub_budget_amount ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_affiche_sous_total_ib" ) );
    gsb_data_report_set_budget_show_without_budget ( report_number,
    				  etats_prefs_button_toggle_get_actif ( "bouton_afficher_pas_de_sous_ib" ) );
}


/*ONGLET_AFFICHAGE_OPERATIONS*/
/**
 * Initialise les informations de l'onglet opérations
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_initialise_onglet_affichage_operations ( gint report_number )
{
    GtkWidget *combo;

    /* on affiche ou pas le choix des données des opérations */
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_opes",
                        gsb_data_report_get_show_report_transactions ( report_number ) );

    /* données des opérations à afficher */
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_no_ope",
                        gsb_data_report_get_show_report_transaction_number ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_date_opes",
                        gsb_data_report_get_show_report_date ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_value_date_opes",
                        gsb_data_report_get_show_report_value_date ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_tiers_opes",
                        gsb_data_report_get_show_report_payee ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_categ_opes",
                        gsb_data_report_get_show_report_category ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_sous_categ_opes",
                        gsb_data_report_get_show_report_sub_category ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_ib_opes",
                        gsb_data_report_get_show_report_budget ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_sous_ib_opes",
                        gsb_data_report_get_show_report_sub_budget ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_notes_opes",
                        gsb_data_report_get_show_report_note ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_type_ope",
                        gsb_data_report_get_show_report_method_of_payment ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_no_cheque",
                        gsb_data_report_get_show_report_method_of_payment_content ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_pc_opes",
                        gsb_data_report_get_show_report_voucher ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_exo_opes",
                        gsb_data_report_get_show_report_financial_year ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_infobd_opes",
                        gsb_data_report_get_show_report_bank_references ( report_number ) );
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_no_rappr",
                        gsb_data_report_get_show_report_marked ( report_number ) );

    /* affichage des titres des colonnes */
    etats_prefs_button_toggle_set_actif ( "bouton_afficher_titres_colonnes",
                        gsb_data_report_get_column_title_show ( report_number ) );

    if ( !gsb_data_report_get_column_title_type ( report_number ) )
        etats_prefs_button_toggle_set_actif ( "bouton_titre_en_haut", TRUE );

    /* sélectionner le type de classement des opérations */
    combo = etats_prefs_widget_get_widget_by_name ( "bouton_choix_classement_ope_etat", NULL );
    gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ),
                        gsb_data_report_get_sorting_report ( report_number ) );

    /* rendre les opérations cliquables */
    etats_prefs_button_toggle_set_actif ( "bouton_rendre_ope_clickables",
                        gsb_data_report_get_report_can_click ( report_number ) );
}


/**
 * Récupère les informations de l'onglet opérations
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_affichage_operations ( gint report_number )
{
    GtkWidget *combo;
	gboolean affich_opes = FALSE;
	gboolean detail_ope;
	gboolean is_actif = FALSE;

	affich_opes = etats_prefs_button_toggle_get_actif ("bouton_afficher_opes");
    gsb_data_report_set_show_report_transactions (report_number, affich_opes);

    /* données des opérations */
	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_no_ope");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_transaction_number (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_date_opes");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_date (report_number, detail_ope);

	if (detail_ope && !is_actif)
		is_actif = TRUE;
	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_value_date_opes");
    gsb_data_report_set_show_report_value_date (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_tiers_opes");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_payee (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_categ_opes");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_category (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_sous_categ_opes");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_sub_category (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_ib_opes");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_budget (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_sous_ib_opes");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_sub_budget (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_notes_opes");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_note (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_type_ope");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_method_of_payment (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_no_cheque");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_method_of_payment_content (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_pc_opes");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_voucher (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_exo_opes");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_financial_year (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_infobd_opes");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_bank_references (report_number, detail_ope);

	detail_ope = etats_prefs_button_toggle_get_actif ("bouton_afficher_no_rappr");
	if (detail_ope && !is_actif)
		is_actif = TRUE;
    gsb_data_report_set_show_report_marked (report_number, detail_ope);

	if (affich_opes && !is_actif)
		gsb_data_report_set_show_report_transactions (report_number, FALSE);

    /* titres des colonnes */
    gsb_data_report_set_column_title_show ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_afficher_titres_colonnes" ) );

    gsb_data_report_set_column_title_type ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_titre_changement" ) );

    /* type de classement des opérations */
    combo = etats_prefs_widget_get_widget_by_name ( "bouton_choix_classement_ope_etat", NULL );
    gsb_data_report_set_sorting_report ( report_number,
                        gtk_combo_box_get_active ( GTK_COMBO_BOX ( combo ) ) );

    /* opérations cliquables */
    gsb_data_report_set_report_can_click ( report_number,
                        etats_prefs_button_toggle_get_actif ( "bouton_rendre_ope_clickables" ) );
}


/*ONGLET_AFFICHAGE_DEVISES*/
/**
 * Initialise les informations de l'onglet devises
 *
 * \param report_number
 *
 * \return
 */
static void etats_config_initialise_onglet_affichage_devises ( gint report_number )
{
    GtkWidget *button;

    button = etats_prefs_widget_get_widget_by_name ( "hbox_devise_general_etat", "button" );
    gsb_currency_set_combobox_history ( button,
                        gsb_data_report_get_currency_general ( report_number ) );

    button = etats_prefs_widget_get_widget_by_name ( "hbox_devise_tiers_etat", "button" );
    gsb_currency_set_combobox_history ( button,
                        gsb_data_report_get_payee_currency ( report_number ) );

    button = etats_prefs_widget_get_widget_by_name ( "hbox_devise_categ_etat", "button" );
    gsb_currency_set_combobox_history ( button,
                        gsb_data_report_get_category_currency ( report_number ) );

    button = etats_prefs_widget_get_widget_by_name ( "hbox_devise_ib_etat", "button" );
    gsb_currency_set_combobox_history ( button,
                        gsb_data_report_get_budget_currency ( report_number ) );

    button = etats_prefs_widget_get_widget_by_name ( "hbox_devise_montant_etat", "button" );
    gsb_currency_set_combobox_history ( button,
                        gsb_data_report_get_amount_comparison_currency ( report_number ) );

}


/**
 * Récupère les informations de l'onglet devises
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
static void etats_config_recupere_info_onglet_affichage_devises ( gint report_number )
{
    GtkWidget *button;

    button = etats_prefs_widget_get_widget_by_name ( "hbox_devise_general_etat", "button" );
    gsb_data_report_set_currency_general ( report_number,
                        gsb_currency_get_currency_from_combobox ( button ) );

    button = etats_prefs_widget_get_widget_by_name ( "hbox_devise_tiers_etat", "button" );
    gsb_data_report_set_payee_currency ( report_number,
                        gsb_currency_get_currency_from_combobox ( button ) );

    button = etats_prefs_widget_get_widget_by_name ( "hbox_devise_categ_etat", "button" );
    gsb_data_report_set_category_currency ( report_number,
                        gsb_currency_get_currency_from_combobox ( button ) );

    button = etats_prefs_widget_get_widget_by_name ( "hbox_devise_ib_etat", "button" );
    gsb_data_report_set_budget_currency ( report_number,
                        gsb_currency_get_currency_from_combobox ( button ) );

    button = etats_prefs_widget_get_widget_by_name ( "hbox_devise_montant_etat", "button" );
    gsb_data_report_set_amount_comparison_currency ( report_number,
                        gsb_currency_get_currency_from_combobox ( button ) );

}


/**
 * ajoute les combobox pour les devises
 *
 * \param
 *
 * \return
 */
void etats_config_onglet_affichage_devises_make_combobox ( void )
{
    GtkWidget *hbox;
    GtkWidget *button;

    hbox = etats_prefs_widget_get_widget_by_name ( "hbox_devise_general_etat", NULL );
    button = gsb_currency_make_combobox (FALSE);
    g_object_set_data ( G_OBJECT ( hbox ), "button", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    hbox = etats_prefs_widget_get_widget_by_name ( "hbox_devise_tiers_etat", NULL );
    button = gsb_currency_make_combobox (FALSE);
    g_object_set_data ( G_OBJECT ( hbox ), "button", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    hbox = etats_prefs_widget_get_widget_by_name ( "hbox_devise_categ_etat", NULL );
    button = gsb_currency_make_combobox (FALSE);
    g_object_set_data ( G_OBJECT ( hbox ), "button", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    hbox = etats_prefs_widget_get_widget_by_name ( "hbox_devise_ib_etat", NULL );
    button = gsb_currency_make_combobox (FALSE);
    g_object_set_data ( G_OBJECT ( hbox ), "button", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    hbox = etats_prefs_widget_get_widget_by_name ( "hbox_devise_montant_etat", NULL );
    button = gsb_currency_make_combobox (FALSE);
    g_object_set_data ( G_OBJECT ( hbox ), "button", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );
}
/*END_ONGLETS*/
/**
 * Initialise la boite de dialogue propriétés de l'état.
 *
 * \param
 *
 * \return
 */
static gboolean etats_config_initialise_dialog_from_etat (GtkWidget *etats_prefs,
														  gint report_number)
{
    /* onglet période */
    etats_page_period_initialise_onglet (etats_prefs, report_number);

    /* onglet virements */
    etats_page_transfer_initialise_onglet (etats_prefs, report_number);

    /* onglet comptes */
    etats_page_accounts_initialise_onglet (etats_prefs, report_number);

    /* onglet tiers */
    etats_config_initialise_onglet_tiers ( report_number );

    /* onglet Categories */
    etats_config_initialise_onglet_categ_budget ( report_number, TRUE );

    /* onglet Budgets */
    etats_config_initialise_onglet_categ_budget ( report_number, FALSE );

    /* onglet Textes */
    etats_config_initialise_onglet_textes ( report_number );

    /* onglet Montants */
    etats_config_initialise_onglet_montants ( report_number );

    /* onglet modes de paiement */
    etats_config_initialise_onglet_mode_paiement ( report_number );

    /* onglet divers */
    etats_config_initialise_onglet_divers ( report_number );

    /* onglet data grouping */
    etats_config_initialise_onglet_data_grouping ( report_number );

    /* onglet data separation */
    etats_config_initialise_onglet_data_separation ( report_number );

    /* onglet generalites */
    etats_config_initialise_onglet_affichage_generalites ( report_number );

    /* onglet titres */
    etats_config_initialise_onglet_affichage_titres ( report_number );

    /* onglet opérations */
    etats_config_initialise_onglet_affichage_operations ( report_number );

    /* onglet devises */
    etats_config_initialise_onglet_affichage_devises ( report_number );

    /* return */
    return TRUE;
}


/**
 *
 *
 * \param
 *
 * \return
 */
static gboolean etats_config_recupere_info_to_etat (GtkWidget *etats_prefs,
													gint report_number )
{
	gboolean payee_new_state = FALSE;

    /* onglet période */
    etats_page_period_get_info	(etats_prefs, report_number);

    /* onglet virements */
    etats_page_transfer_get_info (etats_prefs, report_number);

    /* onglet comptes */
    etats_page_accounts_get_info	(etats_prefs, report_number);

    /* onglet tiers */
    etats_config_recupere_info_onglet_tiers ( report_number );

    /* onglet Categories */
    etats_config_recupere_info_onglet_categ_budget ( report_number, TRUE );

    /* onglet Budgets */
    etats_config_recupere_info_onglet_categ_budget ( report_number, FALSE );

    /* onglet Textes */
    etats_config_recupere_info_onglet_textes ( report_number );

    /* onglet Montants */
    etats_config_recupere_info_onglet_montants ( report_number );

    /* onglet mode de paiement */
    etats_config_recupere_info_onglet_mode_paiement ( report_number );

    /* onglet modes divers */
    etats_config_recupere_info_onglet_divers ( report_number );

    /* onglet modes data grouping */
    etats_config_recupere_info_onglet_data_grouping ( report_number );

    /* onglet modes data separation */
    etats_config_recupere_info_onglet_data_separation ( report_number );

    /* onglet generalites */
    etats_config_recupere_info_onglet_affichage_generalites ( report_number );

    /* onglet titres */
    etats_config_recupere_info_onglet_affichage_titres ( report_number );

    /* onglet opérations */
    etats_config_recupere_info_onglet_affichage_operations ( report_number );

    /* onglet devises */
    etats_config_recupere_info_onglet_affichage_devises ( report_number );

    /* update the payee combofix in the form, to add that report if asked */
	payee_new_state = gsb_data_report_get_append_in_payee (report_number);
    if (payee_last_state || payee_new_state)
        gsb_form_widget_update_payee_combofix (report_number, payee_new_state);

    /* on avertit grisbi de la modification à enregistrer */
    gsb_file_set_modified ( TRUE );

    /* on réaffiche l'état */
    rafraichissement_etat ( report_number );

    /* on repasse à la 1ère page du notebook */
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( etats_onglet_get_notebook_etats ( ) ), 0 );
    gtk_widget_set_sensitive ( gsb_gui_navigation_get_tree_view ( ), TRUE );

    gsb_gui_navigation_update_report ( report_number );

    /* return */
    return TRUE;
}

/**
 * affiche la fenetre de personnalisation
 *
 * \param
 *
 * \return
 */
gint etats_config_personnalisation_etat ( void )
{
    GtkWidget *dialog;
    GtkWidget *notebook_general;
    gint current_report_number;
	gint result = 0;

    devel_debug (NULL);

    if ( !( current_report_number = gsb_gui_navigation_get_current_report ( ) ) )
        return result;

    notebook_general = grisbi_win_get_notebook_general ( );
    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != GSB_REPORTS_PAGE )
        gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general), GSB_REPORTS_PAGE );

    /* Création de la fenetre de dialog */
    dialog = etats_prefs_new (GTK_WIDGET (grisbi_app_get_active_window (NULL)));
    if ( dialog == NULL )
        return result;

    /* initialisation des données de la fenetre de dialog */
    etats_config_initialise_dialog_from_etat (dialog, current_report_number);

    gtk_widget_show_all ( dialog );

    /* on se repositionne sur le dernier onglet si on a le même rapport */
	if ( current_report_number == last_report )
		etats_prefs_left_panel_tree_view_select_last_page ();

	result = gtk_dialog_run (GTK_DIALOG (dialog));
    switch (result)
    {
        case GTK_RESPONSE_OK:
            etats_config_recupere_info_to_etat (dialog, current_report_number);
            last_report = current_report_number;
            break;

		case GTK_RESPONSE_CANCEL:
			{
				GrisbiWinRun *w_run;

				w_run = grisbi_win_get_w_run ();
				if (w_run->empty_report)
				{
					gsb_gui_navigation_remove_report (current_report_number);
					gsb_data_report_remove (current_report_number);
				}
			}
			break;
        default:
            break;
    }

    etats_prefs_free_all_var ();

    gtk_widget_destroy ( dialog );

	return result;
}


/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
