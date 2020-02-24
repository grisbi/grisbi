#ifndef __PREFS_PAGE_MSG_DELETE_H__
#define __PREFS_PAGE_MSG_DELETE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_MSG_DELETE_TYPE    	(prefs_page_msg_delete_get_type ())
#define PREFS_PAGE_MSG_DELETE(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_MSG_DELETE_TYPE, PrefsPageMsgDelete))
#define PREFS_IS_PAGE_MSG_DELETE(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_MSG_DELETE_TYPE))

typedef struct _PrefsPageMsgDelete			PrefsPageMsgDelete;
typedef struct _PrefsPageMsgDeleteClass		PrefsPageMsgDeleteClass;


struct _PrefsPageMsgDelete
{
    GtkBox parent;
};

struct _PrefsPageMsgDeleteClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				prefs_page_msg_delete_get_type				(void) G_GNUC_CONST;

PrefsPageMsgDelete * 	prefs_page_msg_delete_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_MSG_DELETE_H__ */
