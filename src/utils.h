#ifndef _UTILS_H
#define _UTILS_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean        utils_event_box_change_state                            (GtkWidget *event_box,
                                                                         GdkEventMotion *event,
                                                                         GtkStyleContext *context);

gboolean        assert_account_loaded                                   (void);
GtkWidget *     utils_combo_box_make_from_string_array                  (const gchar **array);
gboolean        desensitive_widget                                      (gpointer object,
                                                                         GtkWidget *widget);
gchar *         get_gtk_run_version                                     (void);
void            lance_mailer                                            (const gchar *uri);
gboolean        lance_navigateur_web                                    (const gchar *url);
GtkWidget *     new_paddingbox_with_title                               (GtkWidget *parent,
                                                                         gboolean fill,
                                                                         const gchar *title);
GtkWidget *     new_vbox_with_title_and_icon                            (const gchar *title,
                                                                         const gchar *image_filename);
gboolean        radio_set_active_linked_widgets                         (GtkWidget *widget);
void            register_button_as_linked                               (GtkWidget *widget,
                                                                         GtkWidget *linked);
gboolean        sens_desensitive_pointeur                               (GtkWidget *bouton,
                                                                         GtkWidget *widget);
gboolean        sensitive_widget                                        (gpointer object,
                                                                         GtkWidget *widget);
void            update_gui                                              (void);
GtkWidget *		utils_menu_item_new_from_image_label 					(const gchar *image_name,
																		 const gchar *label_name);
GtkWidget *		utils_menu_item_new_from_resource_label 				(const gchar *image_name,
																		 const gchar *label_name);
void            utils_container_remove_children                         (GtkWidget *widget);
GtkWidget *     utils_get_image_with_state                              (GtkMessageType msg,
                                                                         gint initial,
                                                                         const gchar *tooltip_0,
                                                                         const gchar *tooltip_1);
void            utils_gtk_combo_box_set_text_renderer                   (GtkComboBox *combo,
                                                                         gint num_col);
GtkListStore *  utils_list_store_create_from_string_array               (const gchar **array);
void            utils_labels_set_alignment                             	(GtkLabel *label,
                                                                         gfloat xalign,
                                                                         gfloat yalign);
gboolean        utils_set_image_with_state                              (GtkWidget *widget,
                                                                         gint state);
gboolean		utils_set_list_store_background_color 					(GtkWidget *tree_view,
																		 gint color_column);
gboolean        utils_set_tree_store_background_color                    (GtkWidget *tree_view,
                                                                         gint color_column);
gboolean        utils_tree_view_all_rows_are_selected                   (GtkTreeView *tree_view);
void            utils_tree_view_set_expand_all_and_select_path_realize  (GtkWidget *tree_view,
                                                                         const gchar *str_path);
/* END_DECLARATION */
#endif
