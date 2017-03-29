#ifndef __GRISBI_PREFS_H__
#define __GRISBI_PREFS_H__

#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "grisbi_win.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GRISBI_PREFS_TYPE              (grisbi_prefs_get_type())
#define GRISBI_PREFS(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GRISBI_PREFS_TYPE, GrisbiPrefs))
#define GRISBI_IS_PREFS(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GRISBI_PREFS_TYPE))

typedef struct _GrisbiPrefs 		GrisbiPrefs;
typedef struct _GrisbiPrefsClass 	GrisbiPrefsClass;

/* START_DECLARATION */
GType grisbi_prefs_get_type (void) G_GNUC_CONST;

GrisbiPrefs *grisbi_prefs_new (GrisbiWin *win);



//~ GtkWidget *grisbi_prefs_get_widget_by_name ( const gchar *name );
//~ void grisbi_prefs_refresh_preferences ( gboolean new_file );
//~ void grisb_prefs_show_dialog ( GrisbiWin *parent );
/* END_DECLARATION */

G_END_DECLS

#endif  /* __GRISBI_PREFS_H__ */
