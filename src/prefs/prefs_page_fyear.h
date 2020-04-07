#ifndef __PREFS_PAGE_FYEAR_H__
#define __PREFS_PAGE_FYEAR_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_FYEAR_TYPE    	(prefs_page_fyear_get_type ())
#define PREFS_PAGE_FYEAR(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_FYEAR_TYPE, PrefsPageFyear))
#define PREFS_IS_PAGE_FYEAR(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_FYEAR_TYPE))

typedef struct _PrefsPageFyear			PrefsPageFyear;
typedef struct _PrefsPageFyearClass		PrefsPageFyearClass;


struct _PrefsPageFyear
{
    GtkBox parent;
};

struct _PrefsPageFyearClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				prefs_page_fyear_get_type				(void) G_GNUC_CONST;

PrefsPageFyear * 	prefs_page_fyear_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_FYEAR_H__ */
