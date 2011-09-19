/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2011 Pierre Biava (grisbi@pierre.biava.name)             */
/*                   2011 Guillaume Verger                                    */
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
#include "gsb_data_account.h"
#include "gsb_data_fyear.h"
#include "navigation.h"
#include "structures.h"
#include "utils_gtkbuilder.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/



typedef struct _struct_bet_sectors_data     struct_bet_sectors_data;

struct _struct_bet_sectors_data
{
    GtkTreeView *tree_view;
    gint account_number;

    /* Données pour les secteurs */
    GtkWidget *pie;
    gchar *title;
    gint type_infos;    /* 0 type crédit ou < 0, 1 type débit ou >= 0, -1 tous types */
    gint nbre_elemnts;
    gdouble tab_montant[MAX_SEGMENT_CAMEMBERT];
    gchar tab_libelle[MAX_SEGMENT_CAMEMBERT][TAILLE_MAX_LIBELLE];
    gchar **tab_vue_libelle;
};



/*START_STATIC*/
static gboolean bet_graph_affiche_camemberts ( struct_bet_sectors_data *self );
static void bet_graph_create_pie_canvas  ( struct_bet_sectors_data *self );
static GtkWidget *bet_graph_get_canvas  ( void );
static gboolean bet_graph_populate_sectors_by_hist_data ( struct_bet_sectors_data *self );
static  void struct_free_bet_sectors_data ( struct_bet_sectors_data *self );
struct_bet_sectors_data *struct_initialise_bet_sectors_data ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

static GtkBuilder *bet_graph_builder = NULL;


/**
 *
 *
 *
 *
 * */
void bet_graph_sectors_graph_new ( GtkWidget *menu_item, GtkTreeView *tree_view )
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *sw_canvas;
    GtkWidget *box_pie;
    gchar *filename;
    gchar *title;
    gint result;
    gint account_number;
    GError *error = NULL;
    struct_bet_sectors_data *self_credit;
    struct_bet_sectors_data *self_debit;

    devel_debug (NULL);

    /* Creation d'un nouveau GtkBuilder */
    bet_graph_builder = gtk_builder_new ( );
    if ( bet_graph_builder == NULL )
        return;

    /* Chargement du XML dans bet_graph_builder */
    filename = utils_gtkbuilder_get_full_path ( "bet_graph.ui" );
    if ( !g_file_test ( filename, G_FILE_TEST_EXISTS ) )
    {
        gchar* tmpstr = g_strdup_printf ( _("Cannot open file '%s': %s"),
                        filename,
                        _("File does not exist") );
        dialogue_error ( tmpstr );
        g_free ( tmpstr );
        g_free ( filename );
        return;
    }

    result = gtk_builder_add_from_file ( bet_graph_builder, filename, &error );
    if ( result == 0 )
    {
        g_error ("%s", error->message);
        g_free ( filename );
        g_error_free ( error );

        return;
    }
    g_free ( filename );

    /* initialisation des structures de données */
    account_number = gsb_gui_navigation_get_current_account ( );

    self_credit = struct_initialise_bet_sectors_data ( );
    self_credit -> tree_view = tree_view;
    self_credit -> account_number = account_number;
    self_credit -> type_infos = 0;

    self_debit = struct_initialise_bet_sectors_data ( );
    self_debit -> tree_view = tree_view;
    self_debit -> account_number = account_number;
    self_debit -> type_infos = 1;

    /* Création de la fenêtre de dialogue pour le graph */
    dialog = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "bet_graph_dialog" ) );
    gtk_window_set_transient_for ( GTK_WINDOW ( dialog ), GTK_WINDOW ( run.window ) );
    gtk_widget_set_usize ( dialog, PAGE_WIDTH+30, PAGE_HEIGHT+70 );
    gtk_signal_connect ( GTK_OBJECT ( dialog ),
                        "destroy",
		                GTK_SIGNAL_FUNC ( gtk_widget_destroy ),
                        NULL);

    sw_canvas = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "sw_canvas" ) );
    box_pie = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "box_pie" ) );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( sw_canvas ), box_pie );

    /* set the title */
    label = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( tree_view ), "label_title") );
    title = make_pango_attribut ( "weight=\"bold\" size=\"x-large\"",
                    gtk_label_get_text ( GTK_LABEL ( label ) ) );

    label = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "label_canvas" ) );
    gtk_label_set_markup ( GTK_LABEL ( label ), title );
    g_free ( title );
    
    /* Set the graph for credit */
    self_credit -> pie = go_graph_widget_new ( NULL );
    self_credit -> title = g_strdup ( _("Credit") );
    bet_graph_create_pie_canvas ( self_credit );
    gtk_box_pack_start ( GTK_BOX ( box_pie ), self_credit -> pie, TRUE, TRUE, 0 );

    /* Set the graph for debit */
    self_debit -> pie = go_graph_widget_new ( NULL );
    self_debit -> title = g_strdup ( _("Debit") );
    bet_graph_create_pie_canvas ( self_debit );
    gtk_box_pack_start ( GTK_BOX ( box_pie ), self_debit -> pie, TRUE, TRUE, 0 );

    /* populate and show the data for credit */
    result = bet_graph_populate_sectors_by_hist_data ( self_credit );
    if ( ! result )
        return;
    result = bet_graph_affiche_camemberts ( self_credit );
    if ( ! result )
        return;

    /* populate and show the data for debit */
    result = bet_graph_populate_sectors_by_hist_data ( self_debit );
    if ( ! result )
        return;
   result = bet_graph_affiche_camemberts ( self_debit );
    if ( ! result )
        return;

    gtk_widget_show_all ( dialog );

    result = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    /* free the data */
    struct_free_bet_sectors_data ( self_debit );
    struct_free_bet_sectors_data ( self_credit );
    gtk_widget_destroy ( dialog );
}


/**
 *
 *
 *
 *
 * */
void bet_graph_create_pie_canvas  ( struct_bet_sectors_data *self )
{
	GtkWidget *w;
	GogChart *chart;
	GogGraph *graph;
	GogLabel *label;
	GogPlot *pie;
	GogSeries *series;
	GOStyle *style;
	GOData *data;
	GError *error = NULL;
	PangoFontDescription *desc;

	/* Set the graph widget */
	w = self -> pie;

	/* Get the embedded graph */
	graph = go_graph_widget_get_graph ( GO_GRAPH_WIDGET ( w ) );

	label = (GogLabel *) g_object_new ( GOG_TYPE_LABEL, NULL );
	data = go_data_scalar_str_new ( self -> title, FALSE );
	gog_dataset_set_dim ( GOG_DATASET ( label ), 0, data, NULL );
	gog_object_add_by_name ( GOG_OBJECT ( graph ), "Title", GOG_OBJECT ( label ) );

	/* Change the title font */
	style = go_styled_object_get_style ( GO_STYLED_OBJECT ( label ) );
	desc = pango_font_description_from_string ( "Sans bold 12" );
	go_style_set_font_desc ( style, desc );

	/* Get the chart created by the widget initialization */
	chart = go_graph_widget_get_chart ( GO_GRAPH_WIDGET ( w ) );

	/* Create a pie plot and add it to the chart */
	pie = ( GogPlot *) gog_plot_new_by_name ( "GogPiePlot" );
	gog_object_add_by_name ( GOG_OBJECT ( chart ), "Plot", GOG_OBJECT ( pie ) );

	/* Add a legend to the chart */
	gog_object_add_by_name ( GOG_OBJECT ( chart ), "Legend", NULL);

	/* Create a series for the plot and populate it with some simple data */
	series = gog_plot_new_series ( pie );
	data = go_data_vector_str_new ( NULL, 0, NULL );
	gog_series_set_dim ( series, 0, data, &error );
    if ( error != NULL )
    {
        g_error ("gog_series_set_dim : erreur = %s\n", error -> message);
        g_error_free ( error );
        error = NULL;

        return;
    }

    data = go_data_vector_val_new ( NULL, 0, NULL );
	gog_series_set_dim ( series, 1, data, &error );
    if ( error )
    {
        g_error ("gog_series_set_dim : erreur = %s\n", error -> message);
        g_error_free ( error );
        error = NULL;
    }
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_populate_sectors_by_hist_data ( struct_bet_sectors_data *self )
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
        gdouble *tab_montant_division = self -> tab_montant;

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
            gint sub_div;
            gint type_infos;

            gtk_tree_model_get ( GTK_TREE_MODEL( model ),
                        &iter,
                        SPP_HISTORICAL_DESC_COLUMN, &desc,
                        SPP_HISTORICAL_RETAINED_AMOUNT, &amount,
                        SPP_HISTORICAL_DIV_NUMBER, &div,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div,
                        -1 );

            type_infos = bet_data_get_div_type ( div );
            if ( self -> type_infos == -1 || type_infos == self -> type_infos )
            {
                if ( self -> nbre_elemnts >= MAX_SEGMENT_CAMEMBERT )
                    continue;

                strncpy ( &libelle_division[self -> nbre_elemnts * TAILLE_MAX_LIBELLE], desc, TAILLE_MAX_LIBELLE );
                tab_montant_division[self -> nbre_elemnts] = utils_str_strtod ( ( amount == NULL) ? "0" : amount, NULL );

                self -> nbre_elemnts++;
            }

            if ( self -> nbre_elemnts > MAX_SEGMENT_CAMEMBERT )
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
gboolean bet_graph_affiche_camemberts ( struct_bet_sectors_data *self )
{
    GogChart *chart;
    GogObject *cur_plot;
    GogSeries *series;
    GOData *data;
    GError * error=NULL;

    /* Afficher les montants des divisions de type débit dans le camembert */
    chart = go_graph_widget_get_chart ( GO_GRAPH_WIDGET ( self -> pie ) );
    cur_plot = gog_object_get_child_by_name ( GOG_OBJECT ( chart ), "Plot" );

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

    data = go_data_vector_val_new ( self -> tab_montant, self -> nbre_elemnts, NULL);
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
struct_bet_sectors_data *struct_initialise_bet_sectors_data ( void )
{
    struct_bet_sectors_data *self;
    gint i;

    self = g_new0 ( struct_bet_sectors_data, 1 );

    self -> tab_vue_libelle = g_malloc ( MAX_SEGMENT_CAMEMBERT * sizeof ( gchar* ) );

    for ( i = 0; i < MAX_SEGMENT_CAMEMBERT; i++ )
    {
        self -> tab_vue_libelle[i] = self -> tab_libelle[i];
    }

    return self;
}


 void struct_free_bet_sectors_data ( struct_bet_sectors_data *self )
{

    g_free ( self -> title );
    g_free ( self -> tab_vue_libelle );

    g_free ( self );

}


#endif /* HAVE_GOFFICE */

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
