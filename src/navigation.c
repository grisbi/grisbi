/* ************************************************************************** */
/*     Copyright (C)	     2005 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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


#include "include.h"

/*START_INCLUDE*/
#include "navigation.h"
#include "etats_onglet.h"
#include "echeancier_infos.h"
#include "utils_devises.h"
#include "gsb_account.h"
#include "operations_comptes.h"
#include "fenetre_principale.h"
#include "operations_liste.h"
#include "comptes_gestion.h"
#include "echeancier_liste.h"
#include "fichier_configuration.h"
#include "navigation.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void create_account_list ( GtkTreeModel * model, GtkTreeIter * account_iter );
static void create_report_list ( GtkTreeModel * model, GtkTreeIter * reports_iter );
static gboolean gsb_gui_navigation_check_key_press ( GtkWidget *tree_view,
					      GdkEventKey *ev,
					      GtkTreeModel *model );
static gboolean gsb_gui_navigation_line_visible_p ( GtkTreeModel * model, GtkTreeIter * iter,
					     gpointer data );
static  gboolean gsb_gui_navigation_remove_account_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data );
static  gboolean gsb_gui_navigation_remove_report_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data );
static gboolean gsb_gui_navigation_select_line ( GtkTreeSelection *selection,
					  GtkTreeModel * model );
static void gsb_gui_navigation_update_account_iter ( GtkTreeModel * model, 
					      GtkTreeIter * account_iter,
					      gint account_nb );
static  gboolean gsb_gui_navigation_update_account_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data );
static gboolean gsb_gui_navigation_update_notebook ( GtkTreeSelection * selection,
					      GtkTreeModel * model );
static void gsb_gui_navigation_update_report_iter ( GtkTreeModel * model, 
					      GtkTreeIter * report_iter,
					      struct struct_etat * report );
static  gboolean gsb_gui_navigation_update_report_iterator ( GtkTreeModel * tree_model, 
							    GtkTreePath *path, 
							    GtkTreeIter *iter, 
							    gpointer data );
/*END_STATIC*/


/*START_EXTERN*/
extern gint compte_courant_onglet;
extern GSList *liste_struct_etats;
extern GtkTreeStore *model;
extern GtkWidget *notebook_general;
extern GtkTreeSelection * selection;
extern gchar *titre_fichier;
extern GtkWidget *tree_view;
extern GtkWidget *tree_view_liste_echeances;
/*END_EXTERN*/


/** Navigation tree view. */
GtkWidget * navigation_tree_view;

/** Model of the navigation tree. */
GtkTreeModel * navigation_model;

/** Fitered model, without closed accounts. */
GtkTreeModelFilter * navigation_model_filtered;

/** Widget that hold the scheduler calendar. */
GtkWidget * scheduler_calendar;




/**
 * Create the navigation pane on the left of the GUI.  It contains
 * account list as well as shortcuts.
 *
 * \return The newly allocated pane.
 */
GtkWidget * create_navigation_pane ( void )
{
    GtkWidget * sw, *column, *vbox;
    GdkPixbuf * pixbuf;
    GtkTreeIter iter, account_iter, reports_iter;
    GtkCellRenderer * renderer;
    GtkTreeSelection * selection;

    vbox = gtk_vbox_new ( FALSE, 6 );

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER,
				    GTK_POLICY_AUTOMATIC);

    navigation_model = GTK_TREE_MODEL ( gtk_tree_store_new ( NAVIGATION_TOTAL, 
							     GDK_TYPE_PIXBUF,
							     G_TYPE_BOOLEAN, G_TYPE_STRING, 
							     G_TYPE_INT, G_TYPE_INT, 
							     G_TYPE_INT, G_TYPE_POINTER,
							     G_TYPE_INT ) );

    /* Filter to display (or not) closed accounts. */
    navigation_model_filtered = GTK_TREE_MODEL_FILTER ( gtk_tree_model_filter_new ( navigation_model, NULL ) );
    gtk_tree_model_filter_set_visible_func ( navigation_model_filtered, 
					     gsb_gui_navigation_line_visible_p,
					     NULL, NULL );

    /* Create the view */
    navigation_tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL(navigation_model_filtered));
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW(navigation_tree_view), FALSE );
    gtk_container_add ( GTK_CONTAINER(sw), navigation_tree_view );

    /* check the keyboard before all, if we need to move other things that the navigation
     * tree view (for example, up and down on transactions list) */
    g_signal_connect ( navigation_tree_view,
		       "key-press-event", 
		       G_CALLBACK (gsb_gui_navigation_check_key_press),
		       navigation_model_filtered );

    g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (navigation_tree_view)), 
		      "changed", ((GCallback) gsb_gui_navigation_select_line), 
		      navigation_model_filtered );

    renderer = gtk_cell_renderer_pixbuf_new ();
    column = GTK_WIDGET ( gtk_tree_view_column_new_with_attributes ( "", renderer,
								     "visible", NAVIGATION_PIX_VISIBLE, 
								     "pixbuf", NAVIGATION_PIX,
								     NULL) );
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, TRUE);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer, 
				       "text", NAVIGATION_TEXT);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer, 
				       "weight", NAVIGATION_FONT);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer, 
				       "sensitive", NAVIGATION_SENSITIVE);

    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, FALSE);

    gtk_tree_view_append_column ( GTK_TREE_VIEW ( navigation_tree_view ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );
    /* Account list */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "resume.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &account_iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &account_iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Accounts"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_HOME_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1);
    create_account_list ( navigation_model, &account_iter );

    /* Scheduler */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "scheduler.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Scheduler"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_SCHEDULER_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );

    /* Payees */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "payees.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Payees"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_PAYEES_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );

    /* Categories */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "categories.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Categories"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_CATEGORIES_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );

    /* Budgetary lines */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "budgetary_lines.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Budgetary lines"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_BUDGETARY_LINES_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );

#ifdef HAVE_G2BANKING
    /* Gbanking */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "aqbanking.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Aqbanking"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_AQBANKING_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );
#endif

    /* Reports */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "reports.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(navigation_model), &reports_iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(navigation_model), &reports_iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Reports"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_REPORTS_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_SENSITIVE, 1,
		       -1 );
    create_report_list ( navigation_model, &reports_iter );

    /* Finish tree. */
    gtk_tree_view_expand_all ( GTK_TREE_VIEW(navigation_tree_view) );
    gtk_box_pack_start ( GTK_BOX(vbox), sw, TRUE, TRUE, 0 );

    /* Create calendar (hidden for now). */
    scheduler_calendar = creation_partie_gauche_echeancier();
    gtk_box_pack_end ( GTK_BOX(vbox), scheduler_calendar, FALSE, FALSE, 0 );

    gtk_widget_show_all ( vbox );
    gtk_widget_hide_all ( scheduler_calendar );

    return vbox;
}



/**
 * Create a list of tree items that are shortcuts to accounts.
 *
 * \param model		Tree model to insert items into.
 * \param account_iter	Parent iter.
 */
void create_account_list ( GtkTreeModel * model, GtkTreeIter * account_iter )
{
    GSList *list_tmp;

    /* Fill in with accounts. */
    list_tmp = gsb_account_get_list_accounts ();
    while ( list_tmp )
    {
	gint i = gsb_account_get_no_account ( list_tmp -> data );
	GtkTreeIter iter;

	if ( !gsb_account_get_closed_account (i))
	{
	    gtk_tree_store_append(GTK_TREE_STORE(model), &iter, account_iter);
	    gsb_gui_navigation_update_account_iter ( model, &iter, i);
	}

	list_tmp = list_tmp -> next;
    }
}



/**
 * Create a list of tree items that are shortcuts to reports.
 *
 * \param model		Tree model to insert items into.
 * \param reports_iter	Parent iter.
 */
void create_report_list ( GtkTreeModel * model, GtkTreeIter * reports_iter )
{
    GSList *tmp_list;
    GtkTreeIter iter;

    /* Fill in with accounts. */
    tmp_list = liste_struct_etats;
    while ( tmp_list )
    {
	struct struct_etat * etat = tmp_list -> data;

	gtk_tree_store_append(GTK_TREE_STORE(model), &iter, reports_iter);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
			   NAVIGATION_PIX_VISIBLE, FALSE, 
			   NAVIGATION_TEXT, etat -> nom_etat,
			   NAVIGATION_FONT, 400,
			   NAVIGATION_PAGE, GSB_REPORTS_PAGE,
			   NAVIGATION_ACCOUNT, -1,
			   NAVIGATION_SENSITIVE, 1,
			   NAVIGATION_REPORT, etat,
			   -1 );
	
	tmp_list = tmp_list -> next;
    }
}


/**
 *
 *
 */
gboolean gsb_gui_navigation_line_visible_p ( GtkTreeModel * model, GtkTreeIter * iter,
					     gpointer data )
{
    guint account_nb, page;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), iter,
			 NAVIGATION_PAGE, &page,
			 NAVIGATION_ACCOUNT, &account_nb, 
			 -1 );

    if ( page != GSB_ACCOUNT_PAGE || account_nb < 0 )
	return TRUE;

    if ( (! gsb_account_get_closed_account(account_nb)) || etat.show_closed_accounts )
    {
        return TRUE;
    }

    return FALSE;
}



/**
 * Selects the account or management page with an optional account
 * selected in general notebook.
 *
 * \param selection	Selection of tree that triggered event.
 * \param selection	Model of tree that triggered event.
 *
 * \return		FALSE
 */
gboolean gsb_gui_navigation_update_notebook ( GtkTreeSelection * selection,
					      GtkTreeModel * model )
{
    GtkTreeIter iter;
    GValue value = {0, }, value2 = {0, };

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return(FALSE);

    /* we go on the account page */ 

    gtk_tree_model_get_value (model, &iter, NAVIGATION_PAGE, &value);
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), g_value_get_int(&value) );
    g_value_unset (&value);

    gtk_tree_model_get_value (model, &iter, NAVIGATION_ACCOUNT, &value);
    if ( g_value_get_int(&value) >= 0 )
    {
	gsb_account_list_gui_change_current_account ( GINT_TO_POINTER(g_value_get_int(&value) ));
	compte_courant_onglet = g_value_get_int(&value);
	remplissage_details_compte ();
    }

    gtk_tree_model_get_value (model, &iter, NAVIGATION_REPORT, &value2);
    if ( g_value_get_pointer(&value2) > 0 )
    {
	changement_etat ( g_value_get_pointer(&value2) );
    }

    return FALSE;
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
static gboolean gsb_gui_navigation_update_account_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data )
{
    gint account_nb;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_ACCOUNT, &account_nb, 
			 -1 );

    if ( account_nb == GPOINTER_TO_INT ( data ) )
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
static gboolean gsb_gui_navigation_update_report_iterator ( GtkTreeModel * tree_model, 
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
	gsb_gui_navigation_update_report_iter ( tree_model, iter, data );
	return TRUE;
    }

    return FALSE;
}



/**
 * Update informations for an report in navigation pane.
 *
 * \param report_nb	Number of the report that has to be updated.
 */
void gsb_gui_navigation_update_report ( struct struct_etat * report )
{
    gtk_tree_model_foreach ( navigation_model,
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_update_report_iterator,
			     GINT_TO_POINTER ( report ) );
}



/**
 * Update contents of an iter with report data.
 *
 * \param model		Pointer to the model of the navigation tree.
 * \param report_iter	Iter to update.
 * \param data		Number of report as a reference.
 */
void gsb_gui_navigation_update_report_iter ( GtkTreeModel * model, 
					      GtkTreeIter * report_iter,
					      struct struct_etat * report )
{
    gtk_tree_store_set(GTK_TREE_STORE(model), report_iter, 
		       NAVIGATION_TEXT, report -> nom_etat, 
		       NAVIGATION_PAGE, GSB_REPORTS_PAGE,
		       NAVIGATION_REPORT, report,
		       NAVIGATION_ACCOUNT, -1,
		       NAVIGATION_SENSITIVE, 1,
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
static gboolean gsb_gui_navigation_remove_report_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data )
{
    gpointer report;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_REPORT, &report, 
			 -1 );

    if ( report == data )
    {
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
void gsb_gui_navigation_add_report ( struct struct_etat * report )
{
    GtkTreeIter parent, iter;
    GtkTreeSelection * selection;
    GtkTreePath * path;

    path = gtk_tree_path_new ();
    gtk_tree_path_prepend_index ( path, GSB_REPORTS_PAGE - 1 );
    gtk_tree_model_get_iter ( navigation_model, &parent, path );
    gtk_tree_store_append ( GTK_TREE_STORE(navigation_model), &iter, &parent );

    gsb_gui_navigation_update_report_iter ( navigation_model, &iter, report );    

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    gtk_tree_selection_select_iter ( selection, &iter );
}



/**
 * Remove report from the navigation pane.
 *
 * \param report_nb	Report ID to add.
 */
void gsb_gui_navigation_remove_report ( struct struct_etat * report )
{
    gtk_tree_model_foreach ( navigation_model, 
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_remove_report_iterator, 
			     GINT_TO_POINTER ( report ) );
   
}



/**
 * Update informations for an account in navigation pane.
 *
 * \param account_nb	Number of the account that has to be updated.
 */
void gsb_gui_navigation_update_account ( gint account_nb )
{
    gtk_tree_model_foreach ( navigation_model, 
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_update_account_iterator, 
			     GINT_TO_POINTER ( account_nb ) );
}



/**
 * Update contents of an iter with account data.
 *
 * \param model		Pointer to the model of the navigation tree.
 * \param account_iter	Iter to update.
 * \param data		Number of account as a reference.
 */
void gsb_gui_navigation_update_account_iter ( GtkTreeModel * model, 
					      GtkTreeIter * account_iter,
					      gint account_nb )
{
    GdkPixbuf * pixbuf;
    gchar * account_icon;
	    
    switch ( gsb_account_get_kind ( account_nb ) )
    {
	case GSB_TYPE_BANK:
	    account_icon = "bank-account";
	    break;

	case GSB_TYPE_CASH:
	    account_icon = "money";
	    break;

	case GSB_TYPE_ASSET:
	case GSB_TYPE_LIABILITIES:
	    account_icon = "asset";
	    break;

	default:
	    account_icon = "warnings";
	    break;
    }

    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, 
						     C_DIRECTORY_SEPARATOR,
						     account_icon, ".png", NULL ), NULL );
    gtk_tree_store_set(GTK_TREE_STORE(model), account_iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_TEXT, gsb_account_get_name ( account_nb ), 
		       NAVIGATION_FONT, 400,
		       NAVIGATION_PAGE, GSB_ACCOUNT_PAGE,
		       NAVIGATION_ACCOUNT, account_nb,
		       NAVIGATION_SENSITIVE, !gsb_account_get_closed_account ( account_nb ),
		       -1 );
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
static gboolean gsb_gui_navigation_remove_account_iterator ( GtkTreeModel * tree_model, 
							     GtkTreePath *path, 
							     GtkTreeIter *iter, 
							     gpointer data )
{
    gint account_nb;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 NAVIGATION_ACCOUNT, &account_nb, 
			 -1 );

    if ( account_nb == GPOINTER_TO_INT ( data ) )
    {
	gtk_tree_store_remove ( GTK_TREE_STORE(tree_model), iter );
	return TRUE;
    }

    return FALSE;
}



/**
 * Add an account to the navigation pane.
 *
 * \param account_nb	Account ID to add.
 */
void gsb_gui_navigation_add_account ( gint account_nb )
{
    GtkTreeIter parent, iter;
    GtkTreeSelection * selection;

    gtk_tree_model_get_iter_first ( navigation_model, &parent );
    gtk_tree_store_append ( GTK_TREE_STORE(navigation_model), &iter, &parent );

    gsb_gui_navigation_update_account_iter ( navigation_model, &iter, account_nb );    

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    gtk_tree_selection_select_iter ( selection, &iter );
}



/**
 * Remove account from the navigation pane.
 *
 * \param account_nb	Account ID to add.
 */
void gsb_gui_navigation_remove_account ( gint account_nb )
{
    gtk_tree_model_foreach ( navigation_model, 
			     (GtkTreeModelForeachFunc) gsb_gui_navigation_remove_account_iterator, 
			     GINT_TO_POINTER ( account_nb ) );
   
}



/**
 * Callback executed when the selection of the navigation tree
 * changed.
 *
 * \param selection	The selection that triggered event.
 * \param model		Tree model associated to selection.
 *
 * \return FALSE
 */
gboolean gsb_gui_navigation_select_line ( GtkTreeSelection *selection,
					  GtkTreeModel * model )
{
    GtkTreeIter iter;
    GValue value = {0, }, pointer = {0, };
    gchar * title, * suffix = "";
    gint account_nb;

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return FALSE;

    gtk_tree_model_get_value (model, &iter, NAVIGATION_PAGE, &value);

    switch ( g_value_get_int(&value) )
    {
	case GSB_HOME_PAGE:
	    title = g_strconcat ( "Grisbi : " , titre_fichier, NULL );
	    break;

	case GSB_ACCOUNT_PAGE:
	    gtk_tree_model_get (model, &iter, NAVIGATION_ACCOUNT, &account_nb, -1);
	    title = g_strconcat ( _("Account transactions"), " : ",
				  gsb_account_get_name ( account_nb ),
				  NULL );
	    if ( gsb_account_get_closed_account ( account_nb ) )
	    {
		title = g_strconcat ( title, " (", _("closed"), ")", NULL );
	    }
	    suffix = g_strdup_printf ( "%4.2f %s", 
				       gsb_account_get_current_balance ( compte_courant_onglet ),
				       devise_code ( devise_par_no ( gsb_account_get_currency ( compte_courant_onglet ) ) ) );
	    break;

	case GSB_PAYEES_PAGE:
	    title = _("Payees");
	    break;

	case GSB_CATEGORIES_PAGE:
	    title = _("Categories");
	    break;

	case GSB_BUDGETARY_LINES_PAGE:
	    title = _("Budgetary lines");
	    break;

	case GSB_SCHEDULER_PAGE:
	    title = _("Scheduled transactions");
	    break;

	case GSB_REPORTS_PAGE:
	    gtk_tree_model_get_value (model, &iter, NAVIGATION_REPORT, &pointer);
	    if ( g_value_get_pointer(&pointer) )
	    {
		struct struct_etat * etat =  g_value_get_pointer ( &pointer );
		title = g_strconcat ( _("Report"), " : ", etat -> nom_etat, NULL );
	    }
	    else
	    {
		title = _("Reports");
	    }
	    break;

	default:
	    title = "B0rk";
	    break;
    }

    gsb_gui_headings_update ( title, suffix );
    gsb_gui_navigation_update_notebook ( selection, model );

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

   if (! gtk_tree_selection_get_selected ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
									NULL,
									&iter))
	return FALSE;

   gtk_tree_model_get (model,
		       &iter,
		       NAVIGATION_PAGE, &page,
		       -1 );

    switch ( page )
    {
	case GSB_HOME_PAGE:
	    /* nothing to do here for now */
	    break;

	case GSB_ACCOUNT_PAGE:
	    switch ( ev -> keyval )
	    {
		case GDK_Return :		/* entrée */
		case GDK_KP_Enter :
		case GDK_Up :		/* touches flèche haut */
		case GDK_KP_Up :
		case GDK_Down :		/* touches flèche bas */
		case GDK_KP_Down :
		case GDK_Delete:		/*  del  */
		case GDK_P:			/* touche P */
		case GDK_p:			/* touche p */
		case GDK_r:			/* touche r */
		case GDK_R:			/* touche R */
		    gsb_transactions_list_key_press ( gsb_transactions_list_get_tree_view (),
						      ev );
		    return TRUE;
	    }
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
	    switch ( ev -> keyval )
	    {
		case GDK_Return :		/* touches entrée */
		case GDK_KP_Enter :
		case GDK_Up :		/* touches flèche haut */
		case GDK_KP_Up :
		case GDK_Down :		/* touches flèche bas */
		case GDK_KP_Down :
		case GDK_Delete :               /*  del  */

		    traitement_clavier_liste_echeances  ( tree_view_liste_echeances,
							  ev );
		    return TRUE;
	    }
	    break;

	case GSB_REPORTS_PAGE:
	    /* nothing to do here for now */
	    break;
    }

    return FALSE;
}
