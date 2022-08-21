#ifndef __GRISBI_WIN_H__
#define __GRISBI_WIN_H__

#include <gtk/gtk.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define GRISBI_WIN_TYPE         (grisbi_win_get_type ())
#define GRISBI_WIN(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRISBI_WIN_TYPE, GrisbiWin))
#define GRISBI_IS_WINDOW(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj), GRISBI_WIN_TYPE))

typedef struct _GrisbiWin       GrisbiWin;
typedef struct _GrisbiWinClass  GrisbiWinClass;

GType 			grisbi_win_get_type (void) G_GNUC_CONST;

void			grisbi_win_close_window					(GtkWindow *win);

void 			grisbi_win_form_expander_hide_frame			(void);
gboolean 		grisbi_win_form_expander_is_expanded 		(void);
void 			grisbi_win_form_expander_show_frame			(void);
void			grisbi_win_form_label_align_right 			(GtkAllocation *allocation);

void 			grisbi_win_general_notebook_set_page 		(gint page);
GtkWidget *     grisbi_win_get_account_page					(void);
GtkWidget *     grisbi_win_get_account_property_page		(void);
const gchar *   grisbi_win_get_filename                     (GrisbiWin *win);
GtkWidget *		grisbi_win_get_form_expander				(void);
gpointer 		grisbi_win_get_form_organization 			(void);
GtkWidget *		grisbi_win_get_label_last_statement 		(void);
GtkWidget *     grisbi_win_get_notebook_general             (void);
GtkWidget *		grisbi_win_get_prefs_dialog 				(GrisbiWin *win);
GtkWidget *     grisbi_win_get_stack_box                    (GrisbiWin *win);
GtkWidget *		grisbi_win_get_vbox_transactions_list 		(GrisbiWin *win);
gpointer		grisbi_win_get_w_etat						(void);
gpointer		grisbi_win_get_w_run						(void);

void            grisbi_win_headings_update_suffix           (const gchar *suffix);
void            grisbi_win_headings_update_title            (gchar *title);
gboolean        grisbi_win_headings_update_show_headings    (void);
void            grisbi_win_headings_sensitive_headings      (gboolean sensitive);

gboolean 		grisbi_win_file_is_loading 					(void);
void			grisbi_win_free_account_property_page		(void);
void            grisbi_win_free_general_notebook            (void);
void            grisbi_win_free_general_vbox                (void);

void            grisbi_win_init_menubar                     (GrisbiWin *win,
                                                             gpointer app);
void            grisbi_win_menu_move_to_acc_delete          (void);
void            grisbi_win_menu_move_to_acc_new             (void);
void            grisbi_win_menu_move_to_acc_update          (gboolean active);

void            grisbi_win_new_file_gui						(void);
void 			grisbi_win_no_file_page_update 				(GrisbiWin *win);

gboolean 		grisbi_win_on_account_switch_page			(GtkNotebook *notebook,
															 gpointer page,
															 guint page_number,
															 gpointer null);

void            grisbi_win_set_filename                     (GrisbiWin *win,
                                                             const gchar *filename);
gboolean 		grisbi_win_set_form_expander_visible 		(gboolean visible,
															 gboolean transactions_list);
gboolean 		grisbi_win_set_form_organization			(gpointer FormOrganization);
void 			grisbi_win_set_prefs_dialog 				(GrisbiWin *win,
															 GtkWidget *prefs_dialog);
void            grisbi_win_set_size_and_position            (GtkWindow *win);
gboolean 		grisbi_win_set_vbox_transactions_list 		(GrisbiWin *win,
															 GtkWidget *vbox);
gboolean        grisbi_win_set_window_title                 (gint account_number);

void 			grisbi_win_stack_box_show					(GrisbiWin *win,
															 const gchar *page_name);
void            grisbi_win_status_bar_clear                 (void);
gboolean		grisbi_win_status_bar_get_wait_state		(void);
void			grisbi_win_status_bar_init					(GrisbiWin *win);
void            grisbi_win_status_bar_message               (gchar *message);
void			grisbi_win_status_bar_set_font_size			(gint font_size);
void            grisbi_win_status_bar_stop_wait             (gboolean force_update);
void            grisbi_win_status_bar_wait                  (gboolean force_update);
gboolean 		grisbi_win_switch_form_expander 			(void);

void 			grisbi_win_update_all_toolbars 				(void);
G_END_DECLS

#endif  /* __GRISBI_APP_H__ */
