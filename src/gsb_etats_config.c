/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)           */
/*          2008-2011 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>


/*START_INCLUDE*/
#include "gsb_etats_config.h"
#include "dialog.h"
#include "etats_calculs.h"
#include "etats_config.h"
#include "etats_config_ui.h"
#include "etats_onglet.h"
#include "fenetre_principale.h"
#include "gsb_calendar_entry.h"
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
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gboolean gsb_etats_config_initialise_dialog_from_etat ( gint report_number );
static void gsb_etats_config_initialise_onglet_comptes ( gint report_number );
static void gsb_etats_config_initialise_onglet_data_grouping ( gint report_number );
static void gsb_etats_config_initialise_onglet_data_separation ( gint report_number );
static void gsb_etats_config_initialise_onglet_divers ( gint report_number );
static void gsb_etats_config_initialise_onglet_mode_paiement ( gint report_number );
static void gsb_etats_config_initialise_onglet_periode ( gint report_number );
static void gsb_etats_config_initialise_onglet_tiers ( gint report_number );
static void gsb_etats_config_initialise_onglet_virements ( gint report_number );
static void gsb_etats_config_initialise_onglet_categ_budget ( gint report_number,
                        gboolean is_categ );

static gboolean gsb_etats_config_onglet_categ_budget_fill_model ( GtkTreeModel *model,
                        gboolean is_categ );
static GSList *gsb_etats_config_onglet_categ_budget_get_selected ( const gchar *treeview_name );
static void gsb_etats_config_onglet_categ_budget_init_treeview ( const gchar *treeview_name,
                            GSList *tmp_list );
static gint gsb_etats_config_onglet_categ_budget_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gpointer ptr );
static GSList *gsb_etats_config_onglet_data_grouping_get_list ( gint report_number );
static gboolean gsb_etats_config_onglet_data_grouping_update_model ( gint report_number );

static gboolean gsb_etats_config_recupere_info_to_etat ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_comptes ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_data_grouping ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_data_separation ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_divers ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_mode_paiement ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_periode ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_tiers ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_virements ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_categ_budget ( gint report_number,
                        gboolean is_categ );


/**********************************************************************************************************************/
static GtkWidget *gsb_etats_config_affichage_etat_devises ( void );
static GtkWidget *gsb_etats_config_affichage_etat_generalites ( void );
static GtkWidget *gsb_etats_config_affichage_etat_operations ( void );
static GtkWidget *gsb_etats_config_affichage_etat_titres ( void );

static GtkWidget *gsb_etats_config_onglet_etat_divers ( void );

static GtkWidget *gsb_etats_config_onglet_etat_montant ( void );
static GtkWidget *gsb_etats_config_onglet_etat_texte ( void );

static void gsb_etats_config_onglet_etat_combo_set_model ( GtkWidget *combo,
                        gchar **tab );
static void gsb_etats_config_onglet_etat_texte_combo_changed ( GtkComboBox *combo,
                        GtkWidget *widget );
static void gsb_etats_config_onglet_etat_texte_get_buttons_add_remove ( GtkWidget *parent,
                        gboolean button_2_visible );
static GtkWidget *gsb_etats_config_onglet_etat_texte_new_line ( GtkWidget *parent );
static GtkWidget *gsb_etats_config_page_data_grouping ( void );
static GtkWidget *gsb_etats_config_page_data_separation ( void );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/


/* the def of the columns in the categ and budget list to filter by categ and budget */
enum
{
    GSB_ETAT_CATEG_BUDGET_LIST_NAME = 0,
    GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE,
    GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE,
    GSB_ETAT_CATEG_BUDGET_LIST_NUMBER,
    GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER,

    GSB_ETAT_CATEG_BUDGET_LIST_NB,
};


static gchar *champs_type_recherche_texte[] =
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


static gchar *champs_operateur_recherche_texte[] =
{
    N_("contains"),
    N_("doesn't contain"),
    N_("begins with"),
    N_("ends with"),
    N_("is empty"),
    N_("isn't empty"),
    NULL
};

/*
static gchar *champs_comparateur_montant[] =
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
*/

static gchar *champs_comparateur_nombre[] =
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


static gchar *champs_comparateur_nombre_2[] =
{
    N_("stop"),
    N_("and"),
    N_("or"),
    N_("except"),
    NULL
};


static GtkBuilder *etat_config_builder = NULL;


/**
 * affiche la fenetre de personnalisation
 *
 * \param
 *
 * \return
 */
void gsb_etats_config_personnalisation_etat ( void )
{
    GtkWidget *dialog;
    GtkWidget *notebook_general;
    gint current_report_number;

    devel_debug (NULL);

    if ( !( current_report_number = gsb_gui_navigation_get_current_report ( ) ) )
        return;

    notebook_general = gsb_gui_get_general_notebook ( );
    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != GSB_REPORTS_PAGE )
        gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general), GSB_REPORTS_PAGE );

    /* Création de la fenetre de dialog */
    dialog = etats_config_ui_create_dialog ( );
    if ( dialog == NULL )
        return;

    /* initialisation des données de la fenetre de dialog */
    gsb_etats_config_initialise_dialog_from_etat ( current_report_number );

    gtk_widget_show ( dialog );

    switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
        case GTK_RESPONSE_OK:
            gsb_etats_config_recupere_info_to_etat ( current_report_number );
            break;

        default:
            break;
    }

    etats_config_ui_free_all_var ( );

    gtk_widget_destroy ( dialog );
}


/**
 * Initialise la boite de dialogue propriétés de l'état.
 *
 * \param
 *
 * \return
 */
gboolean gsb_etats_config_initialise_dialog_from_etat ( gint report_number )
{
    /* onglet période */
    gsb_etats_config_initialise_onglet_periode ( report_number );

    /* onglet virements */
    gsb_etats_config_initialise_onglet_virements ( report_number );

    /* onglet comptes */
    gsb_etats_config_initialise_onglet_comptes ( report_number );

    /* onglet tiers */
    gsb_etats_config_initialise_onglet_tiers ( report_number );

    /* onglet Categories */
    gsb_etats_config_initialise_onglet_categ_budget ( report_number, TRUE );

    /* onglet Budgets */
    gsb_etats_config_initialise_onglet_categ_budget ( report_number, FALSE );

    /* onglet modes de paiement */
    gsb_etats_config_initialise_onglet_mode_paiement ( report_number );

    /* onglet divers */
    gsb_etats_config_initialise_onglet_divers ( report_number );

    /* onglet data grouping */
    gsb_etats_config_initialise_onglet_data_grouping ( report_number );

    /* onglet data separation */
    gsb_etats_config_initialise_onglet_data_separation ( report_number );


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
gboolean gsb_etats_config_recupere_info_to_etat ( gint report_number )
{
    /* onglet période */
    gsb_etats_config_recupere_info_onglet_periode ( report_number );

    /* onglet virements */
    gsb_etats_config_recupere_info_onglet_virements ( report_number );

    /* onglet comptes */
    gsb_etats_config_recupere_info_onglet_comptes ( report_number );

    /* onglet tiers */
    gsb_etats_config_recupere_info_onglet_tiers ( report_number );

    /* onglet Categories */
    gsb_etats_config_recupere_info_onglet_categ_budget ( report_number, TRUE );

    /* onglet Budgets */
    gsb_etats_config_recupere_info_onglet_categ_budget ( report_number, FALSE );

    /* onglet mode de paiement */
    gsb_etats_config_recupere_info_onglet_mode_paiement ( report_number );

    /* onglet modes divers */
    gsb_etats_config_recupere_info_onglet_divers ( report_number );

    /* onglet modes data grouping */
    gsb_etats_config_recupere_info_onglet_data_grouping ( report_number );

    /* onglet modes data separation */
    gsb_etats_config_recupere_info_onglet_data_separation ( report_number );



    /* update the payee combofix in the form, to add that report if asked */
    gsb_form_widget_update_payee_combofix ( );

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

/*ONGLET_PERIODE*/
/**
 * Initialise les informations de l'onglet periode
 *
 * \param report_number
 *
 * \return
 */
void gsb_etats_config_initialise_onglet_periode ( gint report_number )
{
    if ( gsb_data_report_get_use_financial_year ( report_number ) )
    {
        gint financial_year_type;

        etats_config_ui_widget_set_actif ( "radio_button_utilise_exo", TRUE );

        financial_year_type = gsb_data_report_get_financial_year_type ( report_number );
        etats_config_ui_buttons_radio_set_active_index ( "bouton_exo_tous", financial_year_type );

        if ( financial_year_type == 3 )
            etats_config_ui_tree_view_select_rows_from_list (
                                gsb_data_report_get_financial_year_list ( report_number ),
                                "treeview_exer",
                                1 );
    }
    else
    {
        etats_config_ui_widget_set_actif ( "radio_button_utilise_dates", TRUE );
        etats_config_ui_tree_view_select_single_row ( "treeview_dates",
                        gsb_data_report_get_date_type ( report_number ) );

        if ( gsb_data_report_get_date_type ( report_number ) != 1 )
            etats_config_ui_onglet_periode_date_interval_sensitive ( FALSE );
        else
        {
            GDate *date;

            etats_config_ui_onglet_periode_date_interval_sensitive ( TRUE );

            /* on initialise le type de date à sélectionner */
            etats_config_ui_widget_set_actif ( "button_sel_value_date",
                                gsb_data_report_get_date_select_value ( report_number ) );

            /* on remplit les dates perso si elles existent */
            if ( ( date = gsb_data_report_get_personal_date_start ( report_number ) ) )
                gsb_calendar_entry_set_date (
                                etats_config_ui_widget_get_widget_by_name ( "hbox_date_init",
                                "entree_date_init_etat" ),
                                date );

            if ( ( date = gsb_data_report_get_personal_date_end ( report_number ) ) )
            gsb_calendar_entry_set_date (
                                etats_config_ui_widget_get_widget_by_name ( "hbox_date_finale",
                                "entree_date_finale_etat" ),
                                date );
        }
    }

    /* return */
}


/**
 * retourne la liste des exercices
 *
 * \param
 *
 * \return un GtkTreeModel
 */
GtkTreeModel *gsb_etats_config_onglet_get_liste_exercices ( void )
{
    GtkListStore *list_store;
    GSList *list_tmp;

    list_store = gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_INT );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( list_store ),
                        0, GTK_SORT_ASCENDING );

    /* on remplit la liste des exercices */
    list_tmp = gsb_data_fyear_get_fyears_list ();

    while ( list_tmp )
    {
        GtkTreeIter iter;
        gchar *name;
        gint fyear_number;

        fyear_number = gsb_data_fyear_get_no_fyear ( list_tmp -> data );

        name = my_strdup ( gsb_data_fyear_get_name ( fyear_number ) );

        gtk_list_store_append ( list_store, &iter );
        gtk_list_store_set ( list_store, &iter, 0, name, 1, fyear_number, -1 );

        if ( name )
            g_free ( name );
    
        list_tmp = list_tmp -> next;
    }

    return GTK_TREE_MODEL ( list_store );
}


/**
 * Récupère les informations de l'onglet periode
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void gsb_etats_config_recupere_info_onglet_periode ( gint report_number )
{
    gint active;

    active = etats_config_ui_widget_get_actif ( "radio_button_utilise_exo" );
    gsb_data_report_set_use_financial_year ( report_number, active );

    if ( !active )
    {
        gint item_selected;

        /* Check that custom dates are OK, but only if custom date range
         * has been selected. */
        if ( ( item_selected = etats_config_ui_tree_view_get_single_row_selected ( "treeview_dates" ) ) == 1 )
        {
            GtkWidget *entry;

            gsb_data_report_set_date_select_value ( report_number,
                                etats_config_ui_widget_get_actif ( "button_sel_value_date" ) );

            entry = etats_config_ui_widget_get_widget_by_name ( "hbox_date_init", "entree_date_init_etat" );
            if ( !gsb_date_check_entry ( entry ) )
            {
                gchar *text;
                gchar *hint;

                text = g_strdup ( _("Grisbi can't parse date.  For a list of date formats"
                                    " that Grisbi can use, refer to Grisbi manual.") );
                hint = g_strdup_printf ( _("Invalid initial date '%s'"),
                                gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );
                dialogue_error_hint ( text, hint );
                g_free ( text );
                g_free ( hint );

                return;
            }
            else
                gsb_data_report_set_personal_date_start ( report_number,
                                gsb_calendar_entry_get_date ( entry ) );

            entry = etats_config_ui_widget_get_widget_by_name ( "hbox_date_finale", "entree_date_finale_etat" );
            if ( !gsb_date_check_entry ( entry ) )
            {
                gchar *text;
                gchar *hint;

                text = g_strdup ( _("Grisbi can't parse date.  For a list of date formats"
                                    " that Grisbi can use, refer to Grisbi manual.") );
                hint = g_strdup_printf ( _("Invalid final date '%s'"),
                                gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );
                dialogue_error_hint ( text, hint );
                g_free ( text );
                g_free ( hint );

                return;
            }
            else
                gsb_data_report_set_personal_date_end ( report_number,
                                gsb_calendar_entry_get_date ( entry ) );
        }
        else
            gsb_data_report_set_date_type ( report_number, item_selected );
    }
    else
    {
        gint index;

        index = etats_config_ui_buttons_radio_get_active_index ( "bouton_exo_tous" );
        gsb_data_report_set_financial_year_type ( report_number, index );

        if ( index == 3 )
        {
            gsb_data_report_free_financial_year_list ( report_number );
            gsb_data_report_set_financial_year_list ( report_number,
                                etats_config_ui_tree_view_get_list_rows_selected ( "treeview_exer" ) );
            if ( utils_tree_view_all_rows_are_selected ( GTK_TREE_VIEW (
             etats_config_ui_widget_get_widget_by_name ( "treeview_exer", NULL ) ) ) )
            {
                gchar *text;
                gchar *hint;

                hint = g_strdup ( _("Performance issue.") );
                text = g_strdup ( _("All financial years have been selected.  Grisbi will run "
                                "faster without the \"Detail financial years\" option activated.") );

                dialogue_special ( GTK_MESSAGE_INFO, make_hint ( hint, text ) );
                etats_config_ui_widget_set_actif ( "bouton_exo_tous", FALSE );
                gsb_data_report_set_financial_year_type ( report_number, 0 );

                g_free ( text );
                g_free ( hint );
            }
        }
    }
}


/*ONGLET_VIREMENTS*/
/**
 * Initialise les informations de l'onglet virements
 *
 * \param report_number
 *
 * \return
 */
void gsb_etats_config_initialise_onglet_virements ( gint report_number )
{
    gint index;

    index = gsb_data_report_get_transfer_choice ( report_number );
    etats_config_ui_buttons_radio_set_active_index ( "bouton_non_inclusion_virements", index );

    if ( index == 3 )
    {
        etats_config_ui_tree_view_select_rows_from_list (
                                gsb_data_report_get_transfer_account_numbers_list ( report_number ),
                                "treeview_virements",
                                1 );
        if ( g_slist_length ( gsb_data_report_get_account_numbers_list ( report_number ) ) )
        {
            utils_togglebutton_set_label_position_unselect (
                                etats_config_ui_widget_get_widget_by_name (
                                "togglebutton_select_all_virements", NULL ),
                                G_CALLBACK ( etats_config_ui_onglet_comptes_select_unselect ),
                                etats_config_ui_widget_get_widget_by_name ( "treeview_virements", NULL ) );
        }
    }

    if ( index > 0 )
    {
        gtk_widget_set_sensitive ( etats_config_ui_widget_get_widget_by_name (
                                "bouton_exclure_non_virements_etat", NULL ), TRUE );
        etats_config_ui_widget_set_actif ( "bouton_exclure_non_virements_etat",
                                gsb_data_report_get_transfer_reports_only ( report_number ) );
    }
    else
        gtk_widget_set_sensitive ( etats_config_ui_widget_get_widget_by_name (
                                "bouton_exclure_non_virements_etat", NULL ), FALSE );
}


/**
 * Récupère les informations de l'onglet virements
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void gsb_etats_config_recupere_info_onglet_virements ( gint report_number )
{
    gint index;

    index = etats_config_ui_buttons_radio_get_active_index ( "bouton_non_inclusion_virements" );
    gsb_data_report_set_transfer_choice ( report_number, index );

    if ( index == 3 )
    {
        gsb_data_report_free_transfer_account_numbers_list ( report_number );
        gsb_data_report_set_transfer_account_numbers_list ( report_number,
                            etats_config_ui_tree_view_get_list_rows_selected ( "treeview_virements" ) );
    }

    gsb_data_report_set_transfer_reports_only ( report_number,
                        etats_config_ui_widget_get_actif ( "bouton_exclure_non_virements_etat" ) );
}


/*ONGLET_COMPTES*/
/**
 * Initialise les informations de l'onglet comptes
 *
 * \param report_number
 *
 * \return
 */
void gsb_etats_config_initialise_onglet_comptes ( gint report_number )
{
    gint active;

    active = gsb_data_report_get_account_use_chosen ( report_number );
    etats_config_ui_widget_set_actif ( "bouton_detaille_comptes_etat", active );

    if ( active )
    {
        etats_config_ui_tree_view_select_rows_from_list (
                                gsb_data_report_get_account_numbers_list ( report_number ),
                                "treeview_comptes",
                                1 );

        if ( g_slist_length ( gsb_data_report_get_account_numbers_list ( report_number ) ) )
            utils_togglebutton_set_label_position_unselect (
                                etats_config_ui_widget_get_widget_by_name (
                                "togglebutton_select_all_comptes", NULL ),
                                G_CALLBACK ( etats_config_ui_onglet_comptes_select_unselect ),
                                etats_config_ui_widget_get_widget_by_name ( "treeview_comptes", NULL ) );
    }
}


/**
 * Récupère les informations de l'onglet comptes
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void gsb_etats_config_recupere_info_onglet_comptes ( gint report_number )
{
    gint active;

    active = etats_config_ui_widget_get_actif ( "bouton_detaille_comptes_etat" );
    gsb_data_report_set_account_use_chosen ( report_number, active );

    if ( active )
    {
        gsb_data_report_free_account_numbers_list ( report_number );

        if ( utils_tree_view_all_rows_are_selected ( GTK_TREE_VIEW (
         etats_config_ui_widget_get_widget_by_name ( "treeview_comptes", NULL ) ) ) )
        {
            gchar *text;
            gchar *hint;

            hint = g_strdup ( _("Performance issue.") );
            text = g_strdup ( _("All accounts have been selected.  Grisbi will run "
                            "faster without the \"Detail accounts used\" option activated") );

            dialogue_special ( GTK_MESSAGE_INFO, make_hint ( hint, text ) );
            etats_config_ui_widget_set_actif ( "gsb_data_report_set_account_use_chosen", FALSE );
            gsb_data_report_set_account_use_chosen ( report_number, FALSE );

            g_free ( text );
            g_free ( hint );
        }
        else
            gsb_data_report_set_account_numbers_list ( report_number,
                            etats_config_ui_tree_view_get_list_rows_selected ( "treeview_comptes" ) );
    }
}


/**
 * retourne la liste des comptes dans un GtkTreeModel
 *
 * \return model
 */
GtkTreeModel *gsb_etats_config_onglet_get_liste_comptes ( void )
{
    GtkListStore *list_store;
    GSList *list_tmp;

    list_store = gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_INT );

    /* on remplit la liste des exercices */
    list_tmp = gsb_data_account_get_list_accounts ( );

    while ( list_tmp )
    {
        GtkTreeIter iter;
        gchar *name;
        gint account_number;

        account_number = gsb_data_account_get_no_account ( list_tmp -> data );

        name = my_strdup ( gsb_data_account_get_name ( account_number ) );

        gtk_list_store_append ( list_store, &iter );
        gtk_list_store_set ( list_store, &iter, 0, name, 1, account_number, -1 );

        if ( name )
            g_free ( name );
    
        list_tmp = list_tmp -> next;
    }

    return GTK_TREE_MODEL ( list_store );
}


/**
 *
 *
 *
 */
void gsb_etats_config_onglet_select_partie_liste_comptes ( GtkWidget *tree_view,
                        gint type_compte )
{
        GtkTreeModel *model;
        GtkTreeIter iter;
        GtkTreeSelection *selection;

        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );

        model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
        if ( !gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
            return;

        do
        {
            gint account_number;

            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter, 1, &account_number, -1 );
            if ( gsb_data_account_get_kind ( account_number ) == type_compte )
                gtk_tree_selection_select_iter ( selection, &iter );
        }
        while (gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );
}

/*ONGLET_TIERS*/
/**
 * Initialise les informations de l'onglet tiers
 *
 * \param report_number
 *
 * \return
 */
void gsb_etats_config_initialise_onglet_tiers ( gint report_number )
{
    gint active;

    active = gsb_data_report_get_payee_detail_used ( report_number );
    etats_config_ui_widget_set_actif ( "bouton_detaille_tiers_etat", active );

    if ( active )
    {
        etats_config_ui_tree_view_select_rows_from_list (
                                gsb_data_report_get_payee_numbers_list ( report_number ),
                                "treeview_tiers",
                                1 );

        if ( g_slist_length ( gsb_data_report_get_payee_numbers_list ( report_number ) ) )
            utils_togglebutton_set_label_position_unselect (
                                etats_config_ui_widget_get_widget_by_name (
                                "togglebutton_select_all_tiers", NULL ),
                                NULL,
                                etats_config_ui_widget_get_widget_by_name ( "treeview_tiers", NULL ) );
    }
}


/**
 *
 *
 *
 */
GtkTreeModel *gsb_etats_config_onglet_get_liste_tiers ( void )
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
void gsb_etats_config_recupere_info_onglet_tiers ( gint report_number )
{
    gint active;

    active = etats_config_ui_widget_get_actif ( "bouton_detaille_tiers_etat" );
    gsb_data_report_set_payee_detail_used ( report_number, active );
    if ( active )
    {
        gsb_data_report_free_payee_numbers_list ( report_number );

        if ( utils_tree_view_all_rows_are_selected ( GTK_TREE_VIEW (
         etats_config_ui_widget_get_widget_by_name ( "treeview_tiers", NULL ) ) ) )
        {
            gchar *text;
            gchar *hint;

            hint = g_strdup ( _("Performance issue.") );
            text = g_strdup ( _("All payees have been selected.  Grisbi will run "
                            "faster without the \"Detail payees used\" option activated.") );

            dialogue_special ( GTK_MESSAGE_INFO, make_hint ( hint, text ) );
            etats_config_ui_widget_set_actif ( "togglebutton_select_all_tiers", FALSE );
            gsb_data_report_set_payee_detail_used ( report_number, FALSE );

            g_free ( text );
            g_free ( hint );
        }
        else
            gsb_data_report_set_payee_numbers_list ( report_number,
                            etats_config_ui_tree_view_get_list_rows_selected ( "treeview_tiers" ) );
    }
}


/*ONGLET_CATEGORIES BUDGETS*/
/**
 * Initialise les informations des onglets catégories ou budgets
 *
 * \param report_number
 * \param is_categ TRUE = Categories FALSE = Budgets
 *
 * \return
 */
void gsb_etats_config_initialise_onglet_categ_budget ( gint report_number,
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

    etats_config_ui_widget_set_actif ( checkbutton_name, active );

    if ( active )
    {
        gsb_etats_config_onglet_categ_budget_init_treeview ( treeview_name, tmp_list );

        if ( g_slist_length ( tmp_list ) )
            utils_togglebutton_set_label_position_unselect (
                                etats_config_ui_widget_get_widget_by_name (
                                button_name, NULL ),
                                G_CALLBACK ( etats_config_ui_onglet_categ_budget_check_uncheck_all ),
                                etats_config_ui_widget_get_widget_by_name ( treeview_name, NULL ) );
    }

    g_free ( checkbutton_name );
    g_free ( treeview_name );
    g_free ( button_name );
}


/**
 * Récupère les informations des onglets catégories ou budgets
 *
 * \param report_number
 * \param is_categ TRUE = Categories FALSE = Budgets
 *
 * \return
 */
void gsb_etats_config_recupere_info_onglet_categ_budget ( gint report_number,
                        gboolean is_categ )
{
    gint active;
    gchar *treeview_name;

    if ( is_categ )
    {
        active = etats_config_ui_widget_get_actif ( "bouton_detaille_categ_etat" );
        treeview_name = g_strdup ( "treeview_categ" );

        gsb_data_report_set_category_detail_used ( report_number, active );
        if ( active )
        {
            gsb_data_report_set_category_struct_list ( report_number,
                                gsb_etats_config_onglet_categ_budget_get_selected ( treeview_name ) );
        }
    }
    else
    {
        active = etats_config_ui_widget_get_actif ( "bouton_detaille_budget_etat" );
        treeview_name = g_strdup ( "treeview_budget" );

        gsb_data_report_set_budget_detail_used ( report_number, active );
        if ( active )
        {
            gsb_data_report_set_budget_struct_list ( report_number,
                                gsb_etats_config_onglet_categ_budget_get_selected ( treeview_name ) );
        }
    }

    g_free ( treeview_name );
}


/**
 * create the category/budget list for reports
 * it's a tree with categories/budget and sub-categories/budgets,
 * and a check-button to select them
 *
 * \param
 *
 * \return a GtkWidget : the GtkTreeView
 * */
GtkTreeModel *gsb_etats_config_onglet_categ_budget_get_model ( gboolean is_categ )
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
                        (GtkTreeIterCompareFunc) gsb_etats_config_onglet_categ_budget_sort_function,
                        GINT_TO_POINTER ( is_categ ),
                        NULL );

    gsb_etats_config_onglet_categ_budget_fill_model ( GTK_TREE_MODEL ( store ), is_categ );

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
gboolean gsb_etats_config_onglet_categ_budget_toggled ( GtkCellRendererToggle *radio_renderer,
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
void gsb_etats_config_onglet_categ_budget_tree_model_check_uncheck_all ( GtkTreeModel *model,
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
 *
 */
gboolean gsb_etats_config_onglet_categ_budget_select_partie_liste ( GtkWidget *button,
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

    gsb_etats_config_onglet_categ_budget_tree_model_check_uncheck_all ( model, FALSE );

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


/**
 * fill the categories selection list for report
 *
 * \param model
 * \param is_categ  TRUE for category, FALSE for budget
 *
 * \return FALSE
 * */
gboolean gsb_etats_config_onglet_categ_budget_fill_model ( GtkTreeModel *model,
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
 *
 *
 *
 */
gint gsb_etats_config_onglet_categ_budget_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gpointer ptr )
{
    gchar *name_1;
    gchar *name_2;
    gchar *without_name;
    gchar *without_sub_name;
    gint number_1;
    gint number_2;
    gint sub_number_1;
    gint sub_number_2;
    gint return_value = 0;
    gboolean is_categ;

    is_categ = GPOINTER_TO_INT ( ptr );

    if ( is_categ )
    {
        without_name = _("No category");
        without_sub_name = _("No subcategory");
    }
    else
    {
        without_name = _("No budgetary line");
        without_sub_name = _("No sub-budgetary line");
    }

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

        if ( name_1) g_free ( name_1);
        if ( name_2) g_free ( name_2);
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
 * get all the selected categ and sub-categ and place them in a list of struct_categ_budget_sel
 * to be saved in the report
 *
 * \param is_categ  TRUE for categ, FALSE for budget
 *
 * \return a GSList of struct_categ_budget_sel or NULL if all the categories/sub-categ were selected
 *          to avoid to filter by categ, to improve speed
 * */
GSList *gsb_etats_config_onglet_categ_budget_get_selected ( const gchar *treeview_name )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeIter parent_iter;
    gboolean all_selected = TRUE;
    GSList *tmp_list = NULL;

    /* on récupère le modèle */
    tree_view = etats_config_ui_widget_get_widget_by_name ( treeview_name, NULL );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    if ( !gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &parent_iter ) )
        return NULL;

    do
    {
        gint div_number;
        gboolean active;
        struct_categ_budget_sel *categ_budget_struct;
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
        categ_budget_struct = g_malloc0 ( sizeof ( struct_categ_budget_sel ) );

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
            dialogue_special ( GTK_MESSAGE_INFO,
                        make_hint ( _("Performance issue."),
                        _("All categories have been selected.  Grisbi will run faster without "
                        "the \"Detail categories used\" option activated.") ) );
        else
            dialogue_special ( GTK_MESSAGE_INFO,
                        make_hint ( _("Performance issue."),
                        _("All budgets have been selected.  Grisbi will run faster without the "
                        "\"Detail budgets used\" option activated.") ) );

        gsb_etats_config_onglet_categ_budget_tree_model_check_uncheck_all ( model, FALSE );
        gsb_data_report_free_categ_budget_struct_list ( tmp_list );

        tmp_list = NULL;
    }
    return tmp_list;
}


/**
 * toggle the categories and sub-categories selected
 *
 * \param is_categ  TRUE for categ, FALSE for budgets
 *
 * \return
 * */
void gsb_etats_config_onglet_categ_budget_init_treeview ( const gchar *treeview_name,
                            GSList *tmp_list )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeIter parent_iter;

    /* on récupère le modèle */
    tree_view = etats_config_ui_widget_get_widget_by_name ( treeview_name, NULL );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    gsb_etats_config_onglet_categ_budget_tree_model_check_uncheck_all ( model, FALSE );

    if ( !gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &parent_iter ) )
        return;

    while ( tmp_list )
    {
        struct_categ_budget_sel *categ_budget_struct = tmp_list -> data;

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


/*ONGLET_MODE_PAIEMENT*/
/**
 * Initialise les informations de l'onglet modes de paiement
 *
 * \param report_number
 *
 * \return
 */
void gsb_etats_config_initialise_onglet_mode_paiement ( gint report_number )
{
    gint active;

    active = gsb_data_report_get_method_of_payment_used ( report_number );
    etats_config_ui_widget_set_actif ( "bouton_detaille_mode_paiement_etat", active );

    if ( active )
    {
        etats_config_ui_onglet_mode_paiement_select_rows_from_list (
                                gsb_data_report_get_method_of_payment_list ( report_number ),
                                "treeview_mode_paiement" );

        if ( g_slist_length ( gsb_data_report_get_method_of_payment_list ( report_number ) ) )
            utils_togglebutton_set_label_position_unselect (
                                etats_config_ui_widget_get_widget_by_name (
                                "togglebutton_select_all_mode_paiement", NULL ),
                                NULL,
                                etats_config_ui_widget_get_widget_by_name ( "treeview_mode_paiement", NULL ) );
    }
}


/**
 * Récupère les informations de l'onglet mode de paiement
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void gsb_etats_config_recupere_info_onglet_mode_paiement ( gint report_number )
{
    gint active;

    active = etats_config_ui_widget_get_actif ( "bouton_detaille_mode_paiement_etat" );
    gsb_data_report_set_method_of_payment_used ( report_number, active );
    if ( active )
    {
        gsb_data_report_free_method_of_payment_list ( report_number );

        if ( utils_tree_view_all_rows_are_selected ( GTK_TREE_VIEW (
         etats_config_ui_widget_get_widget_by_name ( "treeview_mode_paiement", NULL ) ) ) )
        {
            gchar *text;
            gchar *hint;

            hint = g_strdup ( _("Performance issue.") );
            text = g_strdup ( _("All methods of payment have been selected.  Grisbi will run "
                            "faster without the \"Detail methods of payment used\" option activated.") );

            dialogue_special ( GTK_MESSAGE_INFO, make_hint ( hint, text ) );
            etats_config_ui_widget_set_actif ( "togglebutton_select_all_mode_paiement", FALSE );
            gsb_data_report_set_method_of_payment_used ( report_number, 0 );

            g_free ( text );
            g_free ( hint );
        }
        else
            gsb_data_report_set_method_of_payment_list ( report_number,
                            etats_config_ui_onglet_mode_paiement_get_list_rows_selected (
                            "treeview_mode_paiement" ) );

    }
    /* return */
}


/**
 *
 *
 *
 */
GtkTreeModel *gsb_etats_config_onglet_mode_paiement_get_model ( void )
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
    g_slist_foreach ( liste_nom_types, ( GFunc ) g_free, NULL );
    g_slist_free ( liste_nom_types );

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
void gsb_etats_config_initialise_onglet_divers ( gint report_number )
{
    gint index;

    index = gsb_data_report_get_show_m ( report_number );
    etats_config_ui_buttons_radio_set_active_index ( "radiobutton_marked_all", index );

    etats_config_ui_widget_set_actif ( "checkbutton_marked_P",
                        gsb_data_report_get_show_p ( report_number ) );
    etats_config_ui_widget_set_actif ( "checkbutton_marked_R",
                        gsb_data_report_get_show_r ( report_number ) );
    etats_config_ui_widget_set_actif ( "checkbutton_marked_T",
                        gsb_data_report_get_show_t ( report_number ) );

    etats_config_ui_widget_set_actif ( "bouton_pas_detailler_ventilation",
                        gsb_data_report_get_not_detail_split ( report_number ) );
}


/**
 * Récupère les informations de l'onglet divers
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void gsb_etats_config_recupere_info_onglet_divers ( gint report_number )
{
    gint index;

    index = etats_config_ui_buttons_radio_get_active_index ( "radiobutton_marked_all" );
    gsb_data_report_set_show_m ( report_number, index );

    gsb_data_report_set_show_p ( report_number,
                        etats_config_ui_widget_get_actif ( "checkbutton_marked_P" ) );
    gsb_data_report_set_show_r ( report_number,
                        etats_config_ui_widget_get_actif ( "checkbutton_marked_R" ) );
    gsb_data_report_set_show_t ( report_number,
                        etats_config_ui_widget_get_actif ( "checkbutton_marked_T" ) );

    gsb_data_report_set_not_detail_split ( report_number,
                        etats_config_ui_widget_get_actif ( "bouton_pas_detailler_ventilation" ) );
}


/*ONGLET_DATA_GROUPING*/
/**
 * Initialise les informations de l'onglet groupement des données
 *
 * \param report_number
 *
 * \return
 */
void gsb_etats_config_initialise_onglet_data_grouping ( gint report_number )
{
    etats_config_ui_widget_set_actif ( "bouton_regroupe_ope_compte_etat",
                        gsb_data_report_get_account_group_reports ( report_number ) );
    etats_config_ui_widget_set_actif ( "bouton_utilise_tiers_etat",
                        gsb_data_report_get_payee_used ( report_number ) );
    etats_config_ui_widget_set_actif ( "bouton_group_by_categ",
                        gsb_data_report_get_category_used ( report_number ) );
    etats_config_ui_widget_set_actif ( "bouton_utilise_ib_etat",
                        gsb_data_report_get_budget_used ( report_number ) );

    gsb_etats_config_onglet_data_grouping_update_model ( report_number );
}


/**
 * Récupère les informations de l'onglet groupement des données
 *
 * \param numéro d'état à mettre à jour
 *
 * \return
 */
void gsb_etats_config_recupere_info_onglet_data_grouping ( gint report_number )
{
    gsb_data_report_set_account_group_reports ( report_number,
                        etats_config_ui_widget_get_actif ( "bouton_regroupe_ope_compte_etat" ) );
    gsb_data_report_set_payee_used ( report_number,
                        etats_config_ui_widget_get_actif ( "bouton_utilise_tiers_etat" ) );
    gsb_data_report_set_category_used ( report_number,
                        etats_config_ui_widget_get_actif ( "bouton_group_by_categ" ) );
    gsb_data_report_set_budget_used ( report_number,
                        etats_config_ui_widget_get_actif ( "bouton_utilise_ib_etat" ) );

    /* on récupère les données servant au tri des données pour l'affichage */
    gsb_data_report_free_sorting_type_list ( report_number );
    gsb_data_report_set_sorting_type_list ( report_number,
                        gsb_etats_config_onglet_data_grouping_get_list ( report_number ) );
}


/**
 * Update le modèle avec les données sauvegardées
 *
 * \param report_number
 *
 * \return TRUE
 */
gboolean gsb_etats_config_onglet_data_grouping_update_model ( gint report_number )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GSList *tmp_list;
    gint i = 0;

    tree_view = etats_config_ui_widget_get_widget_by_name ( "treeview_data_grouping", NULL );
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

        string = gsb_etats_config_onglet_data_grouping_get_string ( type_data, i );
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
 * retourne une chaine de caractère formatée en fonction du type de donnée
 * et de la position dans la liste
 *
 * \param type_data     type de donnée : 1 Categ, 3 IB, 5 Account, 6 Payee.
 * \param pos           numéro de ligne dans le modèle
 *
 * \return NULL or a formatted string
 */
gchar *gsb_etats_config_onglet_data_grouping_get_string ( gint type_data,
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
 * return la liste des données par ordre de tri
 *
 * \param report_number
 *
 * \return a GSList
 * */
GSList *gsb_etats_config_onglet_data_grouping_get_list ( gint report_number )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GSList *tmp_list = NULL;

    tree_view = etats_config_ui_widget_get_widget_by_name ( "treeview_data_grouping", NULL );
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
 * déplace un item suite à un drag and drop dans le tree_view
 *
 * \param src_pos           position avant
 * \param src_type_data     type de donnée
 * \param dst_pos           position après drag and drop
 *
 * \return
 * */
void gsb_etats_config_onglet_data_grouping_move_in_list ( gint src_pos,
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

    tree_view = etats_config_ui_widget_get_widget_by_name ( "treeview_data_grouping", NULL );
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

        string = gsb_etats_config_onglet_data_grouping_get_string ( type_data, i );

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
void gsb_etats_config_initialise_onglet_data_separation ( gint report_number )
{
    GtkWidget *combo_1;
    GtkWidget *combo_2;

    etats_config_ui_widget_set_actif ( "bouton_separer_revenus_depenses",
                        gsb_data_report_get_split_credit_debit ( report_number ) );
    etats_config_ui_widget_set_actif ( "bouton_separe_exo_etat",
                        gsb_data_report_get_financial_year_split ( report_number ) );

    /* on initialise le combo bouton_type_separe_plages_etat */
    combo_1 = etats_config_ui_widget_get_widget_by_name ( "bouton_type_separe_plages_etat", NULL );
    gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo_1 ),
                        gsb_data_report_get_period_split_type ( report_number ) );

    /* on initialise le combo bouton_debut_semaine */
    combo_2 = etats_config_ui_widget_get_widget_by_name ( "bouton_debut_semaine", NULL );
    gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo_2 ),
                        gsb_data_report_get_period_split_day ( report_number ) );

    if ( gsb_data_report_get_period_split ( report_number ) )
    {
        GtkWidget *button;

        button = etats_config_ui_widget_get_widget_by_name ( "bouton_separe_plages_etat", NULL );
        etats_config_ui_widget_set_actif ( "bouton_separe_plages_etat", TRUE );
        sens_desensitive_pointeur ( button,
                        etats_config_ui_widget_get_widget_by_name ( "paddingbox_data_separation2", NULL ) );

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
void gsb_etats_config_recupere_info_onglet_data_separation ( gint report_number )
{
    gsb_data_report_set_split_credit_debit ( report_number,
                        etats_config_ui_widget_get_actif ( "bouton_separer_revenus_depenses" ) );
    gsb_data_report_set_financial_year_split ( report_number,
                        etats_config_ui_widget_get_actif ( "bouton_separe_exo_etat" ) );
    gsb_data_report_set_period_split ( report_number,
                        etats_config_ui_widget_get_actif ( "bouton_separe_plages_etat" ) );

    /* récupère des index des GtkComboBox */
    gsb_data_report_set_period_split_type ( report_number,
                        gtk_combo_box_get_active ( GTK_COMBO_BOX (
                        etats_config_ui_widget_get_widget_by_name ( "bouton_type_separe_plages_etat", NULL ) ) ) );

    gsb_data_report_set_period_split_day ( report_number,
                        gtk_combo_box_get_active ( GTK_COMBO_BOX (
                        etats_config_ui_widget_get_widget_by_name ( "bouton_debut_semaine", NULL ) ) ) );
}


/**
 * fonction de callback appellée quand on change le type de période
 *
 * \param combo         le GtkComboBox qui change
 * \param widget        le widget qu'on rend sensible ou pas.
 *
 * \return
 */
void gsb_etats_config_onglet_data_separation_combo_changed ( GtkComboBox *combo,
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


/*OLD_FUNCTIONS*/

/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_etat_texte ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *sw;
    GtkWidget *lignes;

    devel_debug (NULL);

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "onglet_etat_texte" ) );

    vbox = new_vbox_with_title_and_icon ( _("Transaction content"), "text.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    gtk_widget_set_sensitive ( utils_gtkbuilder_get_widget_by_name (etat_config_builder,
                        "vbox_generale_texte_etat", NULL ), FALSE );

    /* on attache la vbox pour les lignes de recherche à sw_texte */
    sw = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "sw_texte", NULL );
    lignes = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "liste_texte_etat", NULL );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( sw ), lignes );

    /* on crée la première ligne de la recherche */
/*     gsb_etats_config_onglet_etat_texte_new_line ( lignes );  */

    /* on met la connection pour rendre sensitif la vbox_generale_textes_etat */
/*     g_signal_connect ( G_OBJECT ( utils_gtkbuilder_get_widget_by_name (etat_config_builder,
 *                         "bouton_utilise_texte", NULL ) ),
 *                         "toggled",
 *                         G_CALLBACK ( sens_desensitive_pointeur ),
 *                         utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "vbox_generale_texte_etat", NULL ) );
 */

    /* on retourne la vbox */
    return vbox_onglet;
}


/**
 * crée une ligne de recherche de texte
 *
 *\parent       vbox qui contiendra toutes les lignes
 *\first_line   première ligne ou ligne supplémentaire
 *
 *\return la nouvelle ligne
 */
GtkWidget *gsb_etats_config_onglet_etat_texte_new_line ( GtkWidget *parent )
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *hbox_1;
    GtkWidget *hbox_2;
    GtkWidget *combo;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *radio_1;
    GtkWidget *radio_2;

    /* la vbox qui contient la ligne complète */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_show ( vbox );

    /* la première hbox pour le type de donnée concernée */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_widget_show ( hbox );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( "Transactions whose " );
    gtk_widget_show ( label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* on crée et initialise le combobox du type de choix pour la recherche de texte */
    combo = gtk_combo_box_new ( );
    gtk_widget_show ( combo );
    gsb_etats_config_onglet_etat_combo_set_model ( combo, champs_type_recherche_texte );
    g_object_set_data ( G_OBJECT ( vbox ), "combobox_texte_etat", combo );
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 5 );

    /* on définit l'action a faire lorsque l'on change le choix du combobox */
    g_signal_connect ( G_OBJECT ( combo ),
                        "changed",
                        G_CALLBACK ( gsb_etats_config_onglet_etat_texte_combo_changed ),
                        vbox );

    /* on ajoute le bouton ajouter une nouvelle ligne */
    gsb_etats_config_onglet_etat_texte_get_buttons_add_remove ( hbox, FALSE );

    /* la deuxième hbox pour le type recherche de texte */
    hbox_1 = gtk_hbox_new ( FALSE, 5 );
    gtk_widget_show ( hbox_1 );
    g_object_set_data ( G_OBJECT ( vbox ), "hbox_etat_texte_compare_texte", hbox_1 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox_1, FALSE, FALSE, 0 );

    /* on crée le radio bouton de sélection entre les deux types de recherche caché par défaut */
    radio_1 = gtk_radio_button_new ( NULL );
    g_object_set_data ( G_OBJECT ( vbox ), "radio_1_texte_etat", radio_1 );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), radio_1, FALSE, FALSE, 5 );

    /* on crée et initialise le combobox de l'opérateur pour la recherche de texte */
    combo = gtk_combo_box_new ( );
    gtk_widget_show ( combo );
    gsb_etats_config_onglet_etat_combo_set_model ( combo, champs_operateur_recherche_texte );
    g_object_set_data ( G_OBJECT ( vbox ), "combobox_operateur_txt", combo );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), combo, FALSE, FALSE, 5 );

    /* on crée le champs texte pour entrer le texte recherché */
    entry = gtk_entry_new ( );
    gtk_widget_show ( entry );
    g_object_set_data ( G_OBJECT ( vbox ), "entry_operateur_txt", entry );
    gtk_box_pack_start ( GTK_BOX ( hbox_1 ), entry, FALSE, FALSE, 5 );

    /* la troisième hbox pour le type recherche de nombre */
    hbox_2 = gtk_hbox_new ( FALSE, 5 );
    g_object_set_data ( G_OBJECT ( vbox ), "hbox_etat_texte_compare_nbre", hbox_2 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox_2, FALSE, FALSE, 0 );

    radio_2 = gtk_radio_button_new_from_widget ( GTK_RADIO_BUTTON ( radio_1 ) );
    g_object_set_data ( G_OBJECT ( vbox ), "radio_2_texte_etat", radio_2 );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), radio_2, FALSE, FALSE, 5 );

    label = gtk_label_new ( _("is ") );
    gtk_widget_show ( label );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), label, FALSE, FALSE, 5 );

    /* on crée et initialise le combobox pour la première comparaison de nombre */
    combo = gtk_combo_box_new ( );
    gtk_widget_show ( combo );
    gsb_etats_config_onglet_etat_combo_set_model ( combo, champs_comparateur_nombre );
    g_object_set_data ( G_OBJECT ( vbox ), "combobox_first_comparison", combo );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), combo, FALSE, FALSE, 5 );

    label = gtk_label_new ( _("at ") );
    gtk_widget_show ( label );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), label, FALSE, FALSE, 5 );

    /* on crée le champs texte pour entrer la première comparaison */
    entry = gtk_entry_new ( );
    gtk_widget_show ( entry );
    g_object_set_data ( G_OBJECT ( vbox ), "entry_first_comparison", entry );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), entry, FALSE, FALSE, 5 );

    /* on crée et initialise le combobox pour autoriser la seconde comparaison de nombre */
    combo = gtk_combo_box_new ( );
    gtk_widget_show ( combo );
    gsb_etats_config_onglet_etat_combo_set_model ( combo, champs_comparateur_nombre_2 );
    g_object_set_data ( G_OBJECT ( vbox ), "combobox_valid_second_comparison", combo );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), combo, FALSE, FALSE, 5 );

    /* on crée et initialise le combobox pour la seconde comparaison de nombre */
    combo = gtk_combo_box_new ( );
    gsb_etats_config_onglet_etat_combo_set_model ( combo, champs_comparateur_nombre_2 );
    g_object_set_data ( G_OBJECT ( vbox ), "combobox_second_comparison", combo );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), combo, FALSE, FALSE, 5 );

    label = gtk_label_new ( _("at ") );
    g_object_set_data ( G_OBJECT ( vbox ), "label_second_comparison", label );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), label, FALSE, FALSE, 5 );

    /* on crée le champs texte pour entrer la première comparaison */
    entry = gtk_entry_new ( );
    g_object_set_data ( G_OBJECT ( vbox ), "entry_second_comparison", entry );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ), entry, FALSE, FALSE, 5 );

    /* on met la ligne complète (vbox) dans son parent */
    gtk_box_pack_start ( GTK_BOX ( parent ), vbox, FALSE, FALSE, 5 );

    return vbox;
}


/**
 *
 *
 *
 */
void gsb_etats_config_onglet_etat_texte_get_buttons_add_remove ( GtkWidget *parent,
                        gboolean button_2_visible )
{
    GtkWidget *alignement;
    GtkWidget *button;

    alignement = gtk_alignment_new ( 1, 0, 0, 0 );
    gtk_widget_show ( alignement );
    gtk_box_pack_start ( GTK_BOX ( parent ), alignement, TRUE, TRUE, 0 );

    button = gtk_button_new_with_label ( _("Add") );
    gtk_widget_show ( button );
    gtk_button_set_relief ( GTK_BUTTON ( button ), GTK_RELIEF_NONE );

/*    g_signal_connect_swapped ( G_OBJECT ( button ),
			       "clicked",
			       G_CALLBACK ( ajoute_ligne_liste_comparaisons_textes_etat ),
			       GINT_TO_POINTER (text_comparison_number) );
*/
    gtk_container_add ( GTK_CONTAINER ( alignement ), button );

    button = gtk_button_new_with_label ( _("Remove") );
    if ( button_2_visible )
        gtk_widget_show ( button );
    gtk_button_set_relief ( GTK_BUTTON ( button ), GTK_RELIEF_NONE );
/*
    g_signal_connect_swapped ( G_OBJECT ( button ),
			       "clicked",
			       G_CALLBACK ( retire_ligne_liste_comparaisons_textes_etat ),
			       GINT_TO_POINTER (text_comparison_number) );
*/
    gtk_container_add ( GTK_CONTAINER ( alignement ), button );
}


/**
 *
 *
 *
 */
void gsb_etats_config_onglet_etat_combo_set_model ( GtkWidget *combo,
                        gchar **tab )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    gint i = 0;

    model = GTK_TREE_MODEL ( gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_INT ) );

    while ( tab[i] )
    {
        gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
        gtk_list_store_set ( GTK_LIST_STORE ( model ),
                            &iter,
                            0,  gettext ( tab[i] ),
                            1, i,
                            -1 );
        i++;
    }

    gtk_combo_box_set_model ( GTK_COMBO_BOX ( combo ), model );

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start ( GTK_CELL_LAYOUT ( combo ), renderer, TRUE );
    gtk_cell_layout_set_attributes ( GTK_CELL_LAYOUT ( combo ),
                        renderer,
                        "text", 0,
                        NULL);

    gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), 0 );
}


/**
 *
 *
 *
 */
void gsb_etats_config_onglet_etat_texte_combo_changed ( GtkComboBox *combo,
                        GtkWidget *widget )
{
     GtkTreeIter iter;

    if ( gtk_combo_box_get_active_iter ( combo, &iter ) )
    {
        GtkTreeModel *model;
        gchar *text;

        model = gtk_combo_box_get_model ( combo );
        gtk_tree_model_get ( model, &iter, 0, &text, -1 );
        printf ("text = %s\n", text );
    }
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_etat_montant ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;

    devel_debug (NULL);

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "onglet_etat_montant" ) );

    vbox = new_vbox_with_title_and_icon ( _("Amount"), "amount.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    gtk_widget_set_sensitive ( utils_gtkbuilder_get_widget_by_name (etat_config_builder,
                        "vbox_generale_montant_etat", NULL ), FALSE );

    /* on attache la vbox pour les lignes de recherche à sw_montant */
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (
                        utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "sw_montant", NULL ) ),
                        utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "liste_montant_etat", NULL ) );

    /* on remplit le combobox de choix du type de texte dans lequel chercher */
/*     gsb_etats_config_onglet_etat_combo_set_model (
 *                         utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "combobox_comparateur_1", NULL ),
 *                         champs_comparateur_montant );
 */


    /* on met la connection pour rendre sensitif la vbox_generale_textes_etat */
/*     g_signal_connect ( G_OBJECT ( utils_gtkbuilder_get_widget_by_name (etat_config_builder,
 *                         "bouton_utilise_montant", NULL ) ),
 *                         "toggled",
 *                         G_CALLBACK ( sens_desensitive_pointeur ),
 *                         utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "vbox_generale_montant_etat", NULL ) );
 */

    /* on retourne la vbox */
    return vbox_onglet;
}


/**
 *
 *
 *
 */
/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_affichage_etat_generalites ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "affichage_etat_generalites" ) );

    vbox = new_vbox_with_title_and_icon ( _("Generalities"), "generalities.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    return vbox_onglet;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_affichage_etat_titres ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "affichage_etat_divers" ) );

    vbox = new_vbox_with_title_and_icon ( _("Titles"), "title.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    return vbox_onglet;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_affichage_etat_operations ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "affichage_etat_operations" ) );

    vbox = new_vbox_with_title_and_icon ( _("Transactions display"), "transdisplay.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    return vbox_onglet;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_affichage_etat_devises ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "affichage_etat_devises" ) );

    vbox = new_vbox_with_title_and_icon ( _("Totals currencies"), "currencies.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    return vbox_onglet;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
