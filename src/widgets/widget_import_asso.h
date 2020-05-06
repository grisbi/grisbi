#ifndef __WIDGET_IMPORT_ASSO_H__
#define __WIDGET_IMPORT_ASSO_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_IMPORT_ASSO_TYPE    	(widget_import_asso_get_type ())
#define WIDGET_IMPORT_ASSO(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_IMPORT_ASSO_TYPE, WidgetImportAsso))
#define PREFS_IS_PAGE_IMPORT_ASSO(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_IMPORT_ASSO_TYPE))

typedef struct _WidgetImportAsso          WidgetImportAsso;
typedef struct _WidgetImportAssoClass     WidgetImportAssoClass;


struct _WidgetImportAsso
{
    GtkBox parent;
};

struct _WidgetImportAssoClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               	widget_import_asso_get_type		(void) G_GNUC_CONST;

WidgetImportAsso * 	widget_import_asso_new			(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_IMPORT_ASSO_H__ */
