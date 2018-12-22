#ifndef __PREFS_PAGE_FORM_COMPLETION_H__
#define __PREFS_PAGE_FORM_COMPLETION_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_FORM_COMPLETION_TYPE    	(prefs_page_form_completion_get_type ())
#define PREFS_PAGE_FORM_COMPLETION(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_FORM_COMPLETION_TYPE, PrefsPageFormCompletion))
#define PREFS_IS_PAGE_FORM_COMPLETION(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_FORM_COMPLETION_TYPE))

typedef struct _PrefsPageFormCompletion          PrefsPageFormCompletion;
typedef struct _PrefsPageFormCompletionClass     PrefsPageFormCompletionClass;


struct _PrefsPageFormCompletion
{
    GtkBox parent;
};

struct _PrefsPageFormCompletionClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               		prefs_page_form_completion_get_type				(void) G_GNUC_CONST;

PrefsPageFormCompletion * 	prefs_page_form_completion_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_FORM_COMPLETION_H__ */
