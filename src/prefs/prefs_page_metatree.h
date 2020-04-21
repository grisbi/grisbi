#ifndef __PREFS_PAGE_METATREE_H__
#define __PREFS_PAGE_METATREE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_METATREE_TYPE    	(prefs_page_metatree_get_type ())
#define PREFS_PAGE_METATREE(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_METATREE_TYPE, PrefsPageMetatree))
#define PREFS_IS_PAGE_METATREE(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_METATREE_TYPE))

typedef struct _PrefsPageMetatree          PrefsPageMetatree;
typedef struct _PrefsPageMetatreeClass     PrefsPageMetatreeClass;


struct _PrefsPageMetatree
{
    GtkBox parent;
};

struct _PrefsPageMetatreeClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               	prefs_page_metatree_get_type            	(void) G_GNUC_CONST;

PrefsPageMetatree * 	prefs_page_metatree_new						(GrisbiPrefs *prefs);
GtkWidget *				prefs_page_metatree_get_currency_combobox	(const gchar *widget_name);
void 					prefs_page_metatree_sensitive_widget		(const gchar *widget_name,
																 gboolean sensitive);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_METATREE_H__ */
