#ifndef __PREFS_PAGE_DISPLAY_GUI_H__
#define __PREFS_PAGE_DISPLAY_GUI_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_DISPLAY_GUI_TYPE    	(prefs_page_display_gui_get_type ())
#define PREFS_PAGE_DISPLAY_GUI(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_DISPLAY_GUI_TYPE, PrefsPageDisplayGui))
#define PREFS_IS_PAGE_DISPLAY_GUI(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_DISPLAY_GUI_TYPE))

typedef struct _PrefsPageDisplayGui          PrefsPageDisplayGui;
typedef struct _PrefsPageDisplayGuiClass     PrefsPageDisplayGuiClass;


struct _PrefsPageDisplayGui
{
    GtkBox parent;
};

struct _PrefsPageDisplayGuiClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					prefs_page_display_gui_get_type				(void) G_GNUC_CONST;

PrefsPageDisplayGui * 	prefs_page_display_gui_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_DISPLAY_GUI_H__ */
