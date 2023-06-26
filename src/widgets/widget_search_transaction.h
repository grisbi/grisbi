#ifndef __WIDGET_SEARCH_TRANSACTION_H__
#define __WIDGET_SEARCH_TRANSACTION_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_SEARCH_TRANSACTION_TYPE		(widget_search_transaction_get_type ())
#define WIDGET_SEARCH_TRANSACTION(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_SEARCH_TRANSACTION_TYPE, WidgetSearchTransaction))
#define WIDGET_IS_SEARCH_TRANSACTION(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), WIDGET_SEARCH_TRANSACTION_TYPE))

typedef struct _WidgetSearchTransaction				WidgetSearchTransaction;
typedef struct _WidgetSearchTransactionClass		WidgetSearchTransactionClass;


struct _WidgetSearchTransaction
{
    GtkDialog dialog;
};

struct _WidgetSearchTransactionClass
{
    GtkDialogClass parent_class;
};

/* START_DECLARATION */
GType						widget_search_transaction_get_type				(void) G_GNUC_CONST;

WidgetSearchTransaction * 	widget_search_transaction_new					(GtkWidget *win,
																			 gint *transaction_number);
void 						widget_search_transaction_dialog_response		(GtkDialog *dialog,
																			 gint result_id);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_SEARCH_TRANSACTION_H__ */
