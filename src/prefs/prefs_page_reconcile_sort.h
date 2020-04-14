#ifndef __PREFS_PAGE_RECONCILE_SORT_H__
#define __PREFS_PAGE_RECONCILE_SORT_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_RECONCILE_SORT_TYPE    	(prefs_page_reconcile_sort_get_type ())
#define PREFS_PAGE_RECONCILE_SORT(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_RECONCILE_SORT_TYPE, PrefsPageReconcileSort))
#define PREFS_IS_PAGE_RECONCILE_SORT(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_RECONCILE_SORT_TYPE))

typedef struct _PrefsPageReconcileSort			PrefsPageReconcileSort;
typedef struct _PrefsPageReconcileSortClass		PrefsPageReconcileSortClass;


struct _PrefsPageReconcileSort
{
    GtkBox parent;
};

struct _PrefsPageReconcileSortClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType						prefs_page_reconcile_sort_get_type				(void) G_GNUC_CONST;

PrefsPageReconcileSort * 	prefs_page_reconcile_sort_new					(GrisbiPrefs *prefs);
void						prefs_page_reconcile_sort_fill					(void);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_RECONCILE_SORT_H__ */
