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

typedef struct _GsbNavigationViewPrivate    GsbNavigationViewPrivate;

typedef struct _GsbNavigationView           GsbNavigationView;
typedef struct _GsbNavigationViewClass      GsbNavigationViewClass;

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
GtkWidget*gsb_navigation_view_new ( void );
/* END_DECLARATION */

G_END_DECLS

#endif /* __GSB_NAVIGATION_VIEW_H__ */
