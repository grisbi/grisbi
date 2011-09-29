/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2011 Pierre Biava (grisbi@pierre.biava.name)             */
/*                   2011 Guillaume Verger (guillaume.verger@laposte.net)     */
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

#include <gdk/gdk.h>
#include <glib/gi18n.h>
#include <strings.h>

#ifdef HAVE_GOFFICE
#include <goffice/goffice.h>

/*START_INCLUDE*/
#include "bet_graph.h"
#include "bet_data.h"
#include "bet_hist.h"
#include "dialog.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_fyear.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "utils_gtkbuilder.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


typedef struct _struct_bet_graph_data   struct_bet_graph_data;
typedef struct _struct_bet_graph_button struct_bet_graph_button;
typedef struct _struct_bet_graph_prefs  struct_bet_graph_prefs;

struct _struct_bet_graph_data
{
    GtkTreeView *tree_view;
    GtkNotebook *notebook;
    gint account_number;

    /* Données pour le graphique */
    GtkWidget *widget;
    GogChart *chart;
    GogPlot *plot;
    gchar *title;
    gchar *service_id;          /* définit le type de graphique : GogPiePlot, GogLinePlot, GogBarColPlot = défaut */
    gboolean is_legend;

    /* données communes */
    gint nbre_elemnts;

    /* données pour les histogrammes et XY */
    gdouble tab_X[MAX_POINTS_GRAPHIQUE];
    gdouble tab_Y[MAX_POINTS_GRAPHIQUE];
    gboolean valid_data;

    /* données pour les camemberts */
    gint type_infos;            /* 0 type crédit ou < 0, 1 type débit ou >= 0, -1 tous types */
    gdouble montant;            /* montant annuel toutes catégories. sert au calculde pourcentage */
    gchar tab_libelle[MAX_POINTS_GRAPHIQUE][TAILLE_MAX_LIBELLE];
    gchar **tab_vue_libelle;

};


struct _struct_bet_graph_button
{
    gchar *name;
    gchar *filename;
    gchar *service_id;          /* définit le type de graphique */
    GCallback callback;         /* fonction de callback */
    gboolean is_visible;        /* TRUE si le bouton est visible dans la barre d'outils */
    GtkWidget *box;
    GtkWidget *button;
    GtkWidget *tree_view;
};


struct _struct_bet_graph_prefs
{
    gint type_graph;                /* type de graphique : -1 secteurs, 1 ligne, 0 barres par défaut */
    gboolean major_tick_out;        /* TRUE par défaut */
    gboolean major_tick_in;         /* FALSE par défaut */
    gboolean major_tick_labeled;    /* affichage des libellés de l'axe X. TRUE par défaut*/
    gint position;                  /* position de l'axe des X. En bas par défaut*/
    gboolean new_axis_line;         /* ligne supplémentaire. Croise l'axe des Y à 0. TRUE par défaut*/
    gint cross_entry;               /* position du croisement avec l'axe Y. 0 par défaut */
    gint degrees;                   /* rotation des étiquettes de l'axe X en degrés. 90° par défaut */
    gint gap_spinner;               /* espace entre deux barres en %. 50 par défaut*/
    gboolean before_grid;           /* les étiquettes sont cachées par les barres par défaut */
    gboolean major_grid_y;          /* ajoute une grille principale sur l'axe Y */
    gboolean minor_grid_y;          /* ajoute une grille secondaire sur l'axe Y */
};


/*START_STATIC*/
static gboolean bet_graph_affiche_camemberts ( struct_bet_graph_data *self );
static gboolean bet_graph_affiche_XY_line ( struct_bet_graph_data *self );
static GogPlot *bet_graph_create_graph_page  ( struct_bet_graph_data *self,
                        gboolean add_page );
static void bet_graph_create_pie_canvas  ( struct_bet_graph_data *self );
static GtkWidget *bet_graph_create_line_preferences ( struct_bet_graph_data *self );
static void bet_graph_create_prefs_page  ( struct_bet_graph_data *self );
static void bet_graph_gap_spinner_changed ( GtkSpinButton *spinbutton,
                        struct_bet_graph_data *self );
static GtkWidget *bet_graph_get_canvas  ( void );
static gboolean bet_graph_initialise_builder ( void );
static void bet_graph_map_type_changed ( GtkComboBox *combo,
                        struct_bet_graph_data *self );
static gboolean bet_graph_populate_lines_by_forecast_data ( struct_bet_graph_data *self );
static gboolean bet_graph_populate_sectors_by_hist_data ( struct_bet_graph_data *self );
static void bet_graph_popup_choix_graph_activate ( GtkMenuItem *menuitem,
                        struct_bet_graph_button *self );
static gboolean bet_graph_popup_choix_graph_menu ( GtkWidget *button,
                        GdkEventButton *event,
                        GList *liste );
static void bet_graph_rotation_changed ( GORotationSel *rotation,
                        int angle,
                        struct_bet_graph_data *self );
static void bet_graph_toggle_button_changed ( GtkToggleButton *togglebutton,
                        struct_bet_graph_data *self );
static void bet_graph_update_graph ( struct_bet_graph_data *self );
static void struct_free_bet_graph_button ( struct_bet_graph_button *self );
static struct_bet_graph_button *struct_initialise_bet_graph_button ( void );
static void struct_free_bet_graph_data ( struct_bet_graph_data *self );
static struct_bet_graph_data *struct_initialise_bet_graph_data ( void );
static void struct_free_bet_graph_prefs ( struct_bet_graph_prefs *self );
static struct_bet_graph_prefs *struct_initialise_bet_graph_prefs ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

static GtkBuilder *bet_graph_builder = NULL;
static struct_bet_graph_prefs *prefs_lines = NULL;


/**
 * Crée un builder et récupère les widgets du fichier bet_graph.ui
 *
 *
 *
 * */
gboolean bet_graph_initialise_builder ( void )
{
    /* Creation d'un nouveau GtkBuilder */
    bet_graph_builder = gtk_builder_new ( );
    if ( bet_graph_builder == NULL )
        return FALSE;

    /* récupère les widgets */
    if ( utils_gtkbuilder_merge_ui_data_in_builder ( bet_graph_builder, "bet_graph.ui" ) )
        return TRUE;
    else
        return FALSE;

}


/**
 *
 *
 *
 */
gboolean bet_graph_on_motion ( GtkWidget *event_box,
                        GdkEventMotion *event,
                        struct_bet_graph_data *self )
{
    GogRenderer *rend = NULL;
    GogView *graph_view = NULL;
    GogView *view = NULL;
    GogChartMap *map = NULL;
    GogSeries *series;
    gchar *buf = NULL;
    gint index;

    rend = go_graph_widget_get_renderer ( GO_GRAPH_WIDGET ( self->widget ) );
    g_object_get ( G_OBJECT ( rend ), "view", &graph_view, NULL );
    view = gog_view_find_child_view ( graph_view, GOG_OBJECT ( self->plot ) );

    if ( strcmp ( self->service_id, "GogBarColPlot" ) == 0 )
    {
        index = gog_plot_view_get_data_at_point ( GOG_PLOT_VIEW ( view ), event->x, event->y, &series );

        if ( index == -1 )
            buf = NULL;
        else
            buf = g_strdup_printf ("date %s : solde %s", self->tab_vue_libelle[index],
                        utils_real_get_string_with_currency_from_double (
                        self->tab_Y[index], self->account_number ) );
    }

    else if (  strcmp ( self->service_id, "GogLinePlot")  == 0 )
    {
        GogAxis *x_axis, *y_axis;

        x_axis = GOG_AXIS ( gog_object_get_child_by_name ( GOG_OBJECT ( self->chart ), "X-Axis" ) );
        y_axis = GOG_AXIS ( gog_object_get_child_by_name ( GOG_OBJECT ( self->chart ), "Y-Axis" ) );

        map = gog_chart_map_new ( self->chart, &(view->allocation), x_axis, y_axis, NULL, FALSE );

        if (gog_chart_map_is_valid ( map )
         &&
         event->x >= view->allocation.x && event->x < view->allocation.x + view->allocation.w
         &&
        event->y >= view->allocation.y && event->y < view->allocation.y + view->allocation.h )
        {
            GogAxisMap *x_map;

            x_map = gog_chart_map_get_axis_map (map, 0);
            index = (gint) gog_axis_map_from_view (x_map, event->x);

            buf = g_strdup_printf ("date %s : solde %s", self->tab_vue_libelle[index-1],
                        utils_real_get_string_with_currency_from_double (
                        self->tab_Y[index-1], self->account_number ) );
        }
        else
            buf = NULL;
    }
    else if (  strcmp ( self->service_id, "GogPiePlot" ) == 0 )
    {

        index = gog_plot_view_get_data_at_point ( GOG_PLOT_VIEW ( view ), event->x, event->y, &series );
        if ( index == -1 )
            buf = NULL;
        else
            buf = g_strdup_printf ("%s : %s (%.2f%%)", self->tab_vue_libelle[index],
                        utils_real_get_string_with_currency_from_double (
                        self->tab_Y[index], self->account_number ),
                        ( 100*self->tab_Y[index]/self->montant ) );
    }

    if ( buf )
    {
        gtk_widget_set_tooltip_text ( GTK_WIDGET ( self->widget ), buf );
        g_free ( buf );
    }
    else
        gtk_widget_set_tooltip_text ( GTK_WIDGET ( self->widget ), "" );

    return TRUE;
}


/**
 * Création de la page pour le graphique initialisée
 *
 *
 *
 * */
GogPlot *bet_graph_create_graph_page  ( struct_bet_graph_data *self,
                        gboolean add_page )
{
    GtkWidget *child;
    GtkWidget *w;
    GogGraph *graph;
    GogLabel *label;
    GogPlot *plot;
    GOStyle *style;
    GOData *data;
    PangoFontDescription *desc;

    devel_debug_int (add_page);
    if ( add_page )
    {
        /* Set the new page */
        child = gtk_vbox_new (FALSE, 0);
        gtk_notebook_prepend_page ( self->notebook, child, gtk_label_new ( _("Graph") ) );
    }
    else
        child = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( self->notebook ), 0 );

    /* Set the graph widget */
    w = go_graph_widget_new ( NULL );
    g_signal_connect ( G_OBJECT ( w ),
                        "motion-notify-event",
                        G_CALLBACK ( bet_graph_on_motion ), self );
    gtk_box_pack_end ( GTK_BOX ( child ), w, TRUE, TRUE, 0 );

    self->widget = w;

    /* Get the embedded graph */
    graph = go_graph_widget_get_graph ( GO_GRAPH_WIDGET ( w ) );

    /* set the title */
    if ( self->title && strlen ( self->title ) )
    {
        label = (GogLabel *) g_object_new ( GOG_TYPE_LABEL, NULL );
        data = go_data_scalar_str_new ( self->title, FALSE );
        gog_dataset_set_dim ( GOG_DATASET ( label ), 0, data, NULL );
        gog_object_add_by_name ( GOG_OBJECT ( graph ), "Title", GOG_OBJECT ( label ) );

        /* Change the title font */
        style = go_styled_object_get_style ( GO_STYLED_OBJECT ( label ) );
        desc = pango_font_description_from_string ( "Sans bold 12" );
        go_style_set_font_desc ( style, desc );
    }

    /* Get the chart created by the widget initialization */
    self->chart = go_graph_widget_get_chart ( GO_GRAPH_WIDGET ( w ) );

    /* Create a pie plot and add it to the chart */
    plot = ( GogPlot *) gog_plot_new_by_name ( self->service_id );
    gog_object_add_by_name ( GOG_OBJECT ( self->chart ), "Plot", GOG_OBJECT ( plot ) );

    /* Add a legend to the chart */
    if ( self->is_legend )
        gog_object_add_by_name ( GOG_OBJECT ( self->chart ), "Legend", NULL);

    return plot;
}


/**
 * Création de la page des préférences pour le graphique si nécessaire.
 *
 *
 *
 * */
void bet_graph_create_prefs_page  ( struct_bet_graph_data *self )
{
    GtkWidget *child;
    GtkWidget *label;

    label = gtk_label_new ( _("Options") );
    child = bet_graph_create_line_preferences ( self );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( self->notebook ), child, label );
}


/**
 *
 *
 *
 *
 * */
void bet_graph_sectors_graph_new ( GtkWidget *button,
                        GtkTreeView *tree_view )
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *notebook;
    GtkWidget *box_pie;
    gchar *title;
    gint result;
    gint account_number;
    struct_bet_graph_data *self_credit;
    struct_bet_graph_data *self_debit;

    devel_debug (NULL);

    /* Initialisation d'un nouveau GtkBuilder */
    if ( !bet_graph_initialise_builder ( ) )
        return;

    /* Création de la fenêtre de dialogue pour le graph */
    dialog = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "bet_graph_dialog" ) );
    gtk_window_set_transient_for ( GTK_WINDOW ( dialog ), GTK_WINDOW ( run.window ) );
    gtk_widget_set_size_request ( dialog, PAGE_WIDTH+30, PAGE_HEIGHT+70 );
    gtk_signal_connect ( GTK_OBJECT ( dialog ),
                        "destroy",
                        GTK_SIGNAL_FUNC ( gtk_widget_destroy ),
                        NULL);

    /* set the title */
    label = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( tree_view ), "label_title") );
    title = make_pango_attribut ( "weight=\"bold\" size=\"x-large\"",
                    gtk_label_get_text ( GTK_LABEL ( label ) ) );

    label = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "label_canvas" ) );
    gtk_label_set_markup ( GTK_LABEL ( label ), title );
    g_free ( title );

    /* initialise les pages pour les graphiques ligne et barre */
    notebook = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "notebook" ) );
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( notebook ), FALSE );

    box_pie = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "box_pie" ) );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), box_pie, gtk_label_new ( _("Graph") ) );

    /* initialisation des structures de données */
    account_number = gsb_gui_navigation_get_current_account ( );

    /* Set the graph for debit */
    self_debit = struct_initialise_bet_graph_data ( );
    self_debit->notebook = GTK_NOTEBOOK ( notebook );
    self_debit->tree_view = tree_view;
    self_debit->account_number = account_number;
    self_debit->type_infos = 1;
    self_debit->title = g_strdup ( _("Expenses") );
    self_debit->service_id = g_strdup ( "GogPiePlot" );
    self_debit->is_legend = TRUE;

    self_debit->plot = bet_graph_create_graph_page ( self_debit, FALSE );

    /* populate the data for debit */
    if ( self_debit->valid_data == FALSE )
        self_debit->valid_data = bet_graph_populate_sectors_by_hist_data ( self_debit );
    if ( !self_debit->valid_data )
        return;

    /* show the data for debit */
    result = bet_graph_affiche_camemberts ( self_debit );
    if ( !result )
        return;

    /* Set the graph for credit */
    self_credit = struct_initialise_bet_graph_data ( );
    self_credit->notebook = GTK_NOTEBOOK ( notebook );
    self_credit->tree_view = tree_view;
    self_credit->account_number = account_number;
    self_credit->type_infos = 0;
    self_credit->title = g_strdup ( _("Incomes") );
    self_credit->service_id = g_strdup ( "GogPiePlot" );
    self_credit->is_legend = TRUE;

    self_credit->plot = bet_graph_create_graph_page ( self_credit, FALSE );

    /* populate the data for credit */
    if ( self_credit->valid_data == FALSE )
        self_credit->valid_data = bet_graph_populate_sectors_by_hist_data ( self_credit );
    if ( !self_credit->valid_data )
        return;

    /* show the data for credit */
    result = bet_graph_affiche_camemberts ( self_credit );
    if ( !result )
        return;

    gtk_widget_show_all ( dialog );

    result = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    /* free the data */
    struct_free_bet_graph_data ( self_debit );
    struct_free_bet_graph_data ( self_credit );
    g_object_unref ( G_OBJECT ( bet_graph_builder ) );
    gtk_widget_destroy ( dialog );
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_populate_sectors_by_hist_data ( struct_bet_graph_data *self )
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( self -> tree_view ) );
    if ( model == NULL )
        return FALSE;

    if ( gtk_tree_model_get_iter_first ( model, &iter ) )
    {
        gint account_number;
        gchar *libelle_division = self -> tab_libelle[0];
        gchar **tab_libelle_division;
        gdouble *tab_montant_division = self -> tab_Y;

        tab_libelle_division = &libelle_division;

        /* test du numero de compte */
        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
                        -1 );
        if ( account_number != self -> account_number )
            return FALSE;
        do
        {
            gchar *desc;
            gchar *amount;
            gint div;
            gint type_infos;

            gtk_tree_model_get ( GTK_TREE_MODEL( model ),
                        &iter,
                        SPP_HISTORICAL_DESC_COLUMN, &desc,
                        SPP_HISTORICAL_BALANCE_AMOUNT, &amount,
                        SPP_HISTORICAL_DIV_NUMBER, &div,
                        -1 );

            type_infos = bet_data_get_div_type ( div );
            if ( self -> type_infos == -1 || type_infos == self -> type_infos )
            {
                strncpy ( &libelle_division[self -> nbre_elemnts * TAILLE_MAX_LIBELLE], desc, TAILLE_MAX_LIBELLE );
                tab_montant_division[self -> nbre_elemnts] = utils_str_strtod ( ( amount == NULL) ? "0" : amount, NULL );

                self->montant += tab_montant_division[self -> nbre_elemnts];
                self -> nbre_elemnts++;
            }

            if ( self -> nbre_elemnts >= MAX_SEGMENT_CAMEMBERT )
                break;

        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );

        return TRUE;
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_affiche_camemberts ( struct_bet_graph_data *self )
{
    GogPlot *cur_plot;
    GogSeries *series;
    GOData *data;
    GError * error = NULL;

    /* Afficher les montants des divisions de type débit dans le camembert */
    cur_plot = self->plot;

    gog_plot_clear_series ( GOG_PLOT ( cur_plot ) );
    series = GOG_SERIES ( gog_plot_new_series ( GOG_PLOT ( cur_plot ) ) );

    data = go_data_vector_str_new ( (const char * const*) self -> tab_vue_libelle, self -> nbre_elemnts, NULL );
    gog_series_set_dim ( series, 0, data, &error );
    if ( error != NULL )
    {
        printf ("tab_vue_libelle_division : erreur = %s\n", error -> message);
        g_error_free ( error );
        error = NULL;
        return FALSE;
    }

    data = go_data_vector_val_new ( self -> tab_Y, self -> nbre_elemnts, NULL);
    gog_series_set_dim (series, 1, data, &error);
    if ( error != NULL )
    {
        printf ("tab_montant_division : erreur = %s\n", error -> message);
        g_error_free ( error );
        error = NULL;
        return FALSE;
    }

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
void bet_graph_line_graph_new ( GtkWidget *button, GtkTreeView *tree_view )
{
    GtkWidget *dialog;
    GtkWidget *label;
    gchar *title;
    gchar *service_id;
    gint result;
    gint account_number;
    struct_bet_graph_data *self;

    devel_debug (NULL);

    /* Initialisation d'un nouveau GtkBuilder */
    if ( !bet_graph_initialise_builder ( ) )
        return;

    /* initialisation des structures de données */
    if ( prefs_lines == NULL )
        prefs_lines = struct_initialise_bet_graph_prefs ( );

    account_number = gsb_gui_navigation_get_current_account ( );
    service_id = g_object_get_data ( G_OBJECT ( button ), "service_id" );

    self = struct_initialise_bet_graph_data ( );
    self->tree_view = tree_view;
    self->account_number = account_number;
    self->service_id = g_strdup ( service_id );

    /* Création de la fenêtre de dialogue pour le graph */
    dialog = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "bet_graph_dialog" ) );
    gtk_window_set_transient_for ( GTK_WINDOW ( dialog ), GTK_WINDOW ( run.window ) );
    gtk_widget_set_size_request ( dialog, PAGE_WIDTH+30, PAGE_HEIGHT+70 );
    gtk_signal_connect ( GTK_OBJECT ( dialog ),
                        "destroy",
                        GTK_SIGNAL_FUNC ( gtk_widget_destroy ),
                        NULL);

    /* set the title */
    label = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( tree_view ), "label_title") );
    title = make_pango_attribut ( "weight=\"bold\" size=\"x-large\"",
                    gtk_label_get_text ( GTK_LABEL ( label ) ) );

    label = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "label_canvas" ) );
    gtk_label_set_markup ( GTK_LABEL ( label ), title );
    g_free ( title );

    /* initialise les pages pour les graphiques ligne et barre */
    self->notebook = GTK_NOTEBOOK ( gtk_builder_get_object ( bet_graph_builder, "notebook" ) );

    /* set the graphique page */
    self->plot = bet_graph_create_graph_page ( self, TRUE );

    /* set the preferences page */
    bet_graph_create_prefs_page  ( self );

    /* populate data */
    self->valid_data = bet_graph_populate_lines_by_forecast_data ( self );
    if ( !self->valid_data )
        return;

    /* affiche les données */
    result = bet_graph_affiche_XY_line ( self );
    if ( !result )
        return;

    gtk_widget_show_all ( dialog );
    gtk_notebook_set_current_page ( self->notebook, 0 );

    result = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    /* free the data */
    struct_free_bet_graph_data ( self );
    g_object_unref ( G_OBJECT ( bet_graph_builder ) );
    gtk_widget_destroy ( dialog );
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_populate_lines_by_forecast_data ( struct_bet_graph_data *self )
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;
    gdouble prev_montant;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( self -> tree_view ) );
    if ( model == NULL )
        return FALSE;

    if ( gtk_tree_model_get_iter_first ( model, &iter ) )
    {
        gchar *libelle_axe_x = self -> tab_libelle[0];
        gchar **tab_libelle_axe_x;
        gdouble *tab_Y = self -> tab_Y;
        gdouble montant = 0.;
        GDate *first_date;
        GDate *last_date;
        GDate *date_courante;
        GDateDay day_courant;
        gint nbre_iterations;

        tab_libelle_axe_x = &libelle_axe_x;

        do
        {
            gchar *amount;
            gchar *str_date;
            GValue date_value = {0,};
            GDate *date;
            GDateDay day;
            gint diff_jours;
            gint i;

            gtk_tree_model_get_value ( model,
                        &iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, &date_value );

            gtk_tree_model_get ( GTK_TREE_MODEL( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, &amount,
                        -1 );
            date = g_value_get_boxed ( &date_value );

            montant += (gdouble) utils_str_strtod ( ( amount == NULL) ? "0" : amount, NULL );

            if ( self -> nbre_elemnts == 0 )
            {
                /* on ajoute 1 jour pour passer au 1er du mois */
                g_date_add_days ( date, 1 );

                /* on calcule le nombre maxi d'itération pour une année */
                first_date = gsb_date_copy ( date );
                last_date = gsb_date_copy ( date );
                g_date_add_years ( last_date, 1 );
                nbre_iterations = g_date_days_between ( first_date, last_date );

                date_courante = gsb_date_copy ( date );
                day_courant = g_date_get_day ( date );

                str_date = gsb_format_gdate ( date_courante );
                strncpy ( &libelle_axe_x[self -> nbre_elemnts * TAILLE_MAX_LIBELLE], str_date, TAILLE_MAX_LIBELLE );

                self -> nbre_elemnts++;
                g_free ( str_date );
                g_date_free ( first_date );
                g_date_free ( last_date );
            }
            else
            {
                day = g_date_get_day ( date );
                if ( day != day_courant )
                {
                    /* nombre de jours manquants */
                    diff_jours = g_date_days_between ( date_courante, date );
                    for ( i = diff_jours; i > 0; i-- )
                    {
                        g_date_add_days ( date_courante, 1 );
                        str_date = gsb_format_gdate ( date_courante );

                        strncpy ( &libelle_axe_x[self -> nbre_elemnts * TAILLE_MAX_LIBELLE],
                                    str_date, TAILLE_MAX_LIBELLE );
                        tab_Y[self->nbre_elemnts-1] = prev_montant;
                        self -> nbre_elemnts++;

                        /* on dépasse d'un jour pour obtenir le solde du dernier jour */
                        if ( self -> nbre_elemnts > nbre_iterations )
                        {
                            self -> nbre_elemnts = nbre_iterations + 1;
                            break;
                        }

                        g_free ( str_date );
                    }
                    day_courant = day;
                }
            }
            prev_montant = montant;

            if ( self -> nbre_elemnts > nbre_iterations )
            {
                self -> nbre_elemnts = nbre_iterations;
                dialogue_hint ( _("You can not exceed one year of visualization"), _("Overflow") );

                break;
            }

        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );

        tab_Y[self->nbre_elemnts-1] = prev_montant;

        g_date_free ( date_courante );

        return TRUE;
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_affiche_XY_line ( struct_bet_graph_data *self )
{
    GogPlot *cur_plot;
    GogSeries *series;
    GOData *data;
    GOStyle *style;
    GogObject *axis;
    GogObject *axis_line = NULL;
    GogObject *grid_line;
    GError *error = NULL;
    gchar *position;

    devel_debug (NULL);

    /* Afficher données dans le graphique */
    cur_plot = self->plot;

    /* on met en vertical les libellés de l'axe X */
    axis = gog_object_get_child_by_name ( GOG_OBJECT ( self->chart ), "X-Axis" );
    style = go_styled_object_get_style ( GO_STYLED_OBJECT ( axis ) );
    go_style_set_text_angle ( style, prefs_lines->degrees );

    /* on positionne l'axe des x sur le 0 par défaut */
    switch ( prefs_lines->position )
    {
    case 0:
        position = "low";
        break;
    case 1:
        position = "high";
        break;
    case 2:
        position = "cross";
        break;
    default :
        position = "cross";
    }
    g_object_set ( G_OBJECT ( axis ),
                        "pos-str", position,
                        "cross-axis-id", gog_object_get_id ( GOG_OBJECT ( axis ) ),
                        NULL );

    g_object_set ( G_OBJECT ( axis ),
                        "major-tick-in", prefs_lines->major_tick_in,
                        "major-tick-out", prefs_lines->major_tick_out,
                        "minor-tick-out", prefs_lines->major_tick_out,
                        "major-tick-labeled", prefs_lines->major_tick_labeled,
                        NULL);

    /* add a line which crosses the Y axis at 0 */
    if ( prefs_lines->new_axis_line )
    {
        axis_line = gog_object_add_by_name ( axis, "AxisLine", NULL );
        g_object_set ( G_OBJECT ( axis_line ),
                        "pos-str", "cross",
                        "cross-axis-id", gog_object_get_id ( GOG_OBJECT ( axis_line ) ),
                        "major-tick-out", FALSE,
                        "major-tick-labeled", FALSE,
                        NULL );
    }

    if ( prefs_lines->type_graph == 0 )
    {
        /* affichage de la grille au dessus ou en dessous des barres */
        /* sert aussi à afficher les étiquettes au dessus des barres à cause d'un bug de goffice */
        if ( prefs_lines->before_grid )
            g_object_set ( G_OBJECT ( self->plot ),
                        "before-grid", TRUE,
                        NULL );

        /* modification du ratio de largeur des barres. 0 pas d'espace entre deux barres */
        g_object_set ( G_OBJECT ( self->plot ),
                        "gap-percentage", prefs_lines->gap_spinner,
                        NULL );
    }

    /* on ajoute une grille pour l'axe Y si necessaire */
    if ( prefs_lines->major_grid_y || prefs_lines->minor_grid_y )
    {
        axis = gog_object_get_child_by_name ( GOG_OBJECT ( self->chart ), "Y-Axis" );

        if ( prefs_lines->major_grid_y )
        {
            grid_line = gog_object_add_by_name ( GOG_OBJECT ( axis ), "MajorGrid", NULL );

            if ( prefs_lines->minor_grid_y )
                grid_line = gog_object_add_by_name ( GOG_OBJECT ( axis ), "MinorGrid", NULL );
        }
    }

    /* on met à jour les données à afficher */
    series = GOG_SERIES ( gog_plot_new_series ( GOG_PLOT ( cur_plot ) ) );

    data = go_data_vector_str_new ( (const char * const*) self -> tab_vue_libelle, self -> nbre_elemnts, NULL );
    gog_series_set_dim (series, 0, data, &error);
    data = go_data_vector_val_new ( self->tab_Y, self->nbre_elemnts, NULL);
    gog_series_set_dim (series, 1, data, &error);

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_graph_button_menu_new ( GsbButtonStyle style,
                        const gchar *type_graph,
                        GCallback callback,
                        GtkWidget *tree_view )
{
    GtkWidget *arrow_button = NULL;
    GtkWidget *box;
    GtkWidget *box_button;
    GList *liste = NULL;
    struct_bet_graph_button *self;

    /* Initialisation d'un nouveau GtkBuilder */
    if ( !bet_graph_initialise_builder ( ) )
        return NULL;

    /* boite qui contiendra les deux boutons */
    box = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "button_menu_box" ) );

    /* boite qui contiendra le bouton actif */
    box_button = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "box_button" ) );

    if ( strcmp ( type_graph, "forecast_graph" ) == 0 )
    {
        self = struct_initialise_bet_graph_button ( );

        self->name = g_strdup ( _("Column") );
        self->filename = g_strdup ( "graph-histo.png" );
        self->service_id = g_strdup ( "GogBarColPlot" );
        self->callback = callback;
        self->box = box_button;
        self->tree_view = tree_view;

        if ( prefs_lines->type_graph == 0 )
        {
            self->is_visible = TRUE;
            self->button = gsb_automem_imagefile_button_new ( style,
                        self->name,
                        self->filename,
                        NULL,
                        NULL );

            g_object_set_data ( G_OBJECT ( self->button ), "service_id", self->service_id );
            g_signal_connect ( G_OBJECT ( self->button ),
                        "clicked",
                        self->callback,
                        self->tree_view );
            gtk_box_pack_start ( GTK_BOX ( self->box ), self->button, TRUE, TRUE, 0 );
        }
        liste = g_list_append ( liste, self );

        self = struct_initialise_bet_graph_button ( );

        self->name = g_strdup ( _("Line") );
        self->filename = g_strdup ( "graph-line.png" );
        self->service_id = g_strdup ( "GogLinePlot" );
        self->callback = callback;
        self->box = box_button;
        self->tree_view = tree_view;

        if ( prefs_lines->type_graph == 1 )
        {
            self->is_visible = TRUE;

            self->button = gsb_automem_imagefile_button_new ( style,
                        self->name,
                        self->filename,
                        NULL,
                        NULL );

            g_object_set_data ( G_OBJECT ( self->button ), "service_id", self->service_id );
            g_signal_connect ( G_OBJECT ( self->button ),
                        "clicked",
                        self->callback,
                        self->tree_view );
            gtk_box_pack_start ( GTK_BOX ( self->box ), self->button, TRUE, TRUE, 0 );
        }
        liste = g_list_append ( liste, self );
    }

    /* on attache la liste des boutons servira  plus tard */
    g_object_set_data ( G_OBJECT ( box ), "button_list", liste );

    /* bouton qui ouvre le menu de choix du bouton actif */
    arrow_button = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "arrow_button" ) );
    g_signal_connect ( G_OBJECT ( arrow_button ),
                        "button-press-event",
                        G_CALLBACK ( bet_graph_popup_choix_graph_menu ),
                        liste );

    return box;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_popup_choix_graph_menu ( GtkWidget *button,
                        GdkEventButton *event,
                        GList *liste )
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    GtkWidget *icon;
    GList *tmp_list;

    menu = gtk_menu_new ();

    tmp_list = liste;
    
    while (tmp_list)
    {
        struct_bet_graph_button *self;

        self = tmp_list -> data;

        icon = gtk_image_new_from_file ( g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), self->filename, NULL ) );
        menu_item = gtk_image_menu_item_new_with_label  ( self->name );
        gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ), icon );
        g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_graph_popup_choix_graph_activate ),
                        self );
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        /* on rend insensible le bouton visible */
        gtk_widget_set_sensitive ( menu_item, !self->is_visible );

        tmp_list = tmp_list -> next;
    }

    gtk_widget_show_all ( menu );
    gtk_menu_popup ( GTK_MENU ( menu ),
                        NULL, button,
                        set_popup_position,
                        button,
                        1,
                        gtk_get_current_event_time ( ) );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
void bet_graph_popup_choix_graph_activate ( GtkMenuItem *menuitem,
                        struct_bet_graph_button *self )
{
    GtkWidget *parent;
    GList *tmp_list;

    parent = gtk_widget_get_parent ( self->box );
    tmp_list = g_object_get_data ( G_OBJECT ( parent ), "button_list" );
    
    while ( tmp_list )
    {
        struct_bet_graph_button *self;

        self = tmp_list -> data;

        if ( self->is_visible == TRUE )
        {
            self->is_visible = FALSE;
            break;
        }

        tmp_list = tmp_list -> next;
    }

    gtk_container_foreach ( GTK_CONTAINER ( self->box ), ( GtkCallback ) gtk_widget_destroy, NULL );

    self->is_visible = TRUE;
    self->button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
                        self->name,
                        self->filename,
                        NULL,
                        NULL );

    g_object_set_data ( G_OBJECT ( self->button ), "service_id", self->service_id );
    g_signal_connect ( G_OBJECT ( self->button ),
                        "clicked",
                        self->callback,
                        self->tree_view );

    gtk_box_pack_start ( GTK_BOX ( self->box ), self->button, TRUE, TRUE, 0 );

    gsb_file_set_modified ( TRUE );
}


/**
 * Création de la page de préférences pour les graphiques lignes
 *
 *
 *
 * */
GtkWidget *bet_graph_create_line_preferences ( struct_bet_graph_data *self )
{
    GtkWidget *box_prefs_line;
    GtkWidget *box_options_col;
    GtkWidget *rot_align;
    GtkWidget *widget;
    GtkWidget *button_1;
    GtkWidget *button_2;
    GtkWidget *button_3;
    GtkWidget *rotation;

    box_prefs_line = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "box_prefs_line") );
    box_options_col = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "box_options_col") );
    rot_align = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "rot_align") );

    /* définition du type de graphique */
    if ( strcmp ( self->service_id, "GogLinePlot" ) == 0 )
        prefs_lines->type_graph = 1;
    else if ( strcmp ( self->service_id, "GogPiePlot" ) == 0 )
        prefs_lines->type_graph = -1;
    else
        prefs_lines->type_graph = 0;

    /* configuration de l'axe X */
    /* configure les options d'affichage de l'axe X */
    widget = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "major_tick_out" ) );
    if ( prefs_lines->major_tick_out )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );

    g_object_set_data ( G_OBJECT ( widget ), "rang", GINT_TO_POINTER ( 0 ) );
    g_signal_connect ( widget,
                        "toggled",
                        G_CALLBACK ( bet_graph_toggle_button_changed ),
                        self );

    widget = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "major_tick_in" ) );
    if ( prefs_lines->major_tick_in )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );

    g_object_set_data ( G_OBJECT ( widget ), "rang", GINT_TO_POINTER ( 1 ) );
    g_signal_connect ( widget,
                        "toggled",
                        G_CALLBACK ( bet_graph_toggle_button_changed ),
                        self );

    widget = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "major_tick_labeled" ) );
    if ( prefs_lines->major_tick_labeled )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );

    g_object_set_data ( G_OBJECT ( widget ), "rang", GINT_TO_POINTER ( 2 ) );
    g_signal_connect ( widget,
                        "toggled",
                        G_CALLBACK ( bet_graph_toggle_button_changed ),
                        self );

    /* Configuration de la position de l'axe des X */
    button_1 = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "axis_low" ) );
    if ( prefs_lines->position == 0 )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_1 ), TRUE );
    g_object_set_data ( G_OBJECT ( button_1 ), "rang", GINT_TO_POINTER ( 3 ) );

    button_2 = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "axis_high" ) );
    if ( prefs_lines->position == 1 )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (button_2 ), TRUE );
    g_object_set_data ( G_OBJECT ( button_2 ), "rang", GINT_TO_POINTER ( 4 ) );

    button_3 = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "axis_cross" ) );
    g_object_set_data ( G_OBJECT ( button_3 ), "rang", GINT_TO_POINTER ( 5 ) );

    widget = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "new_axis_line" ) );
    g_object_set_data ( G_OBJECT ( widget ), "rang", GINT_TO_POINTER ( 6 ) );

    /* on lie les deux boutons pour les retrouver plus tard */
    g_object_set_data ( G_OBJECT ( widget ), "other_axis", button_3 );
    g_object_set_data ( G_OBJECT ( button_3 ), "other_axis", widget );

    if ( prefs_lines->new_axis_line )
    {
        gtk_widget_set_sensitive ( button_3, FALSE );
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
    }

    if ( prefs_lines->position == 2 )
    {
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_3 ), TRUE );
        gtk_widget_set_sensitive ( widget, FALSE );
    }

    /* set the signal */
    g_signal_connect ( button_1,
                        "toggled",
                        G_CALLBACK ( bet_graph_toggle_button_changed ),
                        self );

    g_signal_connect ( button_2,
                        "toggled",
                        G_CALLBACK ( bet_graph_toggle_button_changed ),
                        self );

    g_signal_connect ( button_3,
                        "toggled",
                        G_CALLBACK ( bet_graph_toggle_button_changed ),
                        self );

    g_signal_connect ( widget,
                        "toggled",
                        G_CALLBACK ( bet_graph_toggle_button_changed ),
                        self );

    /* configure l'orientation des étiquettes de l'axe X */
    rotation = go_rotation_sel_new ( );
    go_rotation_sel_set_rotation (GO_ROTATION_SEL ( rotation ), prefs_lines->degrees );
    g_signal_connect ( G_OBJECT ( rotation ),
                        "rotation-changed",
                        G_CALLBACK ( bet_graph_rotation_changed ),
                        self );

    gtk_container_add ( GTK_CONTAINER ( rot_align ), rotation );

    if ( prefs_lines->type_graph == 0 )
    {
        /* configure la visibilité de la grille et provisoirement des étiquettes de l'axe X (bug goffice) */
        widget = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "before_grid" ) );
        g_object_set_data ( G_OBJECT ( widget ), "rang", GINT_TO_POINTER ( 7 ) );
        if ( prefs_lines->before_grid )
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        g_signal_connect ( widget,
                        "toggled",
                        G_CALLBACK ( bet_graph_toggle_button_changed ),
                        self );

        /* configure la valeur de la largeur des colonnes du graph */
        widget = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "gap_spinner" ) );
        if ( prefs_lines->gap_spinner )
            gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( widget ), prefs_lines->gap_spinner );
        g_signal_connect ( widget,
                        "value-changed",
                        G_CALLBACK ( bet_graph_gap_spinner_changed ),
                        self );
    }
    else
        gtk_widget_hide ( box_options_col );

    /* configuration de l'axe Y */
    /* onfiguration de la grille */
    button_1 = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "major_grid_y" ) );
    g_object_set_data ( G_OBJECT ( button_1 ), "rang", GINT_TO_POINTER ( 8 ) );
    g_signal_connect ( button_1,
                        "toggled",
                        G_CALLBACK ( bet_graph_toggle_button_changed ),
                        self );

    button_2 = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "minor_grid_y" ) );
    if ( prefs_lines->minor_grid_y )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (button_2 ), TRUE );

    g_object_set_data ( G_OBJECT ( button_2 ), "rang", GINT_TO_POINTER ( 9 ) );
    g_signal_connect ( button_2,
                        "toggled",
                        G_CALLBACK ( bet_graph_toggle_button_changed ),
                        self );

    /* on lie les deux boutons pour les retrouver plus tard */
    g_object_set_data ( G_OBJECT ( button_1 ), "grid_y", button_2 );

    if ( prefs_lines->major_grid_y )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_1 ), TRUE );
    else
        gtk_widget_set_sensitive ( button_2, FALSE );


    return box_prefs_line;
}


/**
 *
 *
 *
 *
 * */
void bet_graph_toggle_button_changed ( GtkToggleButton *togglebutton,
                        struct_bet_graph_data *self )
{
    GtkWidget *button;
    gint rang;
    gboolean active;

    active = gtk_toggle_button_get_active ( togglebutton );

    rang = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( togglebutton ), "rang" ) );

    switch ( rang )
    {
    case 0:
        prefs_lines->major_tick_out = active;
        break;
    case 1:
        prefs_lines->major_tick_in = active;
        break;
    case 2:
        prefs_lines->major_tick_labeled = active;
        break;
    case 3:
        prefs_lines->position = 0;
        break;
    case 4:
        prefs_lines->position = 1;
        break;
    case 5:
        prefs_lines->position = 2;
        button = g_object_get_data ( G_OBJECT ( togglebutton ), "other_axis" );
        gtk_widget_set_sensitive ( button, !active );
        break;
    case 6:
        prefs_lines->new_axis_line = active;
        button = g_object_get_data ( G_OBJECT ( togglebutton ), "other_axis" );
        gtk_widget_set_sensitive ( button, !active );
        break;
    case 7:
        prefs_lines->before_grid = active;
        break;
    case 8:
        prefs_lines->major_grid_y = active;
        button = g_object_get_data ( G_OBJECT ( togglebutton ), "grid_y" );
        gtk_widget_set_sensitive ( button, active );
        break;
    case 9:
        prefs_lines->minor_grid_y = active;
        break;
    }

    /* on met à jour le graph */
    bet_graph_update_graph ( self );
}


/**
 *
 *
 *
 *
 * */
void bet_graph_rotation_changed ( GORotationSel *rotation,
                        int angle,
                        struct_bet_graph_data *self )
{
    prefs_lines->degrees = angle;

    /* on met à jour le graph */
    bet_graph_update_graph ( self );
}


/**
 *
 *
 *
 *
 * */
void bet_graph_gap_spinner_changed ( GtkSpinButton *spinbutton,
                        struct_bet_graph_data *self )
{
    prefs_lines->gap_spinner = (gint) gtk_spin_button_get_value ( spinbutton );

    /* on met à jour le graph */
    bet_graph_update_graph ( self );
}


/**
 *
 *
 *
 *
 * */
void bet_graph_update_graph ( struct_bet_graph_data *self )
{
    devel_debug (NULL);

    gtk_notebook_remove_page ( self->notebook, 0 );

    self->plot = bet_graph_create_graph_page ( self, TRUE );
    gtk_widget_show_all ( GTK_WIDGET ( self->notebook ) );

    bet_graph_affiche_XY_line ( self );

    gsb_file_set_modified ( TRUE );
}


/**
 *  sérialise les préférences des graphes
 *
 *
 * \return a newly allocated string which must be freed when no more used.
 * */
gchar *bet_graph_get_configuration_string ( void )
{
    gchar *new_str = NULL;

    if ( prefs_lines == NULL )
        return NULL;

    new_str = g_markup_printf_escaped ( "\t<Bet_graph prefs=\"%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d\" />\n",
                        prefs_lines->type_graph,
                        prefs_lines->major_tick_out,
                        prefs_lines->major_tick_in,
                        prefs_lines->major_tick_labeled,
                        prefs_lines->position,
                        prefs_lines->new_axis_line,
                        prefs_lines->cross_entry,
                        prefs_lines->degrees,
                        prefs_lines->gap_spinner,
                        prefs_lines->before_grid,
                        prefs_lines->major_grid_y,
                        prefs_lines->minor_grid_y );

    return new_str;
}


/**
 *
 *
 *
 *
 * */
void bet_graph_set_configuration_variables ( const gchar *string )
{
    gchar **tab;

    /* initialisation des préférences si nécessaire */
    if ( prefs_lines == NULL )
        prefs_lines = struct_initialise_bet_graph_prefs ( );

    tab = g_strsplit ( string, ":", 0 );

    prefs_lines->type_graph = utils_str_atoi ( tab[0] );
    prefs_lines->major_tick_out = utils_str_atoi ( tab[1] );
    prefs_lines->major_tick_in = utils_str_atoi ( tab[2] );
    prefs_lines->major_tick_labeled = utils_str_atoi ( tab[3] );
    prefs_lines->position = utils_str_atoi ( tab[4] );
    prefs_lines->new_axis_line = utils_str_atoi ( tab[5] );
    prefs_lines->cross_entry = utils_str_atoi ( tab[6] );
    prefs_lines->degrees = utils_str_atoi ( tab[7] );
    prefs_lines->gap_spinner = utils_str_atoi ( tab[8] );
    prefs_lines->before_grid = utils_str_atoi ( tab[9] );
    prefs_lines->major_grid_y = utils_str_atoi ( tab[10] );
    prefs_lines->minor_grid_y = utils_str_atoi ( tab[11] );
}


/**
 *
 *
 *
 *
 * */
struct_bet_graph_button *struct_initialise_bet_graph_button ( void )
{
    struct_bet_graph_button *self;

    self = g_new0 ( struct_bet_graph_button, 1 );

   return self;
}


void struct_free_bet_graph_button ( struct_bet_graph_button *self )
{
    g_free ( self->name );
    g_free ( self->filename );
    g_free ( self->service_id );

    g_free ( self );
}


/**
 *
 *
 *
 *
 * */
struct_bet_graph_data *struct_initialise_bet_graph_data ( void )
{
    struct_bet_graph_data *self;
    gint i;

    self = g_new0 ( struct_bet_graph_data, 1 );

    self -> tab_vue_libelle = g_malloc ( MAX_POINTS_GRAPHIQUE * sizeof ( gchar* ) );

    for ( i = 0; i < MAX_POINTS_GRAPHIQUE; i++ )
    {
        self -> tab_vue_libelle[i] = self -> tab_libelle[i];
    }

   return self;
}


/**
 *
 *
 *
 *
 * */
void struct_free_bet_graph_data ( struct_bet_graph_data *self )
{
    g_free ( self->title );
    g_free ( self->service_id );
    g_free ( self->tab_vue_libelle );

    g_free ( self );
}


/**
 *
 *
 *
 *
 * */
struct_bet_graph_prefs *struct_initialise_bet_graph_prefs ( void )
{
    struct_bet_graph_prefs *self;

    self = g_new0 ( struct_bet_graph_prefs, 1 );

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
 *
 *
 * */
void struct_free_bet_graph_prefs ( struct_bet_graph_prefs *self )
{
    g_free ( self );
}


/**
 * free the gtk_builder
 *
 *
 *
 * */
void bet_graph_free_builder ( void )
{
    g_object_unref ( G_OBJECT ( bet_graph_builder ) );
}


#endif /* HAVE_GOFFICE */

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
