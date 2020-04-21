#ifndef __WIDGET_CURRENCY_DETAILS_H__
#define __WIDGET_CURRENCY_DETAILS_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_CURRENCY_DETAILS_TYPE    	(widget_currency_details_get_type ())
#define WIDGET_CURRENCY_DETAILS(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_CURRENCY_DETAILS_TYPE, WidgetCurrencyDetails))
#define WIDGET_IS_CURRENCY_DETAILS(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_CURRENCY_DETAILS_TYPE))

typedef struct _WidgetCurrencyDetails			WidgetCurrencyDetails;
typedef struct _WidgetCurrencyDetailsClass		WidgetCurrencyDetailsClass;


struct _WidgetCurrencyDetails
{
    GtkBox parent;
};

struct _WidgetCurrencyDetailsClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType						widget_currency_details_get_type			(void) G_GNUC_CONST;

WidgetCurrencyDetails * 	widget_currency_details_new					(GtkWidget *page_currency,
																		 gboolean no_callback);
GtkWidget *					widget_currency_details_get_entry			(GtkWidget *w_currency_details,
																		 const gchar *entry_name);
void						widget_currency_details_set_entry_editable	(GtkWidget *w_currency_details,
																		 gboolean editable);
void 						widget_currency_details_update_currency		(gint currency_number,
																		 GtkWidget *w_currency_details);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_CURRENCY_DETAILS_H__ */
