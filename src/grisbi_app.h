#ifndef __GRISBI_APP_H__
#define __GRISBI_APP_H__

#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "grisbi_win.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

extern gboolean darkmode;

#define GRISBI_APP_TYPE         (grisbi_app_get_type ())
#define GRISBI_APP(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRISBI_APP_TYPE, GrisbiApp))

typedef struct _GrisbiApp       GrisbiApp;
typedef struct _GrisbiAppClass  GrisbiAppClass;

struct _GrisbiApp
{
    GtkApplication parent;
};

struct _GrisbiAppClass
{
    GtkApplicationClass parent_class;
};

GType				grisbi_app_get_type						(void) G_GNUC_CONST;

void				grisbi_app_display_gui_dump_accels		(GtkApplication *application,
															 GtkWidget *text_view);
gpointer			grisbi_app_get_a_conf					(void);
GrisbiWin *			grisbi_app_get_active_window			(GrisbiApp *app);
const gchar *		grisbi_app_get_css_data					(void);
GtkCssProvider *	grisbi_app_get_css_provider				(void);
gboolean			grisbi_app_get_has_app_menu				(GrisbiApp *app);
gboolean			grisbi_app_get_low_definition_screen	(void);
GMenu *				grisbi_app_get_menu_edit				(void);
GAction *			grisbi_app_get_prefs_action				(void);
gchar **			grisbi_app_get_recent_files_array		(void);
gboolean			grisbi_app_is_duplicated_file			(const gchar *filename);
void 				grisbi_app_set_css_data			 		(const gchar *new_css_data);
void				grisbi_app_set_recent_files_array 		(gchar **recent_array);
void				grisbi_app_update_recent_files_menu 	(void);
void				grisbi_app_window_style_updated			(GtkWidget *win,
															 gpointer null);
G_END_DECLS

#endif  /* __GRISBI_APP_H__ */
