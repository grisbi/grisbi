#ifndef __PREFS_PAGE_CURRENCY_LINK_H__
#define __PREFS_PAGE_CURRENCY_LINK_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_CURRENCY_LINK_TYPE    	(prefs_page_currency_link_get_type ())
#define PREFS_PAGE_CURRENCY_LINK(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_CURRENCY_LINK_TYPE, PrefsPageCurrencyLink))
#define PREFS_IS_PAGE_CURRENCU_LINK(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_CURRENCY_LINK_TYPE))

typedef struct _PrefsPageCurrencyLink			PrefsPageCurrencyLink;
typedef struct _PrefsPageCurrencyLinkClass		PrefsPageCurrencyLinkClass;


struct _PrefsPageCurrencyLink
{
    GtkBox parent;
};

struct _PrefsPageCurrencyLinkClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					prefs_page_currency_link_get_type					(void) G_GNUC_CONST;

PrefsPageCurrencyLink *	prefs_page_currency_link_new						(GrisbiPrefs *prefs);
void					prefs_page_currency_link_checkbutton_fixed_changed	(GtkWidget *checkbutton,
														  					 PrefsPageCurrencyLink *page);
gint					prefs_page_currency_link_get_selected_link_number	(PrefsPageCurrencyLink *page);
void 					prefs_page_currency_link_widget_link_changed		(GtkWidget *widget,
																			 PrefsPageCurrencyLink *page);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_CURRENCY_LINK_H__ */
