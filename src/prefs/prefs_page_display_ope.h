#ifndef __PREFS_PAGE_DISPLAY_OPE_H__
#define __PREFS_PAGE_DISPLAY_OPE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_DISPLAY_OPE_TYPE    	(prefs_page_display_ope_get_type ())
#define PREFS_PAGE_DISPLAY_OPE(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_DISPLAY_OPE_TYPE, PrefsPageDisplayOpe))
#define PREFS_IS_PAGE_DISPLAY_OPE(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_DISPLAY_OPE_TYPE))

typedef struct _PrefsPageDisplayOpe          PrefsPageDisplayOpe;
typedef struct _PrefsPageDisplayOpeClass     PrefsPageDisplayOpeClass;


struct _PrefsPageDisplayOpe
{
    GtkBox parent;
};

struct _PrefsPageDisplayOpeClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               	prefs_page_display_ope_get_type            (void) G_GNUC_CONST;

PrefsPageDisplayOpe * 	prefs_page_display_ope_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_DISPLAY_OPE_H__ */
