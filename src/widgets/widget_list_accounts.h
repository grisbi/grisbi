#ifndef __WIDGET_LIST_ACCOUNTS_H__
#define __WIDGET_LIST_ACCOUNTS_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_LIST_ACCOUNTS_TYPE    	(widget_list_accounts_get_type ())
#define WIDGET_LIST_ACCOUNTS(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_LIST_ACCOUNTS_TYPE, WidgetListAccounts))
#define WIDGET_IS_LIST_ACCOUNTS(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_LIST_ACCOUNTS_TYPE))

typedef struct _WidgetListAccounts			WidgetListAccounts;
typedef struct _WidgetListAccountsClass		WidgetListAccountsClass;


struct _WidgetListAccounts
{
    GtkBox parent;
};

struct _WidgetListAccountsClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					widget_list_accounts_get_type				(void) G_GNUC_CONST;

WidgetListAccounts *	widget_list_accounts_new					(GtkWidget *page);
void					widget_list_accounts_init_liste				(GtkWidget *widget,
																	 gint report_number,
																	 gint type_page);
gboolean				widget_list_accounts_get_info				(GtkWidget *widget,
																	 gint report_number,
																	 gint type_page);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_LIST_ACCOUNTS_H__ */
