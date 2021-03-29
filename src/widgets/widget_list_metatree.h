#ifndef __WIDGET_LIST_METATREE_H__
#define __WIDGET_LIST_METATREE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_LIST_METATREE_TYPE		(widget_list_metatree_get_type ())
#define WIDGET_LIST_METATREE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_LIST_METATREE_TYPE, WidgetListMetatree))
#define WIDGET_IS_LIST_METATREE(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_LIST_METATREE_TYPE))

typedef struct _WidgetListMetatree			WidgetListMetatree;
typedef struct _WidgetListMetatreeClass		WidgetListMetatreeClass;


struct _WidgetListMetatree
{
    GtkBox parent;
};

struct _WidgetListMetatreeClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					widget_list_metatree_get_type				(void) G_GNUC_CONST;

WidgetListMetatree * 	widget_list_metatree_new					(GtkWidget *page,
																	 gint type_page);
void					widget_list_metatree_init_liste				(GtkWidget *widget,
																	 gint report_number,
																	 gint type_page);
gboolean				widget_list_metatree_get_info				(GtkWidget *widget,
																	 gint report_number,
																	 gint type_page);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_LIST_METATREE_H__ */
