/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2008-2012 Pierre Biava (grisbi@pierre.biava.name)     */
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
#include <gdk/gdkkeysyms.h>

/*START_INCLUDE*/
#include "gsb_navigation_view.h"
#include "grisbi_app.h"
#include "gsb_data_account.h"
#include "gsb_data_report.h"
#include "gsb_dirs.h"
#include "gsb_navigation.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "structures.h"
#include "utils.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


#define GSB_NAVIGATION_VIEW_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object),\
                        GSB_TYPE_NAVIGATION_VIEW, GsbNavigationViewPrivate))


/* Holds data for the navigation tree.  */
enum _gsb_navigation_view_cols {
    GSB_NAVIGATION_VIEW_PIX,
    GSB_NAVIGATION_VIEW_PIX_VISIBLE,
    GSB_NAVIGATION_VIEW_TEXT,
    GSB_NAVIGATION_VIEW_FONT,
    GSB_NAVIGATION_VIEW_PAGE,
    GSB_NAVIGATION_VIEW_ACCOUNT,
    GSB_NAVIGATION_VIEW_REPORT,
    GSB_NAVIGATION_VIEW_SENSITIVE,
    GSB_NAVIGATION_VIEW_ORDRE,        /* ordre des pages dans le modèle */
    GSB_NAVIGATION_VIEW_TOTAL,
};


#define DEFAULT_NAVIGATION_ORDER_LIST "0-2-3-4-5-6-7"

struct _GsbNavigationViewPrivate
{
    /** Model of the navigation tree. */
    GtkTreeModel *model;

    /* liste des pages */
    GQueue *pages_list;

    /* organisation des pages */
    gint navigation_sorting_accounts;

    /* expander */
    gboolean account_expander;
    gboolean report_expander;
};

G_DEFINE_TYPE(GsbNavigationView, gsb_navigation_view, GTK_TYPE_TREE_VIEW)


static GtkTargetEntry row_targets[] =
{
    { "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
};


/* CALLBACKS */
/**
 * remplit le model
 *
 * \param window
 * \param priv      private structure of GsbNavigationView
 *
 * \return
 **/
static void gsb_navigation_view_realized ( GtkWidget *window,
                        GsbNavigationViewPrivate *priv )
{

    /* return */
}


/**
 * Check the key pressed on the navigation tree view,
 * if need, stop the event to do another thing with that key
 *
 * \param tree_view     the navigation tree_view
 * \param ev            the key event pressed
 * \param priv          private structure of GsbNavigationView
 *
 * \return FALSE : the signal continue / TRUE : the signal is stopped here
 * */
static gboolean gsb_navigation_view_check_key_press ( GtkWidget *tree_view,
                        GdkEventKey *ev,
                        GsbNavigationViewPrivate *priv )
{
    gint page;
    GtkTreeIter iter;
    GtkTreePath *path;

    if ( !gtk_tree_selection_get_selected ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ),
                        NULL,
                        &iter ) )
        return FALSE;

    gtk_tree_model_get ( priv->model, &iter, GSB_NAVIGATION_VIEW_PAGE, &page, -1 );
    path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( priv->model ), &iter );

    switch ( page )
    {
    case GSB_HOME_PAGE:
        /* expand or collapse subtree */
        if (ev -> keyval == GDK_KEY_Right)
        {
            gtk_tree_view_expand_row ( GTK_TREE_VIEW ( tree_view ), path, FALSE );
            priv->account_expander = TRUE;
        }
        if (ev -> keyval == GDK_KEY_Left)
        {
            gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( tree_view ), path );
            priv->account_expander = FALSE;
        }
        if ( ev->keyval == GDK_KEY_space )
        {
            priv->account_expander = !priv->account_expander;
            if ( priv->account_expander )
                gtk_tree_view_expand_row ( GTK_TREE_VIEW ( tree_view ), path, FALSE );
            else
                gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( tree_view ), path );
        }
        break;

    case GSB_REPORTS_PAGE:
        /* expand or collapse subtree */
        if (ev -> keyval == GDK_KEY_Right)
        {
            gtk_tree_view_expand_row ( GTK_TREE_VIEW ( tree_view ), path, FALSE );
            priv->report_expander = TRUE;
        }
        if (ev -> keyval == GDK_KEY_Left)
        {
            gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( tree_view ), path );
            priv->report_expander = FALSE;
        }
        if ( ev->keyval == GDK_KEY_space )
        {
            priv->report_expander = !priv->report_expander;
            if ( priv->report_expander )
                gtk_tree_view_expand_row ( GTK_TREE_VIEW ( tree_view ), path, FALSE );
            else
                gtk_tree_view_collapse_row ( GTK_TREE_VIEW ( tree_view ), path );
        }
        break;

    case GSB_ACCOUNT_PAGE:
        /* when come here, if we press the right key, give the focus to the list */
        if (ev -> keyval == GDK_KEY_Right)
        gtk_widget_grab_focus ( gsb_transactions_list_get_tree_view () );
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
        gtk_widget_grab_focus ( gsb_scheduler_list_get_tree_view () );
        break;
    }

    gtk_tree_path_free ( path );

    return FALSE;
}


/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
static gboolean gsb_navigation_view_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null )
{
    /* show the popup */
    if ( ev -> button == GSB_RIGHT_BUTTON )
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
 * fonction qui mémorise l'etat de l'expander pour les comptes et les états
 *
 * \param tree_view
 * \param iter
 * \param path 
 * \param priv          private structure of GsbNavigationView
 *
 * \return
 **/
static void gsb_navigation_view_activate_expander ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreePath *path,
                        GsbNavigationViewPrivate *priv )
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter_selected;
    GtkTreePath *path_selected = NULL;
    gint type_page;
    gint etat_expander;

    etat_expander = gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( tree_view ), path );
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    if ( gtk_tree_selection_get_selected ( selection, &model, &iter_selected ) )
        path_selected = gtk_tree_model_get_path ( model, &iter_selected );

    gtk_tree_model_get ( model, iter, GSB_NAVIGATION_VIEW_PAGE, &type_page, -1 );

    switch ( type_page )
    {
        case GSB_HOME_PAGE :
        case GSB_ACCOUNT_PAGE :
            priv->account_expander = etat_expander;
        break;
        case GSB_REPORTS_PAGE :
            priv->report_expander = etat_expander;
        break;
    }

    if ( etat_expander == 0 && path_selected == NULL )
        gtk_tree_selection_select_iter ( selection, iter );
}


/* SELECTION */
/**
 * Set the selection of the navigation list depending on desired page
 * and/or account or report, but only for a branch of the tree.
 *
 * \param selection         Selection to set.
 * \param iter              Current iter to iterate over.
 * \param page              Page to switch to.
 * \param account_number    If page is GSB_ACCOUNT_PAGE, switch to given account.
 * \param report            If page is GSB_REPORTS, switch to given report.
 *
 * \return                  TRUE on success.
 */
static void gsb_navigation_view_set_selection_branch ( GtkTreeSelection *selection,
                        GtkTreeIter *iter,
                        gint page,
                        gint account_number,
                        gpointer report )
{
    GtkTreeModel *navigation_model;

    navigation_model = gsb_gui_navigation_get_model ();

    do
    {
        gint iter_page, iter_account_nb;
        gpointer iter_report;

        gtk_tree_model_get ( GTK_TREE_MODEL ( navigation_model ), iter,
                     GSB_NAVIGATION_VIEW_REPORT, &iter_report,
                     GSB_NAVIGATION_VIEW_ACCOUNT, &iter_account_nb,
                     GSB_NAVIGATION_VIEW_PAGE, &iter_page,
                     -1 );

        if ( iter_page == page
         &&
         ! ( page == GSB_ACCOUNT_PAGE
         &&
         iter_account_nb != account_number )
         &&
         ! ( page == GSB_REPORTS_PAGE
         &&
         iter_report != report ) )
        {
            gtk_tree_selection_select_iter ( selection, iter );
        }

        if ( gtk_tree_model_iter_has_child ( GTK_TREE_MODEL ( navigation_model ), iter ) )
        {
            GtkTreeIter child;

            gtk_tree_model_iter_children ( GTK_TREE_MODEL ( navigation_model ), &child, iter );
            gsb_navigation_view_set_selection_branch ( selection, &child,
                                  page, account_number, report );
        }
    }
    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( navigation_model ), iter ) );

    return;
}


/*  ACCOUNTS */
/**
 * Update contents of an iter with account data.
 *
 * \param model         Pointer to the model of the navigation tree.
 * \param account_iter  Iter to update.
 * \param data          Number of account as a reference.
 *
 * \return
 */
static void gsb_navigation_view_update_account_iter ( GtkTreeModel *model,
                        GtkTreeIter *account_iter,
                        gint account_number )
{
    GdkPixbuf * pixbuf = NULL;

    pixbuf = gsb_data_account_get_account_icon_pixbuf ( account_number );

    gtk_tree_store_set ( GTK_TREE_STORE ( model ), account_iter,
                        GSB_NAVIGATION_VIEW_PIX, pixbuf,
                        GSB_NAVIGATION_VIEW_PIX_VISIBLE, TRUE,
                        GSB_NAVIGATION_VIEW_TEXT, gsb_data_account_get_name ( account_number ),
                        GSB_NAVIGATION_VIEW_FONT, 400,
                        GSB_NAVIGATION_VIEW_PAGE, GSB_ACCOUNT_PAGE,
                        GSB_NAVIGATION_VIEW_ACCOUNT, account_number,
                        GSB_NAVIGATION_VIEW_SENSITIVE, !gsb_data_account_get_closed_account ( account_number ),
                        GSB_NAVIGATION_VIEW_REPORT, -1,
                        -1 );
}


/**
 * Add an account to the navigation pane.
 *
 * \param tree_view
 * \param account_number        Account ID to add.
 * \param switch_to_account     TRUE to show the account, FALSE to just create it
 *
 * \return
 */
static void gsb_navigation_view_add_account ( GsbNavigationView *tree_view,
                        gint account_number,
                        gboolean switch_to_account )
{
    GtkTreeModel *model;
    GtkTreeIter parent, iter;
    GtkTreePath *path;

    model = tree_view->priv->model;
    path = gsb_navigation_view_get_page_path ( model, GSB_HOME_PAGE );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL( model ), &parent, path );
    gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, &parent );

    gsb_navigation_view_update_account_iter ( GTK_TREE_MODEL ( model ), &iter, account_number );

    if ( switch_to_account )
    {
        GtkTreeSelection * selection;

        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
        gtk_tree_selection_select_iter ( selection, &iter );
    }
}


/*  REPORTS */
/**
 * Create a list of tree items that are shortcuts to reports.
 *
 * \param tree_view Tree model to insert items into.
 *
 * \return
 */
static void gsb_navigation_view_create_report_list ( GsbNavigationView *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter parent, child;
    GtkTreePath *path;
    GSList *tmp_list;

    devel_debug (NULL);
    model = tree_view->priv->model;
    path = gsb_navigation_view_get_page_path ( model, GSB_REPORTS_PAGE );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL( model ), &parent, path );

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

        gtk_tree_store_append ( GTK_TREE_STORE ( model ), &child, &parent );
        gtk_tree_store_set ( GTK_TREE_STORE ( model ), &child,
                        GSB_NAVIGATION_VIEW_PIX_VISIBLE, FALSE,
                        GSB_NAVIGATION_VIEW_TEXT, gsb_data_report_get_report_name ( report_number ),
                        GSB_NAVIGATION_VIEW_FONT, 400,
                        GSB_NAVIGATION_VIEW_PAGE, GSB_REPORTS_PAGE,
                        GSB_NAVIGATION_VIEW_ACCOUNT, -1,
                        GSB_NAVIGATION_VIEW_SENSITIVE, 1,
                        GSB_NAVIGATION_VIEW_REPORT, report_number,
                        -1 );

        tmp_list = tmp_list -> next;
    }

    /* Expand stuff */
    if ( tree_view->priv->report_expander )
    {
        path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( model ), &parent );
        gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( tree_view ), path );
        gtk_tree_path_free ( path );
    }
}


/*  PAGES_LIST */
/**
 * insère dans le modèle le type de page passé en paramètre
 *
 * \param tree_view
 * \param type de page
 * \param ordre dans la liste
 *
 * \return
 **/
static void gsb_navigation_view_set_navigation_pages ( GsbNavigationView *tree_view,
                        gint type_page,
                        gint ordre )
{
    GdkPixbuf *pixbuf;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *title = NULL;
    gchar *tmp_str = NULL;

    model = tree_view->priv->model;

    switch ( type_page )
    {
        case GSB_HOME_PAGE :
            tmp_str = g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), "ac_home.png", NULL );
            title = g_strdup ( _("Accounts") );
        break;
        case GSB_SCHEDULER_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "scheduler.png", NULL );
            title = g_strdup ( _("Scheduler") );
        break;
        case GSB_PAYEES_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "payees.png", NULL );
            title = g_strdup ( _("Payees") );
        break;
        case GSB_SIMULATOR_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "ac_liability.png", NULL );
            title = g_strdup ( _("Credits simulator") );
        break;
        case GSB_CATEGORIES_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "categories.png", NULL );
            title = g_strdup ( _("Categories") );
        break;
        case GSB_BUDGETARY_LINES_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "budgetary_lines.png", NULL );
            title = g_strdup ( _("Budgetary lines") );
        break;
        case GSB_REPORTS_PAGE :
            tmp_str = g_build_filename( gsb_dirs_get_pixmaps_dir ( ), "reports.png", NULL );
            title = g_strdup ( _("Reports") );
        break;
    }

    pixbuf = gdk_pixbuf_new_from_file ( tmp_str , NULL );
    gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, NULL );
    gtk_tree_store_set( GTK_TREE_STORE ( model ), &iter,
                        GSB_NAVIGATION_VIEW_PIX, pixbuf,
                        GSB_NAVIGATION_VIEW_TEXT, title,
                        GSB_NAVIGATION_VIEW_PIX_VISIBLE, TRUE,
                        GSB_NAVIGATION_VIEW_FONT, 800,
                        GSB_NAVIGATION_VIEW_PAGE, type_page,
                        GSB_NAVIGATION_VIEW_ACCOUNT, -1,
                        GSB_NAVIGATION_VIEW_REPORT, -1,
                        GSB_NAVIGATION_VIEW_SENSITIVE, 1,
                        GSB_NAVIGATION_VIEW_ORDRE, ordre,
                        -1);

    g_free ( tmp_str );
    g_free ( title );

    switch ( type_page )
    {
        case GSB_HOME_PAGE :
            gsb_navigation_view_create_account_list ( tree_view );
        break;
        case GSB_REPORTS_PAGE :
            gsb_navigation_view_create_report_list ( tree_view );
        break;
    }

}


/**
 * remplit la liste de l'ordre des pages
 *
 * \param priv                  private structure of GsbNavigationView
 * \param ordre de la liste
 *
 * \return                      always TRUE
 **/
static gboolean gsb_navigation_view_set_page_list_order ( GsbNavigationViewPrivate *priv,
                        const gchar *order_list )
{
    gchar **pointeur_char;
    gint i;
    gint nbre_pages;

    devel_debug (NULL);
    pointeur_char = g_strsplit ( order_list, "-", 0 );

    nbre_pages = g_strv_length ( pointeur_char );

    if ( nbre_pages != ( GSB_GENERAL_NOTEBOOK_PAGES -1 ) )
    {
        g_strfreev ( pointeur_char );
        pointeur_char = g_strsplit ( DEFAULT_NAVIGATION_ORDER_LIST, "-", 0 );
        nbre_pages = GSB_GENERAL_NOTEBOOK_PAGES -1;
    }

    for ( i = 0 ; i < nbre_pages ; i++ )
    {
        struct_page *page;

        page = g_malloc0 ( sizeof ( struct_page ) );
        page -> ordre = i;
        page -> type_page = utils_str_atoi ( pointeur_char[i] );

        g_queue_push_tail ( priv->pages_list, page );
    }

    g_strfreev ( pointeur_char );

    /* return */
    return TRUE;
}


/**
 * initialise la liste des pages du tree_view
 *
 * \param priv      private structure of GsbNavigationView
 *
 * \return
 **/
static void gsb_navigation_view_init_pages_list ( GsbNavigationViewPrivate *priv )
{
    GQueue *new_queue;
    GrisbiWindowEtat *etat;

    devel_debug (NULL);
    etat = grisbi_window_get_struct_etat ();

    new_queue = g_queue_new ( );
    priv->pages_list = new_queue;

    if ( etat->navigation_list_order )
        gsb_navigation_view_set_page_list_order ( priv, etat->navigation_list_order );
    else
        gsb_navigation_view_set_page_list_order ( priv, DEFAULT_NAVIGATION_ORDER_LIST );
}


/**
 * free pages_list
 *
 * \param page list
 *
 * \return
 **/
static void gsb_navigation_view_free_pages_list ( GQueue *pages_list )
{
    if ( pages_list == NULL )
        return;

    g_queue_foreach ( pages_list, (GFunc) g_free, NULL );
    g_queue_free ( pages_list );
}


/* TREE_MODEL */
/**
 * fonction de tri des items du model passé en paramètre
 *
 * \param model         model
 * \param iter          a
 * \param iter          b
 * \param user_data     not use
 *
 * \return gint         -1, 0 or 1, if a is < (before), == or > (after) than b.
 **/
static gint gsb_navigation_view_sort_column ( GtkTreeModel *model,
                        GtkTreeIter *a,
                        GtkTreeIter *b,
                        gpointer user_data )
{
    gint ordre_a, page_a, account_a, report_a;
    gint ordre_b, page_b, account_b, report_b;

    if ( ! model )
        return FALSE;

    gtk_tree_model_get ( model, a,
                        GSB_NAVIGATION_VIEW_PAGE, &page_a,
                        GSB_NAVIGATION_VIEW_ACCOUNT, &account_a,
                        GSB_NAVIGATION_VIEW_REPORT, &report_a,
                        GSB_NAVIGATION_VIEW_ORDRE, &ordre_a,
                        -1 );

    gtk_tree_model_get ( model, b,
                        GSB_NAVIGATION_VIEW_PAGE, &page_b,
                        GSB_NAVIGATION_VIEW_ACCOUNT, &account_b,
                        GSB_NAVIGATION_VIEW_REPORT, &report_b,
                        GSB_NAVIGATION_VIEW_ORDRE, &ordre_b,
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
 * remplit le modèle
 *
 * \param tree_view
 *
 * \return
 **/
static void gsb_navigation_view_fill_model ( GsbNavigationView *tree_view )
{
    GQueue *tmp_queue;
    gint i;

    devel_debug (NULL);
    tmp_queue = tree_view->priv->pages_list;

    for ( i = 0 ; i < tmp_queue -> length ; i++ )
    {
        struct_page *page;

        page = g_queue_peek_nth ( tmp_queue, i );
        gsb_navigation_view_set_navigation_pages ( tree_view, page -> type_page, i );
    }
}


/**
 * create navigation_view model
 *
 * \param priv  private structure of GsbNavigationView
 *
 * \return
 **/
static void gsb_navigation_view_create_model ( GsbNavigationViewPrivate *priv )
{
    GtkTreeStore *model;

    devel_debug (NULL);
    model = gtk_tree_store_new ( GSB_NAVIGATION_VIEW_TOTAL,
                        GDK_TYPE_PIXBUF,        /* GSB_NAVIGATION_VIEW_PIX */
                        G_TYPE_BOOLEAN,         /* GSB_NAVIGATION_VIEW_PIX_VISIBLE */
                        G_TYPE_STRING,          /* GSB_NAVIGATION_VIEW_TEXT */
                        G_TYPE_INT,             /* GSB_NAVIGATION_VIEW_FONT */
                        G_TYPE_INT,             /* GSB_NAVIGATION_VIEW_PAGE */
                        G_TYPE_INT,             /* GSB_NAVIGATION_VIEW_ACCOUNT */
                        G_TYPE_INT,             /* GSB_NAVIGATION_VIEW_REPORT */
                        G_TYPE_INT,             /* GSB_NAVIGATION_VIEW_SENSITIVE */
                        G_TYPE_INT );           /* GSB_NAVIGATION_VIEW_ORDRE */

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( model ),
                        GSB_NAVIGATION_VIEW_ORDRE,
                        GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( model ),
                        GSB_NAVIGATION_VIEW_ORDRE,
                        gsb_navigation_view_sort_column,
                        NULL,
                        NULL );

    priv->model = GTK_TREE_MODEL ( model );
}


/* DRAG AND DROP */
/**
 * change the position of an item in the list of pages
 *
 * \param src_ordre     the position of item we want to move
 * \param dest_ordre    the position before we want to move, or -1 to set at the end of list
 *
 * \return FALSE
 * */
gboolean gsb_navigation_view_move_ordre ( gint src_ordre,
                        gint dst_ordre )
{
/*     GQueue *tmp_queue;
 *     GList *dst_list;
 *     gint i;
 *     struct_page *page = NULL;
 */
/* 
 *     tmp_queue = pages_list;
 *     for ( i = 0 ; i < tmp_queue -> length ; i++ )
 *     {
 *         page = g_queue_peek_nth ( tmp_queue, i );
 *         if ( page -> ordre == src_ordre )
 *             break;
 *     }
 * 
 *     g_queue_pop_nth ( pages_list, i );
 * 
 *     dst_list = g_queue_peek_nth_link ( pages_list, dst_ordre );
 *     if ( dst_list )
 *         g_queue_insert_before ( pages_list, dst_list, page );
 *     else
 *         g_queue_push_tail ( tmp_queue, page );
 */

    /* on reconstruit le modèle */
/*     gtk_tree_store_clear ( GTK_TREE_STORE ( navigation_model ) );
 * 
 *     tmp_queue = pages_list;
 */

/*     for ( i = 0 ; i < tmp_queue -> length ; i++ )
 *     {
 *         page = g_queue_peek_nth ( tmp_queue, i );
 *         page -> ordre = i;
 *         gsb_gui_navigation_set_navigation_pages ( navigation_model, page -> type_page, i );
 *     }
 */

    /* return */
    return FALSE;
}


/**
 * Fill the drag & drop structure with the path of selected column.
 * This is an interface function called from GTK, much like a callback.
 *
 * \param drag_source       Not used.
 * \param path              Original path for the gtk selection.
 * \param selection_data    A pointer to the drag & drop structure.
 *
 * \return FALSE, to allow future processing by the callback chain.
 */
gboolean gsb_navigation_view_drag_data_get ( GtkTreeDragSource *drag_source,
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
/*         gtk_tree_set_row_drag_data ( selection_data, GTK_TREE_MODEL ( navigation_model ), path );  */
    }

    /* return */
    return FALSE;
}



/**
 * Performs the actions to do at the end of the drag and drop.
 * This is an interface function called from GTK, much like a callback.
 *
 * \param drag_dest         Not used.
 * \param path              Original path for the gtk selection.
 * \param selection_data    A pointer to the drag & drop structure.
 *
 * \return FALSE, to allow future processing by the callback chain.
 */
gboolean gsb_navigation_view_drag_data_received ( GtkTreeDragDest *drag_dest,
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
        gint src_report, dst_report = -1;
        gint src_account, dst_account = -1;
        gint src_ordre, dst_ordre = -1;

        gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);

        if ( gtk_tree_model_get_iter ( GTK_TREE_MODEL(model), &iter, dest_path ) )
            gtk_tree_model_get (model , &iter,
                        GSB_NAVIGATION_VIEW_REPORT, &dst_report,
                        GSB_NAVIGATION_VIEW_ACCOUNT, &dst_account,
                        GSB_NAVIGATION_VIEW_ORDRE, &dst_ordre,
                        -1 );

        if ( gtk_tree_model_get_iter ( GTK_TREE_MODEL(model), &iter, orig_path ) )
            gtk_tree_model_get ( model, &iter,
                        GSB_NAVIGATION_VIEW_REPORT, &src_report,
                        GSB_NAVIGATION_VIEW_ACCOUNT, &src_account,
                        GSB_NAVIGATION_VIEW_ORDRE, &src_ordre,
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
            gsb_navigation_view_move_ordre ( src_ordre, dst_ordre );

        /* update the tree view */
        gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( model ),
                        GSB_NAVIGATION_VIEW_ORDRE, GTK_SORT_ASCENDING );
        gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( model ),
                        GSB_NAVIGATION_VIEW_ORDRE, gsb_navigation_view_sort_column,
                        NULL, NULL );

        /* update the order of accounts in first page */
/*         mise_a_jour_liste_comptes_accueil = TRUE;  */

/*         gsb_file_set_modified ( TRUE );  */
    }
    return FALSE;
}


/**
 * Verifies that the actions to do at the end of the drag and drop are possible.
 * This is an interface function called from GTK, much like a callback.
 *
 * \param drag_dest         Not used.
 * \param path              Original path for the gtk selection.
 * \param selection_data    A pointer to the drag & drop structure.
 *
 * \return FALSE, to allow future processing by the callback chain.
 */
gboolean gsb_navigation_view_row_drop_possible ( GtkTreeDragDest *drag_dest,
                        GtkTreePath *dest_path,
                        GtkSelectionData *selection_data )
{
    if ( dest_path && selection_data )
    {
        GtkTreePath *orig_path;
        GtkTreeModel *model;
        gint src_report, dst_report = -1;
        gint src_account, dst_account = -1;
        gint src_ordre, dst_ordre = -1;
        GtkTreeIter iter;

        gtk_tree_get_row_drag_data ( selection_data, &model, &orig_path );

        if ( gtk_tree_model_get_iter ( model, &iter, orig_path ) )
            gtk_tree_model_get ( model, &iter,
                        GSB_NAVIGATION_VIEW_REPORT, &src_report,
                        GSB_NAVIGATION_VIEW_ACCOUNT, &src_account,
                        GSB_NAVIGATION_VIEW_ORDRE, &src_ordre,
                        -1 );

        if ( gtk_tree_model_get_iter ( model, &iter, dest_path ) )
            gtk_tree_model_get ( model, &iter,
                        GSB_NAVIGATION_VIEW_REPORT, &dst_report,
                        GSB_NAVIGATION_VIEW_ACCOUNT, &dst_account,
                        GSB_NAVIGATION_VIEW_ORDRE, &dst_ordre,
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


/* FONCTION DE DESTRUCTION */
/**
 * finalise GsbVavigationView
 *
 * \param object
 *
 * \return
 */
static void gsb_navigation_view_finalize ( GObject *object )
{
    GsbNavigationView *self;
    GsbNavigationViewPrivate *priv;

    self = GSB_NAVIGATION_VIEW ( object );
    devel_debug (NULL);

    priv = self->priv;

    /* libération de la mémoire utilisée pour l'ordre des pages */
    gsb_navigation_view_free_pages_list ( priv->pages_list );

    /* libération de l'objet */
    G_OBJECT_CLASS ( gsb_navigation_view_parent_class )->finalize ( object );
}


/* FONCTION DE CONSTRUCTION */
/**
 * Initialise GrisbiPrefsClass
 *
 * \param
 *
 * \return
 */
static void gsb_navigation_view_class_init ( GsbNavigationViewClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS ( klass );

    object_class->finalize = gsb_navigation_view_finalize;

    g_type_class_add_private ( object_class, sizeof ( GsbNavigationViewPrivate ) );
}


static void gsb_navigation_view_init ( GsbNavigationView *self )
{
    GtkCellRenderer *renderer;
/*     GtkTreeDragDestIface *navigation_dst_iface;
 *     GtkTreeDragSourceIface *navigation_src_iface;
 */
    GtkTreeViewColumn *column;
    gint xpad;
    gint ypad;
    GsbNavigationViewPrivate *priv;
    GrisbiAppConf *conf;

    devel_debug (NULL);
    self->priv = priv = GSB_NAVIGATION_VIEW_GET_PRIVATE ( self );

    /* initialisation d'account_expander */
    priv->account_expander = TRUE;

    /* create model */
    gsb_navigation_view_create_model ( priv );

    /* initialise la liste des pages */
    gsb_navigation_view_init_pages_list ( priv );

    /* Enable drag & drop */
    gtk_tree_view_enable_model_drag_source ( GTK_TREE_VIEW ( self ),
                        GDK_BUTTON1_MASK,
                        row_targets, 1,
                        GDK_ACTION_MOVE | GDK_ACTION_COPY );
    gtk_tree_view_enable_model_drag_dest ( GTK_TREE_VIEW ( self ),
                        row_targets,
                        1,
                        GDK_ACTION_MOVE | GDK_ACTION_COPY );

    gtk_tree_view_set_reorderable ( GTK_TREE_VIEW ( self ), TRUE );
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( self ) ),
                        GTK_SELECTION_SINGLE );
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW ( self ), FALSE );
    gtk_tree_view_set_model ( GTK_TREE_VIEW ( self ), GTK_TREE_MODEL( priv->model ) );

    /* Handle drag & drop */
/*     navigation_dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE ( priv->model );
 *     if ( navigation_dst_iface )
 *     {
 *         navigation_dst_iface -> drag_data_received = &gsb_navigation_view_drag_data_received;
 *         navigation_dst_iface -> row_drop_possible = &gsb_navigation_view_row_drop_possible;
 *     }
 */

/*     navigation_src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE ( priv->model );
 *     if ( navigation_src_iface )
 *     {
 *         gtk_selection_add_target ( self,
 *                         GDK_SELECTION_PRIMARY,
 *                         GDK_SELECTION_TYPE_ATOM,
 *                         1 );
 *         navigation_src_iface -> drag_data_get = &gsb_navigation_view_drag_data_get;
 *     }
 */

    /* Create column */
    column = gtk_tree_view_column_new ();

    /* Pixbuf renderer. */
    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start ( GTK_TREE_VIEW_COLUMN ( column ), renderer, FALSE );
    gtk_tree_view_column_add_attribute ( GTK_TREE_VIEW_COLUMN ( column ), renderer,
                        "pixbuf", GSB_NAVIGATION_VIEW_PIX );
    gtk_tree_view_column_add_attribute ( GTK_TREE_VIEW_COLUMN ( column ), renderer,
                        "visible", GSB_NAVIGATION_VIEW_PIX_VISIBLE );
    gtk_tree_view_column_set_expand ( column, FALSE );

    /* Text renderer. */
    renderer = gtk_cell_renderer_text_new ( );
    gtk_cell_renderer_get_padding ( GTK_CELL_RENDERER ( renderer ), &xpad, &ypad );
    gtk_cell_renderer_set_padding ( GTK_CELL_RENDERER ( renderer ), 6, ypad );

    gtk_tree_view_column_pack_start ( GTK_TREE_VIEW_COLUMN ( column ), renderer, TRUE );
    gtk_tree_view_column_add_attribute ( GTK_TREE_VIEW_COLUMN ( column ), renderer,
                        "text", GSB_NAVIGATION_VIEW_TEXT );
    gtk_tree_view_column_add_attribute ( GTK_TREE_VIEW_COLUMN ( column ), renderer,
                        "weight", GSB_NAVIGATION_VIEW_FONT);
    gtk_tree_view_column_add_attribute ( GTK_TREE_VIEW_COLUMN ( column ), renderer,
                        "sensitive", GSB_NAVIGATION_VIEW_SENSITIVE);

    gtk_tree_view_append_column ( GTK_TREE_VIEW ( self ), GTK_TREE_VIEW_COLUMN ( column ) );

    /* set the signals */
    conf = grisbi_app_get_conf ();
    g_signal_connect ( self,
                        "key-press-event",
                        G_CALLBACK ( gsb_navigation_view_check_key_press ),
                        priv );

    g_signal_connect ( self,
                        "scroll-event",
                        G_CALLBACK ( gsb_navigation_view_check_scroll ),
                        NULL );

    if ( conf->active_scrolling_left_pane == 0 )
        g_signal_handlers_block_by_func ( self,
                        G_CALLBACK ( gsb_navigation_view_check_scroll ),
                        NULL );

    g_signal_connect ( G_OBJECT ( self ),
                        "button-press-event",
                        G_CALLBACK ( gsb_navigation_view_button_press ),
                        NULL );

    g_signal_connect ( G_OBJECT ( self ),
                        "row-collapsed",
                        G_CALLBACK ( gsb_navigation_view_activate_expander ),
                        priv );

    g_signal_connect ( G_OBJECT ( self ),
                        "row-expanded",
                        G_CALLBACK ( gsb_navigation_view_activate_expander ),
                        priv );

    g_signal_connect_after ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( self ) ),
                        "changed",
                        G_CALLBACK ( gsb_gui_navigation_select_line ),
                        priv->model );

    g_signal_connect ( self, 
                        "realize",
                        (GCallback) gsb_navigation_view_realized,
                        priv );

}


/* PUBLIC FUNCTIONS */
/**
 * crée un tree_view pour la navigation
 *
 * \param
 *
 * \return
 **/
GtkWidget *gsb_navigation_view_new ( gint navigation_sorting_accounts )
{
    GsbNavigationView *tree_view;

    devel_debug (NULL);
    tree_view = GSB_NAVIGATION_VIEW ( g_object_new ( GSB_TYPE_NAVIGATION_VIEW, NULL ) );

    tree_view->priv->navigation_sorting_accounts = navigation_sorting_accounts;
    gsb_navigation_view_fill_model ( tree_view );

    return GTK_WIDGET ( tree_view );
}


/**
 * retourne le type d'arrangement de la liste des comptes
 *
 * \param
 *
 * \return gint     type de tri pour les comptes
 **/
gint gsb_navigation_view_get_navigation_sorting_accounts ( void )
{
    GsbNavigationView *tree_view;

    tree_view = GSB_NAVIGATION_VIEW ( grisbi_window_get_navigation_tree_view () );
    if ( tree_view )
        return tree_view->priv->navigation_sorting_accounts;
    else
        return 0;
}


/**
 * définit le type de tri pour les comptes
 *
 * \param gint      type de tri pour les comptes
 *
 * \return          TRUE if OK FALSE other
 **/
gboolean gsb_navigation_view_set_navigation_sorting_accounts ( gint navigation_sorting_accounts )
{
    GsbNavigationView *tree_view;

    tree_view = GSB_NAVIGATION_VIEW ( grisbi_window_get_navigation_tree_view () );
    if ( tree_view )
    {
        tree_view->priv->navigation_sorting_accounts = navigation_sorting_accounts;

        return TRUE;
    }
    else
        return FALSE;
}


/**
 * création de la liste des comptes pour le navigation tree_view
 *
 * \param tree_view
 *
 * \return
 **/
void gsb_navigation_view_create_account_list ( GsbNavigationView *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter parent, child;
    GtkTreePath *path;
    GSList *list_tmp;
    GrisbiAppConf *conf;

    devel_debug (NULL);
    model = tree_view->priv->model;
    conf = grisbi_app_get_conf ( );

    path = gsb_navigation_view_get_page_path ( model, GSB_HOME_PAGE );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL( model ), &parent, path );

    /* Remove childs if any. */
    while ( gtk_tree_model_iter_children ( model, &child, &parent ) )
    {
        if ( gtk_tree_model_iter_has_child ( model, &child ) )
        {
            GtkTreeIter tmp_child;

            while ( gtk_tree_model_iter_children ( model, &tmp_child, &child ) )
            {
                gtk_tree_store_remove ( GTK_TREE_STORE ( model ), &tmp_child );
            }
        }

        gtk_tree_store_remove ( GTK_TREE_STORE ( model ), &child );
    }

    /* Fill in with accounts. */
    list_tmp = gsb_data_account_get_list_accounts ();
    while ( list_tmp )
    {
        gint i = gsb_data_account_get_no_account ( list_tmp -> data );

        if ( conf->show_closed_accounts
         ||
         !gsb_data_account_get_closed_account ( i ) )
        {
            gsb_navigation_view_add_account ( tree_view, i, FALSE );
        }

        list_tmp = list_tmp -> next;
    }

    /* Expand stuff */
    if ( tree_view->priv->account_expander )
    {
        path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( model ), &parent );
        gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( tree_view ), path );
        gtk_tree_path_free ( path );
    }
}


/**
 * return a pointer on the first element of g_queue of navigation pages
 *
 * \param none
 *
 * \return a GList
 * */
GQueue *gsb_navigation_view_get_pages_list ( void )
{
    GsbNavigationView *tree_view;

    tree_view = GSB_NAVIGATION_VIEW ( grisbi_window_get_navigation_tree_view () );
    if ( tree_view )
    {
        return tree_view->priv->pages_list;

    }
    else
        return NULL;
}


/**
 * renvoie le path d'un item de la vue navigation
 *
 * \param tree_model    to insert items into.
 * \param type_page
 *
 * \return              path or NULL
 */
GtkTreePath *gsb_navigation_view_get_page_path ( GtkTreeModel *model,
                        gint type_page )
{
    GtkTreeIter iter;

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
    {
        do
        {
            gint tmp_type_page;
            GtkTreePath *path = NULL;

            gtk_tree_model_get ( GTK_TREE_MODEL( model ), &iter,  GSB_NAVIGATION_VIEW_PAGE, &tmp_type_page, -1 );
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
 * Check mouse scrolling on the navigation tree view.
 *
 * \param tree_view the navigation tree_view
 * \param ev the scroll event
 *
 * \return FALSE : the signal continue / TRUE : the signal is stopped here
 * */
gboolean gsb_navigation_view_check_scroll ( GtkWidget *tree_view,
                                           GdkEventScroll *ev )
{
    switch ( ev->direction )
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
 * Set the selection of the navigation list depending on desired
 * page and/or account or report.
 *
 * \param page              Page to switch to.
 * \param account_number    If page is GSB_ACCOUNT_PAGE, switch to given account.
 *
 * \param report            If page is GSB_REPORTS, switch to given report.
 *
 * \return                  TRUE on success.
 */
gboolean gsb_navigation_view_set_selection ( gint page,
                        gint account_number,
                        gpointer report )
{
    GtkWidget *navigation_tree_view;
    GtkTreeModel *navigation_model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;

    navigation_tree_view = grisbi_window_get_navigation_tree_view ();
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( navigation_tree_view ) );
    g_return_val_if_fail ( selection, FALSE );

    navigation_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( navigation_tree_view ) );

    /* if we select an account, open the expander if necessary */
    if ( page == GSB_ACCOUNT_PAGE )
    {
        GtkTreePath *path;


        path = gsb_navigation_view_get_page_path ( navigation_model, GSB_HOME_PAGE );
        gtk_tree_view_expand_row ( GTK_TREE_VIEW ( navigation_tree_view ), path, TRUE );
        gtk_tree_path_free (path);
    }

    gtk_tree_model_get_iter_first ( GTK_TREE_MODEL(navigation_model), &iter );
    gsb_navigation_view_set_selection_branch ( selection, &iter, page, account_number, report );

    return TRUE;
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
