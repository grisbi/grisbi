#ifndef __GRISBI_WIN_H__
#define __GRISBI_WIN_H__

#include <gtk/gtk.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define GRISBI_TYPE_WIN         (grisbi_win_get_type ())
#define GRISBI_WIN(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRISBI_TYPE_WIN, GrisbiWin))
#define GRISBI_IS_WINDOW(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj), GRISBI_TYPE_WIN))

typedef struct _GrisbiWin       GrisbiWin;
typedef struct _GrisbiWinClass  GrisbiWinClass;

GType grisbi_win_get_type (void) G_GNUC_CONST;

GtkWidget *     grisbi_win_create_general_widgets           (GrisbiWin *win);

const gchar *   grisbi_win_get_filename                     (GrisbiWin *win);
GtkWidget *     grisbi_win_get_notebook_general             (GrisbiWin *win);
GtkWidget *		grisbi_win_get_prefs_dialog 				(GrisbiWin *win);
GtkWidget *     grisbi_win_get_stack_box                    (GrisbiWin *win);
GtkWidget *		grisbi_win_get_vbox_transactions_list 		(GrisbiWin *win);
gpointer		grisbi_win_get_w_etat						(void);
gpointer		grisbi_win_get_w_run						(void);

void            grisbi_win_headings_update_suffix           (gchar *suffix);
void            grisbi_win_headings_update_title            (gchar *title);
gboolean        grisbi_win_headings_update_show_headings    (void);
void            grisbi_win_headings_sensitive_headings      (gboolean sensitive);

gboolean 		grisbi_win_file_is_loading 					(void);
void            grisbi_win_free_general_notebook            (void);
void            grisbi_win_free_general_vbox                (void);

void            grisbi_win_init_menubar                     (GrisbiWin *win,
                                                             gpointer app);
void            grisbi_win_menu_move_to_acc_delete          (void);
void            grisbi_win_menu_move_to_acc_new             (void);
void            grisbi_win_menu_move_to_acc_update          (gboolean active);

void            grisbi_win_new_file_gui						(void);
void 			grisbi_win_no_file_page_update 				(GrisbiWin *win);

void            grisbi_win_set_filename                     (GrisbiWin *win,
                                                             const gchar *filename);
void 			grisbi_win_set_prefs_dialog 				(GrisbiWin *win,
															 GtkWidget *prefs_dialog);
void            grisbi_win_set_size_and_position            (GtkWindow *win);
gboolean 		grisbi_win_set_vbox_transactions_list 		(GrisbiWin *win,
															 GtkWidget *vbox);
gboolean        grisbi_win_set_window_title                 (gint account_number);

void 			grisbi_win_stack_box_show					(GrisbiWin *win,
															 const gchar *page_name);
void            grisbi_win_status_bar_clear                 (void);
void            grisbi_win_status_bar_message               (gchar *message);
void            grisbi_win_status_bar_stop_wait             (gboolean force_update);
void            grisbi_win_status_bar_wait                  (gboolean force_update);

G_END_DECLS

#endif  /* __GRISBI_APP_H__ */
