#ifndef __WIDGET_CURRENCY_LINK_H__
#define __WIDGET_CURRENCY_LINK_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_CURRENCY_LINK_TYPE    	(widget_currency_link_get_type ())
#define WIDGET_CURRENCY_LINK(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_CURRENCY_LINK_TYPE, WidgetCurrencyLink))
#define WIDGET_IS_CURRENCY_LINK(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_CURRENCY_LINK_TYPE))

typedef struct _WidgetCurrencyLink			WidgetCurrencyLink;
typedef struct _WidgetCurrencyLinkClass		WidgetCurrencyLinkClass;


struct _WidgetCurrencyLink
{
    GtkBox parent;
};

struct _WidgetCurrencyLinkClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					widget_currency_link_get_type				(void) G_GNUC_CONST;

WidgetCurrencyLink * 	widget_currency_link_new					(GtkWidget *page);
GtkWidget *				widget_currency_link_get_widget				(GtkWidget *w_currency_link,
																	 const gchar *widget_name);
void					widget_currency_link_set_sensitive			(GtkWidget *w_currency_link,
																	 gboolean sensitive);
void					widget_currency_details_update_link			(gint link_number,
																	 GtkWidget *w_currency_link);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_CURRENCY_LINK_H__ */
