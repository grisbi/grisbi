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

/*START_INCLUDE*/
#include "gsb_navigation_view.h"
#include "gsb_data_account.h"
#include "gsb_data_report.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/


#define GSB_NAVIGATION_VIEW_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), GSB_TYPE_NAVIGATION_VIEW, GsbNavigationViewPrivate))


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


struct _GsbNavigationViewPrivate
{
    /** Model of the navigation tree. */
    GtkTreeModel *model;

    /** Widget that hold the scheduler calendar. */
    GtkWidget *scheduler_calendar;

    /* liste des pages */
    GQueue *pages_list;
};

G_DEFINE_TYPE(GsbNavigationView, gsb_navigation_view, GTK_TYPE_TREE_VIEW)


static GtkTargetEntry row_targets[] =
{
    { "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
};


/**
 * finalise GsbVavigationView
 *
 * \param object
 *
 * \return
 */
static void gsb_navigation_view_finalize ( GObject *object )
{
    devel_debug (NULL);

    /* libération de l'objet */
    G_OBJECT_CLASS ( gsb_navigation_view_parent_class )->finalize ( object );
}


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
}


/* TREE_MODEL */
/**
 * fonction de tri des éléments du model
 *
 * \param model
 * \param iter a
 * \param iter b
 * \param user_data
 *
 * \return 0 for equal, less than zero if lhs is iter a than iter b,
 * greater than zero if iter a is greater than iter b
 */
static gint gsb_navigation_view_sort_column ( GtkTreeModel *model,
                        GtkTreeIter *a,
                        GtkTreeIter *b,
                        gpointer user_data )
{
    gint ordre_a, page_a, account_a, report_a;
    gint ordre_b, page_b, account_b, report_b;

    if ( !model )
        return 0;

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
 *
 *
 * \param
 *
 * \return
 **/
static GtkTreeModel *gsb_navigation_view_create_model ( void )
{
    GtkTreeStore *model;

    model = gtk_tree_store_new ( NAVIGATION_TOTAL,
                        GDK_TYPE_PIXBUF,        /* NAVIGATION_PIX */
                        G_TYPE_BOOLEAN,         /* NAVIGATION_PIX_VISIBLE */
                        G_TYPE_STRING,          /* NAVIGATION_TEXT */
                        G_TYPE_INT,             /* NAVIGATION_FONT */
                        G_TYPE_INT,             /* NAVIGATION_PAGE */
                        G_TYPE_INT,             /* NAVIGATION_ACCOUNT */
                        G_TYPE_INT,             /* NAVIGATION_REPORT */
                        G_TYPE_INT,             /* NAVIGATION_SENSITIVE */
                        G_TYPE_INT );           /* NAVIGATION_ORDRE */

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( model ),
                        NAVIGATION_ORDRE,
                        GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( model ),
                        NAVIGATION_ORDRE,
                        gsb_navigation_view_sort_column,
                        NULL,
                        NULL );

    return GTK_TREE_MODEL ( model );
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
 *
 *
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
            gsb_navigation_view_move_ordre ( src_ordre, dst_ordre );

        /* update the tree view */
/*         gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( model ),
 *                         NAVIGATION_ORDRE, GTK_SORT_ASCENDING );
 *         gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( model ),
 *                         NAVIGATION_ORDRE, navigation_sort_column,
 *                         NULL, NULL );
 */

        /* update the order of accounts in first page */
/*         mise_a_jour_liste_comptes_accueil = TRUE;  */

/*         gsb_file_set_modified ( TRUE );  */
    }
    return FALSE;
}


/**
 *
 *
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


/* FONCTION DE CONSTRUCTION */
static void gsb_navigation_view_init ( GsbNavigationView *self )
{
    GtkCellRenderer *renderer;
/*     GtkTreeDragDestIface *navigation_dst_iface;
 *     GtkTreeDragSourceIface *navigation_src_iface;
 */
    GtkTreeViewColumn *column;
    GsbNavigationViewPrivate *priv;

    devel_debug (NULL);
    self->priv = priv = GSB_NAVIGATION_VIEW_GET_PRIVATE ( self );

    /* create model */
    priv->model = gsb_navigation_view_create_model ();

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


}


/* PUBLIC FUNCTIONS */
/**
 * crée un tree_view pour la navigation
 *
 * \param
 *
 * \return
 **/
GtkWidget *gsb_navigation_view_new ( void )
{
    GsbNavigationView *tree_view;

    tree_view = GSB_NAVIGATION_VIEW ( g_object_new ( GSB_TYPE_NAVIGATION_VIEW, NULL ) );

    return GTK_WIDGET ( tree_view );
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
