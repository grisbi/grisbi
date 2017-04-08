#ifndef __PREFS_PAGE_IMPORT_ASSO_H__
#define __PREFS_PAGE_IMPORT_ASSO_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_IMPORT_ASSO_TYPE    	(prefs_page_import_asso_get_type ())
#define PREFS_PAGE_IMPORT_ASSO(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_IMPORT_ASSO_TYPE, PrefsPageImportAsso))
#define PREFS_IS_PAGE_IMPORT_ASSO(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_IMPORT_ASSO_TYPE))

typedef struct _PrefsPageImportAsso          PrefsPageImportAsso;
typedef struct _PrefsPageImportAssoClass     PrefsPageImportAssoClass;


struct _PrefsPageImportAsso
{
    GtkBox parent;
};

struct _PrefsPageImportAssoClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               	prefs_page_import_asso_get_type		(void) G_GNUC_CONST;

PrefsPageImportAsso * 	prefs_page_import_asso_new			(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_IMPORT_ASSO_H__ */
