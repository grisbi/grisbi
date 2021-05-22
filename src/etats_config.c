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
#include "etats_page_amount.h"
#include "etats_page_accounts.h"
#include "etats_page_budget.h"
#include "etats_page_category.h"
#include "etats_page_payee.h"
#include "etats_page_period.h"
#include "etats_page_text.h"
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
#include "utils_widgets.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
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
/*END*/

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
    etats_page_payee_initialise_onglet (etats_prefs, report_number);

    /* onglet Categories */
    etats_page_category_initialise_onglet (etats_prefs, report_number);

    /* onglet Budgets */
    etats_page_budget_initialise_onglet (etats_prefs, report_number);

    /* onglet Textes */
	etats_page_text_init_data (etats_prefs, report_number);

	/* onglet Montants */
    //~ etats_config_initialise_onglet_montants ( report_number );
    etats_page_amount_init_data (etats_prefs, report_number);

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
    etats_page_accounts_get_info (etats_prefs, report_number);

    /* onglet tiers */
    etats_page_payee_get_info (etats_prefs, report_number);

    /* onglet Categories */
    etats_page_category_get_info (etats_prefs, report_number);

    /* onglet Budgets */
    etats_page_budget_get_info (etats_prefs, report_number);

    /* onglet Textes */
    etats_page_text_get_data (etats_prefs, report_number);

    /* onglet Montants */
    etats_page_amount_get_data (etats_prefs, report_number);

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
