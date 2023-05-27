#ifndef _GSB_FORM_H
#define _GSB_FORM_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/** returned values want ask for the origin
 * must be < 0 because 0 and more are reserved for account numbers */
enum OriginValues
{
    ORIGIN_VALUE_OTHER = -3,
    ORIGIN_VALUE_HOME,
    ORIGIN_VALUE_SCHEDULED
};


/* START_DECLARATION */
gboolean 	gsb_form_button_press_event 			(GtkWidget *entry,
													 GdkEventButton *ev,
													 gint *ptr_origin);
gboolean 	gsb_form_change_sensitive_buttons 		(gboolean sensitive);
void 		gsb_form_check_auto_separator 			(GtkWidget *entry);
gboolean 	gsb_form_clean 							(gint account_number);
void 		gsb_form_create_widgets 				(void);
gboolean 	gsb_form_entry_get_focus 				(GtkWidget *entry);
gboolean 	gsb_form_entry_lose_focus 				(GtkWidget *entry,
													 GdkEventFocus *ev,
													 gint *ptr_origin);
gboolean 	gsb_form_escape_form 					(void);
void	 	gsb_form_expander_is_extanded 			(GtkWidget *expander);
gboolean 	gsb_form_fill_by_transaction 			(gint transaction_number,
													 gint is_transaction,
													 gboolean grab_focus);
void 		gsb_form_fill_element 					(gint element_number,
													 gint account_number,
													 gint transaction_number,
													 gboolean is_transaction);
gboolean 	gsb_form_finish_edition 				(void);
gint 		gsb_form_get_account_number 			(void);
GtkWidget *	gsb_form_get_element_widget_from_list 	(gint element_number,
													 GSList *list);
GtkWidget *	gsb_form_get_recover_split_button		(void);
GtkWidget *	gsb_form_get_form_transaction_part 		(void);
GtkWidget *	gsb_form_get_form_widget 				(void);
gint 		gsb_form_get_origin 					(void);
GtkWidget *	gsb_form_get_scheduler_part 			(void);
gboolean 	gsb_form_key_press_event 				(GtkWidget *widget,
													 GdkEventKey *ev,
													 gint *ptr_origin);
void		gsb_form_sensitive_cancel_valid_buttons	(gboolean sensitive);
gboolean 	gsb_form_set_sensitive 					(gboolean split,
													 gboolean split_child);
gboolean 	gsb_form_show 							(gboolean show);
/* END_DECLARATION */
#endif
