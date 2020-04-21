#ifndef __PREFS_PAGE_CURRENCY_H__
#define __PREFS_PAGE_CURRENCY_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_CURRENCY_TYPE    	(prefs_page_currency_get_type ())
#define PREFS_PAGE_CURRENCY(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_CURRENCY_TYPE, PrefsPageCurrency))
#define PREFS_IS_PAGE_CURRENCY(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_CURRENCY_TYPE))

typedef struct _PrefsPageCurrency			PrefsPageCurrency;
typedef struct _PrefsPageCurrencyClass		PrefsPageCurrencyClass;


struct _PrefsPageCurrency
{
    GtkBox parent;
};

struct _PrefsPageCurrencyClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					prefs_page_currency_get_type			(void) G_GNUC_CONST;

PrefsPageCurrency * 	prefs_page_currency_new					(GrisbiPrefs *prefs);
void 					prefs_page_currency_entry_changed		(GtkWidget *entry,
																 PrefsPageCurrency *page);

void					prefs_page_currency_popup_fill_list		(GtkTreeView *tree_view,
																 gboolean include_obsolete);
GtkWidget *				prefs_page_currency_popup_get_treeview	(GtkWidget *page);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_CURRENCY_H__ */
