/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2010-2011 Pierre Biava (grisbi@pierre.biava.name)        */
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

#include "include.h"

/*START_INCLUDE*/
#include "bet_finance_ui.h"
#include "bet_data_finance.h"
#include "dialog.h"
#include "export_csv.h"
#include "fenetre_principale.h"
#include "gsb_automem.h"
#include "gsb_combo_box.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_file.h"
#include "gsb_form_widget.h"
#include "gsb_real.h"
#include "mouse.h"
#include "navigation.h"
#include "print_tree_view_list.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_file_selection.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void bet_finance_calcule_show_months_tab ( GtkTreeModel *model,
                        struct_echeance *s_echeance,
                        gdouble taux_frais );
static void bet_finance_calculer_clicked ( GtkButton *button, GtkWidget *widget );
static gboolean bet_finance_capital_entry_changed ( GtkWidget *entry, GtkWidget *page  );
static gboolean bet_finance_capital_entry_key_press_event ( GtkWidget *widget,
                        GdkEventKey *ev,
                        GtkWidget *page );
static GtkWidget *bet_finance_create_amortization_page ( void );
static GtkWidget *bet_finance_create_amortization_toolbar ( GtkWidget *parent, GtkWidget *tree_view );
static GtkWidget *bet_finance_create_amortization_tree_view ( GtkWidget *container, gint origin );
static GtkWidget *bet_finance_create_data_tree_view ( GtkWidget *container );
static GtkWidget *bet_finance_create_duration_widget ( GtkWidget *parent );
static GtkWidget *bet_finance_create_saisie_widget ( GtkWidget *parent );
static GtkWidget *bet_finance_create_simulator_page ( void );
static GtkWidget *bet_finance_create_simulator_toolbar ( GtkWidget *parent,
                        GtkWidget *tree_view,
                        gboolean simulator,
                        gboolean amortization );
static void bet_finance_currency_changed ( GtkComboBox *combo_box, GtkWidget *page );
static gboolean bet_finance_data_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        GtkWidget *page );
static void bet_finance_data_list_context_menu ( GtkWidget *tree_view, gint page_num );
static gboolean bet_finance_duration_button_changed ( GtkWidget *combobox, GtkWidget *widget );
static void bet_finance_fill_amortization_array ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static void bet_finance_fill_amortization_ligne ( GtkTreeModel *model,
                        struct_amortissement *s_amortissement );
static void bet_finance_fill_data_ligne ( GtkTreeModel *model,
                        struct_echeance *s_echeance,
                        const gchar *unit );
static gboolean bet_finance_list_set_background_color ( GtkWidget *tree_view, gint color_column );
static void bet_finance_spin_button_fees_changed ( GtkSpinButton *spinbutton, GtkWidget *page );
static void bet_finance_spin_button_taux_changed ( GtkSpinButton *spinbutton, GtkWidget *page );
static void bet_finance_switch_amortization_initial_date ( GtkWidget *button, GtkWidget *tree_view );
static void bet_finance_ui_export_tab ( GtkWidget *menu_item, GtkTreeView *tree_view );
static void bet_finance_ui_struct_amortization_free ( struct_amortissement *s_amortissement );
static void bet_finance_type_taux_changed ( GtkWidget *togglebutton, GdkEventButton *event, GtkWidget *widget );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *account_page;
extern GdkColor calendar_entry_color;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_selection;
extern GtkWidget *window;
/*END_EXTERN*/

/* notebook pour la simulation de crédits */
static GtkWidget *finance_notebook;

enum bet_finance_data_columns
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


enum bet_finance_amortization_columns
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


/**
 * Create the finance page
 *
 *
 *
 * */
GtkWidget *bet_finance_create_page ( void )
{
    GtkWidget *page;

    devel_debug (NULL);

    /* create a notebook for simulator and array of Amortization */
    finance_notebook = gtk_notebook_new ( );
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( finance_notebook ), FALSE );
    gtk_widget_show ( finance_notebook );

    /* create the simulator page */
    page = bet_finance_create_simulator_page ( );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( finance_notebook ), page, NULL );

    /* create the array page */
    page = bet_finance_create_amortization_page ( );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( finance_notebook ), page, NULL );
    
    return finance_notebook;
}


/**
 * Switch to the simulator page
 *
 *
 *
 * */
void bet_finance_switch_simulator_page ( void )
{
    GtkWidget *page;

    page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( finance_notebook ), 0 );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( finance_notebook ), 0 );
    bet_finance_calculer_clicked ( NULL, page );
}


/**
 * Create the simulator page
 *
 *
 *
 * */
GtkWidget *bet_finance_create_simulator_page ( void )
{
    GtkWidget *page;
    GtkWidget *widget;
    GtkWidget *hbox;
    GtkWidget *align;
    GtkWidget *label_title;
    GtkWidget *label;
    GtkWidget *spin_button = NULL;
    GtkWidget *tree_view;
    GtkWidget *toolbar;
    gchar *str_capital;

    devel_debug (NULL);

    page = gtk_vbox_new ( FALSE, 5 );

    /* titre de la page */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5);
 
    label_title = gtk_label_new ( _("Credits simulator") );
    gtk_container_add ( GTK_CONTAINER ( align ), label_title );

    /* Choix des données sources */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5);

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_container_add ( GTK_CONTAINER ( align ), hbox );

    /* capital */
    label = gtk_label_new ( _("Loan capital: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    str_capital = gsb_real_get_string_with_currency ( gsb_real_double_to_real (
                        etat.bet_capital ),
                        etat.bet_currency,
                        FALSE );

    widget = gtk_entry_new ( );
    gtk_entry_set_text ( GTK_ENTRY ( widget ), str_capital );
    g_object_set_data ( G_OBJECT ( page ), "capital", widget );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5 );
    g_signal_connect ( G_OBJECT ( widget ),
                        "changed",
                        G_CALLBACK ( bet_finance_capital_entry_changed ),
                        page );
    g_signal_connect ( G_OBJECT ( widget ),
                        "key-press-event",
                        G_CALLBACK ( bet_finance_capital_entry_key_press_event ),
                        page );

    g_free ( str_capital );

    /* Set the devises */
    widget = gsb_currency_make_combobox ( FALSE );
    gsb_currency_set_combobox_history ( widget, etat.bet_currency );
    g_object_set_data ( G_OBJECT ( page ), "devise", widget );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 0 );
    g_signal_connect ( G_OBJECT (  widget ),
                        "changed",
                        G_CALLBACK ( bet_finance_currency_changed ),
                        page );

    /* taux */
    label = gtk_label_new ( _("Annual interest: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    spin_button = gtk_spin_button_new_with_range ( 0.0, 100,
                        bet_data_finance_get_bet_taux_step ( BET_TAUX_DIGITS ) );
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ), etat.bet_taux_annuel );
    g_object_set_data ( G_OBJECT ( page ), "taux", spin_button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button, FALSE, FALSE, 0 );
    g_signal_connect ( spin_button,
                        "value-changed",
                        G_CALLBACK ( bet_finance_spin_button_taux_changed ),
                        page );

    label = gtk_label_new ( _("%") );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* Duration */
    label = gtk_label_new ( _("Duration: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    widget = bet_finance_create_duration_widget ( page );
    g_object_set_data ( G_OBJECT ( page ), "duree", widget );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 0 );

    /* création du widget saisie détaillée */
    widget = bet_finance_create_saisie_widget ( page );
    gtk_box_pack_start ( GTK_BOX ( page ), widget, FALSE, FALSE, 5);

    /* création de la liste des données */
    tree_view = bet_finance_create_data_tree_view ( page );
    g_object_set_data ( G_OBJECT ( tree_view ), "origin",
                        GINT_TO_POINTER ( SPP_ORIGIN_SIMULATOR ) );
    g_object_set_data ( G_OBJECT ( page ), "tree_view", tree_view );
    g_object_set_data ( G_OBJECT ( tree_view ), "label_title", label_title );

    /* set the color of selected row */
    gtk_widget_modify_base ( tree_view, GTK_STATE_SELECTED, &couleur_selection );
    gtk_widget_modify_base ( tree_view, GTK_STATE_ACTIVE, &couleur_selection );

    /* on y ajoute la barre d'outils */
    toolbar = bet_finance_create_simulator_toolbar ( page, tree_view, TRUE, FALSE );
    gtk_box_pack_start ( GTK_BOX ( page ), toolbar, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( page ), toolbar, 0 );

    gtk_widget_show_all ( page );

    return page;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_finance_create_duration_widget ( GtkWidget *parent )
{
    GtkWidget *combobox;
    gchar *text_duration [] = {
    _("Between 3 and 18 months"),
    _("Between 1 and 15 years"),
    _("Between 15 and 30 years"),
    NULL};

    combobox = gsb_combo_box_new_with_index ( text_duration,
                        G_CALLBACK ( bet_finance_duration_button_changed ),
                        parent );
    
    g_signal_handlers_block_by_func ( G_OBJECT ( combobox ),
                        G_CALLBACK ( bet_finance_duration_button_changed ),
                        parent );
    gsb_combo_box_set_index ( combobox, etat.bet_index_duree );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( combobox ),
                        G_CALLBACK ( bet_finance_duration_button_changed ),
                        parent );

    return combobox;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_finance_create_saisie_widget ( GtkWidget *parent )
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *align;
    GtkWidget *label;
    GtkWidget *spin_button = NULL;
    GtkWidget *button_1, *button_2;
    gchar *tmp_str;

    vbox = gtk_vbox_new ( FALSE, 5 );

    /* Frais */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( vbox ), align, FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_container_add ( GTK_CONTAINER ( align ), hbox );

    label = gtk_label_new ( _("Fees: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    spin_button = gtk_spin_button_new_with_range ( 0.0, 100,
                        bet_data_finance_get_bet_taux_step ( BET_TAUX_DIGITS ) );
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ), etat.bet_frais );
    g_object_set_data ( G_OBJECT ( parent ), "frais", spin_button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button, FALSE, FALSE, 0 );
    g_signal_connect ( spin_button,
                        "value-changed",
                        G_CALLBACK ( bet_finance_spin_button_fees_changed ),
                        parent );
    
    tmp_str = g_strconcat (_("%"), _(" of borrowed capital"), NULL );
    label = gtk_label_new ( tmp_str );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
    g_free ( tmp_str );

    /* Type de taux */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( vbox ), align, FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_container_add ( GTK_CONTAINER ( align ), hbox );

    label = gtk_label_new ( _("Rate Type: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
    button_1 = gtk_radio_button_new_with_label ( NULL, _("CAGR") );

    button_2 = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( button_1 ),
                        _("Proportional rate") );
    g_object_set_data ( G_OBJECT ( parent ), "type_taux", button_2 );

    if ( etat.bet_type_taux )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_2 ), TRUE );
    else
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_1 ), TRUE );

    gtk_box_pack_start ( GTK_BOX ( hbox ), button_1, FALSE, FALSE, 5) ;
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_2, FALSE, FALSE, 5) ;
    g_signal_connect ( button_1,
                        "button-release-event",
                        G_CALLBACK ( bet_finance_type_taux_changed ),
                        parent );
    g_signal_connect ( button_2,
                        "button-release-event",
                        G_CALLBACK ( bet_finance_type_taux_changed ),
                        parent );

    gtk_widget_show_all ( align );

    return vbox;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_finance_create_data_tree_view ( GtkWidget *container )
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkTreeStore *tree_model;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    gchar *title;

    tree_view = gtk_tree_view_new ( );
    gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW ( tree_view ), FALSE );

    /* Create the tree store */
    tree_model = gtk_tree_store_new ( BET_FINANCE_NBRE_COLUMNS,
                        G_TYPE_STRING,      /* BET_FINANCE_DURATION_COLUMN        */
                        G_TYPE_INT,         /* BET_FINANCE_NBRE_ECHEANCE_COLUMN   */
                        G_TYPE_INT,         /* BET_FINANCE_DEVISE_COLUMN          */
                        G_TYPE_STRING,      /* BET_FINANCE_CAPITAL_COLUMN         */
                        G_TYPE_DOUBLE,      /* BET_FINANCE_CAPITAL_DOUBLE         */
                        G_TYPE_STRING,      /* BET_FINANCE_TAUX_COLUMN            */
                        G_TYPE_DOUBLE,      /* BET_FINANCE_TAUX_DOUBLE            */
                        G_TYPE_STRING,      /* BET_FINANCE_HORS_FRAIS_COLUMN      */
                        G_TYPE_DOUBLE,      /* BET_FINANCE_HORS_FRAIS_DOUBLE      */
                        G_TYPE_STRING,      /* BET_FINANCE_FRAIS_COLUMN           */
                        G_TYPE_DOUBLE,      /* BET_FINANCE_FRAIS_DOUBLE           */
                        G_TYPE_STRING,      /* BET_FINANCE_ECHEANCE_COLUMN        */
                        G_TYPE_DOUBLE,      /* BET_FINANCE_ECHEANCE_DOUBLE        */
                        G_TYPE_STRING,      /* BET_FINANCE_COUT_COLUMN            */
                        G_TYPE_DOUBLE,      /* BET_FINANCE_COUT_DOUBLE            */
                        GDK_TYPE_COLOR );   /* BET_FINANCE_BACKGROUND_COLOR       */
    gtk_tree_view_set_model ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_MODEL ( tree_model ) );
    g_object_unref ( G_OBJECT ( tree_model ) );

    /* create columns */

    /* Duration */
    title = g_strdup ( _("Duration") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_FINANCE_DURATION_COLUMN,
                        "cell-background-gdk", BET_FINANCE_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_FINANCE_DURATION_COLUMN ) );
    g_free ( title );

    /* Number of periods */
    title = g_strdup ( _("Number\nof periods") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_FINANCE_NBRE_ECHEANCE_COLUMN,
                        "cell-background-gdk", BET_FINANCE_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_FINANCE_NBRE_ECHEANCE_COLUMN ) );
    g_free ( title );

    /* Loan capital */
    title = g_strdup ( _("Loan\ncapital") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_FINANCE_CAPITAL_COLUMN,
                        "cell-background-gdk", BET_FINANCE_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_FINANCE_CAPITAL_COLUMN ) );
    g_free ( title );

    /* Annuel rate interest */
    title = g_strdup ( _("Annuel\nrate interest") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_FINANCE_TAUX_COLUMN,
                        "cell-background-gdk", BET_FINANCE_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_FINANCE_TAUX_COLUMN ) );
    g_free ( title );

    /* Amount without fees */
    title = g_strdup ( _("Amount\nwithout fees") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_FINANCE_HORS_FRAIS_COLUMN,
                        "cell-background-gdk", BET_FINANCE_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_FINANCE_HORS_FRAIS_COLUMN ) );
    g_free ( title );

    /* Fees */
    title = g_strdup ( _("Fees") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_FINANCE_FRAIS_COLUMN,
                        "cell-background-gdk", BET_FINANCE_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_FINANCE_FRAIS_COLUMN ) );
    g_free ( title );

    /* Monthly paid */
    title = g_strdup ( _("Monthly paid") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_FINANCE_ECHEANCE_COLUMN,
                        "cell-background-gdk", BET_FINANCE_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_FINANCE_ECHEANCE_COLUMN ) );
    g_free ( title );

    /* Total cost */
    title = g_strdup ( _("Total cost") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_FINANCE_COUT_COLUMN,
                        "cell-background-gdk", BET_FINANCE_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( GTK_TREE_VIEW_COLUMN ( column )->button ),
                        _("Approximate cost") );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_FINANCE_COUT_COLUMN ) );
    g_free ( title );

    g_signal_connect ( G_OBJECT ( tree_view ),
                        "button-press-event",
                        G_CALLBACK ( bet_finance_data_list_button_press ),
                        container );

    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_widget_set_size_request ( scrolled_window, -1, 250 );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), tree_view );
    gtk_box_pack_start ( GTK_BOX ( container ), scrolled_window, TRUE, TRUE, 15 );

    gtk_widget_show_all ( scrolled_window );

    return tree_view;
}


/**
 *
 *
 *
 *
 * */
void bet_finance_calculer_clicked ( GtkButton *button, GtkWidget *widget )
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
    struct_echeance *s_echeance;

    tree_view = g_object_get_data ( G_OBJECT ( widget ), "tree_view" );
    if ( !GTK_IS_TREE_VIEW ( tree_view ) )
        return;

    s_echeance = g_malloc0 ( sizeof ( struct_echeance ) );

    /* devise */
    combobox = g_object_get_data ( G_OBJECT ( widget ), "devise" );
    if ( combobox )
        s_echeance -> devise = gsb_currency_get_currency_from_combobox ( combobox );

    /* capital */
    s_echeance -> capital = bet_finance_get_number_from_string ( widget, "capital" );

    if ( s_echeance -> capital == 0 )
    {
        gchar *tmp_str;

        tmp_str = g_strdup ( _("You must enter at least one value for the capital") );
        dialogue_error ( tmp_str );
        g_free ( tmp_str );
        return;
    }
    else if ( s_echeance -> capital == G_MININT64 )
        return;

    /* rate */
    s_echeance -> taux = bet_finance_get_number_from_string ( widget, "taux" );

    /* Duration */
    combobox = g_object_get_data ( G_OBJECT ( widget ), "duree" );
    if ( combobox )
        index = gsb_combo_box_get_index ( combobox );

    switch ( index )
    {
        case 0:
            duree_min = 3;
            duree_max = 18;
            break;
        case 1:
            duree_min = 1;
            duree_max = 15;
            break;
        case 2:
            duree_min = 15;
            duree_max = 30;
            break;
        default :
            duree_min = 1;
            duree_max = 15;
            break;
    }

    /* frais */
    taux_frais = bet_finance_get_number_from_string ( widget, "frais" );

    /* type de taux */
    bouton = g_object_get_data ( G_OBJECT ( widget ), "type_taux" );
    type_taux = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton ) );

    /* taux periodique */
    s_echeance -> taux_periodique = bet_data_finance_get_taux_periodique (
                        s_echeance -> taux,
                        type_taux );

    /* réinitialisation du model */
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_store_clear ( GTK_TREE_STORE ( model ) );

    if ( index == 0 )
        bet_finance_calcule_show_months_tab ( model, s_echeance, taux_frais );
    else
    {
        for ( index = duree_min; index <= duree_max; index++ )
        {
            s_echeance -> duree = index;
            s_echeance -> nbre_echeances = index * 12;
            s_echeance -> frais = bet_data_finance_get_frais_par_echeance ( s_echeance -> capital,
                            taux_frais,
                            s_echeance -> nbre_echeances );

            s_echeance -> echeance = bet_data_finance_get_echeance ( s_echeance -> capital,
                            s_echeance -> taux_periodique,
                            s_echeance -> nbre_echeances );

            s_echeance -> total_echeance = s_echeance -> echeance + s_echeance -> frais;
            s_echeance -> total_cost = bet_data_finance_get_total_cost ( s_echeance );

            if ( s_echeance -> duree == 1 )
                bet_finance_fill_data_ligne ( model, s_echeance, _("year") );
            else
                bet_finance_fill_data_ligne ( model, s_echeance, _("years") );

            s_echeance -> duree = 0;
            s_echeance -> nbre_echeances = 0;
            s_echeance -> echeance = 0;
        }
    }

    bet_finance_list_set_background_color ( tree_view, BET_FINANCE_BACKGROUND_COLOR );
    path = gtk_tree_path_new_first ( );
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW ( tree_view ), path, NULL, TRUE, 0.0, 0.0 );
    gtk_tree_selection_select_path ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ), path );
    
    bouton = g_object_get_data ( G_OBJECT ( widget ), "amortization_button" );
    gtk_widget_set_sensitive ( bouton, TRUE );
    bouton = g_object_get_data ( G_OBJECT ( widget ), "print_button" );
    gtk_widget_set_sensitive ( bouton, TRUE );
    bouton = g_object_get_data ( G_OBJECT ( widget ), "export_button" );
    gtk_widget_set_sensitive ( bouton, TRUE );

    gtk_tree_path_free ( path );
    g_free ( s_echeance );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}


/**
 *
 *
 *
 *
 * */
gdouble bet_finance_get_number_from_string ( GtkWidget *parent, const gchar *name )
{
    GtkWidget *widget;
    gdouble number = 0;

    widget = g_object_get_data ( G_OBJECT ( parent ), name );

    if ( GTK_IS_SPIN_BUTTON  ( widget ) )
    {
        number = gtk_spin_button_get_value ( GTK_SPIN_BUTTON ( widget ) );
    }
    else if ( GTK_IS_ENTRY ( widget ) )
    {
        const gchar *entry;
        gchar *tmp_str;

        entry = gtk_entry_get_text ( GTK_ENTRY ( widget ) );

        if ( entry && strlen ( entry ) > 0 )
        {
            number = utils_str_strtod ( entry, NULL );

            tmp_str = gsb_real_get_string_with_currency (
                                gsb_real_double_to_real ( number ),
                                etat.bet_currency,
                                FALSE );
            gtk_entry_set_text ( GTK_ENTRY ( widget ), tmp_str );
            g_free ( tmp_str );
        }
    }

    return number;
}


/**
 *
 *
 *
 *
 * */
void bet_finance_fill_data_ligne ( GtkTreeModel *model,
                        struct_echeance *s_echeance,
                        const gchar *unit )
{
    GtkTreeIter iter;
    gchar *str_duree;
    gchar *str_capital;
    gchar *str_taux;
    gchar *str_frais;
    gchar *str_echeance;
    gchar *str_totale;
    gchar *str_total_cost;
    gchar buffer[256];
    gint nbre_char;

    str_duree = g_strconcat ( utils_str_itoa ( s_echeance -> duree ), " ", unit, " ", NULL );

    str_capital = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_echeance -> capital ),
                        s_echeance -> devise, TRUE );

    nbre_char = g_sprintf ( buffer, "%.2f", s_echeance -> taux );
    str_taux =  g_strndup ( buffer, nbre_char + 1 );

    str_frais = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_echeance -> frais ),
                        s_echeance -> devise, TRUE );

    str_echeance = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_echeance -> echeance ),
                        s_echeance -> devise, TRUE );

    str_totale = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_echeance -> total_echeance ),
                        s_echeance -> devise, TRUE );

    str_total_cost = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_echeance -> total_cost ),
                        s_echeance -> devise, TRUE );

    gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, NULL );
    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        BET_FINANCE_DURATION_COLUMN, str_duree,
                        BET_FINANCE_NBRE_ECHEANCE_COLUMN, s_echeance -> nbre_echeances,
                        BET_FINANCE_DEVISE_COLUMN, s_echeance -> devise,
                        BET_FINANCE_CAPITAL_COLUMN, str_capital,
                        BET_FINANCE_CAPITAL_DOUBLE, s_echeance -> capital,
                        BET_FINANCE_TAUX_COLUMN, str_taux,
                        BET_FINANCE_TAUX_PERIODIQUE_DOUBLE, s_echeance -> taux_periodique,
                        BET_FINANCE_HORS_FRAIS_COLUMN, str_echeance,
                        BET_FINANCE_HORS_FRAIS_DOUBLE, s_echeance -> echeance,
                        BET_FINANCE_FRAIS_COLUMN, str_frais,
                        BET_FINANCE_FRAIS_DOUBLE, s_echeance -> frais,
                        BET_FINANCE_ECHEANCE_COLUMN, str_totale,
                        BET_FINANCE_ECHEANCE_DOUBLE, s_echeance -> total_echeance,
                        BET_FINANCE_COUT_COLUMN, str_total_cost,
                        BET_FINANCE_COUT_DOUBLE, s_echeance -> total_cost,
                        - 1 );

    g_free ( str_duree );
    g_free ( str_capital );
    g_free ( str_taux );
    g_free ( str_frais );
    g_free ( str_echeance );
    g_free ( str_totale );
}


/**
 * set the background colors of the list
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean bet_finance_list_set_background_color ( GtkWidget *tree_view, gint color_column )
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if ( !tree_view )
        return FALSE;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ));

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
    {
        gint current_color = 0;

        do
        {
            gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        color_column, &couleur_fond[current_color],
                        -1 );

            current_color = !current_color;
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_finance_duration_button_changed ( GtkWidget *combobox, GtkWidget *widget )
{
    bet_finance_calculer_clicked ( NULL, widget );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
void bet_finance_type_taux_changed ( GtkWidget *togglebutton, GdkEventButton *event, GtkWidget *widget )
{
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( togglebutton ), TRUE );
    etat.bet_type_taux = !etat.bet_type_taux;
    bet_finance_calculer_clicked ( NULL, widget );
}


/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
gboolean bet_finance_data_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        GtkWidget *page )
{
    /* show the popup */
    if ( ev -> button == RIGHT_BUTTON )
    {
        gint page_num;

        page_num = gtk_notebook_page_num ( GTK_NOTEBOOK ( finance_notebook ), page );
        bet_finance_data_list_context_menu ( tree_view, page_num );
    }

    return FALSE;
}


/**
 * Pop up a menu with several actions to apply to data_list.
 *
 * \param tree_view
 *
 */
void bet_finance_data_list_context_menu ( GtkWidget *tree_view, gint page_num )
{
    GtkWidget *menu, *menu_item;
    GtkWidget *image;
    GtkTreeModel *model;
    GtkTreeSelection *tree_selection;
    GtkTreeIter iter;
    gchar *tmp_str;
    gint origin;

    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    menu = gtk_menu_new ();

    origin = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( tree_view ), "origin" ) );

    tmp_str = g_build_filename ( GRISBI_PIXMAPS_DIR, "ac_liability_16.png", NULL);
    image = gtk_image_new_from_file ( tmp_str );
    gtk_image_set_pixel_size ( GTK_IMAGE ( image ), GTK_ICON_SIZE_MENU );
    g_free ( tmp_str );

    if ( origin == SPP_ORIGIN_SIMULATOR )
    {
        if ( page_num == 0 )
        {
            menu_item = gtk_image_menu_item_new_with_label ( _("View amortization table") );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_finance_fill_amortization_array ),
                        tree_selection );
        }
        else
        {
            menu_item = gtk_image_menu_item_new_with_label ( _("View credits simulator") );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_finance_switch_simulator_page ),
                        NULL );
        }
        
    }
    else
    {
        gboolean amortization_initial_date;

        amortization_initial_date = GPOINTER_TO_INT ( g_object_get_data (
                        G_OBJECT ( tree_view ), "amortization_initial_date" ) );

        if ( amortization_initial_date )
            menu_item = gtk_image_menu_item_new_with_label ( _("Show amortization schedule to date") );
        else
            menu_item = gtk_image_menu_item_new_with_label ( _("Show amortization schedule from the beginning") );

        g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_finance_switch_amortization_initial_date ),
                        tree_view );
    }

    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ), image );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

    /* Print list */
    menu_item = gtk_image_menu_item_new_with_label ( _("Print the array") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_PRINT, GTK_ICON_SIZE_MENU ) );
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( print_tree_view_list ),
                        tree_view );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Export list */
    menu_item = gtk_image_menu_item_new_with_label ( _("Export the array") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU ) );
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_finance_ui_export_tab ),
                        tree_view );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Finish all. */
    gtk_widget_show_all ( menu );
    gtk_menu_popup ( GTK_MENU( menu ), NULL, NULL, NULL, NULL, 3,
                        gtk_get_current_event_time ( ) );
}


/**
 * Create the amortization page
 *
 *
 *
 * */
GtkWidget *bet_finance_create_amortization_page ( void )
{
    GtkWidget *page;
    GtkWidget *hbox;
    GtkWidget *align;
    GtkWidget *label_title;
    GtkWidget *label;
    GtkWidget *tree_view;
    GtkWidget *toolbar;

    devel_debug (NULL);

    page = gtk_vbox_new ( FALSE, 5 );

    /* titre de la page */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5);
 
    label_title = gtk_label_new ( _("Amortization Table") );
    gtk_container_add ( GTK_CONTAINER ( align ), label_title );

    /* Choix des données sources */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5);

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_container_add ( GTK_CONTAINER ( align ), hbox );

    /* capital */
    label = gtk_label_new ( _("Loan amount: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    label = gtk_label_new ( NULL );
    g_object_set_data ( G_OBJECT ( page ), "capital", label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* taux */
    label = gtk_label_new ( _("Annuel rate interest: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    label = gtk_label_new ( NULL );
    g_object_set_data ( G_OBJECT ( page ), "taux", label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("%") );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* Duration */
    label = gtk_label_new ( _("Duration: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    label = gtk_label_new ( NULL );
    g_object_set_data ( G_OBJECT ( page ), "duree", label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    /* création de la liste des données */
    tree_view = bet_finance_create_amortization_tree_view ( page, SPP_ORIGIN_SIMULATOR );
    g_object_set_data ( G_OBJECT ( tree_view ), "origin",
                        GINT_TO_POINTER ( SPP_ORIGIN_SIMULATOR ) );
    g_object_set_data ( G_OBJECT ( page ), "tree_view", tree_view );
    g_object_set_data ( G_OBJECT ( tree_view ), "label_title", label_title );

    /* set the color of selected row */
    gtk_widget_modify_base ( tree_view, GTK_STATE_SELECTED, &couleur_selection );
    gtk_widget_modify_base ( tree_view, GTK_STATE_ACTIVE, &couleur_selection );

    /* on y ajoute la barre d'outils */
    toolbar = bet_finance_create_simulator_toolbar ( page, tree_view, FALSE, TRUE );
    gtk_box_pack_start ( GTK_BOX ( page ), toolbar, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( page ), toolbar, 0 );

    gtk_widget_show_all ( page );

    return page;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_finance_create_amortization_tree_view ( GtkWidget *container, gint origin )
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkTreeStore *tree_model;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    gchar *title;

    devel_debug ( NULL);
    tree_view = gtk_tree_view_new ( );
    gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW ( tree_view ), FALSE );

    /* Create the tree store */
    tree_model = gtk_tree_store_new ( BET_AMORTIZATION_NBRE_COLUMNS,
                        G_TYPE_INT,         /* BET_AMORTIZATION_NUMBER_COLUMN       */
                        G_TYPE_STRING,      /* BET_AMORTIZATION_DATE_COLUMN         */
                        G_TYPE_STRING,      /* BET_AMORTIZATION_CAPITAL_DU_COLUMN,  */
                        G_TYPE_STRING,      /* BET_AMORTIZATION_INTERETS_COLUMN     */
                        G_TYPE_STRING,      /* BET_AMORTIZATION_PRINCIPAL_COLUMN    */
                        G_TYPE_STRING,      /* BET_AMORTIZATION_FRAIS_COLUMN        */
                        G_TYPE_STRING,      /* BET_AMORTIZATION_ECHEANCE_COLUMN     */
                        GDK_TYPE_COLOR );   /* BET_AMORTIZATION_BACKGROUND_COLOR    */
    gtk_tree_view_set_model ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_MODEL ( tree_model ) );
    g_object_unref ( G_OBJECT ( tree_model ) );

    /* create columns */
    /* numéro ou date de l'échéance */
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    if ( origin == SPP_ORIGIN_FINANCE )
    {
        title = g_strdup ( _("Date") );

        column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_AMORTIZATION_DATE_COLUMN,
                        "cell-background-gdk", BET_AMORTIZATION_BACKGROUND_COLOR,
                        NULL);
        g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_AMORTIZATION_DATE_COLUMN ) );
    }
    else
    {
        title = g_strdup ( _("Number") );

        column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_AMORTIZATION_NUMBER_COLUMN,
                        "cell-background-gdk", BET_AMORTIZATION_BACKGROUND_COLOR,
                        NULL);
        g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_AMORTIZATION_NUMBER_COLUMN ) );
    }

    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_free ( title );

    /* Capital restant dû */
    title = g_strdup ( _("Capital remaining") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_AMORTIZATION_CAPITAL_DU_COLUMN,
                        "cell-background-gdk", BET_AMORTIZATION_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_AMORTIZATION_CAPITAL_DU_COLUMN ) );
    g_free ( title );

    /* Interests */
    title = g_strdup ( _("Interests") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_AMORTIZATION_INTERETS_COLUMN,
                        "cell-background-gdk", BET_AMORTIZATION_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_AMORTIZATION_INTERETS_COLUMN ) );
    g_free ( title );

    /* Capital repaid */
    title = g_strdup ( _("Capital repaid") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_AMORTIZATION_PRINCIPAL_COLUMN,
                        "cell-background-gdk", BET_AMORTIZATION_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_AMORTIZATION_PRINCIPAL_COLUMN ) );
    g_free ( title );

    /* Fees*/
    title = g_strdup ( _("Insurance") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_AMORTIZATION_FRAIS_COLUMN,
                        "cell-background-gdk", BET_AMORTIZATION_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_AMORTIZATION_FRAIS_COLUMN ) );
    g_free ( title );

    /* Monthly paid */
    title = g_strdup ( _("Monthly paid") );
    cell = gtk_cell_renderer_text_new ( );
    g_object_set ( G_OBJECT ( cell ), "xalign", 0.5, NULL );

    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", BET_AMORTIZATION_ECHEANCE_COLUMN,
                        "cell-background-gdk", BET_AMORTIZATION_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    g_object_set_data ( G_OBJECT ( column ), "num_col_model",
                        GINT_TO_POINTER ( BET_AMORTIZATION_ECHEANCE_COLUMN ) );
    g_free ( title );

    g_signal_connect ( G_OBJECT ( tree_view ),
                        "button-press-event",
                        G_CALLBACK ( bet_finance_data_list_button_press ),
                        container );
    g_object_set_data ( G_OBJECT ( tree_view ), "amortization_initial_date",
                        GINT_TO_POINTER ( FALSE ) );

    /* create the scrolled window */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_widget_set_size_request ( scrolled_window, -1, 250 );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), tree_view );
    gtk_box_pack_start ( GTK_BOX ( container ), scrolled_window, TRUE, TRUE, 15 );

    gtk_widget_show_all ( scrolled_window );

    return tree_view;
}


/**
 * remplit le tableau d'amortissement
 *
 * /param menu item
 * /param row selected
 *
 * */
void bet_finance_fill_amortization_array ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
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
    struct_amortissement *s_amortissement;

    devel_debug ( NULL );
    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    gtk_notebook_next_page ( GTK_NOTEBOOK ( finance_notebook ) );

    /* initialise les données utiles */
    s_amortissement = bet_data_finance_structure_amortissement_init ( );
    gtk_tree_model_get ( model,
                        &iter,
                        BET_FINANCE_DURATION_COLUMN, &str_duree,
                        BET_FINANCE_NBRE_ECHEANCE_COLUMN, &nbre_echeances,
                        BET_FINANCE_DEVISE_COLUMN, &s_amortissement -> devise,
                        BET_FINANCE_CAPITAL_COLUMN, &str_capital,
                        BET_FINANCE_CAPITAL_DOUBLE, &s_amortissement -> capital_du,
                        BET_FINANCE_TAUX_COLUMN, &str_taux,
                        BET_FINANCE_TAUX_PERIODIQUE_DOUBLE, &taux_periodique,
                        BET_FINANCE_FRAIS_COLUMN, &s_amortissement -> str_frais,
                        BET_FINANCE_FRAIS_DOUBLE, &s_amortissement -> frais,
                        BET_FINANCE_ECHEANCE_COLUMN, &s_amortissement -> str_echeance,
                        BET_FINANCE_ECHEANCE_DOUBLE, &s_amortissement -> echeance,
                        -1 );

    /* met à jour le titre du tableau d'amortissement */
    page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( finance_notebook ), 1 );
    label = g_object_get_data ( G_OBJECT ( page ), "capital" );
    gtk_label_set_label ( GTK_LABEL ( label ), str_capital );
    label = g_object_get_data ( G_OBJECT ( page ), "taux" );
    gtk_label_set_label ( GTK_LABEL ( label ), str_taux );
    label = g_object_get_data ( G_OBJECT ( page ), "duree" );
    gtk_label_set_label ( GTK_LABEL ( label ), str_duree );

    g_free ( str_duree );
    g_free ( str_capital );
    g_free ( str_taux );

    /* remplit le tableau d'amortissement */
    tree_view = g_object_get_data ( G_OBJECT ( page ), "tree_view" );
    store = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_store_clear ( GTK_TREE_STORE ( store ) );

    for ( index = 1; index <= nbre_echeances; index++ )
    {
        s_amortissement -> num_echeance = index;
        s_amortissement -> interets = bet_data_finance_get_interets ( s_amortissement -> capital_du,
                        taux_periodique );

        if ( index == nbre_echeances )
        {
            s_amortissement -> echeance = bet_data_finance_get_last_echeance (
                        s_amortissement -> capital_du,
                        s_amortissement -> interets,
                        s_amortissement -> frais );
            g_free ( s_amortissement -> str_echeance );
            s_amortissement -> str_echeance = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_amortissement -> echeance ),
                        s_amortissement ->  devise, TRUE );
            s_amortissement -> principal = s_amortissement -> capital_du;
        }
        else
            s_amortissement -> principal = bet_data_finance_get_principal (
                        s_amortissement -> echeance,
                        s_amortissement -> interets,
                        s_amortissement -> frais );

        bet_finance_fill_amortization_ligne ( store, s_amortissement );
        s_amortissement -> capital_du -= s_amortissement -> principal;
    }

    bet_finance_list_set_background_color ( tree_view, BET_AMORTIZATION_BACKGROUND_COLOR );
    path = gtk_tree_path_new_first ( );
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW ( tree_view ), path, NULL, TRUE, 0.0, 0.0 );
    gtk_tree_selection_select_path ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ), path );
    
    gtk_tree_path_free ( path );
    bet_data_finance_structure_amortissement_free ( s_amortissement );
}


/**
 *
 *
 *
 *
 * */
void bet_finance_fill_amortization_ligne ( GtkTreeModel *model,
                        struct_amortissement *s_amortissement )
{
    GtkTreeIter iter;
    gchar *str_capital_du = NULL;
    gchar *str_interets = NULL;
    gchar *str_principal = NULL;

    str_capital_du = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_amortissement -> capital_du ),
                        s_amortissement -> devise, TRUE );

    str_interets = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_amortissement -> interets ),
                        s_amortissement ->  devise, TRUE );

    str_principal = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_amortissement -> principal ),
                        s_amortissement ->  devise, TRUE );

    gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, NULL );

    if ( s_amortissement -> origin == SPP_ORIGIN_FINANCE )
    {
        gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        BET_AMORTIZATION_DATE_COLUMN, s_amortissement -> str_date,
                        BET_AMORTIZATION_CAPITAL_DU_COLUMN, str_capital_du,
                        BET_AMORTIZATION_INTERETS_COLUMN, str_interets,
                        BET_AMORTIZATION_PRINCIPAL_COLUMN, str_principal,
                        BET_AMORTIZATION_FRAIS_COLUMN, s_amortissement -> str_frais,
                        BET_AMORTIZATION_ECHEANCE_COLUMN, s_amortissement -> str_echeance,
                        - 1 );
    }
    else
    {
        gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        BET_AMORTIZATION_NUMBER_COLUMN, s_amortissement -> num_echeance,
                        BET_AMORTIZATION_CAPITAL_DU_COLUMN, str_capital_du,
                        BET_AMORTIZATION_INTERETS_COLUMN, str_interets,
                        BET_AMORTIZATION_PRINCIPAL_COLUMN, str_principal,
                        BET_AMORTIZATION_FRAIS_COLUMN, s_amortissement -> str_frais,
                        BET_AMORTIZATION_ECHEANCE_COLUMN, s_amortissement -> str_echeance,
                        - 1 );
    }

    g_free ( str_capital_du );
    g_free ( str_interets );
    g_free ( str_principal );
}


/**
 * Create the account amortization page
 *
 *
 *
 * */
GtkWidget *bet_finance_create_account_page ( void )
{
    GtkWidget *page;
    GtkWidget *hbox;
    GtkWidget *align;
    GtkWidget *label_title;
    GtkWidget *label;
    GtkWidget *tree_view;
    GtkWidget *toolbar;

    devel_debug (NULL);

    page = gtk_vbox_new ( FALSE, 5 );

    /* titre de la page */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5);
 
    label_title = gtk_label_new ( _("Amortization Table") );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_finance_amortization_title", label_title );
    gtk_container_add ( GTK_CONTAINER ( align ), label_title );

    /* Choix des données sources */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5);

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_container_add ( GTK_CONTAINER ( align ), hbox );

    /* capital */
    label = gtk_label_new ( _("Loan capital: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    label = gtk_label_new ( NULL );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_finance_capital", label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* Annuel rate interest */
    label = gtk_label_new ( _("Annuel rate interest: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    label = gtk_label_new ( NULL );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_finance_taux", label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("%") );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* Duration */
    label = gtk_label_new ( _("Duration: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    label = gtk_label_new ( NULL );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_finance_duree", label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    /* création de la liste des données */
    tree_view = bet_finance_create_amortization_tree_view ( page, SPP_ORIGIN_FINANCE );
    g_object_set_data ( G_OBJECT ( tree_view ), "origin",
                        GINT_TO_POINTER ( SPP_ORIGIN_FINANCE ) );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_finance_tree_view", tree_view );
    g_object_set_data ( G_OBJECT ( tree_view ), "label_title", label_title );

    /* set the color of selected row */
    gtk_widget_modify_base ( tree_view, GTK_STATE_SELECTED, &couleur_selection );
    gtk_widget_modify_base ( tree_view, GTK_STATE_ACTIVE, &couleur_selection );

    /* on y ajoute la barre d'outils */
    toolbar = bet_finance_create_simulator_toolbar ( page, tree_view, FALSE, FALSE );
    gtk_box_pack_start ( GTK_BOX ( page ), toolbar, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( page ), toolbar, 0 );

    gtk_widget_show_all ( page );

    return page;
}


/**
 *
 *
 *
 *
 * */
void bet_finance_ui_update_amortization_tab ( gint account_number )
{
    GtkWidget *label;
    GtkWidget *tree_view;
    GtkTreeModel *store;
    GtkTreePath *path;
    gchar *tmp_str;
    gchar *tmp_str_2;
    gint index = 0;
    gint nbre_ans;
    gint nbre_echeances;
    gint type_taux;
    gdouble taux;
    gdouble taux_periodique;
    GDate *date;
    GDate *last_paid_date = NULL;
    struct_amortissement *s_amortissement;
    gboolean amortization_initial_date;

    devel_debug ( NULL );
    if ( gsb_gui_navigation_get_current_account ( ) != account_number )
        return;

    s_amortissement = g_malloc0 ( sizeof ( struct_amortissement ) );
    s_amortissement -> origin = SPP_ORIGIN_FINANCE;

    /* récupère le tableau d'amortissement */
    tree_view = g_object_get_data ( G_OBJECT ( account_page ), "bet_finance_tree_view" );
    amortization_initial_date = GPOINTER_TO_INT ( g_object_get_data (
                        G_OBJECT ( tree_view ), "amortization_initial_date" ) );

    /* récupère les paramètres du compte */
    s_amortissement -> devise = gsb_data_account_get_currency ( account_number );
    nbre_echeances = gsb_data_account_get_bet_months ( account_number );
    date = gsb_data_account_get_bet_start_date ( account_number );
    s_amortissement -> str_date = gsb_format_gdate ( date );
    if ( amortization_initial_date == FALSE )
        last_paid_date = bet_data_finance_get_date_last_installment_paid ( date );
    else
        last_paid_date = gsb_date_copy ( date );

    /* met à jour le titre du tableau */
    label = g_object_get_data ( G_OBJECT ( account_page ), "bet_finance_amortization_title" );
    tmp_str = g_strconcat ( _("Amortization Table"), _(" at "),
                        gsb_format_gdate ( last_paid_date ), NULL );
    gtk_label_set_label ( GTK_LABEL ( label ), tmp_str );
    g_free ( tmp_str );

    /* set capital */
    s_amortissement -> capital_du = gsb_data_account_get_bet_finance_capital ( account_number );
    label = g_object_get_data ( G_OBJECT ( account_page ), "bet_finance_capital" );
    tmp_str = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_amortissement -> capital_du ),
                        s_amortissement -> devise, TRUE );
    gtk_label_set_label ( GTK_LABEL ( label ), tmp_str );
    g_free ( tmp_str );

    /* set taux */
    label = g_object_get_data ( G_OBJECT ( account_page ), "bet_finance_taux" );
    taux = gsb_data_account_get_bet_finance_taux_annuel ( account_number );
    type_taux = gsb_data_account_get_bet_finance_type_taux ( account_number );
    taux_periodique = bet_data_finance_get_taux_periodique ( taux, type_taux );
    tmp_str = utils_str_dtostr ( taux, 2, FALSE );
    gtk_label_set_label ( GTK_LABEL ( label ), tmp_str );
    g_free ( tmp_str );

    /* set duration */
    label = g_object_get_data ( G_OBJECT ( account_page ), "bet_finance_duree" );
    nbre_ans = gsb_data_account_get_bet_months ( account_number ) / 12;
    if ( nbre_ans == 1 )
        tmp_str_2 = g_strdup ( _(" year ") );
    else
        tmp_str_2 = g_strdup ( _(" years ") );
    tmp_str = g_strconcat ( utils_str_itoa ( nbre_ans ), tmp_str_2, NULL );
    gtk_label_set_label ( GTK_LABEL ( label ), tmp_str );
    g_free ( tmp_str );
    g_free ( tmp_str_2 );

    /* set frais */
    s_amortissement -> frais = gsb_data_account_get_bet_finance_frais ( account_number );
    s_amortissement -> str_frais = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_amortissement -> frais ),
                        s_amortissement -> devise, TRUE );

    /* remplit le tableau d'amortissement */
    store = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_store_clear ( GTK_TREE_STORE ( store ) );

    /* set echeance */
    s_amortissement -> echeance = bet_data_finance_get_echeance ( s_amortissement -> capital_du,
                        taux_periodique, nbre_echeances );
    s_amortissement -> echeance += s_amortissement -> frais;
    s_amortissement -> str_echeance = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_amortissement -> echeance ),
                        s_amortissement -> devise, TRUE );

    for ( index = 1; index <= nbre_echeances; index++ )
    {
        s_amortissement -> interets = bet_data_finance_get_interets ( s_amortissement -> capital_du,
                        taux_periodique );

        if ( index == nbre_echeances )
        {
            s_amortissement -> echeance = bet_data_finance_get_last_echeance (
                        s_amortissement -> capital_du,
                        s_amortissement -> interets,
                        s_amortissement -> frais );
            g_free ( s_amortissement -> str_echeance );
            s_amortissement -> str_echeance = gsb_real_get_string_with_currency (
                        gsb_real_double_to_real ( s_amortissement -> echeance ),
                        s_amortissement ->  devise, TRUE );
            s_amortissement -> principal = s_amortissement -> capital_du;
        }
        else
            s_amortissement -> principal = bet_data_finance_get_principal (
                        s_amortissement -> echeance,
                        s_amortissement -> interets,
                        s_amortissement -> frais );

        if ( g_date_compare ( date, last_paid_date ) >= 0 )
            bet_finance_fill_amortization_ligne ( store, s_amortissement );
        date = gsb_date_add_one_month ( date, TRUE );
        s_amortissement -> str_date = gsb_format_gdate ( date );
        s_amortissement -> capital_du -= s_amortissement -> principal;
    }

    bet_finance_ui_struct_amortization_free ( s_amortissement );
    g_date_free ( date );
    g_date_free ( last_paid_date );

    bet_finance_list_set_background_color ( tree_view, BET_AMORTIZATION_BACKGROUND_COLOR );
    path = gtk_tree_path_new_first ( );
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW ( tree_view ), path, NULL, TRUE, 0.0, 0.0 );
    gtk_tree_selection_select_path ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ), path );
    
    gtk_tree_path_free ( path );
}


/**
 *
 *
 *
 *
 * */
void bet_finance_ui_struct_amortization_free ( struct_amortissement *s_amortissement )
{
    if ( s_amortissement -> str_date )
        g_free ( s_amortissement -> str_date );
    if ( s_amortissement -> str_echeance )
        g_free ( s_amortissement -> str_echeance );
    if ( s_amortissement -> str_frais )
        g_free ( s_amortissement -> str_frais );

    g_free ( s_amortissement );
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_finance_create_simulator_toolbar ( GtkWidget *parent,
                        GtkWidget *tree_view,
                        gboolean simulator,
                        gboolean amortization )
{
    GtkWidget *handlebox;
    GtkWidget *hbox;
    GtkWidget *button;
    GtkTreeSelection *selection;

    /* HandleBox */
    handlebox = gtk_handle_box_new ( );

    /* Hbox */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER ( handlebox ), hbox );

    if ( simulator )
    {
    /* création du bouton calculer */
        button = gsb_automem_stock_button_new ( etat.display_toolbar,
                        GTK_STOCK_EXECUTE,
                        _("Calculate"),
                        NULL,
                        NULL );
        gtk_widget_set_tooltip_text ( GTK_WIDGET ( button ), _("Calculate") );
        g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( bet_finance_calculer_clicked ),
                        parent );
        g_object_set_data ( G_OBJECT ( parent ), "calculate_button", button );
        gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );

        /* création du bouton afficher le tableau d'amortissement */
        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
        button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
                        _("Amortization"),
                        "ac_liability_16.png",
                        NULL,
                        NULL );
        gtk_widget_set_tooltip_text ( GTK_WIDGET ( button ), _("View amortization table") );
        g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( bet_finance_fill_amortization_array ),
                        selection );
        gtk_widget_set_sensitive ( button, FALSE );
        g_object_set_data ( G_OBJECT ( parent ), "amortization_button", button );
        gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );
    }
    else if ( amortization )
    {
        /* création du bouton afficher le simulateur de crédits */
        button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
                        _("Credits"),
                        "ac_liability_16.png",
                        NULL,
                        NULL );
        gtk_widget_set_tooltip_text ( GTK_WIDGET ( button ), _("View credits simulator") );
        g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( bet_finance_switch_simulator_page ),
                        NULL );
        gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );
    }
    else
    {
        gboolean amortization_initial_date = FALSE;

        /* création du bouton afficher le simulateur de crédits */
        button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
                        _("Amortization"),
                        "ac_liability_16.png",
                        NULL,
                        NULL );
        gtk_widget_set_tooltip_text ( GTK_WIDGET ( button ), _("Show amortization schedule from the beginning") );
        g_object_set_data ( G_OBJECT ( tree_view ), "amortization_initial_date_button", button );
        g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( bet_finance_switch_amortization_initial_date ),
                        tree_view );
        gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );

        g_object_set_data ( G_OBJECT ( tree_view ), "amortization_initial_date",
                        GINT_TO_POINTER ( amortization_initial_date ) );
    }

    /* création du bouton print */
    button = gsb_automem_stock_button_new ( etat.display_toolbar,
                        GTK_STOCK_PRINT,
                        _("Print"),
                        NULL,
                        NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( button ), _("Print the array") );
    g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( print_tree_view_list ),
                        tree_view );
    if ( simulator )
        gtk_widget_set_sensitive ( button, FALSE );
    g_object_set_data ( G_OBJECT ( parent ), "print_button", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );

    /* Export button */
    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					   GTK_STOCK_SAVE,
					   _("Export"),
					   NULL,
					   NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( button ), _("Export the array") );
    g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( bet_finance_ui_export_tab ),
                        tree_view );
    if ( simulator )
        gtk_widget_set_sensitive ( button, FALSE );
    g_object_set_data ( G_OBJECT ( parent ), "export_button", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );

    gtk_widget_show_all ( handlebox );

    return ( handlebox );

}


/**
 *
 *
 *
 *
 * */
void bet_finance_ui_export_tab ( GtkWidget *menu_item, GtkTreeView *tree_view )
{
    GtkWidget *dialog;
    gint resultat;
    gchar *filename;

    dialog = gtk_file_chooser_dialog_new ( _("Export the array"),
					   GTK_WINDOW ( window ),
					   GTK_FILE_CHOOSER_ACTION_SAVE,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_SAVE, GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER ( dialog ),  _("credit.csv"));
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( dialog ), gsb_file_get_last_path () );
    gtk_file_chooser_set_do_overwrite_confirmation ( GTK_FILE_CHOOSER ( dialog ), TRUE);
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    filename = file_selection_get_filename ( GTK_FILE_CHOOSER ( dialog ) );
	    gsb_file_update_last_path ( file_selection_get_last_directory ( GTK_FILE_CHOOSER ( dialog ), TRUE ) );
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

	    /* vérification que c'est possible est faite par la boite de dialogue */
	    if ( !gsb_csv_export_tree_view_list ( filename, tree_view ) )
	    {
            dialogue_error ( _("Cannot save file.") );
            return;
	    }

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
	    return;
    }
}


/**
 *
 *
 *
 *
 * */
void bet_finance_switch_amortization_initial_date ( GtkWidget *widget, GtkWidget *tree_view )
{
    GtkWidget *tmp_button;
    gint account_number;
    gboolean amortization_initial_date;

    amortization_initial_date = GPOINTER_TO_INT ( g_object_get_data (
                        G_OBJECT ( tree_view ), "amortization_initial_date" ) );
    amortization_initial_date = !amortization_initial_date;

    g_object_set_data ( G_OBJECT ( tree_view ), "amortization_initial_date",
                        GINT_TO_POINTER ( amortization_initial_date ) );

    if ( GTK_IS_BUTTON ( widget ) )
        tmp_button = widget;
    else
        tmp_button = g_object_get_data ( G_OBJECT ( tree_view ), "amortization_initial_date_button" );
    
    if ( amortization_initial_date )
            gtk_widget_set_tooltip_text ( GTK_WIDGET ( tmp_button ), _("Show amortization schedule to date") );
    else
        gtk_widget_set_tooltip_text ( GTK_WIDGET ( tmp_button ), _("Show amortization schedule from the beginning") );

    account_number = gsb_gui_navigation_get_current_account ( );
    bet_finance_ui_update_amortization_tab ( account_number );
}


/**
 *
 *
 *
 *
 * */
void bet_finance_calcule_show_months_tab ( GtkTreeModel *model,
                        struct_echeance *s_echeance,
                        gdouble taux_frais )
{
    gint duree_min = 3;
    gint duree_max = 18;
    gint index;

    for ( index = duree_min; index <= duree_max; index++ )
    {
        s_echeance -> duree = index;
        s_echeance -> nbre_echeances = index;
        s_echeance -> frais = bet_data_finance_get_frais_par_echeance ( s_echeance -> capital,
                        taux_frais,
                        s_echeance -> nbre_echeances );

        s_echeance -> echeance = bet_data_finance_get_echeance ( s_echeance -> capital,
                        s_echeance -> taux_periodique,
                        s_echeance -> nbre_echeances );

        s_echeance -> total_echeance = s_echeance -> echeance + s_echeance -> frais;
        s_echeance -> total_cost = bet_data_finance_get_total_cost ( s_echeance );

        bet_finance_fill_data_ligne ( model, s_echeance, _("months") );

        s_echeance -> duree = 0;
        s_echeance -> nbre_echeances = 0;
        s_echeance -> echeance = 0;
    }
}


/**
 *
 *
 *
 *
 * */
void bet_finance_spin_button_fees_changed ( GtkSpinButton *spinbutton, GtkWidget *page )
{
    etat.bet_frais = gtk_spin_button_get_value ( GTK_SPIN_BUTTON ( spinbutton ) );
    bet_finance_calculer_clicked ( NULL, page );
}


/**
 *
 *
 *
 *
 * */
void bet_finance_spin_button_taux_changed ( GtkSpinButton *spinbutton, GtkWidget *page )
{
    etat.bet_taux_annuel = gtk_spin_button_get_value ( GTK_SPIN_BUTTON ( spinbutton ) );
    bet_finance_calculer_clicked ( NULL, page );
}


/**
 *
 *
 *
 *
 * */
void bet_finance_currency_changed ( GtkComboBox *combo_box, GtkWidget *page )
{
    etat.bet_currency = gsb_currency_get_currency_from_combobox ( GTK_WIDGET ( combo_box ) );
    bet_finance_calculer_clicked ( NULL, page );
}


/**
 *
 *
 *
 *
 * */
gboolean bet_finance_capital_entry_changed ( GtkWidget *entry, GtkWidget *page  )
{
    const gchar *text;
    gdouble capital;
    gboolean valide;

    text = gtk_entry_get_text ( GTK_ENTRY ( entry ) );
    capital = utils_str_strtod ( text, NULL );

    if ( strlen ( text ) == 0 || capital == 0 )
    {
		gtk_widget_modify_base ( entry, GTK_STATE_NORMAL, NULL );
	    return FALSE;
    }

    valide = gsb_form_widget_get_valide_amout_entry ( text );
    if ( valide )
    {
        /* the entry is valid, make it normal */
	    gtk_widget_modify_base ( entry, GTK_STATE_NORMAL, NULL );
    }
    else
    {
	    /* the entry is not valid, make it red */
		gtk_widget_modify_base ( entry, GTK_STATE_NORMAL,
                        &calendar_entry_color );
    }

    return FALSE;
}


/**
 * called when press a key on an element of the form
 *
 * \param widget wich receive the signal
 * \param ev
 * \param page
 *
 * \return FALSE
 * */
gboolean bet_finance_capital_entry_key_press_event ( GtkWidget *widget,
                        GdkEventKey *ev,
                        GtkWidget *page )
{
    gchar *str_capital;

    switch ( ev -> keyval )
    {
        case GDK_1:
        case GDK_2:
        case GDK_3:
        case GDK_4:
        case GDK_5:
        case GDK_6:
        case GDK_7:
        case GDK_8:
        case GDK_9:
        case GDK_0:
            break;

        case GDK_Escape :
            str_capital = gsb_real_get_string_with_currency ( gsb_real_double_to_real (
                                    etat.bet_capital ),
                                    etat.bet_currency,
                                    FALSE );
            gtk_entry_set_text ( GTK_ENTRY ( widget ), str_capital );
            gtk_editable_set_position ( GTK_EDITABLE ( widget ), -1 );
            g_free ( str_capital );
            return TRUE;
            break;

        case GDK_ISO_Left_Tab:
            etat.bet_capital = bet_finance_get_number_from_string ( page, "capital" );
            bet_finance_calculer_clicked ( NULL, page );
            return TRUE;
            break;

        case GDK_Tab :
            etat.bet_capital = bet_finance_get_number_from_string ( page, "capital" );
            bet_finance_calculer_clicked ( NULL, page );
            return TRUE;
            break;

        case GDK_KP_Enter :
        case GDK_Return :
            etat.bet_capital = bet_finance_get_number_from_string ( page, "capital" );
            bet_finance_calculer_clicked ( NULL, page );
            return TRUE;
            break;
    }

    return FALSE;
}


GtkWidget *bet_finance_get_capital_entry ( void )
{
    GtkWidget *page;
    GtkWidget *entry;

    page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( finance_notebook ), 0 );
    entry = g_object_get_data ( G_OBJECT ( page ), "capital" );

    return entry;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
