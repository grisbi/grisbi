#ifndef __WIDGET_RECONCILE_H__
#define __WIDGET_RECONCILE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_RECONCILE_TYPE    	(widget_reconcile_get_type ())
#define WIDGET_RECONCILE(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_RECONCILE_TYPE, WidgetReconcile))
#define WIDGET_IS_RECONCILE(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_RECONCILE_TYPE))

typedef struct _WidgetReconcile				WidgetReconcile;
typedef struct _WidgetReconcileClass		WidgetReconcileClass;


struct _WidgetReconcile
{
    GtkBox parent;
};

struct _WidgetReconcileClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				widget_reconcile_get_type							(void) G_GNUC_CONST;
gchar *				widget_reconcile_build_label						(gint reconcile_number,
																		 gint account_number);
WidgetReconcile *	widget_reconcile_new								(void);
gboolean			widget_reconcile_run_reconciliation					(GtkWidget *button,
																		 gpointer null);
void				widget_reconcile_set_active_sort_checkbutton		(gboolean active);
void				widget_reconcile_update_amounts						(void);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_RECONCILE_H__ */
