#ifndef __PREFS_PAGE_DISPLAY_FORM_H__
#define __PREFS_PAGE_DISPLAY_FORM_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_DISPLAY_FORM_TYPE    	(prefs_page_display_form_get_type ())
#define PREFS_PAGE_DISPLAY_FORM(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_DISPLAY_FORM_TYPE, PrefsPageDisplayForm))
#define PREFS_IS_PAGE_DISPLAY_FORM(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_DISPLAY_FORM_TYPE))

typedef struct _PrefsPageDisplayForm			PrefsPageDisplayForm;
typedef struct _PrefsPageDisplayFormClass		PrefsPageDisplayFormClass;


struct _PrefsPageDisplayForm
{
    GtkBox parent;
};

struct _PrefsPageDisplayFormClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				prefs_page_display_form_get_type				(void) G_GNUC_CONST;

PrefsPageDisplayForm * 	prefs_page_display_form_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_DISPLAY_FORM_H__ */
