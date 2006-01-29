#ifndef _GSB_FORM_H
#define _GSB_FORM_H (1)

/* START_INCLUDE_H */
#include "gsb_form.h"
/* END_INCLUDE_H */

/* the  the size of the scheduled part of the form,
 * fixed here for now, but can become configurable easily
 * because the filling function use that values for a table */
#define SCHEDULED_WIDTH 6
#define SCHEDULED_HEIGHT 1

enum scheduled_form_widget {
    SCHEDULED_FORM_ACCOUNT = 0,
    SCHEDULED_FORM_AUTO,
    SCHEDULED_FORM_FREQUENCY_BUTTON,
    SCHEDULED_FORM_LIMIT_DATE,
    SCHEDULED_FORM_FREQUENCY_USER_ENTRY,
    SCHEDULED_FORM_FREQUENCY_USER_BUTTON,
    SCHEDULED_FORM_MAX_WIDGETS,
};

enum direction_move {
    GSB_LEFT = 0,
    GSB_RIGHT,
    GSB_UP,
    GSB_DOWN
};

/* START_DECLARATION */
gboolean gsb_form_button_press_event ( GtkWidget *entry,
				       GdkEventButton *ev,
				       gint *ptr_origin );
gboolean gsb_form_change_sensitive_buttons ( gboolean sensitive );
gboolean gsb_form_check_entry_is_empty ( GtkWidget *entry );
gint gsb_form_check_for_transfer ( const gchar *entry_string );
gboolean gsb_form_clean ( gint account_number );
gboolean gsb_form_entry_get_focus ( GtkWidget *entry,
				    GdkEventFocus *ev,
				    gpointer null );
gboolean gsb_form_entry_lose_focus ( GtkWidget *entry,
				     GdkEventFocus *ev,
				     gint *ptr_origin );
gboolean gsb_form_escape_form ( void );
gboolean gsb_form_fill_transaction_part ( gint *ptr_account_number );
gint gsb_form_get_account_number_from_origin ( gint origin );
gchar *gsb_form_get_element_name ( gint element_number );
GtkWidget *gsb_form_get_element_widget ( gint element_number );
GtkWidget *gsb_form_get_element_widget_2 ( gint element_number,
					   gint account_number );
gint gsb_form_get_next_element ( gint account_number,
				 gint element_number,
				 gint direction );
gint gsb_form_get_origin ( void );
gboolean gsb_form_hide ( void );
GtkWidget *gsb_form_new ( void );
gboolean gsb_form_set_expander_visible ( gboolean visible,
					 gboolean transactions_list );
gboolean gsb_form_show ( void );
/* END_DECLARATION */
#endif
