#ifndef _GSB_FORM_H
#define _GSB_FORM_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */

enum direction_move {
    GSB_LEFT = 0,
    GSB_RIGHT,
    GSB_UP,
    GSB_DOWN
};

/** returned values want ask for the origin
 * must be < 0 because 0 and more are reserved for account numbers */
enum origin_values {
    ORIGIN_VALUE_OTHER = -3,
    ORIGIN_VALUE_HOME,
    ORIGIN_VALUE_SCHEDULED
};


/* START_DECLARATION */
gboolean gsb_form_allocate_size ( GtkWidget *table,
                        GtkAllocation *allocation,
                        gpointer null );
gboolean gsb_form_button_press_event ( GtkWidget *entry,
                        GdkEventButton *ev,
                        gint *ptr_origin );
gboolean gsb_form_change_sensitive_buttons ( gboolean sensitive );
void gsb_form_check_auto_separator ( GtkWidget *entry );
gboolean gsb_form_clean ( gint account_number );
void gsb_form_create_widgets ( void );
gboolean gsb_form_entry_get_focus ( GtkWidget *entry );
gboolean gsb_form_entry_lose_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gint *ptr_origin );
gboolean gsb_form_escape_form ( void );
gboolean gsb_form_fill_by_transaction ( gint transaction_number,
                        gint is_transaction,
                        gboolean grab_focus );
void gsb_form_fill_element ( gint element_number,
                        gint account_number,
                        gint transaction_number,
                        gboolean is_transaction );
gboolean gsb_form_fill_from_account ( gint account_number );
gboolean gsb_form_finish_edition ( void );
gint gsb_form_get_account_number ( void );
gint gsb_form_get_element_expandable ( gint element_number );
GtkWidget *gsb_form_get_element_widget_from_list ( gint element_number,
                        GSList *list );
GtkWidget *gsb_form_get_form_widget ( void );
gint gsb_form_get_origin ( void );
GtkWidget *gsb_form_get_scheduler_part ( void );
gboolean gsb_form_is_visible ( void );
gboolean gsb_form_key_press_event ( GtkWidget *widget,
                        GdkEventKey *ev,
                        gint *ptr_origin );
GtkWidget *gsb_form_new ( void );
gboolean gsb_form_set_expander_visible ( gboolean visible,
                        gboolean transactions_list );
gboolean gsb_form_set_sensitive ( gboolean split,
                        gboolean split_child);
gboolean gsb_form_show ( gboolean show );
gboolean gsb_form_switch_expander ( void );
/* END_DECLARATION */
#endif
