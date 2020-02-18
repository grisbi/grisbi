#ifndef __PREFS_PAGE_FORM_OPTIONS_H__
#define __PREFS_PAGE_FORM_OPTIONS_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_FORM_OPTIONS_TYPE    	(prefs_page_form_options_get_type ())
#define PREFS_PAGE_FORM_OPTIONS(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_FORM_OPTIONS_TYPE, PrefsPageFormOptions))
#define PREFS_IS_PAGE_FORM_OPTIONS(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_FORM_OPTIONS_TYPE))

typedef struct _PrefsPageFormOptions			PrefsPageFormOptions;
typedef struct _PrefsPageFormOptionsClass		PrefsPageFormOptionsClass;


struct _PrefsPageFormOptions
{
    GtkBox parent;
};

struct _PrefsPageFormOptionsClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				prefs_page_form_options_get_type				(void) G_GNUC_CONST;

PrefsPageFormOptions * 	prefs_page_form_options_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_FORM_OPTIONS_H__ */
