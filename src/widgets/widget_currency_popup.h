#ifndef __WIDGET_CURRENCY_POPUP_H__
#define __WIDGET_CURRENCY_POPUP_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "prefs_page_currency.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_CURRENCY_POPUP_TYPE    	(widget_currency_popup_get_type ())
#define WIDGET_CURRENCY_POPUP(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_CURRENCY_POPUP_TYPE, WidgetCurrencyPopup))
#define PREFS_IS_PAGE_CURRENCY_POPUP(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_CURRENCY_POPUP_TYPE))

typedef struct _WidgetCurrencyPopup				WidgetCurrencyPopup;
typedef struct _WidgetCurrencyPopupClass		WidgetCurrencyPopupClass;


struct _WidgetCurrencyPopup
{
    GtkBox parent;
};

struct _WidgetCurrencyPopupClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					widget_currency_popup_get_type					(void) G_GNUC_CONST;

WidgetCurrencyPopup * 	widget_currency_popup_new						(GtkWidget *page,
																	 	 GtkTreeModel *model);
GtkWidget *				widget_currency_popup_get_tree_view				(GtkWidget *w_currency_popup);
void					widget_currency_popup_display_old_currencies	(GtkWidget *w_currency_popup,
																		 gboolean display);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_CURRENCY_POPUP_H__ */
