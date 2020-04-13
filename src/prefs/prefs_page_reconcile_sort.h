#ifndef __PREFS_PAGE_XXX_H__
#define __PREFS_PAGE_XXX_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_XXX_TYPE    	(prefs_page_xxxxx_get_type ())
#define PREFS_PAGE_XXX(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_XXX_TYPE, PrefsPageXxxx))
#define PREFS_IS_PAGE_XXX(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_XXX_TYPE))

typedef struct _PrefsPageXxxx			PrefsPageXxxx;
typedef struct _PrefsPageXxxxClass		PrefsPageXxxxClass;


struct _PrefsPageXxxx
{
    GtkBox parent;
};

struct _PrefsPageXxxxClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				prefs_page_xxxxx_get_type				(void) G_GNUC_CONST;

PrefsPageXxxx * 	prefs_page_xxxxx_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_XXX_H__ */
