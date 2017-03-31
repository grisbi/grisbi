#ifndef __PREFS_PAGE_ARCHIVES_H__
#define __PREFS_PAGE_ARCHIVES_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_ARCHIVES_TYPE    	(prefs_page_archives_get_type ())
#define PREFS_PAGE_ARCHIVES(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_ARCHIVES_TYPE, PrefsPageArchives))
#define PREFS_IS_PAGE_ARCHIVES(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_ARCHIVES_TYPE))

typedef struct _PrefsPageArchives          PrefsPageArchives;
typedef struct _PrefsPageArchivesClass     PrefsPageArchivesClass;


struct _PrefsPageArchives
{
    GtkBox parent;
};

struct _PrefsPageArchivesClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               	prefs_page_archives_get_type			(void) G_GNUC_CONST;

PrefsPageArchives * 	prefs_page_archives_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_ARCHIVES_H__ */
