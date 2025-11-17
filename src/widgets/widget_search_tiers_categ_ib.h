#ifndef __WIDGET_SEARCH_TIERS_CATEG_IB_TYPE_H__
#define __WIDGET_SEARCH_TIERS_CATEG_IB_TYPE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/

/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_SEARCH_TIERS_CATEG_IB_TYPE	    (widget_search_tiers_categ_ib_get_type ())
#define WIDGET_SEARCH_TIERS_CATEG_IB(obj)    	(G_TYPE_CHECK_INSTANCE_CAST((obj), WIDGET_SEARCH_TIERS_CATEG_IB_TYPE, WidgetSearchTiersCategIb))
#define WIDGET_IS_SEARCH_TIERS_CATEG_IB(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_SEARCH_TIERS_CATEG_IB_TYPE))

typedef struct _WidgetSearchTiersCategIb			WidgetSearchTiersCategIb;
typedef struct _WidgetSearchTiersCategIbClass		WidgetSearchTiersCategIbClass;


struct _WidgetSearchTiersCategIb
{
    GtkWindow parent;
};

struct _WidgetSearchTiersCategIbClass
{
    GtkWindowClass parent_class;
};

/* START_DECLARATION */
GType						widget_search_tiers_categ_ib_get_type	(void) G_GNUC_CONST;

WidgetSearchTiersCategIb *	widget_search_tiers_categ_ib_new		(GtkWidget *win,
																	 gint page_num);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_SEARCH_TIERS_CATEG_IB_TYPE_H__ */
