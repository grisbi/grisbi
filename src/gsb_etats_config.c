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
static void gsb_etats_config_initialise_onglet_periode ( gint report_number );
static void gsb_etats_config_initialise_onglet_tiers ( gint report_number );
static void gsb_etats_config_initialise_onglet_virements ( gint report_number );

static gboolean gsb_etats_config_recupere_info_to_etat ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_comptes ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_periode ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_tiers ( gint report_number );
static void gsb_etats_config_recupere_info_onglet_virements ( gint report_number );


/**********************************************************************************************************************/
static GtkWidget *gsb_etats_config_affichage_etat_devises ( void );
static GtkWidget *gsb_etats_config_affichage_etat_generalites ( void );
static GtkWidget *gsb_etats_config_affichage_etat_operations ( void );
static GtkWidget *gsb_etats_config_affichage_etat_titres ( void );
static gboolean gsb_etats_config_budget_select_all ( GtkWidget *button,
                        gboolean *select_ptr );
static gint gsb_etats_config_categ_budget_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gchar *sw_name );
static gboolean gsb_etats_config_categ_budget_toggled ( GtkCellRendererToggle *radio_renderer,
                        gchar *path_str,
                        GtkTreeStore *store );
static GtkWidget *gsb_etats_config_get_liste_categ_budget ( gchar *sw_name );
static GtkWidget *gsb_etats_config_onglet_get_liste_dates ( void );
static GtkWidget *gsb_etats_config_get_liste_mode_paiement ( gchar *sw_name );
static GtkWidget *gsb_etats_config_get_report_tree_view ( void );
static GtkWidget *gsb_etats_config_get_scrolled_window_with_tree_view ( gchar *sw_name,
                        GtkTreeModel *model );

static gboolean gsb_etats_config_fill_liste_categ_budget ( gboolean is_categ );
static GtkWidget *gsb_etats_config_onglet_etat_categories ( void );
static GtkWidget *gsb_etats_config_onglet_etat_comptes ( void );
static GtkWidget *gsb_etats_config_onglet_etat_period ( void );
static GtkWidget *gsb_etats_config_onglet_etat_divers ( void );
static GtkWidget *gsb_etats_config_onglet_etat_ib ( void );
static GtkWidget *gsb_etats_config_onglet_etat_mode_paiement ( void );
static GtkWidget *gsb_etats_config_onglet_etat_montant ( void );
static GtkWidget *gsb_etats_config_onglet_etat_texte ( void );
static GtkWidget *gsb_etats_config_onglet_etat_tiers ( void );
static GtkWidget *gsb_etats_config_onglet_etat_virements ( void );
static void gsb_etats_config_onglet_etat_combo_set_model ( GtkWidget *combo,
                        gchar **tab );
static void gsb_etats_config_onglet_etat_texte_combo_changed ( GtkComboBox *combo,
                        GtkWidget *widget );
static void gsb_etats_config_onglet_etat_texte_get_buttons_add_remove ( GtkWidget *parent,
                        gboolean button_2_visible );
static GtkWidget *gsb_etats_config_onglet_etat_texte_new_line ( GtkWidget *parent );
static gboolean gsb_etats_config_onglet_select_partie_liste_categ_budget ( GtkWidget *button,
                        GdkEventButton *event,
                        GtkWidget *tree_view );
static GtkWidget *gsb_etats_config_page_data_grouping ( void );
static GtkWidget *gsb_etats_config_page_data_separation ( void );
static void gsb_etats_config_populate_tree_model ( GtkTreeStore *tree_model,
                        GtkWidget *notebook );
static gboolean gsb_etats_config_report_tree_selectable_func (GtkTreeSelection *selection,
                        GtkTreeModel *model,
                        GtkTreePath *path,
                        gboolean path_currently_selected,
                        gpointer data);
static gboolean gsb_etats_config_report_tree_view_selection_changed ( GtkTreeSelection *selection,
                        GtkTreeModel *model );
static gboolean gsb_etats_config_selection_dates_changed ( GtkTreeSelection *selection,
                        GtkWidget *widget );
static void gsb_etats_config_onglet_set_buttons_select_categ_budget ( gchar *sw_name,
                        GtkWidget *tree_view,
                        gboolean is_categ );
static void gsb_etats_config_togglebutton_collapse_expand_lines ( GtkToggleButton *togglebutton,
                        GtkWidget *tree_view );
static void gsb_etats_config_togglebutton_select_all ( GtkToggleButton *togglebutton,
                        GtkWidget *tree_view );
static GtkWidget *gsb_etats_config_togglebutton_set_button_expand ( gchar *sw_name,
                        GtkWidget *tree_view );
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


/* the def of the columns in the model of config tree_view */
enum gsb_report_tree_columns
{
    GSB_REPORT_TREE_TEXT_COLUMN,
    GSB_REPORT_TREE_PAGE_COLUMN,
    GSB_REPORT_TREE_BOLD_COLUMN,
    GSB_REPORT_TREE_ITALIC_COLUMN,
    GSB_REPORT_TREE_NUM_COLUMNS,
};


/*
static gchar *etats_config_jours_semaine[] =
{
    N_("Monday"),
    N_("Tuesday"),
    N_("Wednesday"),
    N_("Thursday"),
    N_("Friday"),
    N_("Saturday"),
    N_("Sunday"),
    NULL };
*/


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

    etats_config_ui_free_builder ( );

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
        etats_config_ui_tree_view_select_rows_from_list (
                                gsb_data_report_get_account_numbers_list ( report_number ),
                                "treeview_comptes",
                                1 );

    if ( g_slist_length ( gsb_data_report_get_account_numbers_list ( report_number ) ) )
    {
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
        etats_config_ui_tree_view_select_rows_from_list (
                                gsb_data_report_get_payee_numbers_list ( report_number ),
                                "treeview_tiers",
                                1 );

    if ( g_slist_length ( gsb_data_report_get_payee_numbers_list ( report_number ) ) )
    {
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
        gsb_data_report_set_payee_numbers_list ( report_number,
                            etats_config_ui_tree_view_get_list_rows_selected ( "treeview_tiers" ) );
    }
}


/*ONGLET_CATEGORIES*/
/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_etat_categories ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkWidget *button;

    devel_debug (NULL);

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "onglet_etat_categories" ) );

    vbox = new_vbox_with_title_and_icon ( _("Categories"), "categories.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    gtk_widget_set_sensitive ( utils_gtkbuilder_get_widget_by_name (etat_config_builder,
                        "vbox_detaille_categ_etat", NULL ), FALSE );

    /* on crée la liste des catégories */
    sw = gsb_etats_config_get_liste_categ_budget ( "sw_categ" );
    tree_view = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "sw_categ", "tree_view" );

    /* on remplit la liste des catégories */
    gsb_etats_config_fill_liste_categ_budget ( TRUE );

    /* on met la connection pour rendre sensitif la vbox_detaille_categ_etat */
    g_signal_connect ( G_OBJECT ( utils_gtkbuilder_get_widget_by_name (etat_config_builder,
                        "button_detail_categ_etat", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "vbox_detaille_categ_etat", NULL ) );

    /* on met la connection pour déplier replier les catégories */
    button = gsb_etats_config_togglebutton_set_button_expand ( "sw_categ", tree_view );

    /* on met la connection pour (dé)sélectionner toutes les catégories */
/*     button = gsb_etats_config_togglebutton_set_button_select ( "sw_categ", tree_view );  */

    /* on met la connection pour sélectionner une partie des catégories */
    gsb_etats_config_onglet_set_buttons_select_categ_budget ( "sw_categ", tree_view, TRUE );

    return vbox_onglet;
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
GtkWidget *gsb_etats_config_get_liste_categ_budget ( gchar *sw_name )
{
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkTreeStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    GtkCellRenderer *radio_renderer;

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
                        (GtkTreeIterCompareFunc) gsb_etats_config_categ_budget_sort_function,
                        sw_name,
                        NULL );

    /* create the tree view */
    tree_view = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( store ) );
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW ( tree_view ), FALSE );

    /* set the color of selected row */
    utils_set_tree_view_selection_and_text_color ( tree_view );

    g_object_unref ( G_OBJECT ( store ) );

    /* create the column */
    column = gtk_tree_view_column_new ();

    /* create the toggle button part */
    radio_renderer = gtk_cell_renderer_toggle_new ();
    g_object_set ( G_OBJECT ( radio_renderer ), "xalign", 0.0, NULL );

    gtk_tree_view_column_pack_start ( column,
                        radio_renderer,
                        FALSE );
    gtk_tree_view_column_set_attributes ( column,
                        radio_renderer,
                        "active", 1,
                        "activatable", 2,
                        NULL);
    g_signal_connect ( G_OBJECT ( radio_renderer ),
                        "toggled",
                        G_CALLBACK ( gsb_etats_config_categ_budget_toggled ),
                        store );

    /* create the text part */
    cell_renderer = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell_renderer ),
                        "xalign", 0.0,
                        NULL );
    gtk_tree_view_column_pack_start ( column,
                        cell_renderer,
                        TRUE );
    gtk_tree_view_column_set_attributes (column,
                        cell_renderer,
                        "text", 0,
                        NULL );

    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ), column );

    /* get the container */
    sw = GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, sw_name ) );
    gtk_container_add ( GTK_CONTAINER ( sw ), tree_view );

    g_object_set_data ( G_OBJECT ( sw ), "tree_view", tree_view );

    gtk_widget_show_all ( sw );

    return sw;
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
gboolean gsb_etats_config_categ_budget_toggled ( GtkCellRendererToggle *radio_renderer,
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
void gsb_etats_config_tree_model_check_uncheck_all ( GtkTreeModel *model,
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
void gsb_etats_config_togglebutton_collapse_expand_lines ( GtkToggleButton *togglebutton,
                        GtkWidget *tree_view )
{
    GtkWidget *hbox_expand;
    GtkWidget *hbox_collapse;

    hbox_expand = g_object_get_data ( G_OBJECT ( togglebutton ), "hbox_expand" );
    hbox_collapse = g_object_get_data ( G_OBJECT ( togglebutton ), "hbox_collapse" );

    if ( gtk_toggle_button_get_active ( togglebutton ) )
    {
        gtk_widget_hide ( hbox_expand );
        gtk_widget_show ( hbox_collapse );
        gtk_tree_view_expand_all ( GTK_TREE_VIEW ( tree_view ) );
    }
    else
    {
        gtk_widget_show ( hbox_expand );
        gtk_widget_hide ( hbox_collapse );
        gtk_tree_view_collapse_all ( GTK_TREE_VIEW ( tree_view ) );
    }
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_togglebutton_set_button_expand ( gchar *sw_name,
                        GtkWidget *tree_view )
{
    GtkWidget *button;
    gchar *tmp_str;

    tmp_str = g_strconcat ( "togglebutton_expand", sw_name + 2, NULL );
    button = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, tmp_str, NULL );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "hbox_toggle_expand", sw_name + 2, NULL );
    g_object_set_data ( G_OBJECT ( button ), "hbox_expand",
                        utils_gtkbuilder_get_widget_by_name ( etat_config_builder, tmp_str, NULL ) );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "hbox_toggle_collapse", sw_name + 2, NULL );
    g_object_set_data ( G_OBJECT ( button ), "hbox_collapse",
                        utils_gtkbuilder_get_widget_by_name ( etat_config_builder, tmp_str, NULL ) );
    g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( gsb_etats_config_togglebutton_collapse_expand_lines ),
                        tree_view );

    return button;
}


/**
 *
 *
 *
 */
void gsb_etats_config_onglet_set_buttons_select_categ_budget ( gchar *sw_name,
                        GtkWidget *tree_view,
                        gboolean is_categ )
{
    GtkWidget *button;
    gchar *tmp_str;

    tmp_str = g_strconcat ( "button_income", sw_name + 2, NULL );
    button = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, tmp_str, NULL );
    g_object_set_data ( G_OBJECT ( button ), "is_categ", GINT_TO_POINTER ( is_categ ) );
    g_object_set_data ( G_OBJECT ( button ), "type_div", GINT_TO_POINTER ( FALSE ) );
    g_signal_connect ( G_OBJECT  ( button ),
                        "button-press-event",
                        G_CALLBACK ( gsb_etats_config_onglet_select_partie_liste_categ_budget ),
                        tree_view );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "button_outgoing", sw_name + 2, NULL );
    button = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, tmp_str, NULL );
    g_object_set_data ( G_OBJECT ( button ), "is_categ", GINT_TO_POINTER ( is_categ ) );
    g_object_set_data ( G_OBJECT ( button ), "type_div", GINT_TO_POINTER ( TRUE ) );
    g_signal_connect ( G_OBJECT  ( button ),
                        "button-press-event",
                        G_CALLBACK ( gsb_etats_config_onglet_select_partie_liste_categ_budget ),
                        tree_view );
    g_free ( tmp_str );
}


/**
 *
 *
 *
 */
gboolean gsb_etats_config_onglet_select_partie_liste_categ_budget ( GtkWidget *button,
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

    gsb_etats_config_tree_model_check_uncheck_all ( model, FALSE );

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
 * \param is_categ	TRUE for category, FALSE for budget
 *
 * \return FALSE
 * */
gboolean gsb_etats_config_fill_liste_categ_budget ( gboolean is_categ )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeIter parent_iter;
    GtkTreeIter child_iter;
    GSList *list_tmp;
    gchar *name;
    gchar *without_name;
    gchar *without_sub_name;

    if ( is_categ )
    {
        list_tmp = gsb_data_category_get_categories_list ( );

        tree_view = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "sw_categ", "tree_view" );
        model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

        gtk_tree_store_clear ( GTK_TREE_STORE ( model ) );

        without_name = _("No category");
        without_sub_name = _("No subcategory");
    }
    else
    {
        list_tmp = gsb_data_budget_get_budgets_list ( );
        tree_view = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "sw_budget", "tree_view" );
        model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

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
GtkWidget *gsb_etats_config_onglet_etat_ib ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkWidget *button;

    devel_debug (NULL);

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "onglet_etat_ib" ) );

    vbox = new_vbox_with_title_and_icon ( _("Budgetary lines"), "budgetary_lines.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    gtk_widget_set_sensitive ( utils_gtkbuilder_get_widget_by_name (etat_config_builder,
                        "vbox_detaille_budget_etat", NULL ), FALSE );

    /* on crée la liste des IB et on récupère le tree_view*/
    sw = gsb_etats_config_get_liste_categ_budget ( "sw_budget" );
    tree_view = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "sw_budget", "tree_view" );

    /* on remplit la liste des IB */
    gsb_etats_config_fill_liste_categ_budget ( FALSE );

    /* on met la connection pour rendre sensitif la hbox_detaille_budget_etat */
    g_signal_connect ( G_OBJECT ( utils_gtkbuilder_get_widget_by_name (etat_config_builder,
                        "bouton_detaille_budget_etat", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "vbox_detaille_budget_etat", NULL ) );

    /* on met la connection pour déplier replier les IB */
    button = gsb_etats_config_togglebutton_set_button_expand ( "sw_budget", tree_view );

    /* on met la connection pour sélectionner toutes les IB */
/*     button = gsb_etats_config_togglebutton_set_button_select ( "sw_budget", tree_view );  */

    /* on met la connection pour sélectionner une partie des IB */
    gsb_etats_config_onglet_set_buttons_select_categ_budget ( "sw_budget", tree_view, FALSE );

    return vbox_onglet;
}


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
GtkWidget *gsb_etats_config_onglet_etat_mode_paiement ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *sw;

    devel_debug (NULL);

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "onglet_etat_mode_paiement" ) );

    vbox = new_vbox_with_title_and_icon ( _("Payment methods"), "payment.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    gtk_widget_set_sensitive ( utils_gtkbuilder_get_widget_by_name ( etat_config_builder,
                        "vbox_mode_paiement_etat", NULL ), FALSE );

    /* on crée la liste des catégories */
    sw = gsb_etats_config_get_liste_mode_paiement ( "sw_mode_paiement" );

    /* on met la connection pour rendre sensitif la vbox_generale_comptes_etat */
    g_signal_connect ( G_OBJECT ( utils_gtkbuilder_get_widget_by_name ( etat_config_builder,
                        "bouton_detaille_mode_paiement_etat", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "vbox_mode_paiement_etat", NULL ) );

    return vbox_onglet;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_get_liste_mode_paiement ( gchar *sw_name )
{
    GtkWidget *sw;
    GtkListStore *list_store;
    GSList *liste_nom_types = NULL;
    GSList *list_tmp;

    list_store = gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_INT );

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( list_store ),
                        0, GTK_SORT_ASCENDING );

    /* create a list of unique names */
    list_tmp = gsb_data_payment_get_payments_list ( );

    while (list_tmp)
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

    sw = gsb_etats_config_get_scrolled_window_with_tree_view ( sw_name, GTK_TREE_MODEL ( list_store ) );

    gtk_widget_show_all ( sw );

    return sw;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_etat_divers ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *paddingbox;
    GtkWidget *button;

    devel_debug (NULL);

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "onglet_etat_divers" ) );

    vbox = new_vbox_with_title_and_icon ( _("Miscellaneous"), "generalities.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    /* on peut sélectionner les opérations marquées */
    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Selecting Transactions") );

    vbox = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "vbox_select_transactions_buttons", NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), vbox, TRUE, TRUE, 5 );

    button = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "radiobutton_marked", NULL );

    /* on met la connection pour rendre sensitif la vbox_detaille_categ_etat */
    g_signal_connect ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "vbox_marked_buttons", NULL ) );

    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Split of transactions detail") );

    button = utils_gtkbuilder_get_widget_by_name ( etat_config_builder, "bouton_pas_detailler_ventilation", NULL );
/*    g_signal_connect_swapped ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( report_tree_update_style ),
                        GINT_TO_POINTER ( 9 ) );
*/
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, TRUE, TRUE, 5 );
    gtk_widget_show_all ( vbox_onglet );

    return vbox_onglet;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_page_data_grouping ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "page_data_grouping" ) );

    vbox = new_vbox_with_title_and_icon ( _("Data grouping"), "organization.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    return vbox_onglet;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_page_data_separation ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "page_data_separation" ) );

    vbox = new_vbox_with_title_and_icon ( _("Data separation"), "organization.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    return vbox_onglet;
}


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


/**
 *
 *
 *
 */
gint gsb_etats_config_categ_budget_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        gchar *sw_name )
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

    if ( strcmp ( sw_name, "sw_categ" ) == 0 )
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
 *
 *
 *
 */
GtkWidget *gsb_etats_config_get_scrolled_window_with_tree_view ( gchar *sw_name,
                        GtkTreeModel *model )
{
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

/*     devel_debug (sw_name);  */

    tree_view = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( model ) );
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW ( tree_view ), FALSE );
    utils_set_tree_view_selection_and_text_color ( tree_view );

    gtk_tree_selection_set_mode (
                        gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ),
                        GTK_SELECTION_MULTIPLE );
    g_object_unref ( G_OBJECT ( model ) );

    /* set the column */
    cell = gtk_cell_renderer_text_new ( );

    column = gtk_tree_view_column_new_with_attributes ( NULL,
                        cell,
                        "text", 0,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_resizable ( column, TRUE );

    /* get the container */
    sw = GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, sw_name ) );
    gtk_container_add ( GTK_CONTAINER ( sw ), tree_view );

    g_object_set_data ( G_OBJECT ( sw ), "tree_view", tree_view );

    gtk_widget_show_all ( sw );

    return sw;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
