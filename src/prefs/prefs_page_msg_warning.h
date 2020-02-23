#ifndef __PREFS_PAGE_MSG_WARNING_H__
#define __PREFS_PAGE_MSG_WARNING_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_MSG_WARNING_TYPE    	(prefs_page_msg_warning_get_type ())
#define PREFS_PAGE_MSG_WARNING(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_MSG_WARNING_TYPE, PrefsPageMsgWarning))
#define PREFS_IS_PAGE_MSG_WARNING(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_MSG_WARNING_TYPE))

typedef struct _PrefsPageMsgWarning			PrefsPageMsgWarning;
typedef struct _PrefsPageMsgWarningClass	PrefsPageMsgWarningClass;


struct _PrefsPageMsgWarning
{
    GtkBox parent;
};

struct _PrefsPageMsgWarningClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					prefs_page_msg_warning_get_type			(void) G_GNUC_CONST;

PrefsPageMsgWarning * 	prefs_page_msg_warning_new				(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_MSG_WARNING_H__ */
