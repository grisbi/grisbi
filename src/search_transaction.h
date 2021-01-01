#ifndef __SEARCH_TRANSACTION_H__
#define __SEARCH_TRANSACTION_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define SEARCH_TRANSACTION_TYPE    	(search_transaction_get_type ())
#define SEARCH_TRANSACTION(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), SEARCH_TRANSACTION_TYPE, SearchTransaction))
#define WIDGET_IS_XXX(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), SEARCH_TRANSACTION_TYPE))

typedef struct _SearchTransaction			SearchTransaction;
typedef struct _SearchTransactionClass		SearchTransactionClass;


struct _SearchTransaction
{
    GtkDialog dialog;
};

struct _SearchTransactionClass
{
    GtkDialogClass parent_class;
};

/* START_DECLARATION */
GType					search_transaction_get_type				(void) G_GNUC_CONST;

SearchTransaction * 	search_transaction_new					(GrisbiWin *win,
																 gint *transaction_number);
void 					search_transaction_dialog_response		(GtkDialog *dialog,
																 gint result_id);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __SEARCH_TRANSACTION_H__ */
