#ifndef __GRISBI_APP_H__
#define __GRISBI_APP_H__

#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "grisbi_win.h"
#include "structures.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define GRISBI_APP_TYPE (grisbi_app_get_type ())
#define GRISBI_APP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRISBI_APP_TYPE, GrisbiApp))


typedef struct _GrisbiApp       GrisbiApp;
typedef struct _GrisbiAppClass  GrisbiAppClass;


GType grisbi_app_get_type ( void );

GrisbiApp *grisbi_app_new ( char *app_name );
GrisbiWin *grisbi_app_create_window ( GrisbiApp *app,
                        GdkScreen *screen );
GrisbiWin *grisbi_app_get_active_window ( GrisbiApp *app );
GtkRecentManager *grisbi_app_get_recent_manager ( void );
gboolean grisbi_app_is_duplicated_file ( const gchar *filename );

G_END_DECLS

#endif  /* __GRISBI_APP_H__ */
