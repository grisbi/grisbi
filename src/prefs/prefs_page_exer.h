#ifndef __PREFS_PAGE_EXER_H__
#define __PREFS_PAGE_EXER_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_EXER_TYPE    	(prefs_page_exer_get_type ())
#define PREFS_PAGE_EXER(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_EXER_TYPE, PrefsPageExer))
#define PREFS_IS_PAGE_EXER(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_EXER_TYPE))

typedef struct _PrefsPageExer			PrefsPageExer;
typedef struct _PrefsPageExerClass		PrefsPageExerClass;


struct _PrefsPageExer
{
    GtkBox parent;
};

struct _PrefsPageExerClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				prefs_page_exer_get_type				(void) G_GNUC_CONST;

PrefsPageExer * 	prefs_page_exer_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_EXER_H__ */
