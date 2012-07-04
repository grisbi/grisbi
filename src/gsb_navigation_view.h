#ifndef __GSB_NAVIGATION_VIEW_H__
#define __GSB_NAVIGATION_VIEW_H__

#include <gtk/gtk.h>

/*START_INCLUDE*/
/*END_INCLUDE*/


G_BEGIN_DECLS

#define GSB_TYPE_NAVIGATION_VIEW            (gsb_navigation_view_get_type ())
#define GSB_NAVIGATION_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSB_TYPE_NAVIGATION_VIEW, GsbNavigationView))
#define GSB_NAVIGATION_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GSB_TYPE_NAVIGATION_VIEW, GsbNavigationViewClass))
#define GSB_IS_NAVIGATION_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSB_TYPE_NAVIGATION_VIEW))
#define GSB_IS_NAVIGATION_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GSB_TYPE_NAVIGATION_VIEW))
#define GSB_NAVIGATION_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),   GSB_TYPE_NAVIGATION_VIEW, GsbNavigationViewClass))

typedef struct _GsbNavigationView           GsbNavigationView;
typedef struct _GsbNavigationViewClass      GsbNavigationViewClass;
typedef struct _GsbNavigationViewPrivate    GsbNavigationViewPrivate;

struct _GsbNavigationView
{
    GtkTreeView parent;

    /*< private > */
    GsbNavigationViewPrivate *priv;
};

struct _GsbNavigationViewClass
{
    GtkTreeViewClass parent_class;
};


/* START_DECLARATION */
GType gsb_navigation_view_get_type ( void ) G_GNUC_CONST;
gboolean gsb_navigation_view_check_scroll ( GtkWidget *tree_view,
                                           GdkEventScroll *ev );
void gsb_navigation_view_create_account_list ( GsbNavigationView *tree_view );
gint gsb_navigation_view_get_navigation_sorting_accounts ( void );
GQueue *gsb_navigation_view_get_pages_list ( void );
GtkTreePath *gsb_navigation_view_get_page_path ( GtkTreeModel *model,
                        gint type_page );
GtkWidget *gsb_navigation_view_new ( gint navigation_sorting_accounts );
gboolean gsb_navigation_view_set_navigation_sorting_accounts ( gint navigation_sorting_accounts );
gboolean gsb_navigation_view_set_selection ( gint page,
                        gint account_number,
                        gpointer report );
/* END_DECLARATION */

G_END_DECLS

#endif /* __GSB_NAVIGATION_VIEW_H__ */
