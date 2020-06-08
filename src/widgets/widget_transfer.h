#ifndef __WIDGET_TRANSFER_H__
#define __WIDGET_TRANSFER_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "bet_data.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_TRANSFER_TYPE		(widget_transfer_get_type ())
#define WIDGET_TRANSFER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_TRANSFER_TYPE, WidgetTransfer))
#define WIDGET_IS_TRANSFER(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_TRANSFER_TYPE))

typedef struct _WidgetTransfer			WidgetTransfer;
typedef struct _WidgetTransferClass		WidgetTransferClass;


struct _WidgetTransfer
{
    GtkDialog parent;
};

struct _WidgetTransferClass
{
    GtkDialogClass parent_class;
};

/* START_DECLARATION */
GType				widget_transfer_get_type				(void) G_GNUC_CONST;

WidgetTransfer * 	widget_transfer_new						(gint account_number,
															 gint create_mode);
void				widget_transfer_fill_data_from_line		(GtkWidget *dialog,
															 gint account_number,
															 gint number);
void				widget_transfer_set_date_debit			(GtkWidget *dialog,
															 GDate *date_debit);
TransfertData *		widget_transfer_take_data				(GtkWidget *dialog,
															 gint account_number,
															 gint number);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_TRANSFER_H__ */
