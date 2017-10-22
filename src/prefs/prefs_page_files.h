#ifndef __PREFS_PAGE_FILES_H__
#define __PREFS_PAGE_FILES_H__


#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_FILES_TYPE    	(prefs_page_files_get_type ())
#define PREFS_PAGE_FILES(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_FILES_TYPE, PrefsPageFiles))
#define PREFS_IS_PAGE_FILES(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_FILES_TYPE))

typedef struct _PrefsPageFiles          PrefsPageFiles;
typedef struct _PrefsPageFilesClass     PrefsPageFilesClass;


struct _PrefsPageFiles
{
    GtkBox parent;
};

struct _PrefsPageFilesClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               prefs_page_files_get_type            	(void) G_GNUC_CONST;

PrefsPageFiles * 	prefs_page_files_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_FILES_H__ */
