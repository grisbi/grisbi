#ifndef __WIDGET_IMPORT_FILES_H__
#define __WIDGET_IMPORT_FILES_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_IMPORT_FILES_TYPE    	(widget_import_files_get_type ())
#define WIDGET_IMPORT_FILES(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_IMPORT_FILES_TYPE, WidgetImportFiles))
#define PREFS_IS_PAGE_IMPORT_FILES(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_IMPORT_FILES_TYPE))

typedef struct _WidgetImportFiles          WidgetImportFiles;
typedef struct _WidgetImportFilesClass     WidgetImportFilesClass;


struct _WidgetImportFiles
{
    GtkBox parent;
};

struct _WidgetImportFilesClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               	widget_import_files_get_type            (void) G_GNUC_CONST;

WidgetImportFiles * 	widget_import_files_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_IMPORT_FILES_H__ */
