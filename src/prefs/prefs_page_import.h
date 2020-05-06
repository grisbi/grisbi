#ifndef __PREFS_PAGE_IMPORT_H__
#define __PREFS_PAGE_IMPORT_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_IMPORT_TYPE    	(prefs_page_import_get_type ())
#define PREFS_PAGE_IMPORT(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_IMPORT_TYPE, PrefsPageImport))
#define PREFS_IS_PAGE_IMPORT(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_IMPORT_TYPE))

typedef struct _PrefsPageImport			PrefsPageImport;
typedef struct _PrefsPageImportClass		PrefsPageImportClass;


struct _PrefsPageImport
{
    GtkBox parent;
};

struct _PrefsPageImportClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				prefs_page_import_get_type				(void) G_GNUC_CONST;

PrefsPageImport * 	prefs_page_import_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_IMPORT_H__ */
