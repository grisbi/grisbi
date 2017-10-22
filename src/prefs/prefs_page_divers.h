#ifndef __PREFS_PAGE_DIVERS_H__
#define __PREFS_PAGE_DIVERS_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_DIVERS_TYPE    	(prefs_page_divers_get_type ())
#define PREFS_PAGE_DIVERS(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_DIVERS_TYPE, PrefsPageDivers))
#define PREFS_IS_PAGE_DIVERS(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_DIVERS_TYPE))

typedef struct _PrefsPageDivers          PrefsPageDivers;
typedef struct _PrefsPageDiversClass     PrefsPageDiversClass;


struct _PrefsPageDivers
{
    GtkBox parent;
};

struct _PrefsPageDiversClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               prefs_page_divers_get_type            (void) G_GNUC_CONST;

PrefsPageDivers * 	prefs_page_divers_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_DIVERS_H__ */
