#ifndef _GSB_FORM_SCHEDULER_H
#define _GSB_FORM_SCHEDULER_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* the  the size of the scheduled part of the form,
 * fixed here for now, but can become configurable easily
 * because the filling function use that values for a table */
#define SCHEDULED_WIDTH 6
#define SCHEDULED_HEIGHT 1

/* that enum is in the order of the widgets, if we switch 2 of them,
 * they will be switched on the form */
enum scheduled_form_widget {
    SCHEDULED_FORM_ACCOUNT = 0,
    SCHEDULED_FORM_AUTO,
    SCHEDULED_FORM_FREQUENCY_BUTTON,
    SCHEDULED_FORM_LIMIT_DATE,
    SCHEDULED_FORM_FREQUENCY_USER_ENTRY,
    SCHEDULED_FORM_FREQUENCY_USER_BUTTON,
    SCHEDULED_FORM_MAX_WIDGETS,
};


/* START_DECLARATION */
gboolean gsb_form_scheduler_button_press_event ( GtkWidget *entry,
                        GdkEventButton *ev,
                        gint *ptr_origin );
gboolean gsb_form_scheduler_change_account ( GtkWidget *button,
                        gpointer null );
gboolean gsb_form_scheduler_clean ( void );
gboolean gsb_form_scheduler_create ( GtkWidget *table );
gboolean gsb_form_scheduler_entry_lose_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gint *ptr_origin );
gboolean gsb_form_scheduler_free_list ( void );
gint gsb_form_scheduler_get_account ( void );
GtkWidget *gsb_form_scheduler_get_element_widget ( gint element_number );
gboolean gsb_form_scheduler_get_scheduler_part ( gint scheduled_number );
gboolean gsb_form_scheduler_recover_splits_of_transaction ( gint scheduled_transaction,
							    gint transaction_number );
gboolean gsb_form_scheduler_sensitive_buttons ( gboolean sensitive );
gboolean gsb_form_scheduler_set ( gint scheduled_number );
gboolean gsb_form_scheduler_set_frequency ( gint frequency );
/* END_DECLARATION */
#endif
