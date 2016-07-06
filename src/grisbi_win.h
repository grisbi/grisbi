#ifndef __GRISBI_WIN_H__
#define __GRISBI_WIN_H__

#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "structures.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define GRISBI_WIN_TYPE (grisbi_win_get_type ())
#define GRISBI_WIN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRISBI_WIN_TYPE, GrisbiWin))


typedef struct _GrisbiWin       GrisbiWin;
typedef struct _GrisbiWinClass  GrisbiWinClass;


GType grisbi_win_get_type ( void );

void            grisbi_win_del_menu_move_to_acc     ( void );
const gchar *   grisbi_win_get_filename             ( GrisbiWin *win );
GtkWidget *     grisbi_win_get_main_box             ( GrisbiWin *win );
void            grisbi_win_init_menubar             ( GrisbiWin *win,
                                                      gpointer app );
void            grisbi_win_new_menu_move_to_acc     ( void );
void            grisbi_win_set_filename             ( GrisbiWin *win,
                                                      const gchar *filename );
gboolean        grisbi_win_set_grisbi_title         ( gint account_number );
void            grisbi_win_set_size_and_position    ( GtkWindow *win );
void            grisbi_win_update_menu_move_to_acc  ( gboolean active );
G_END_DECLS

#endif  /* __GRISBI_APP_H__ */
