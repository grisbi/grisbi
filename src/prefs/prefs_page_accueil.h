#ifndef __PREFS_PAGE_ACCUEIL_H__
#define __PREFS_PAGE_ACCUEIL_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_ACCUEIL_TYPE    	(prefs_page_accueil_get_type ())
#define PREFS_PAGE_ACCUEIL(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_ACCUEIL_TYPE, PrefsPageAccueil))
#define PREFS_IS_PAGE_ACCUEIL(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_ACCUEIL_TYPE))

typedef struct _PrefsPageAccueil          PrefsPageAccueil;
typedef struct _PrefsPageAccueilClass     PrefsPageAccueilClass;


struct _PrefsPageAccueil
{
    GtkBox parent;
};

struct _PrefsPageAccueilClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               prefs_page_accueil_get_type				(void) G_GNUC_CONST;

PrefsPageAccueil * 	prefs_page_accueil_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_ACCUEIL_H__ */
