#ifndef UTILS_BUTTONS_H
#define UTILS_BUTTONS_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


typedef enum GSB_BUTTON_STYLE {
    GSB_BUTTON_TEXT,
    GSB_BUTTON_ICON,
    GSB_BUTTON_BOTH,
    GSB_BUTTON_BOTH_HORIZ
} GsbButtonStyle;


/*START_DECLARATION*/
gboolean        gsb_button_sensitive_by_checkbutton             (GtkWidget *check_button,
                                                                 GtkWidget *widget);
GtkWidget *     new_image_label                                 (GsbButtonStyle style,
                                                                 const gchar *image_name,
                                                                 const gchar *name );
GtkWidget *     new_stock_image_label                           (GsbButtonStyle style,
                                                                 const gchar *stock_id,
                                                                 const gchar *name);
void            set_popup_position                              (GtkMenu *menu,
                                                                 gint *x,
                                                                 gint *y,
                                                                 gboolean *push_in,
                                                                 gpointer user_data);
GtkWidget *     utils_buttons_button_new_from_image             (const gchar *image_name);
GtkWidget *     utils_buttons_button_new_from_stock             (const gchar *icon_name,
                                                                 const gchar *label_name);
GtkToolItem *   utils_buttons_tool_button_new_from_image_label  (const gchar *image_name,
                                                                 const gchar *label_name);
GtkToolItem *   utils_buttons_tool_menu_new_from_image_label    (const gchar *image_name,
                                                                 const gchar *label_name);
gint            utils_radiobutton_get_active_index              (GtkWidget *radiobutton);
void            utils_radiobutton_set_active_index              (GtkWidget *radiobutton,
                                                                 gint index);
void            utils_togglebutton_collapse_expand_all_rows     (GtkToggleButton *togglebutton,
                                                                 GtkWidget *tree_view);
void            utils_togglebutton_change_label_select_unselect (GtkToggleButton *togglebutton,
                                                                 gint toggle);
void            utils_togglebutton_select_unselect_all_rows     (GtkToggleButton *togglebutton,
                                                                 GtkWidget *tree_view);
void            utils_togglebutton_set_label_position_unselect  (GtkWidget *togglebutton,
                                                                 GCallback callback,
                                                                 GtkWidget *tree_view);
/*END_DECLARATION*/

#endif
