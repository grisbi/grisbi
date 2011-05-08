/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2011 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>


/*START_INCLUDE*/
#include "gsb_etats_config.h"
#include "etats_calculs.h"
#include "etats_config.h"
#include "fenetre_principale.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_str.h"

#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static void gsb_etats_config_dates_interval_sensitive ( gboolean show );
static void gsb_etats_config_add_line_ ( GtkTreeStore *tree_model,
                        GtkTreeIter *iter,
                        GtkWidget *notebook,
                        GtkWidget *child,
                        const gchar *title,
                        gint page );
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
static gchar *gsb_etats_config_get_full_path ( const gchar *name );
static GtkWidget *gsb_etats_config_get_liste_categ_budget ( gchar *sw_name );
static GtkWidget *gsb_etats_config_onglet_get_liste_comptes ( gchar *sw_name );
static GtkWidget *gsb_etats_config_onglet_get_liste_dates ( void );
static GtkWidget *gsb_etats_config_onglet_get_liste_exercices ( void );
static GtkWidget *gsb_etats_config_onglet_get_liste_tiers ( gchar *sw_name );
static GtkWidget *gsb_etats_config_get_liste_mode_paiement ( gchar *sw_name );
static GtkWidget *gsb_etats_config_get_report_tree_view ( void );
static GtkWidget *gsb_etats_config_get_scrolled_window_with_tree_view ( gchar *sw_name,
                        GtkTreeModel *model );
static gboolean gsb_etats_config_fill_liste_categ_budget ( gboolean is_categ );
static GtkWidget *gsb_etats_config_onglet_etat_categories ( void );
static GtkWidget *gsb_etats_config_onglet_etat_comptes ( void );
static GtkWidget *gsb_etats_config_onglet_etat_dates ( void );
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
static gboolean gsb_etats_config_onglet_select_partie_liste_comptes ( GtkWidget *button,
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
static void gsb_etats_config_onglet_set_buttons_select_comptes ( gchar *sw_name,
                        GtkWidget *tree_view );
static void gsb_etats_config_togglebutton_check_uncheck_all ( GtkTreeModel *model,
                        gboolean toggle_value );
static void gsb_etats_config_togglebutton_collapse_expand_lines ( GtkToggleButton *togglebutton,
                        GtkWidget *tree_view );
static void gsb_etats_config_togglebutton_select_all ( GtkToggleButton *togglebutton,
                        GtkWidget *tree_view );
static GtkWidget *gsb_etats_config_togglebutton_set_button_expand ( gchar *sw_name,
                        GtkWidget *tree_view );
static GtkWidget *gsb_etats_config_togglebutton_set_button_select ( gchar *sw_name,
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


/* liste des plages de date possibles */
static gchar *etats_config_liste_plages_dates[] =
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
    NULL };


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
 *
 *
 * */
GtkWidget *gsb_etats_config_get_variable_by_name ( const gchar *gtk_builder_var,
                        const gchar *optional_var )
{
    GtkWidget *var_1;
    GtkWidget *var_2 = NULL;

    var_1 = GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, gtk_builder_var ) );

    if ( optional_var == NULL )
        return var_1;

    var_2 = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( var_1 ), optional_var ) );

    return var_2;
}


/**
 * affiche la fenetre de personnalisation
 *
 *
 *
 * */
void gsb_etats_config_personnalisation_etat ( void )
{
    GtkWidget *dialog;
    GtkWidget *notebook_general;
    GtkWidget *tree_view;
    gchar *filename;
    gint current_report_number;
    gint result;
    GError *error = NULL;

    devel_debug (NULL);

    /* Creation d'un nouveau GtkBuilder */
    etat_config_builder = gtk_builder_new ( );

    if ( etat_config_builder == NULL )
        return;

    /* Chargement du XML dans etat_config_builder */
    filename = gsb_etats_config_get_full_path ( "gsb_etats_config.ui" );

    result = gtk_builder_add_from_file ( etat_config_builder, filename, &error );
    if ( result == 0 )
    {
        g_error ("%s", error->message);
        g_free ( filename );
        g_error_free ( error );

        return;
    }

    if ( !( current_report_number = gsb_gui_navigation_get_current_report ( ) ) )
        return;

    notebook_general = gsb_gui_get_general_notebook ( );
    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != GSB_REPORTS_PAGE )
        gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general), GSB_REPORTS_PAGE );

    /* Recuparation d'un pointeur sur la fenetre. */
    dialog = GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "config_etats_dialog" ) );
    gtk_window_set_transient_for ( GTK_WINDOW ( dialog ), GTK_WINDOW ( run.window ) );

    g_free ( filename );

    /* Recuparation d'un pointeur sur le gtk_tree_view. */
    tree_view = gsb_etats_config_get_report_tree_view ( );

    gtk_widget_grab_focus ( tree_view );

    gtk_widget_show ( dialog );

    switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
        case GTK_RESPONSE_OK:
            break;

        default:
            break;
    }

    gtk_widget_destroy ( dialog );
}


/**
 * obtient le fichier de l'interface graphique
 *
 *
 *
 * */
gchar *gsb_etats_config_get_full_path ( const gchar *name )
{
    gchar *filename;

/* #ifdef GRISBI_RUN_IN_SOURCE_TREE  */
    filename = g_build_filename ( PACKAGE_SOURCE_DIR, "src/ui", name, NULL );
/*     if ( g_file_test ( filename, G_FILE_TEST_EXISTS ) == FALSE )
 *  {
 *      g_free (filename);
 */
        /* Try the local file */
/* 		filename = g_build_filename ( DATADIR, "grisbi", name, NULL );
 * 
 * 		if ( g_file_test ( filename, G_FILE_TEST_EXISTS ) == FALSE )
 * 		{
 * 			g_free ( filename );
 * 
 * 			return NULL;
 * 		}
 * 	}
 */

/* #else
 * 	filename = g_build_filename (DATADIR, "grisbi", name, NULL);
 */
/*#endif */
    return filename;
}


/**
 * retourne le tree_view pour la configuration des états
 *
 *
 *
 * */
GtkWidget *gsb_etats_config_get_report_tree_view ( void )
{
    GtkWidget *sw;
    GtkWidget *tree_view = NULL;
    GtkWidget *notebook;
    GtkTreeStore *report_tree_model = NULL;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection ;

    devel_debug (NULL);

    /* Création du model */
    report_tree_model = gtk_tree_store_new ( GSB_REPORT_TREE_NUM_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_INT,
                        G_TYPE_INT );

    /* Create container + TreeView */
    tree_view = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( report_tree_model ) );
    g_object_unref ( G_OBJECT ( report_tree_model ) );

    /* set the color of selected row */
    utils_set_tree_view_selection_and_text_color ( tree_view );

    /* make column */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( "Categories",
                        cell,
                        "text", GSB_REPORT_TREE_TEXT_COLUMN,
                        "weight", GSB_REPORT_TREE_BOLD_COLUMN,
                        "style", GSB_REPORT_TREE_ITALIC_COLUMN,
                        NULL );

    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW ( tree_view ), FALSE );

    /* Handle select */
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    g_signal_connect ( selection,
                        "changed",
                        G_CALLBACK ( gsb_etats_config_report_tree_view_selection_changed ),
                        report_tree_model);

    /* Choose which entries will be selectable */
    gtk_tree_selection_set_select_function ( selection,
                        gsb_etats_config_report_tree_selectable_func, NULL, NULL );

    /* expand all rows after the treeview widget has been realized */
    g_signal_connect ( tree_view,
                        "realize",
                        G_CALLBACK ( gtk_tree_view_expand_all ),
                        NULL );

    /* initialisation du notebook pour les pages de la configuration */
    notebook = GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "notebook_config_etat" ) );
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( notebook ), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK ( notebook ), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER ( notebook ), 0 );

    /* remplissage du paned gauche */
    gsb_etats_config_populate_tree_model ( report_tree_model, notebook );

    sw = GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "sw_dialog" ) );
    gtk_container_add ( GTK_CONTAINER ( sw ), tree_view );

    gtk_widget_show_all ( tree_view );

    return tree_view;
}


/**
 *
 *
 *
 */
gboolean gsb_etats_config_report_tree_view_selection_changed ( GtkTreeSelection *selection,
                        GtkTreeModel *model )
{
    GtkTreeIter iter;
    gint selected;

    if (! gtk_tree_selection_get_selected ( selection, NULL, &iter ) )
        return(FALSE);

    gtk_tree_model_get ( model, &iter, 1, &selected, -1 );

    gtk_notebook_set_current_page ( GTK_NOTEBOOK (
                        gsb_etats_config_get_variable_by_name ( "notebook_config_etat", NULL ) ),
                        selected );

    return FALSE;
}


/**
 *
 *
 *
 */
gboolean gsb_etats_config_report_tree_selectable_func (GtkTreeSelection *selection,
                        GtkTreeModel *model,
                        GtkTreePath *path,
                        gboolean path_currently_selected,
                        gpointer data)
{
    GtkTreeIter iter;
    gint selectable;

    gtk_tree_model_get_iter ( model, &iter, path );
    gtk_tree_model_get ( model, &iter, 1, &selectable, -1 );

    return ( selectable != -1 );
}


/**
 * remplit le model pour la configuration des états
 *
 *
 *
 * */
void gsb_etats_config_populate_tree_model ( GtkTreeStore *tree_model,
                        GtkWidget *notebook )
{
    GtkWidget *widget;
    GtkTreeIter iter;
    gint page = 0;

    /* append group page */
    gsb_etats_config_add_line_ ( tree_model, &iter, NULL, NULL, _("Data selection"), -1 );

    /* append page Dates */
    widget = gsb_etats_config_onglet_etat_dates ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Dates"), page );
    page++;

    /* append page Transferts */
    widget = gsb_etats_config_onglet_etat_virements ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Transfers"), page );
    page++;

    /* append page Accounts */
    widget = gsb_etats_config_onglet_etat_comptes ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Accounts"), page );
    page++;

    /* append page Payee */
    widget = gsb_etats_config_onglet_etat_tiers ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Payee"), page );
    page++;

    /* append page Categories */
    widget = gsb_etats_config_onglet_etat_categories ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Categories"), page );
    page++;

    /* append page Budgetary lines */
    widget = gsb_etats_config_onglet_etat_ib ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Budgetary lines"), page );
    page++;

    /* append page Texts */
    widget = gsb_etats_config_onglet_etat_texte ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Texts"), page );
    page++;

    /* append page Amounts */
    widget = gsb_etats_config_onglet_etat_montant ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Amounts"), page );
    page++;

    /* append page Payment methods */
    widget = gsb_etats_config_onglet_etat_mode_paiement ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Payment methods"), page );
    page++;

    /* append page Misc. */
    widget = gsb_etats_config_onglet_etat_divers ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Miscellaneous"), page );
    page++;

    /* remplissage de l'onglet d'organisation */
    gsb_etats_config_add_line_ ( tree_model, &iter, NULL, NULL, _("Data organization"), -1 );

    /* Data grouping */
    widget = gsb_etats_config_page_data_grouping ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Data grouping"), page );
    page++;

    /* Data separation */
    widget = gsb_etats_config_page_data_separation ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Data separation"), page );
    page++;

    /* remplissage de l'onglet d'affichage */
    gsb_etats_config_add_line_ ( tree_model, &iter, NULL, NULL, _("Data display"), -1 );

    /* append page Generalities */
    widget = gsb_etats_config_affichage_etat_generalites ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Generalities"), page );
    page++;

    /* append page Titles */
    widget = gsb_etats_config_affichage_etat_titres ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Titles"), page );
    page++;

    /* append page Transactions */
    widget = gsb_etats_config_affichage_etat_operations ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Transactions"), page );
    page++;

    /* append page Currencies */
    widget = gsb_etats_config_affichage_etat_devises ( );
    gsb_etats_config_add_line_ ( tree_model, &iter, notebook, widget, _("Currencies"), page );
}


/**
 * ajoute une ligne 
 *
 *
 *
 * */
void gsb_etats_config_add_line_ ( GtkTreeStore *tree_model,
                        GtkTreeIter *iter,
                        GtkWidget *notebook,
                        GtkWidget *child,
                        const gchar *title,
                        gint page )
{
    GtkTreeIter iter2;

    if ( page == -1 )
    {
        /* append page groupe */
        gtk_tree_store_append ( GTK_TREE_STORE ( tree_model ), iter, NULL );
        gtk_tree_store_set (GTK_TREE_STORE ( tree_model ), iter,
                        GSB_REPORT_TREE_TEXT_COLUMN, title,
                        GSB_REPORT_TREE_PAGE_COLUMN, -1,
                        GSB_REPORT_TREE_BOLD_COLUMN, 800,
                        -1 );
    }
    else
    {
        /* append page onglet*/ 
        gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        child,
                        gtk_label_new ( title ) );

        gtk_tree_store_append (GTK_TREE_STORE ( tree_model ), &iter2, iter );
        gtk_tree_store_set (GTK_TREE_STORE ( tree_model ), &iter2,
                        GSB_REPORT_TREE_TEXT_COLUMN, title,
                        GSB_REPORT_TREE_PAGE_COLUMN, page,
                        GSB_REPORT_TREE_BOLD_COLUMN, 400,
                        -1);
    }
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_etat_dates ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *sw;
    GtkWidget *hbox;
    GtkWidget *entree_date_init_etat;
    GtkWidget *entree_date_finale_etat;

    devel_debug (NULL);

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "onglet_etat_dates" ) );

    vbox = new_vbox_with_title_and_icon ( _("Date selection"), "scheduler.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    /* on traite la partie gauche de l'onglet dates */
    sw = gsb_etats_config_onglet_get_liste_dates ( );
    gtk_container_set_border_width ( GTK_CONTAINER (
                        gsb_etats_config_get_variable_by_name ( "vbox_utilisation_date", NULL ) ),
                        10 );

    hbox =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "hbox_date_init" ) );
    entree_date_init_etat = gsb_calendar_entry_new ( FALSE );
    gtk_widget_set_size_request ( entree_date_init_etat, 100, -1 );
    g_object_set_data ( G_OBJECT ( hbox ), "entree_date_init_etat", entree_date_init_etat );
    gtk_box_pack_end ( GTK_BOX ( hbox ), entree_date_init_etat, FALSE, FALSE, 0 );

    hbox =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "hbox_date_finale" ) );
    entree_date_finale_etat = gsb_calendar_entry_new ( FALSE );
    gtk_widget_set_size_request ( entree_date_finale_etat, 100, -1 );
    g_object_set_data ( G_OBJECT ( hbox ), "entree_date_finale_etat", entree_date_finale_etat );
    gtk_box_pack_end ( GTK_BOX ( hbox ), entree_date_finale_etat, FALSE, FALSE, 0 );

    gsb_etats_config_dates_interval_sensitive ( FALSE );

    /* on traite la partie droite de l'onglet dates */
    sw = gsb_etats_config_onglet_get_liste_exercices ( );
    gtk_container_set_border_width ( GTK_CONTAINER (
                        gsb_etats_config_get_variable_by_name ( "vbox_utilisation_exo", NULL ) ),
                        10 );

    gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name ( "vbox_utilisation_exo", NULL ), FALSE );
    gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name ( "sw_exer", NULL ), FALSE );

    /* on met la connection pour rendre sensitif la frame vbox_utilisation_date */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "radio_button_utilise_dates", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "vbox_utilisation_date", NULL ) );

    /* on met la connection pour rendre sensitif la frame vbox_utilisation_exo */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "radio_button_utilise_exo", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "vbox_utilisation_exo", NULL ) );

    /* on connecte les signaux nécessaires pour gérer la sélection de l'exercice */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "bouton_detaille_exo_etat", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "sw_exer", NULL ) );

    gtk_widget_show_all ( vbox_onglet );

    return vbox_onglet;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_get_liste_dates ( void )
{
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkListStore *list_store;
    GtkTreeSelection *selection;
    gchar **plages_dates;
    gint i;

    list_store = gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_INT );

    /* on remplit la liste des dates */
    plages_dates = etats_config_liste_plages_dates;

    i = 0;

    while ( plages_dates[i] )
    {
        GtkTreeIter iter;
        gchar *plage = gettext ( plages_dates[i] );

        gtk_list_store_append ( list_store, &iter );
        gtk_list_store_set ( list_store, &iter, 0, plage, 1, i, -1 );
    
        i++;
    }

    sw = gsb_etats_config_get_scrolled_window_with_tree_view ( "sw_dates", GTK_TREE_MODEL ( list_store ) );

    tree_view = gsb_etats_config_get_variable_by_name ( "sw_dates", "tree_view" );
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_selection_set_mode ( selection, GTK_SELECTION_SINGLE );
    g_signal_connect ( G_OBJECT ( selection ),
                        "changed",
                        G_CALLBACK ( gsb_etats_config_selection_dates_changed ),
                        sw );

    return sw;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_get_liste_exercices ( void )
{
    GtkWidget *sw;
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

    sw = gsb_etats_config_get_scrolled_window_with_tree_view ( "sw_exer", GTK_TREE_MODEL ( list_store ) );

    gtk_widget_show_all ( sw );

    return sw;
}


/**
 *
 *
 *
 *
 * */
gboolean gsb_etats_config_selection_dates_changed ( GtkTreeSelection *selection,
                        GtkWidget *widget )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint selected;

    if ( !gtk_tree_selection_get_selected ( selection, &model, &iter ) )
        return FALSE;

    gtk_tree_model_get ( model, &iter, 1, &selected, -1 );
    gsb_etats_config_dates_interval_sensitive ( selected );

    return FALSE;
}


/**
 *
 *
 *
 */
void gsb_etats_config_dates_interval_sensitive ( gboolean show )
{
    if ( show > 1 )
        show = 0;

        gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "bouton_en_date_valeur", NULL ), show );
        gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "hbox_date_init", "entree_date_init_etat" ), show );
        gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "hbox_date_finale", "entree_date_finale_etat" ), show );
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_etat_virements ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkWidget *button;

    devel_debug (NULL);

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "onglet_etat_virements" ) );

    vbox = new_vbox_with_title_and_icon ( _("Transfers"), "transfer.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "hbox_liste_comptes_virements", NULL ), FALSE );

    /* on crée la liste des comptes */
    sw = gsb_etats_config_onglet_get_liste_comptes ( "sw_virements" );
    tree_view = gsb_etats_config_get_variable_by_name ( "sw_virements", "tree_view" );

    /* on connecte les signaux nécessaires pour gérer l'affichage de la liste des comptes */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "bouton_inclusion_virements_perso", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "hbox_liste_comptes_virements", NULL ) );

    /* on met la connection pour (dé)sélectionner tous les comptes */
    button = gsb_etats_config_togglebutton_set_button_select ( "sw_virements", tree_view );

    /* on met la connection pour sélectionner une partie des comptes */
    gsb_etats_config_onglet_set_buttons_select_comptes ( "sw_virements", tree_view );

    gtk_widget_show_all ( vbox_onglet );

    return vbox_onglet;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_get_liste_comptes ( gchar *sw_name )
{
    GtkWidget *sw;
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

    sw = gsb_etats_config_get_scrolled_window_with_tree_view ( sw_name, GTK_TREE_MODEL ( list_store ) );

    gtk_widget_show_all ( sw );

    return sw;
}


/**
 *
 *
 *
 */
void gsb_etats_config_onglet_set_buttons_select_comptes ( gchar *sw_name,
                        GtkWidget *tree_view )
{
    GtkWidget *button;
    gchar *tmp_str;

    tmp_str = g_strconcat ( "button_bank", sw_name + 2, NULL );
    button = gsb_etats_config_get_variable_by_name ( tmp_str, NULL );
    g_object_set_data ( G_OBJECT ( button ), "type_compte", GINT_TO_POINTER ( GSB_TYPE_BANK ) );
    g_signal_connect ( G_OBJECT  ( button ),
                        "button-press-event",
                        G_CALLBACK ( gsb_etats_config_onglet_select_partie_liste_comptes ),
                        tree_view );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "button_cash", sw_name + 2, NULL );
    button = gsb_etats_config_get_variable_by_name ( tmp_str, NULL );
    g_object_set_data ( G_OBJECT ( button ), "type_compte", GINT_TO_POINTER ( GSB_TYPE_CASH ) );
    g_signal_connect ( G_OBJECT  ( button ),
                        "button-press-event",
                        G_CALLBACK ( gsb_etats_config_onglet_select_partie_liste_comptes ),
                        tree_view );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "button_liabilities", sw_name + 2, NULL );
    button = gsb_etats_config_get_variable_by_name ( tmp_str, NULL );
    g_object_set_data ( G_OBJECT ( button ), "type_compte", GINT_TO_POINTER ( GSB_TYPE_LIABILITIES ) );
    g_signal_connect ( G_OBJECT  ( button ),
                        "button-press-event",
                        G_CALLBACK ( gsb_etats_config_onglet_select_partie_liste_comptes ),
                        tree_view );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "button_assets", sw_name + 2, NULL );
    button = gsb_etats_config_get_variable_by_name ( tmp_str, NULL );
    g_object_set_data ( G_OBJECT ( button ), "type_compte", GINT_TO_POINTER ( GSB_TYPE_ASSET ) );
    g_signal_connect ( G_OBJECT  ( button ),
                        "button-press-event",
                        G_CALLBACK ( gsb_etats_config_onglet_select_partie_liste_comptes ),
                        tree_view );
    g_free ( tmp_str );
}


/**
 *
 *
 *
 */
gboolean gsb_etats_config_onglet_select_partie_liste_comptes ( GtkWidget *button,
                        GdkEventButton *event,
                        GtkWidget *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gint type_compte;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    if ( !gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
        return FALSE;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_selection_unselect_all ( selection );
    type_compte = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( button ), "type_compte" ) );

    do
    {
        gint account_number;

        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter, 1, &account_number, -1 );
        if ( gsb_data_account_get_kind ( account_number ) == type_compte )
            gtk_tree_selection_select_iter ( selection, &iter );
    }
    while (gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );

    return FALSE;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_etat_comptes ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkWidget *button;

    devel_debug (NULL);

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "onglet_etat_comptes" ) );

    vbox = new_vbox_with_title_and_icon ( _("Account selection"), "ac_bank.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "vbox_generale_comptes_etat", NULL ), FALSE );

    /* on crée la liste des comptes */
    sw = gsb_etats_config_onglet_get_liste_comptes ( "sw_comptes" );
    tree_view = gsb_etats_config_get_variable_by_name ( "sw_comptes", "tree_view" );

    /* on met la connection pour rendre sensitif la vbox_generale_comptes_etat */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "bouton_detaille_comptes_etat", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "vbox_generale_comptes_etat", NULL ) );

    /* on met la connection pour (dé)sélectionner tous les comptes */
    button = gsb_etats_config_togglebutton_set_button_select ( "sw_comptes", tree_view );

    /* on met la connection pour sélectionner une partie des comptes */
    gsb_etats_config_onglet_set_buttons_select_comptes ( "sw_comptes", tree_view );

    gtk_widget_show_all ( vbox_onglet );

    return vbox_onglet;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_etat_tiers ( void )
{
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkWidget *button;

    devel_debug (NULL);

    vbox_onglet =  GTK_WIDGET ( gtk_builder_get_object ( etat_config_builder, "onglet_etat_tiers" ) );

    vbox = new_vbox_with_title_and_icon ( _("Payees"), "payees.png" );

    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ), vbox, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( vbox_onglet ), vbox, 0 );

    gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "vbox_detaille_tiers_etat", NULL ), FALSE );

    /* on crée la liste des tiers */
    sw = gsb_etats_config_onglet_get_liste_tiers ( "sw_tiers" );
    tree_view = gsb_etats_config_get_variable_by_name ( "sw_tiers", "tree_view" );

    /* on met la connection pour rendre sensitif la vbox_generale_comptes_etat */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "bouton_detaille_tiers_etat", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "vbox_detaille_tiers_etat", NULL ) );

    /* on met la connection pour (dé)sélectionner tous les tiers */
    button = gsb_etats_config_togglebutton_set_button_select ( "sw_tiers", tree_view );

    gtk_widget_show_all ( vbox_onglet );

    return vbox_onglet;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_etats_config_onglet_get_liste_tiers ( gchar *sw_name )
{
    GtkWidget *sw;
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

    sw = gsb_etats_config_get_scrolled_window_with_tree_view ( sw_name, GTK_TREE_MODEL ( list_store ) );

    gtk_widget_show_all ( sw );

    return sw;
}


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

    gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "vbox_detaille_categ_etat", NULL ), FALSE );

    /* on crée la liste des catégories */
    sw = gsb_etats_config_get_liste_categ_budget ( "sw_categ" );
    tree_view = gsb_etats_config_get_variable_by_name ( "sw_categ", "tree_view" );

    /* on remplit la liste des catégories */
    gsb_etats_config_fill_liste_categ_budget ( TRUE );

    /* on met la connection pour rendre sensitif la vbox_detaille_categ_etat */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "button_detail_categ_etat", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "vbox_detaille_categ_etat", NULL ) );

    /* on met la connection pour déplier replier les catégories */
    button = gsb_etats_config_togglebutton_set_button_expand ( "sw_categ", tree_view );

    /* on met la connection pour (dé)sélectionner toutes les catégories */
    button = gsb_etats_config_togglebutton_set_button_select ( "sw_categ", tree_view );

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
 * select or unselect all the categories
 *
 * \param button
 * \param select_ptr    TRUE or FALSE to select/unselect all
 *
 * \return FALSE
 * */
void gsb_etats_config_togglebutton_select_all ( GtkToggleButton *togglebutton,
                        GtkWidget *tree_view )
{
    GtkTreeModel *model;
    GtkWidget *hbox_select_all;
    GtkWidget *hbox_unselect_all;
    gboolean toggle_value;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    toggle_value = gtk_toggle_button_get_active ( togglebutton );
    hbox_select_all = g_object_get_data ( G_OBJECT ( togglebutton ), "hbox_select_all" );
    hbox_unselect_all = g_object_get_data ( G_OBJECT ( togglebutton ), "hbox_unselect_all" );

    if ( toggle_value )
    {
        gtk_widget_hide ( hbox_select_all );
        gtk_widget_show ( hbox_unselect_all );
        if ( GTK_IS_TREE_STORE ( model ) )
            gsb_etats_config_togglebutton_check_uncheck_all ( model, toggle_value );
        else
            gtk_tree_selection_select_all ( gtk_tree_view_get_selection GTK_TREE_VIEW ( tree_view ) );
    }
    else
    {
        gtk_widget_show ( hbox_select_all );
        gtk_widget_hide ( hbox_unselect_all );
        if ( GTK_IS_TREE_STORE ( model ) )
            gsb_etats_config_togglebutton_check_uncheck_all ( model, toggle_value );
        else
            gtk_tree_selection_unselect_all ( gtk_tree_view_get_selection GTK_TREE_VIEW ( tree_view ) );
    }
}


/**
 * check or uncheck all the budgets or categories
 *
 * \param model         the model to fill (is model_categ or model_budget
 * \param select_ptr    TRUE or FALSE to select/unselect all
 *
 * \return FALSE
 * */
void gsb_etats_config_togglebutton_check_uncheck_all ( GtkTreeModel *model,
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
    button = gsb_etats_config_get_variable_by_name ( tmp_str, NULL );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "hbox_toggle_expand", sw_name + 2, NULL );
    g_object_set_data ( G_OBJECT ( button ), "hbox_expand",
                        gsb_etats_config_get_variable_by_name ( tmp_str, NULL ) );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "hbox_toggle_collapse", sw_name + 2, NULL );
    g_object_set_data ( G_OBJECT ( button ), "hbox_collapse",
                        gsb_etats_config_get_variable_by_name ( tmp_str, NULL ) );
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
GtkWidget *gsb_etats_config_togglebutton_set_button_select ( gchar *sw_name,
                        GtkWidget *tree_view )
{
    GtkWidget *button;
    gchar *tmp_str;

    tmp_str = g_strconcat ( "togglebutton_select_all", sw_name + 2, NULL );
    button = gsb_etats_config_get_variable_by_name ( tmp_str, NULL );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "hbox_toggle_select_all", sw_name + 2, NULL );
    g_object_set_data ( G_OBJECT ( button ), "hbox_select_all",
                        gsb_etats_config_get_variable_by_name ( tmp_str, NULL ) );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "hbox_toggle_unselect_all", sw_name + 2, NULL );
    g_object_set_data ( G_OBJECT ( button ), "hbox_unselect_all",
                        gsb_etats_config_get_variable_by_name ( tmp_str, NULL ) );
    g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( gsb_etats_config_togglebutton_select_all ),
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
    button = gsb_etats_config_get_variable_by_name ( tmp_str, NULL );
    g_object_set_data ( G_OBJECT ( button ), "is_categ", GINT_TO_POINTER ( is_categ ) );
    g_object_set_data ( G_OBJECT ( button ), "type_div", GINT_TO_POINTER ( FALSE ) );
    g_signal_connect ( G_OBJECT  ( button ),
                        "button-press-event",
                        G_CALLBACK ( gsb_etats_config_onglet_select_partie_liste_categ_budget ),
                        tree_view );
    g_free ( tmp_str );

    tmp_str = g_strconcat ( "button_outgoing", sw_name + 2, NULL );
    button = gsb_etats_config_get_variable_by_name ( tmp_str, NULL );
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

    gsb_etats_config_togglebutton_check_uncheck_all ( model, FALSE );

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

        tree_view = gsb_etats_config_get_variable_by_name ( "sw_categ", "tree_view" );
        model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

        gtk_tree_store_clear ( GTK_TREE_STORE ( model ) );

        without_name = _("No category");
        without_sub_name = _("No subcategory");
    }
    else
    {
        list_tmp = gsb_data_budget_get_budgets_list ( );
        tree_view = gsb_etats_config_get_variable_by_name ( "sw_budget", "tree_view" );
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

    gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "vbox_detaille_budget_etat", NULL ), FALSE );

    /* on crée la liste des IB et on récupère le tree_view*/
    sw = gsb_etats_config_get_liste_categ_budget ( "sw_budget" );
    tree_view = gsb_etats_config_get_variable_by_name ( "sw_budget", "tree_view" );

    /* on remplit la liste des IB */
    gsb_etats_config_fill_liste_categ_budget ( FALSE );

    /* on met la connection pour rendre sensitif la hbox_detaille_budget_etat */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "bouton_detaille_budget_etat", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "vbox_detaille_budget_etat", NULL ) );

    /* on met la connection pour déplier replier les IB */
    button = gsb_etats_config_togglebutton_set_button_expand ( "sw_budget", tree_view );

    /* on met la connection pour sélectionner toutes les IB */
    button = gsb_etats_config_togglebutton_set_button_select ( "sw_budget", tree_view );

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

    gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "vbox_generale_texte_etat", NULL ), FALSE );

    /* on attache la vbox pour les lignes de recherche à sw_texte */
    sw = gsb_etats_config_get_variable_by_name ( "sw_texte", NULL );
    lignes = gsb_etats_config_get_variable_by_name ( "liste_texte_etat", NULL );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( sw ), lignes );

    /* on crée la première ligne de la recherche */
    gsb_etats_config_onglet_etat_texte_new_line ( lignes );

    /* on met la connection pour rendre sensitif la vbox_generale_textes_etat */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "bouton_utilise_texte", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "vbox_generale_texte_etat", NULL ) );

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

    gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "vbox_generale_montant_etat", NULL ), FALSE );

    /* on attache la vbox pour les lignes de recherche à sw_montant */
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (
                        gsb_etats_config_get_variable_by_name ( "sw_montant", NULL ) ),
                        gsb_etats_config_get_variable_by_name ( "liste_montant_etat", NULL ) );

    /* on remplit le combobox de choix du type de texte dans lequel chercher */
/*     gsb_etats_config_onglet_etat_combo_set_model (
 *                         gsb_etats_config_get_variable_by_name ( "combobox_comparateur_1", NULL ),
 *                         champs_comparateur_montant );
 */


    /* on met la connection pour rendre sensitif la vbox_generale_textes_etat */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "bouton_utilise_montant", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "vbox_generale_montant_etat", NULL ) );

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

    gtk_widget_set_sensitive ( gsb_etats_config_get_variable_by_name (
                        "vbox_mode_paiement_etat", NULL ), FALSE );

    /* on crée la liste des catégories */
    sw = gsb_etats_config_get_liste_mode_paiement ( "sw_mode_paiement" );

    /* on met la connection pour rendre sensitif la vbox_generale_comptes_etat */
    g_signal_connect ( G_OBJECT ( gsb_etats_config_get_variable_by_name (
                        "bouton_detaille_mode_paiement_etat", NULL ) ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "vbox_mode_paiement_etat", NULL ) );

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

    vbox = gsb_etats_config_get_variable_by_name ( "vbox_select_transactions_buttons", NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), vbox, TRUE, TRUE, 5 );

    button = gsb_etats_config_get_variable_by_name ( "radiobutton_marked", NULL );

    /* on met la connection pour rendre sensitif la vbox_detaille_categ_etat */
    g_signal_connect ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( sens_desensitive_pointeur ),
                        gsb_etats_config_get_variable_by_name ( "vbox_marked_buttons", NULL ) );

    paddingbox = new_paddingbox_with_title ( vbox_onglet, FALSE, _("Split of transactions detail") );

    button = gsb_etats_config_get_variable_by_name ( "bouton_pas_detailler_ventilation", NULL );
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
