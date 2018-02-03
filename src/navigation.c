/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
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
#include "config.h"
#endif

#include "include.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "navigation.h"
#include "accueil.h"
#include "bet_data.h"
#include "bet_finance_ui.h"
#include "categories_onglet.h"
#include "dialog.h"
#include "etats_onglet.h"
#include "fenetre_principale.h"
#include "grisbi_win.h"
#include "gsb_account.h"
#include "gsb_account_property.h"
#include "gsb_assistant_account.h"
#include "gsb_calendar.h"
#include "gsb_data_account.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_import_rule.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_report.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_form_scheduler.h"
#include "gsb_form_widget.h"
#include "gsb_real.h"
#include "gsb_reconcile.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "imputation_budgetaire.h"
#include "menu.h"
#include "metatree.h"
#include "mouse.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "transaction_list_select.h"
#include "transaction_list_sort.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static void gsb_gui_navigation_activate_expander ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreePath *path,
                        gpointer user_data );
static gboolean gsb_gui_navigation_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null );
static gboolean gsb_gui_navigation_check_key_press ( GtkWidget *tree_view,
                        GdkEventKey *ev,
                        GtkTreeModel *model );
static void gsb_gui_navigation_clear_pages_list ( void );
static void gsb_gui_navigation_context_menu ( GtkWidget *tree_view,
                        GtkTreePath *path );
static void gsb_gui_navigation_create_report_list ( GtkTreeModel *model );
static GtkTreePath *gsb_gui_navigation_get_page_path ( GtkTreeModel *model,
                        gint type_page );
static gboolean gsb_gui_navigation_move_ordre ( gint src_ordre,
                        gint dst_ordre );
static gboolean gsb_gui_navigation_remove_account_iterator ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
static gboolean gsb_gui_navigation_remove_report_iterator ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
static gboolean gsb_gui_navigation_select_line ( GtkTreeSelection *selection,
                        GtkTreeModel *model );
static void gsb_gui_navigation_set_selection_branch ( GtkTreeSelection *selection,
					    GtkTreeIter * iter, gint page,
					    gint account_number, gint report_number );
static void gsb_gui_navigation_set_navigation_pages ( GtkTreeModel *model,
                        gint type_page,
                        gint ordre );
static void gsb_gui_navigation_update_account_iter ( GtkTreeModel *model,
                        GtkTreeIter * account_iter,
                        gint account_number );
static gboolean gsb_gui_navigation_update_account_iterator ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
static void gsb_gui_navigation_update_report_iter ( GtkTreeModel *model,
                        GtkTreeIter * report_iter,
                        gint report_number );
static gboolean gsb_gui_navigation_update_report_iterator ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
static gboolean navigation_sort_column ( GtkTreeModel *model,
                        GtkTreeIter *a,
                        GtkTreeIter *b,
                        gpointer user_data );
static gboolean navigation_tree_drag_data_get ( GtkTreeDragSource *drag_source,
                        GtkTreePath *path,
                        GtkSelectionData *selection_data );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *label_last_statement;
extern GtkWidget *menu_import_rules;
/*END_EXTERN*/


/** Holds data for the navigation tree.  */
enum navigation_cols {
    NAVIGATION_PIX,
    NAVIGATION_PIX_VISIBLE,
    NAVIGATION_TEXT,
    NAVIGATION_FONT,
    NAVIGATION_PAGE,
    NAVIGATION_ACCOUNT,
    NAVIGATION_REPORT,
    NAVIGATION_SENSITIVE,
    NAVIGATION_ORDRE,        /* ordre des pages dans le modèle */
    NAVIGATION_TOTAL,
};

/** Navigation tree view. */
static GtkWidget *navigation_tree_view = NULL;

/** Model of the navigation tree. */
static GtkTreeModel *navigation_model = NULL;

/** Widget that hold the scheduler calendar. */
static GtkWidget *scheduler_calendar = NULL;

/** Widget that hold all reconciliation widgets. */
GtkWidget *reconcile_panel;

/* contains the number of the last account
 * when switching between 2 accounts
 * at the end of the switch, contains the current account number */
static gint buffer_last_account = -1;

/* contains a g_queue of struct_page */
static GQueue *pages_list = NULL;

/* nombre de pages du panneau de gauche */
static gint navigation_nbre_pages = 7;

/* ordre par défaut des pages du panneau de gauche */
const gchar *default_navigation_order_list = "0-2-3-4-5-6-7";

/* gestion des expandeurs */
static gboolean account_expander = TRUE;
static gboolean report_expander = FALSE;

static GtkTargetEntry row_targets[] =
{
	{ "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
};


/**
 * Create the navigation pane on the left of the GUI.  It contains
 * account list as well as shortcuts.
 *
 * \return The newly allocated pane.
 */
GtkWidget *gsb_gui_navigation_create_navigation_pane ( void )
{
    GtkWidget *grid;
    GtkWidget *sw;
    GQueue *tmp_queue;
    GtkCellRenderer *renderer;
    GtkTreeDragDestIface *navigation_dst_iface;
    GtkTreeDragSourceIface *navigation_src_iface;
    GtkTreeViewColumn *column;
    gint i;
    gint xpad;
    gint ypad;

    grid = gtk_grid_new ();

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);

    /* Create the view */
    navigation_tree_view = gtk_tree_view_new ();
	gtk_widget_set_name (navigation_tree_view, "tree_view");
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW(navigation_tree_view), FALSE );
    gtk_container_add ( GTK_CONTAINER(sw), navigation_tree_view );

    navigation_model = GTK_TREE_MODEL ( gtk_tree_store_new ( NAVIGATION_TOTAL,
							    GDK_TYPE_PIXBUF,        /* NAVIGATION_PIX */
							    G_TYPE_BOOLEAN,         /* NAVIGATION_PIX_VISIBLE */
                                G_TYPE_STRING,          /* NAVIGATION_TEXT */
							    G_TYPE_INT,             /* NAVIGATION_FONT */
                                G_TYPE_INT,             /* NAVIGATION_PAGE */
							    G_TYPE_INT,             /* NAVIGATION_ACCOUNT */
                                G_TYPE_INT,             /* NAVIGATION_REPORT */
							    G_TYPE_STRING,          /* NAVIGATION_SENSITIVE */
                                G_TYPE_INT ) );         /* NAVIGATION_ORDRE */

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( navigation_model ),
                        NAVIGATION_ORDRE, GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( navigation_model ),
                        NAVIGATION_ORDRE, navigation_sort_column,
                        NULL, NULL );

    /* Enable drag & drop */
    gtk_tree_view_enable_model_drag_source ( GTK_TREE_VIEW ( navigation_tree_view ),
					    GDK_BUTTON1_MASK,
                        row_targets, 1,
					    GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_enable_model_drag_dest ( GTK_TREE_VIEW ( navigation_tree_view ),
                        row_targets,
                        1,
                        GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_set_reorderable ( GTK_TREE_VIEW ( navigation_tree_view ), TRUE );
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( navigation_tree_view ) ),
                        GTK_SELECTION_SINGLE );
    gtk_tree_view_set_model ( GTK_TREE_VIEW ( navigation_tree_view ), GTK_TREE_MODEL( navigation_model ) );
    /* FIXME, since navigation_model is a static variable, we _do_ hold a ref...
       Maybe we should be using a WeakRef? */
    g_object_unref ( G_OBJECT ( navigation_model ) );

    /* Handle drag & drop */
    navigation_dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE ( navigation_model );
    if ( navigation_dst_iface )
    {
        navigation_dst_iface -> drag_data_received = &navigation_drag_data_received;
        navigation_dst_iface -> row_drop_possible = &navigation_row_drop_possible;
    }

    navigation_src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE (navigation_model);
    if ( navigation_src_iface )
    {
        gtk_selection_add_target (navigation_tree_view,
                        GDK_SELECTION_PRIMARY,
                        GDK_SELECTION_TYPE_ATOM,
                        1);
        navigation_src_iface -> drag_data_get = &navigation_tree_drag_data_get;
    }

    /* check the keyboard before all, if we need to move other things that the navigation
     * tree view (for example, up and down on transactions list) */
    g_signal_connect ( navigation_tree_view,
                        "key-press-event",
                        G_CALLBACK ( gsb_gui_navigation_check_key_press ),
                        navigation_model );

    g_signal_connect ( navigation_tree_view,
                        "scroll-event",
                        G_CALLBACK ( gsb_gui_navigation_check_scroll ),
                        NULL );

    if ( conf.active_scrolling_left_pane == 0 )
        g_signal_handlers_block_by_func ( gsb_gui_navigation_get_tree_view ( ),
                        G_CALLBACK ( gsb_gui_navigation_check_scroll ),
                        NULL );

    g_signal_connect_after ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( navigation_tree_view ) ),
                        "changed",
                        G_CALLBACK ( gsb_gui_navigation_select_line ),
                        navigation_model );

    /* Create column */
    column = gtk_tree_view_column_new ( );

    /* Pixbuf renderer. */
    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, FALSE);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer,
				       "pixbuf", NAVIGATION_PIX);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer,
				       "visible", NAVIGATION_PIX_VISIBLE);
    gtk_tree_view_column_set_expand ( column, FALSE );

    /* Text renderer. */
    renderer = gtk_cell_renderer_text_new ();

    gtk_cell_renderer_get_padding ( GTK_CELL_RENDERER ( renderer ), &xpad, &ypad );
    gtk_cell_renderer_set_padding ( GTK_CELL_RENDERER ( renderer ), 6, ypad );
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, TRUE);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer,
				       "text", NAVIGATION_TEXT);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer,
				       "weight", NAVIGATION_FONT);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer,
				       "foreground", NAVIGATION_SENSITIVE);

    gtk_tree_view_append_column ( GTK_TREE_VIEW ( navigation_tree_view ),
				  GTK_TREE_VIEW_COLUMN ( column ) );

    /* crée les pages dans le panneau de gauche */
    tmp_queue = gsb_gui_navigation_get_pages_list ( );

    for ( i = 0 ; i < (gint) tmp_queue->length ; i++ )
    {
        struct_page *page;

        page = g_queue_peek_nth ( tmp_queue, i );
        gsb_gui_navigation_set_navigation_pages ( navigation_model, page -> type_page, i );
    }

    /* Finish tree. */
    gtk_grid_attach (GTK_GRID (grid), sw, 0,0,1,1);
	gtk_widget_set_hexpand (sw, TRUE);
	gtk_widget_set_vexpand (sw, TRUE);

    /* Create calendar (hidden for now). */
    scheduler_calendar = gsb_calendar_new ();
    gtk_grid_attach (GTK_GRID (grid), scheduler_calendar, 0,1,1,1);

    /* Create reconcile stuff (hidden for now). */
    reconcile_panel = gsb_reconcile_create_box ();
    gtk_grid_attach (GTK_GRID (grid), reconcile_panel, 0,2,1,1);

    /* signals of tree_view */
    g_signal_connect ( G_OBJECT ( navigation_tree_view ),
		                "button-press-event",
		                G_CALLBACK ( gsb_gui_navigation_button_press ),
		                NULL );

    g_signal_connect ( G_OBJECT ( navigation_tree_view ),
		                "row-collapsed",
		                G_CALLBACK ( gsb_gui_navigation_activate_expander ),
		                GINT_TO_POINTER ( 0 ) );
    g_signal_connect ( G_OBJECT ( navigation_tree_view ),
		                "row-expanded",
		                G_CALLBACK ( gsb_gui_navigation_activate_expander ),
		                GINT_TO_POINTER ( 1 ) );

    gtk_widget_show_all (grid);
    gtk_widget_hide ( scheduler_calendar );
    gtk_widget_hide ( reconcile_panel );

    return grid;
}



/**
 * return the current page selected
 * the value returned is defined by GSB_GENERAL_NOTEBOOK_PAGES
 *
 * \param
 *
 * \return a gint which is the numero of the page, -1 if problem
 *
 * */
gint gsb_gui_navigation_get_current_page ( void )
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gint page;

    if (!navigation_tree_view)
	return -1;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (navigation_tree_view));

    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
	return GSB_HOME_PAGE;

    gtk_tree_model_get ( GTK_TREE_MODEL (navigation_model),
			 &iter,
			 NAVIGATION_PAGE, &page,
			 -1);
    return page;
}



/**
 * return the account number selected
 * rem : this is only for account number of the transactions list,
 * 	if we want the current account number, for transactions or scheduled, go to
 * 	see gsb_form_get_account_number
 *
 * \param
 *
 * \return a gint, the account number or -1 if none selected
 * */
gint gsb_gui_navigation_get_current_account ( void )
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gint page;
    gint account_number;

    if ( !navigation_tree_view )
	return -1;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (navigation_tree_view));

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return -1;

    gtk_tree_model_get ( GTK_TREE_MODEL (navigation_model),
			 &iter,
			 NAVIGATION_PAGE, &page,
			 NAVIGATION_ACCOUNT, &account_number,
			 -1);

    if ( page == GSB_ACCOUNT_PAGE )
	return account_number;

    return -1;
}


/*
 * return the content of buffer_last_account
 * used while changing an account, as long as the work
 * is not finished, that variable contains the last account number
 *
 * \param
 *
 * \return the last account number (become the current account number once the
 * 		switch is finished...)
 *
 * */
gint gsb_gui_navigation_get_last_account ( void )
{
    return buffer_last_account;
}


/**
 * Return the number of the current selected report
 *
 * \param
 *
 * \return the current number of the report, or 0 if none selected
 * */
gint gsb_gui_navigation_get_current_report ( void )
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gint page;

    if ( ! navigation_tree_view )
    {
	return 0;
    }

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (navigation_tree_view));

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return 0;

    gtk_tree_model_get ( GTK_TREE_MODEL (navigation_model),
			 &iter,
			 NAVIGATION_PAGE, &page,
			 -1);

    if ( page == GSB_REPORTS_PAGE)
    {
	gint report_number;
	gtk_tree_model_get (GTK_TREE_MODEL(navigation_model), &iter, NAVIGATION_REPORT, &report_number, -1);

	return report_number;
    }

    return -1;
}




/**
 * Create a list of tree items that are shortcuts to accounts.
 *
 * \param model		Tree model to insert items into.
 * \param account_iter	Parent iter.
 */
void gsb_gui_navigation_create_account_list ( GtkTreeModel *model )
{
    GSList *list_tmp;
    GtkTreeIter parent, child;
    GtkTreePath *path;

    path = gsb_gui_navigation_get_page_path ( model, GSB_HOME_PAGE );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL( model ), &parent, path );
	gtk_tree_path_free (path);

	/* Remove childs if any. */
    while ( gtk_tree_model_iter_children ( model, &child, &parent ) )
    {
        gtk_tree_store_remove ( GTK_TREE_STORE ( model ), &child );
    }

    /* Fill in with accounts. */
    list_tmp = gsb_data_account_get_list_accounts ();
    while ( list_tmp )
    {
        gint i = gsb_data_account_get_no_account ( list_tmp -> data );

        if ( conf.show_closed_accounts ||
             ! gsb_data_account_get_closed_account ( i ) )
        {
            gsb_gui_navigation_add_account ( i, FALSE );
        }

        list_tmp = list_tmp -> next;
    }

    /* Expand stuff */
    if ( account_expander )
    {
        path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( model ), &parent );
        gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( navigation_tree_view ), path );
        gtk_tree_path_free ( path );
    }
}



/**
 * Create a list of tree items that are shortcuts to reports.
 *
 * \param model		Tree model to insert items into.
 * \param reports_iter	Parent iter.
 */
void gsb_gui_navigation_create_report_list ( GtkTreeModel *model )
{
    GSList *tmp_list;
    GtkTreeIter parent, child;
    GtkTreePath *path;

    path = gsb_gui_navigation_get_page_path ( model, GSB_REPORTS_PAGE );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL( model ), &parent, path );
    gtk_tree_path_free (path);

    /* Remove childs if any. */
    while ( gtk_tree_model_iter_children ( model, &child, &parent ) )
    {
        gtk_tree_store_remove ( GTK_TREE_STORE ( model ), &child );
    }

    /* Fill in with reports */
    tmp_list = gsb_data_report_get_report_list ();

    while ( tmp_list )
    {
        gint report_number;

        report_number = gsb_data_report_get_report_number ( tmp_list -> data );

        gtk_tree_store_append ( GTK_TREE_STORE ( model ), &child, &parent);
        gtk_tree_store_set(GTK_TREE_STORE(model), &child,
                        NAVIGATION_PIX_VISIBLE, FALSE,
                        NAVIGATION_TEXT, gsb_data_report_get_report_name ( report_number ),
                        NAVIGATION_FONT, 400,
                        NAVIGATION_PAGE, GSB_REPORTS_PAGE,
                        NAVIGATION_ACCOUNT, -1,
                        NAVIGATION_SENSITIVE, "black",
                        NAVIGATION_REPORT, report_number,
                        -1 );

        tmp_list = tmp_list -> next;
    }

    /* Expand stuff */
    if ( report_expander )
    {
        path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( model ), &parent );
        gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( navigation_tree_view ), path );
        gtk_tree_path_free ( path );
    }
}




/**
 *
 *
 *
 */
gboolean navigation_sort_column ( GtkTreeModel *model,
                        GtkTreeIter *a,
                        GtkTreeIter *b,
                        gpointer user_data )
{
    gint ordre_a, page_a, account_a, report_a;
    gint ordre_b, page_b, account_b, report_b;

    if ( ! model )
        return FALSE;

    gtk_tree_model_get ( model, a,
                        NAVIGATION_PAGE, &page_a,
                        NAVIGATION_ACCOUNT, &account_a,
                        NAVIGATION_REPORT, &report_a,
                        NAVIGATION_ORDRE, &ordre_a,
                        -1 );

    gtk_tree_model_get ( model, b,
                        NAVIGATION_PAGE, &page_b,
                        NAVIGATION_ACCOUNT, &account_b,
                        NAVIGATION_REPORT, &report_b,
                        NAVIGATION_ORDRE, &ordre_b,
                        -1 );

    if ( ordre_a < ordre_b )
        return - 1;
    if ( ordre_a > ordre_b )
        return 1;

    if ( page_a == GSB_ACCOUNT_PAGE && page_b == GSB_ACCOUNT_PAGE )
    {
        return gsb_data_account_compare_position (account_a, account_b);
    }
    else if ( page_a == GSB_REPORTS_PAGE && page_b == GSB_REPORTS_PAGE )
    {
        return gsb_data_report_compare_position (report_a, report_b);
    }
    else
        return 0;
}


/**
 * Iterator that iterates over the navigation pane model and update
 * iter of account that is equal to `data'.
 *
 * \param tree_model	Pointer to the model of the navigation tree.
 * \param path		Not used.
 * \param iter		Current iter to test.
 * \param data		Number of an account to match against the
 *			NAVIGATION_ACCOUNT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_update_account_iterator ( GtkTreeModel *tree_model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gint account_number;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_ACCOUNT, &account_number,
			 -1 );

    if ( account_number == GPOINTER_TO_INT ( data ) )
    {
	gsb_gui_navigation_update_account_iter ( tree_model, iter, GPOINTER_TO_INT(data) );
	return TRUE;
    }

    return FALSE;
}



/**
 * Iterator that iterates over the navigation pane model and update
 * iter of report that is equal to `data'.
 *
 * \param tree_model	Pointer to the model of the navigation tree.
 * \param path		Not used.
 * \param iter		Current iter to test.
 * \param data		Number of an report to match against the
 *			NAVIGATION_REPORT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_update_report_iterator ( GtkTreeModel *tree_model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gint report_nb;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_REPORT, &report_nb,
			 -1 );

    if ( report_nb == GPOINTER_TO_INT ( data ) )
    {
	gsb_gui_navigation_update_report_iter ( tree_model, iter, GPOINTER_TO_INT ( data ));
	return TRUE;
    }

    return FALSE;
}



/**
 * Update information for an report in navigation pane.
 *
 * \param report_nb	Number of the report that has to be updated.
 */
void gsb_gui_navigation_update_report ( gint report_number )
{
    gtk_tree_model_foreach ( GTK_TREE_MODEL(navigation_model),
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_update_report_iterator,
			     GINT_TO_POINTER ( report_number ) );
}



/**
 * Update contents of an iter with report data.
 *
 * \param model		Pointer to the model of the navigation tree.
 * \param report_iter	Iter to update.
 * \param data		Number of report as a reference.
 */
void gsb_gui_navigation_update_report_iter ( GtkTreeModel *model,
                        GtkTreeIter * report_iter,
                        gint report_number )
{
    gtk_tree_store_set(GTK_TREE_STORE(model), report_iter,
		       NAVIGATION_TEXT, gsb_data_report_get_report_name (report_number),
		       NAVIGATION_PAGE, GSB_REPORTS_PAGE,
		       NAVIGATION_REPORT, report_number,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_SENSITIVE, "black",
		       -1 );
}



/**
 * Iterator that iterates over the navigation pane model and remove
 * iter of report that is equal to `data'.
 *
 * \param tree_model	Pointer to the model of the navigation tree.
 * \param path		Not used.
 * \param iter		Current iter to test.
 * \param data		Number of an report to match against the
 *			NAVIGATION_REPORT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_remove_report_iterator ( GtkTreeModel *tree_model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gint report;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_REPORT, &report,
			 -1 );

    if ( report == GPOINTER_TO_INT (data))
    {
        if ( gsb_data_report_get_append_in_payee ( report ) )
            gsb_form_widget_update_payee_combofix ( report, FALSE );

        gtk_tree_store_remove ( GTK_TREE_STORE(tree_model), iter );
        return TRUE;
    }

    return FALSE;
}



/**
 * Add an report to the navigation pane.
 *
 * \param report_nb	Report ID to add.
 */
void gsb_gui_navigation_add_report ( gint report_number )
{
    GtkTreeIter parent, iter;
    GtkTreeSelection *selection;
    GtkTreePath *path;

    path = gsb_gui_navigation_get_page_path ( navigation_model, GSB_REPORTS_PAGE );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL( navigation_model ), &parent, path );
    gtk_tree_store_append ( GTK_TREE_STORE ( navigation_model ), &iter, &parent );
    gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( navigation_tree_view ), path );

    gsb_gui_navigation_update_report_iter ( GTK_TREE_MODEL ( navigation_model ), &iter,  report_number );

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( navigation_tree_view ) );
    gtk_tree_selection_select_iter ( selection, &iter );
}



/**
 * Remove report from the navigation pane.
 *
 * \param report_nb	Report ID to add.
 */
void gsb_gui_navigation_remove_report ( gint report_number )
{
    gtk_tree_model_foreach ( GTK_TREE_MODEL(navigation_model),
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_remove_report_iterator,
			     GINT_TO_POINTER ( report_number ) );
}



/**
 * Update information for an account in navigation pane.
 *
 * \param account_number	Number of the account that has to be updated.
 */
void gsb_gui_navigation_update_account ( gint account_number )
{
    gtk_tree_model_foreach ( GTK_TREE_MODEL(navigation_model),
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_update_account_iterator,
			     GINT_TO_POINTER ( account_number ) );
}



/**
 * Update contents of an iter with account data.
 *
 * \param model		Pointer to the model of the navigation tree.
 * \param account_iter	Iter to update.
 * \param data		Number of account as a reference.
 */
void gsb_gui_navigation_update_account_iter ( GtkTreeModel *model,
                        GtkTreeIter *account_iter,
                        gint account_number )
{
    GdkPixbuf * pixbuf = NULL;
	gboolean closed_account;
	gchar *str_color = "black";

	closed_account = gsb_data_account_get_closed_account (account_number);
	if (closed_account)
		str_color = "grey";

	pixbuf = gsb_data_account_get_account_icon_pixbuf ( account_number );

    gtk_tree_store_set ( GTK_TREE_STORE ( model ), account_iter,
                        NAVIGATION_PIX, pixbuf,
                        NAVIGATION_PIX_VISIBLE, TRUE,
                        NAVIGATION_TEXT, gsb_data_account_get_name ( account_number ),
                        NAVIGATION_FONT, 400,
                        NAVIGATION_PAGE, GSB_ACCOUNT_PAGE,
                        NAVIGATION_ACCOUNT, account_number,
                        NAVIGATION_SENSITIVE, str_color,
                        NAVIGATION_REPORT, -1,
                        -1 );

    g_object_unref ( G_OBJECT ( pixbuf ) );
}



/**
 * Iterator that iterates over the navigation pane model and remove
 * iter of account that is equal to `data'.
 *
 * \param tree_model	Pointer to the model of the navigation tree.
 * \param path		Not used.
 * \param iter		Current iter to test.
 * \param data		Number of an account to match against the
 *			NAVIGATION_ACCOUNT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_remove_account_iterator ( GtkTreeModel *tree_model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gint account_number;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_ACCOUNT, &account_number,
			 -1 );

    if ( account_number == GPOINTER_TO_INT ( data ) )
    {
	gtk_tree_store_remove ( GTK_TREE_STORE(tree_model), iter );
	return TRUE;
    }

    return FALSE;
}



/**
 * Add an account to the navigation pane.
 *
 * \param account_number	Account ID to add.
 * \param switch_to_account TRUE to show the account, FALSE to just create it
 */
void gsb_gui_navigation_add_account ( gint account_number,
                        gboolean switch_to_account )
{
    GtkTreeIter parent, iter;
    GtkTreePath *path;

    path = gsb_gui_navigation_get_page_path ( navigation_model, GSB_HOME_PAGE );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL( navigation_model ), &parent, path );
    gtk_tree_store_append ( GTK_TREE_STORE ( navigation_model ), &iter, &parent );
	gtk_tree_path_free (path);

    gsb_gui_navigation_update_account_iter ( GTK_TREE_MODEL ( navigation_model ), &iter, account_number );

    if ( switch_to_account )
    {
        GtkTreeSelection * selection;

        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( navigation_tree_view ) );
        gtk_tree_selection_select_iter ( selection, &iter );
    }
}



/**
 * change the list of transactions, according to the new account
 *
 * \param no_account a pointer on the number of the account we want to see
 *
 * \return FALSE
 * */
gboolean navigation_change_account ( gint new_account )
{
    gint current_account;
    //~ printf ("navigation_change_account : new_account = %d\n", new_account);

    devel_debug_int (new_account);
    if ( new_account < 0 )
        return FALSE;

    /* the selection on the navigation bar has already changed, so
     * have to use a buffer variable to get the last account */
    current_account = gsb_gui_navigation_get_last_account ();

    /* mise à jour du menu Editer */
    gsb_menu_gui_sensitive_win_menu_item ( "new-ope", TRUE );

    /* save the row_align of the last account */
    gsb_data_account_set_row_align ( current_account,
                        gsb_transactions_list_get_row_align ( ) );

    /* set the appearance of the list according to the new account */
    transaction_list_sort_set_column ( gsb_data_account_get_sort_column (new_account ),
                        gsb_data_account_get_sort_type ( new_account ) );

    gsb_transactions_list_update_tree_view ( new_account, FALSE );
    gsb_transactions_list_set_row_align ( gsb_data_account_get_row_align ( new_account ) );

    /* mise en place de la date du dernier relevé */
    gsb_navigation_update_statement_label ( new_account );

    /* show or hide the rules button in toolbar */
    if ( gsb_data_import_rule_account_has_rule ( new_account ) )
        gtk_widget_show ( menu_import_rules );
    else
        gtk_widget_hide ( menu_import_rules );

    /* Update the title of the file if needed */
    if ( conf.display_window_title == GSB_ACCOUNT_HOLDER )
        grisbi_win_set_window_title ( new_account );

    /* select the good tab */
    bet_data_select_bet_pages ( new_account );

    /* unset the last date written */
    gsb_date_free_last_date ();

    /* check if balance is < 0 and account_kind == GSB_TYPE_CASH */
    if ( gsb_data_account_get_kind ( new_account ) == GSB_TYPE_CASH )
    {
        gsb_real balance;

        balance = gsb_data_account_get_current_balance ( new_account );

        if ( balance.mantissa < 0 )
        {
            dialogue_error ( _("This account cannot be negative.\n\n"
                        "You must make positive the balance of this account "
                        "before you can use it.") );

            return ( FALSE );
        }
    }

    return FALSE;
}


/**
 * update the statement label for the given account
 *
 * \param account_number
 *
 * \return
 * */
void gsb_navigation_update_statement_label ( gint account_number )
{
    gint reconcile_number;
    gchar* tmp_str;
    gchar* tmp_str1;
    gchar* tmp_str2;
    gsb_real amount;

    reconcile_number = gsb_data_reconcile_get_account_last_number ( account_number );
    amount = gsb_data_account_get_marked_balance ( account_number );
    if ( reconcile_number )
    {
        tmp_str1 = gsb_format_gdate ( gsb_data_reconcile_get_final_date (
                                            reconcile_number ) );
        tmp_str2 = utils_real_get_string_with_currency ( amount,
                        gsb_data_account_get_currency ( account_number ), TRUE );

        tmp_str = g_strconcat ( _("Last statement: "), tmp_str1, " - ",
                               _("Reconciled balance: "), tmp_str2, NULL );
        gtk_label_set_text ( GTK_LABEL ( label_last_statement ), tmp_str);
        g_free ( tmp_str );
        g_free ( tmp_str1 );
        g_free ( tmp_str2 );
    }
    else if ( amount.mantissa != 0 )
    {
        tmp_str2 = utils_real_get_string_with_currency (amount,
                        gsb_data_account_get_currency ( account_number ), TRUE );

        tmp_str = g_strconcat ( _("Last statement: none"), " - ",
                               _("Reconciled balance: "), tmp_str2, NULL );
        gtk_label_set_text ( GTK_LABEL ( label_last_statement ), tmp_str);
        g_free ( tmp_str );
        g_free ( tmp_str2 );
    }
    else
        gtk_label_set_text ( GTK_LABEL ( label_last_statement ),
                                            _("Last statement: none") );
}


/**
 * Update the account name in the heading bar
 *
 * \param account_number
 *
 * \return
 */
void gsb_navigation_update_account_label ( gint account_number )
{
    gchar * title = NULL;

    title = g_strconcat ( _("Account"), " : ",
			  gsb_data_account_get_name ( account_number ),
			  NULL );
    if ( gsb_data_account_get_closed_account ( account_number ) )
    {
	gchar* old_title = title;
	title = g_strconcat ( title, " (", _("closed"), ")", NULL );
	g_free ( old_title );
    }

    gsb_data_account_colorize_current_balance ( account_number );
    grisbi_win_headings_update_title ( title );

    g_free ( title );
}


/**
 * Remove account from the navigation pane.
 *
 * \param account_number	Account ID to remove.
 */
void gsb_gui_navigation_remove_account ( gint account_number )
{
    gtk_tree_model_foreach ( GTK_TREE_MODEL(navigation_model),
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_remove_account_iterator,
			     GINT_TO_POINTER ( account_number ) );
}



/**
 * Callback executed when the selection of the navigation tree
 * changed.
 * we must write here the changes to do when changing something in that selection,
 * not with a callback "switch-page" on the main notebook
 *
 * \param selection	The selection that triggered event.
 * \param model		Tree model associated to selection.
 *
 * \return FALSE
 */
gboolean gsb_gui_navigation_select_line ( GtkTreeSelection *selection,
                        GtkTreeModel *model )
{
    gint account_number, page_number;
    gint report_number;
    gchar *title = NULL;
    gboolean clear_suffix = TRUE;

    devel_debug (NULL);
    page_number = gsb_gui_navigation_get_current_page ();

    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( gsb_gui_get_general_notebook ( ) ), page_number );

    if ( page_number != GSB_ACCOUNT_PAGE )
    {
        /* on update le menu de la liste des comptes */
        grisbi_win_menu_move_to_acc_update ( FALSE );

        /* on dé-sensibilise le menu view account */
        gsb_menu_set_menus_view_account_sensitive ( FALSE );
    }

    if ( page_number != GSB_SCHEDULER_PAGE )
    {
        gtk_widget_hide ( scheduler_calendar );
    }

    switch ( page_number )
	{
		case GSB_HOME_PAGE:
			notice_debug ("Home page selected");

			title = g_strdup(_("My accounts"));

			/* what to be done if switch to that page */
			mise_a_jour_accueil ( FALSE );
			gsb_form_set_expander_visible ( FALSE, FALSE );
			gsb_form_hide ();
			break;

		case GSB_ACCOUNT_PAGE:
			notice_debug ("Account page selected");

			gsb_menu_gui_sensitive_win_menu_item ( "remove-acc", TRUE );

			account_number = gsb_gui_navigation_get_current_account ();

			/* update title now -- different from others */
			gsb_navigation_update_account_label (account_number);

			/* what to be done if switch to that page */
			if (account_number >= 0 )
			{
				navigation_change_account ( account_number );
				gsb_account_property_fill_page ();
				clear_suffix = FALSE;
				if ( gsb_data_archive_store_account_have_transactions_visibles ( account_number ) )
					gsb_transaction_list_set_visible_archived_button ( TRUE );
				else
					gsb_transaction_list_set_visible_archived_button ( FALSE );
			}
			grisbi_win_menu_move_to_acc_update (FALSE);
			gsb_menu_update_view_menu ( account_number );

			/* set the form */
			gsb_gui_on_account_switch_page ( GTK_NOTEBOOK ( gsb_gui_get_account_page () ),
							NULL,
							gtk_notebook_get_current_page ( GTK_NOTEBOOK ( gsb_gui_get_account_page () ) ),
							NULL );
			gsb_form_show (TRUE);

			buffer_last_account = account_number;

			break;

		case GSB_SCHEDULER_PAGE:
			notice_debug ("Scheduler page selected");

			title = g_strdup(_("Scheduled transactions"));

			/* what to be done if switch to that page */
			/* update the list (can do that because short list, so very fast) */
			gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());
			gsb_scheduler_list_set_background_color (gsb_scheduler_list_get_tree_view ());

			gsb_scheduler_list_select (gsb_scheduler_list_get_last_scheduled_number ());

			/* set the form */
			gsb_form_set_expander_visible (TRUE, FALSE );
			gsb_form_scheduler_clean ();
			gsb_form_show ( FALSE );

			/* show the calendar */
			gsb_calendar_update ();
			gtk_widget_show_all ( scheduler_calendar );

			/* show menu NewTransaction */
			gsb_menu_gui_sensitive_win_menu_item ( "new-ope", TRUE );

			/* show menu InitwidthCol */
			gsb_menu_gui_sensitive_win_menu_item ( "reset-width-col", TRUE );
			break;

		case GSB_PAYEES_PAGE:
			notice_debug ("Payee page selected");

			/* what to be done if switch to that page */
			gsb_form_set_expander_visible (FALSE, FALSE );
			payees_fill_list ();
			clear_suffix = FALSE;
			gsb_form_hide ();
			break;

		case GSB_SIMULATOR_PAGE:
			notice_debug ("Credits simulator page selected");

			title = g_strdup(_("Credits simulator"));

			/* what to be done if switch to that page */
			gsb_form_set_expander_visible (FALSE, FALSE);
			bet_finance_switch_simulator_page ( );
			gsb_form_hide ();
			break;

		case GSB_CATEGORIES_PAGE:
			notice_debug ("Category page selected");

			/* what to be done if switch to that page */
			gsb_form_set_expander_visible (FALSE, FALSE );
			categories_fill_list ();
			clear_suffix = FALSE;
			gsb_form_hide ();
			break;

		case GSB_BUDGETARY_LINES_PAGE:
			notice_debug ("Budgetary page selected");

			/* what to be done if switch to that page */
			gsb_form_set_expander_visible (FALSE, FALSE );
			budgetary_lines_fill_list ();
			clear_suffix = FALSE;
			gsb_form_hide ();
			break;

		case GSB_REPORTS_PAGE:
			notice_debug ("Reports page selected");

			report_number = gsb_gui_navigation_get_current_report ();

			if ( report_number >= 0 )
			title = g_strconcat ( _("Report"), " : ", gsb_data_report_get_report_name (report_number), NULL );
			else
			title = g_strdup(_("Reports"));

			/* what to be done if switch to that page */
			gsb_form_set_expander_visible ( FALSE, FALSE );

			if ( report_number > 0 )
			{
				gtk_notebook_set_current_page (GTK_NOTEBOOK (etats_onglet_get_notebook_etats ()), 0);
				etats_onglet_update_gui_to_report ( report_number );
			}
			else
			{
				gtk_notebook_set_current_page (GTK_NOTEBOOK (etats_onglet_get_notebook_etats ()), 1);
				etats_onglet_unsensitive_reports_widgets ();
			}
			gsb_form_hide ();
			break;

		default:
			notice_debug ("B0rk page selected");
			title = g_strdup("B0rk");
			gsb_form_hide ();
			break;
    }

    /* title is set here if necessary */
    if (title)
    {
        grisbi_win_headings_update_title ( title );
        g_free ( title );
    }
    if (clear_suffix)
        grisbi_win_headings_update_suffix ( "" );

    return FALSE;
}



/**
 * Set the selection of the navigation list depending on desired
 * page and/or account or report.
 *
 * \param page		        Page to switch to.
 * \param account_number	If page is GSB_ACCOUNT_PAGE, switch to given
 *			account.
 * \param report	If page is GSB_REPORTS, switch to given
 *			report.
 *
 * \return		TRUE on success.
 */
gboolean gsb_gui_navigation_set_selection ( gint page,
                        gint account_number,
                        gint report_number )
{
    GtkTreeIter iter;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( navigation_tree_view ) );
    g_return_val_if_fail ( selection, FALSE );

    /* if we select an account, open the expander if necessary */
    if (page == GSB_ACCOUNT_PAGE || page == GSB_REPORTS_PAGE)
    {
        GtkTreePath *path;

		if (page == GSB_ACCOUNT_PAGE)
			path = gsb_gui_navigation_get_page_path (navigation_model, GSB_HOME_PAGE);
		else
			path = gsb_gui_navigation_get_page_path (navigation_model,GSB_REPORTS_PAGE);

        gtk_tree_view_expand_row ( GTK_TREE_VIEW ( navigation_tree_view ), path, TRUE );
        gtk_tree_path_free (path);
    }

    gtk_tree_model_get_iter_first ( GTK_TREE_MODEL(navigation_model), &iter );
    gsb_gui_navigation_set_selection_branch (selection, &iter, page, account_number, report_number);

    return TRUE;
}


/**
 * Set the selection of the navigation list depending on desired page
 * and/or account or report, but only for a branch of the tree.
 *
 * \param selection	Selection to set.
 * \param iter		Current iter to iterate over.
 * \param page		Page to switch to.
 * \param account_number	If page is GSB_ACCOUNT_PAGE, switch to given
 *			account.
 * \param report	If page is GSB_REPORTS, switch to given
 *			report.
 *
 * \return		TRUE on success.
 */
void gsb_gui_navigation_set_selection_branch ( GtkTreeSelection *selection,
					    GtkTreeIter *iter,
                        gint page,
					    gint account_number,
                        gint report_number)
{
	do
    {
		gint iter_page, iter_account_nb;
		gint iter_report;

		gtk_tree_model_get ( GTK_TREE_MODEL(navigation_model), iter,
					 NAVIGATION_REPORT, &iter_report,
					 NAVIGATION_ACCOUNT, &iter_account_nb,
					 NAVIGATION_PAGE, &iter_page,
					 -1 );

		if ( iter_page == page &&
			 ! ( page == GSB_ACCOUNT_PAGE &&
			 iter_account_nb != account_number ) &&
			 ! ( page == GSB_REPORTS_PAGE &&
			 iter_report != report_number ) )
		{
			gtk_tree_selection_select_iter ( selection, iter );
		}

		if ( gtk_tree_model_iter_has_child ( GTK_TREE_MODEL(navigation_model), iter ) )
		{
			GtkTreeIter child;

			gtk_tree_model_iter_children ( GTK_TREE_MODEL(navigation_model), &child, iter );
			gsb_gui_navigation_set_selection_branch ( selection, &child,
								  page, account_number, report_number);
		}
    }
    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL(navigation_model), iter ) );

    return;
}



/**
 *
 *
 *
 */
gboolean gsb_gui_navigation_select_prev ( void )
{
    GtkTreeSelection * selection;
    GtkTreePath * path;
    GtkTreeModel * model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    g_return_val_if_fail ( selection, FALSE );

    if ( !gtk_tree_selection_get_selected ( selection, &model, &iter ) )
        return TRUE;
    path = gtk_tree_model_get_path ( model, &iter );
    g_return_val_if_fail ( path, TRUE );

    if ( !gtk_tree_path_prev ( path ) )
    {
        if ( gtk_tree_path_get_depth ( path ) > 1 )
            gtk_tree_path_up ( path );
    }
    else
    {
        gtk_tree_model_get_iter ( model, &iter, path );
        /* if row has children and if row is expanded, go to the last child */
        if ( gtk_tree_model_iter_has_child ( model, &iter )
             && gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( navigation_tree_view ), path ) )
        {
            GtkTreeIter parent = iter;
            gtk_tree_model_iter_nth_child ( model, &iter, &parent,
                                            gtk_tree_model_iter_n_children ( model,
                                                                             &parent ) - 1 );
            path = gtk_tree_model_get_path ( model, &iter );
        }
    }

    gtk_tree_selection_select_path ( selection, path );
    gtk_tree_path_free ( path );

    return FALSE;
}



/**
 *
 *
 *
 */
gboolean gsb_gui_navigation_select_next ( void )
{
    GtkTreeSelection *selection;
    GtkTreePath *path;
    GtkTreeModel *model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    g_return_val_if_fail ( selection, FALSE );

    if ( !gtk_tree_selection_get_selected ( selection, &model, &iter ) )
        return TRUE;

    path = gtk_tree_model_get_path ( model, &iter );
    g_return_val_if_fail ( path, TRUE );

    if ( gtk_tree_model_iter_has_child ( model, &iter ) )
    {
        if ( gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( navigation_tree_view ), path ) )
            gtk_tree_path_down ( path );
        else
            gtk_tree_path_next ( path );
    }
    else
    {
        if ( !gtk_tree_model_iter_next ( model, &iter ) )
        {
            if ( gtk_tree_path_get_depth ( path ) > 1 )
            {
                gtk_tree_path_up ( path );
                gtk_tree_path_next ( path );
            }
            else
            {
                gtk_tree_path_free ( path );
                path = gtk_tree_path_new_first ( );
            }
        }
        else
        {
            path = gtk_tree_model_get_path ( model, &iter );
        }
    }

    gtk_tree_selection_select_path ( selection, path );
    gtk_tree_path_free ( path );

    return FALSE;
}



/**
 * Check the key pressed on the navigation tree view,
 * if need, stop the event to do another thing with that key
 *
 * \param tree_view the navigation tree_view
 * \param ev the key event pressed
 * \param model
 *
 * \return FALSE : the signal continue / TRUE : the signal is stopped here
 * */
gboolean gsb_gui_navigation_check_key_press ( GtkWidget *tree_view,
                        GdkEventKey *ev,
                        GtkTreeModel *model )
{
    gint page;
    GtkTreeIter iter;
    GtkTreePath *path;

    if (! gtk_tree_selection_get_selected ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
					    NULL,
					    &iter))
	return FALSE;

    gtk_tree_model_get (model, &iter, NAVIGATION_PAGE, &page, -1 );
    path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( model ), &iter );

    switch ( page )
    {
	case GSB_HOME_PAGE:
	case GSB_REPORTS_PAGE:
            /* expand or collapse subtree */
	    if (ev -> keyval == GDK_KEY_Right)
                gtk_tree_view_expand_row ( GTK_TREE_VIEW ( tree_view ), path, FALSE );
	    if (ev -> keyval == GDK_KEY_Left)
                gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( tree_view ), path );
	    break;

	case GSB_ACCOUNT_PAGE:
	    /* when come here, if we press the right key, give the focus to the list */
	    if (ev -> keyval == GDK_KEY_Right)
		gtk_widget_grab_focus (gsb_transactions_list_get_tree_view ());
	    break;

	case GSB_PAYEES_PAGE:
	    /* nothing to do here for now */
	    break;

	case GSB_CATEGORIES_PAGE:
	    /* nothing to do here for now */
	    break;

	case GSB_BUDGETARY_LINES_PAGE:
	    /* nothing to do here for now */
	    break;

	case GSB_SCHEDULER_PAGE:
	    /* when come here, if we press the right key, give the focus to the list */
	    if (ev -> keyval == GDK_KEY_Right)
		gtk_widget_grab_focus (gsb_scheduler_list_get_tree_view ());
	    break;
    }

    gtk_tree_path_free ( path );

    return FALSE;
}



/**
 * Check mouse scrolling on the navigation tree view.
 *
 * \param tree_view the navigation tree_view
 * \param ev the scroll event
 *
 * \return FALSE : the signal continue / TRUE : the signal is stopped here
 * */
gboolean gsb_gui_navigation_check_scroll ( GtkWidget *tree_view,
                                           GdkEventScroll *ev )
{
    switch ( ev -> direction )
    {
        case GDK_SCROLL_UP:
            gsb_gui_navigation_select_prev ();
            break;

        case GDK_SCROLL_DOWN:
            gsb_gui_navigation_select_next ();
            break;

        default:
            break;
    }

    return FALSE;
}



/**
 * Fill the drag & drop structure with the path of selected column.
 * This is an interface function called from GTK, much like a callback.
 *
 * \param drag_source		Not used.
 * \param path			Original path for the gtk selection.
 * \param selection_data	A pointer to the drag & drop structure.
 *
 * \return FALSE, to allow future processing by the callback chain.
 */
gboolean navigation_tree_drag_data_get ( GtkTreeDragSource *drag_source,
                        GtkTreePath *path,
                        GtkSelectionData *selection_data )
{
    gchar *tmpstr = gtk_tree_path_to_string (path);
    gchar *tmpstr2 = g_strdup_printf ( "Orig path : %s", tmpstr);
    devel_debug (tmpstr2);
    g_free (tmpstr);
    g_free (tmpstr2);

    if ( path )
    {
	gtk_tree_set_row_drag_data ( selection_data, GTK_TREE_MODEL(navigation_model),
				     path );
    }

    return FALSE;
}



/**
 *
 *
 */
gboolean navigation_drag_data_received ( GtkTreeDragDest *drag_dest,
                        GtkTreePath *dest_path,
                        GtkSelectionData *selection_data )
{
    gchar *tmpstr = gtk_tree_path_to_string (dest_path);
    gchar *tmpstr2 = g_strdup_printf ( "Dest path : %s", tmpstr);
    devel_debug (tmpstr2);
    g_free (tmpstr);
    g_free (tmpstr2);

    if ( dest_path && selection_data )
    {
        GtkTreeModel *model;
        GtkTreeIter iter;
        GtkTreePath *orig_path;
        gint src_report = -1, dst_report = -1;
        gint src_account = -1, dst_account = -1;
        gint src_ordre = -1, dst_ordre = -1;

        gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);

        if ( gtk_tree_model_get_iter ( GTK_TREE_MODEL(model), &iter, dest_path ) )
            gtk_tree_model_get (model , &iter,
                        NAVIGATION_REPORT, &dst_report,
                        NAVIGATION_ACCOUNT, &dst_account,
                        NAVIGATION_ORDRE, &dst_ordre,
                        -1 );

        if ( gtk_tree_model_get_iter ( GTK_TREE_MODEL(model), &iter, orig_path ) )
            gtk_tree_model_get ( model, &iter,
                        NAVIGATION_REPORT, &src_report,
                        NAVIGATION_ACCOUNT, &src_account,
                        NAVIGATION_ORDRE, &src_ordre,
                        -1 );

        /* at this stage, src_account or src_report contains the account/report we move
         * and dst_account/dst_report the account/report destination we want to move before,
         * or dst_account/dst_report can be -1 to set at the end of the list */
        if ( src_account != -1 )
            /* we moved an account */
            gsb_data_account_move_account ( src_account, dst_account );
        if ( src_report != -1 )
            /* we moved a report */
            gsb_data_report_move_report ( src_report, dst_report );
        if ( src_ordre != -1 )
            /* we moved a page */
            gsb_gui_navigation_move_ordre ( src_ordre, dst_ordre );

        /* update the tree view */
        gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( model ),
                        NAVIGATION_ORDRE, GTK_SORT_ASCENDING );
        gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( model ),
                        NAVIGATION_ORDRE, navigation_sort_column,
                        NULL, NULL );

        /* update the order of accounts in first page */
        run.mise_a_jour_liste_comptes_accueil = TRUE;

        gsb_file_set_modified ( TRUE );
    }
    return FALSE;
}



/**
 *
 *
 */
gboolean navigation_row_drop_possible ( GtkTreeDragDest *drag_dest,
                        GtkTreePath *dest_path,
                        GtkSelectionData *selection_data )
{
    if ( dest_path && selection_data )
    {
        GtkTreePath *orig_path;
        GtkTreeModel *model;
        gint src_report = -1, dst_report = -1;
        gint src_account = -1, dst_account = -1;
        gint src_ordre = -1, dst_ordre = -1;
        GtkTreeIter iter;

        gtk_tree_get_row_drag_data ( selection_data, &model, &orig_path );

        if ( gtk_tree_model_get_iter ( model, &iter, orig_path ) )
            gtk_tree_model_get ( model, &iter,
                        NAVIGATION_REPORT, &src_report,
                        NAVIGATION_ACCOUNT, &src_account,
                        NAVIGATION_ORDRE, &src_ordre,
                        -1 );

        if ( gtk_tree_model_get_iter ( model, &iter, dest_path ) )
            gtk_tree_model_get ( model, &iter,
                        NAVIGATION_REPORT, &dst_report,
                        NAVIGATION_ACCOUNT, &dst_account,
                        NAVIGATION_ORDRE, &dst_ordre,
                        -1 );

        /* We handle an account */
        if ( src_account >= 0 && dst_account >= 0 )
        {
            return TRUE;
        }
        /* We handle a report */
        else if ( src_report > 0 && dst_report > 0 )
        {
            return TRUE;
        }
        else if ( src_ordre >= 0 && dst_ordre >= 0 )
        {
            return TRUE;
        }

    }

    return FALSE;
}


/**
 * Met à jour la page d'accueil immédiatement si elle est affichée sinon plus tard
 *
 */
void gsb_gui_navigation_update_home_page ( void )
{
    if ( gsb_gui_navigation_get_current_page ( ) == GSB_HOME_PAGE )
        mise_a_jour_accueil ( TRUE );
    else
        run.mise_a_jour_liste_comptes_accueil = TRUE;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_gui_navigation_get_tree_view ( void )
{
    return navigation_tree_view;
}


/**
 * change the position of an item in the list of pages
 *
 * \param src_ordre     the position of item we want to move
 * \param dest_ordre	the position before we want to move, or -1 to set at the end of list
 *
 * \return FALSE
 * */
gboolean gsb_gui_navigation_move_ordre ( gint src_ordre,
                        gint dst_ordre )
{
    GQueue *tmp_queue;
    GList *dst_list;
    gint i;
    struct_page *page = NULL;

    tmp_queue = pages_list;
    for ( i = 0 ; i < (gint) tmp_queue->length ; i++ )
    {
        page = g_queue_peek_nth ( tmp_queue, i );
        if ( page -> ordre == src_ordre )
            break;
    }

    g_queue_pop_nth ( pages_list, i );

    dst_list = g_queue_peek_nth_link ( pages_list, dst_ordre );
    if ( dst_list )
        g_queue_insert_before ( pages_list, dst_list, page );
    else
        g_queue_push_tail ( tmp_queue, page );

    /* on reconstruit le modèle */
    gtk_tree_store_clear ( GTK_TREE_STORE ( navigation_model ) );

    tmp_queue = pages_list;

    for ( i = 0 ; i < (gint) tmp_queue->length ; i++ )
    {
        page = g_queue_peek_nth ( tmp_queue, i );
        page -> ordre = i;
        gsb_gui_navigation_set_navigation_pages ( navigation_model, page -> type_page, i );
    }

    return FALSE;
}


/**
 *
 *
 *
 */
void gsb_gui_navigation_init_tree_view ( void )
{
    navigation_tree_view = NULL;
}


/**
 *
 *
 *
 */
GtkTreeModel *gsb_gui_navigation_get_model ( void )
{
    if ( navigation_model )
        return navigation_model;
    else
        return NULL;
}


/**
 *
 *
 *
 */
gboolean gsb_gui_navigation_set_page_list_order ( const gchar *order_list )
{
    gchar **pointeur_char;
    gint i;
    gint nbre_pages;

    if ( pages_list )
        gsb_gui_navigation_clear_pages_list ( );

    pointeur_char = g_strsplit ( order_list, "-", 0 );

    nbre_pages = g_strv_length ( pointeur_char );

    if ( nbre_pages != navigation_nbre_pages )
    {
        g_strfreev ( pointeur_char );
        pointeur_char = g_strsplit ( default_navigation_order_list, "-", 0 );
        nbre_pages = navigation_nbre_pages;
    }

    for ( i = 0 ; i < nbre_pages ; i++ )
    {
        struct_page *page;

        page = g_malloc0 ( sizeof ( struct_page ) );
        page -> ordre = i;
        page -> type_page = utils_str_atoi ( pointeur_char[i] );

        g_queue_push_tail ( pages_list, page );
    }

    g_strfreev ( pointeur_char );

    return TRUE;
}


/**
 *
 *
 *
 */
void gsb_gui_navigation_init_pages_list ( void )
{
    GQueue *new_queue;

    gsb_gui_navigation_free_pages_list ( );

    new_queue = g_queue_new ( );
    pages_list = new_queue;
    gsb_gui_navigation_set_page_list_order ( default_navigation_order_list );
}


/**
 *
 *
 *
 */
void gsb_gui_navigation_clear_pages_list ( void )
{
    g_queue_foreach ( pages_list, (GFunc) g_free, NULL );
    g_queue_clear ( pages_list );
}


/**
 *
 *
 *
 */
void gsb_gui_navigation_free_pages_list ( void )
{
    if ( ! pages_list )
        return;

    g_queue_foreach ( pages_list, (GFunc) g_free, NULL );
    g_queue_free ( pages_list );
    pages_list = NULL;
}


/**
 * return a pointer on the first element of g_queue of navigation pages
 *
 * \param none
 * \return a GList
 * */
GQueue *gsb_gui_navigation_get_pages_list ( void )
{
    return pages_list;
}


/**
 *
 *
 *
 */
void gsb_gui_navigation_set_navigation_pages ( GtkTreeModel *model,
                        gint type_page,
                        gint ordre )
{
    GdkPixbuf *pixbuf;
    GtkTreeIter iter;
    gchar *title = NULL;
    gchar *tmp_str = NULL;

    switch ( type_page )
    {
        case GSB_HOME_PAGE :
            tmp_str = g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), "gsb-ac-home-32.png", NULL );
            title = g_strdup ( _("Accounts") );
        break;
        case GSB_SCHEDULER_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "gsb-scheduler-32.png", NULL );
            title = g_strdup ( _("Scheduler") );
        break;
        case GSB_PAYEES_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "gsb-payees-32.png", NULL );
            title = g_strdup ( _("Payees") );
        break;
        case GSB_SIMULATOR_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "gsb-ac-liability-32.png", NULL );
            title = g_strdup ( _("Credits simulator") );
        break;
        case GSB_CATEGORIES_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "gsb-categories-32.png", NULL );
            title = g_strdup ( _("Categories") );
        break;
        case GSB_BUDGETARY_LINES_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "gsb-budgetary_lines-32.png", NULL );
            title = g_strdup ( _("Budgetary lines") );
        break;
        case GSB_REPORTS_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "gsb-reports-32.png", NULL );
            title = g_strdup ( _("Reports") );
        break;
    }

    pixbuf = gdk_pixbuf_new_from_file ( tmp_str , NULL );
    gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, NULL );
    gtk_tree_store_set( GTK_TREE_STORE ( navigation_model ), &iter,
                        NAVIGATION_PIX, pixbuf,
                        NAVIGATION_TEXT, title,
                        NAVIGATION_PIX_VISIBLE, TRUE,
                        NAVIGATION_FONT, 800,
                        NAVIGATION_PAGE, type_page,
                        NAVIGATION_ACCOUNT, -1,
                        NAVIGATION_REPORT, -1,
                        NAVIGATION_SENSITIVE, "black",
                        NAVIGATION_ORDRE, ordre,
                        -1);

    g_object_unref ( G_OBJECT ( pixbuf ) );
    g_free ( tmp_str );
    g_free ( title );

    switch ( type_page )
    {
        case GSB_HOME_PAGE :
            gsb_gui_navigation_create_account_list ( GTK_TREE_MODEL( model ) );
        break;
        case GSB_REPORTS_PAGE :
            gsb_gui_navigation_create_report_list ( GTK_TREE_MODEL( model) );
        break;
    }

}


/**
 * renvoie le path d'un item de la vue navigation
 *
 *
 */
GtkTreePath *gsb_gui_navigation_get_page_path ( GtkTreeModel *model, gint type_page )
{
    GtkTreeIter iter;

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
    {
        do
        {
            gint tmp_type_page;
            GtkTreePath *path = NULL;

            gtk_tree_model_get ( GTK_TREE_MODEL( model ), &iter,  NAVIGATION_PAGE, &tmp_type_page, -1 );
            if ( tmp_type_page == type_page)
            {
                path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( model ), &iter );
                return path;
            }
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );
    }

    return NULL;
}


/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
gboolean gsb_gui_navigation_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null )
{
	/* show the popup */
	if ( ev -> button == RIGHT_BUTTON )
    {
        GtkTreePath *path = NULL;

        if ( gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ), ev -> x, ev -> y, &path, NULL, NULL, NULL ) )
        {
            gsb_gui_navigation_context_menu ( tree_view, path );
            gtk_tree_path_free ( path );

            update_gui ( );

            return FALSE;
        }
    }
    if ( ev -> type == GDK_2BUTTON_PRESS )
    {
        GtkTreePath *path = NULL;

        if ( gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ), ev -> x, ev -> y, &path, NULL, NULL, NULL ) )
        {
            if ( gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( tree_view ), path ) )
                gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( tree_view ), path );
            else
                gtk_tree_view_expand_row ( GTK_TREE_VIEW ( tree_view ), path, FALSE );

            gtk_tree_path_free ( path );

            return FALSE;
        }
    }

    return FALSE;
}


/**
 * Pop up a menu with several actions to apply to array_list.
 *
 * \param gtk_tree_view
 *
 */
void gsb_gui_navigation_context_menu ( GtkWidget *tree_view,
                        GtkTreePath *path )
{
    GtkWidget *menu = NULL;
    GtkWidget *menu_item;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint type_page;
    gint account_number;
    gint report_number;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    gtk_tree_model_get_iter ( model, &iter, path );
    gtk_tree_model_get ( model, &iter,
                        NAVIGATION_PAGE, &type_page,
                        NAVIGATION_ACCOUNT, &account_number,
                        NAVIGATION_REPORT, &report_number,
                        -1 );

    switch ( type_page )
    {
        case GSB_HOME_PAGE :
        case GSB_ACCOUNT_PAGE :
            menu = gtk_menu_new ();
            menu_item = utils_menu_item_new_from_image_label ("gsb-new-file-16.png", _("New account"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( gsb_assistant_account_run ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            if ( account_number == -1 )
                break;

            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            menu_item = utils_menu_item_new_from_image_label ("gtk-delete-16.png", _("Remove this account"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( gsb_account_delete ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        break;
        case GSB_PAYEES_PAGE :
            menu = gtk_menu_new ();
            menu_item = utils_menu_item_new_from_image_label ("gsb-new-payee-16.png",_("New payee"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payees_new_payee ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = utils_menu_item_new_from_image_label ("gtk-delete-16.png", _("Delete selected payee"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payees_delete_payee ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( payees_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            menu_item = utils_menu_item_new_from_image_label ("gtk-edit-16.png", _("Edit selected payee"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payees_edit_payee ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( payees_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            menu_item = utils_menu_item_new_from_image_label ("gsb-payees-manage-16.png", _("Manage payees"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payees_manage_payees ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = utils_menu_item_new_from_image_label ("gtk-delete-16.png", _("Remove unused payees"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payees_remove_unused_payees ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
        break;
        case GSB_CATEGORIES_PAGE :
            menu = gtk_menu_new ();
            menu_item = utils_menu_item_new_from_image_label ("gsb-new-categ-16.png", _("New category"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( categories_new_category ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = utils_menu_item_new_from_image_label ("gtk-delete-16.png", _("Delete selected category") );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( categories_delete_category ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( categories_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            menu_item = utils_menu_item_new_from_image_label ("gtk-edit-16.png", _("Edit selected category"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( categories_edit_category ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( categories_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            menu_item = utils_menu_item_new_from_image_label ("gsb-import-16.png", _("Import a file of categories (.cgsb)"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( categories_importer_list ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = utils_menu_item_new_from_image_label ("gsb-export-16.png", _("Export the list of categories (.cgsb)"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( categories_exporter_list ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
        break;
        case GSB_BUDGETARY_LINES_PAGE :
            menu = gtk_menu_new ();
            menu_item = utils_menu_item_new_from_image_label ("gsb-new-ib-16.png", _("New budgetary line"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( budgetary_lines_new_budgetary_line ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = utils_menu_item_new_from_image_label ("gtk-delete-16.png", _("Delete selected budgetary line"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( budgetary_lines_delete_budgetary_line ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( budgetary_lines_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            menu_item = utils_menu_item_new_from_image_label ("gtk-edit-16.png", _("Edit selected budgetary line") );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( budgetary_lines_edit_budgetary_line ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( budgetary_lines_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            menu_item = utils_menu_item_new_from_image_label ("gsb-import-16.png", _("Import a file of budgetary lines (.igsb)") );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( budgetary_lines_importer_list ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = utils_menu_item_new_from_image_label ("gsb-export-16.png", _("Export the list of budgetary lines (.igsb)") );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( budgetary_lines_exporter_list ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
        break;
        case GSB_REPORTS_PAGE :
            menu = gtk_menu_new ();
            menu_item = utils_menu_item_new_from_image_label ("gsb-new-report-16.png", _("New report"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( etats_onglet_ajoute_etat ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            if ( report_number == -1 )
                break;
            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            menu_item = utils_menu_item_new_from_image_label ("gtk-delete-16.png", _("Remove this report"));
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( etats_onglet_efface_etat ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        break;
    }

    /* Finish all. */
    if ( menu )
    {
        gtk_widget_show_all ( menu );

#if GTK_CHECK_VERSION (3,22,0)
		gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
#else
		gtk_menu_popup ( GTK_MENU( menu ), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time ( ) );
#endif
    }
}


void gsb_gui_navigation_activate_expander ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreePath *path,
                        gpointer user_data )
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter_selected;
    GtkTreePath *path_selected = NULL;
    gint type_page;
    gint etat_expander;

    etat_expander = GPOINTER_TO_INT ( user_data );
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    if ( gtk_tree_selection_get_selected ( selection, &model, &iter_selected ) )
        path_selected = gtk_tree_model_get_path ( model, &iter_selected);

    gtk_tree_model_get ( model, iter, NAVIGATION_PAGE, &type_page, -1 );

    switch ( type_page )
    {
        case GSB_HOME_PAGE :
        case GSB_ACCOUNT_PAGE :
            account_expander = etat_expander;
        break;
        case GSB_REPORTS_PAGE :
            report_expander = etat_expander;
        break;
    }

    if ( etat_expander == 0 && path_selected == NULL )
        gtk_tree_selection_select_iter ( selection, iter );
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
