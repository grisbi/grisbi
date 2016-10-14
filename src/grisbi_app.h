#ifndef __GRISBI_APP_H__
#define __GRISBI_APP_H__

#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "grisbi_settings.h"
#include "grisbi_win.h"
#include "structures.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define GRISBI_TYPE_APP         (grisbi_app_get_type ())

G_DECLARE_DERIVABLE_TYPE (GrisbiApp, grisbi_app, GRISBI, APP, GtkApplication)

struct _GrisbiAppClass
{
    GtkApplicationClass parent_class;
};


GrisbiWin *         grisbi_app_get_active_window        ( GrisbiApp *app );
GrisbiSettings *    grisbi_app_get_grisbi_settings      ( void );
GMenu *             grisbi_app_get_menu_edit            ( void );
GAction *           grisbi_app_get_prefs_action         ( void );
gchar **            grisbi_app_get_recent_files_array   ( void );
void                grisbi_app_init_recent_manager      ( gchar **recent_array );
gboolean            grisbi_app_is_duplicated_file       ( const gchar *filename );
void                grisbi_app_set_recent_files_menu    ( GApplication *app,
                                                          gboolean reset );

G_END_DECLS

#endif  /* __GRISBI_APP_H__ */
