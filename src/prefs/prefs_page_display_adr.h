#ifndef __PREFS_PAGE_DISPLAY_ADR_H__
#define __PREFS_PAGE_DISPLAY_ADR_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_DISPLAY_ADR_TYPE    	(prefs_page_display_adr_get_type ())
#define PREFS_PAGE_DISPLAY_ADR(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_DISPLAY_ADR_TYPE, PrefsPageDisplayAdr))
#define PREFS_IS_PAGE_DISPLAY_ADR(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_DISPLAY_ADR_TYPE))

typedef struct _PrefsPageDisplayAdr          PrefsPageDisplayAdr;
typedef struct _PrefsPageDisplayAdrClass     PrefsPageDisplayAdrClass;


struct _PrefsPageDisplayAdr
{
    GtkBox parent;
};

struct _PrefsPageDisplayAdrClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               	prefs_page_display_adr_get_type        		(void) G_GNUC_CONST;

PrefsPageDisplayAdr * 	prefs_page_display_adr_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_DISPLAY_ADR_H__ */
