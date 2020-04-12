#ifndef __PREFS_PAGE_RECONCILE_H__
#define __PREFS_PAGE_RECONCILE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_RECONCILE_TYPE    	(prefs_page_reconcile_get_type ())
#define PREFS_PAGE_RECONCILE(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_RECONCILE_TYPE, PrefsPageReconcile))
#define PREFS_IS_PAGE_RECONCILE(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_RECONCILE_TYPE))

typedef struct _PrefsPageReconcile			PrefsPageReconcile;
typedef struct _PrefsPageReconcileClass		PrefsPageReconcileClass;


struct _PrefsPageReconcile
{
    GtkBox parent;
};

struct _PrefsPageReconcileClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				prefs_page_reconcile_get_type				(void) G_GNUC_CONST;

PrefsPageReconcile * 	prefs_page_reconcile_new				(GrisbiPrefs *prefs);
void					prefs_page_reconcile_fill				(GtkWidget *tree_view);


/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_RECONCILE_H__ */
