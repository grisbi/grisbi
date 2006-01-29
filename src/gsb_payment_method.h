#ifndef _TYPE_OPERATIONS_H
#define _TYPE_OPERATIONS_H (1)

/** Columns for payment methods tree */
enum payment_methods_columns {
    PAYMENT_METHODS_NAME_COLUMN = 0,
    PAYMENT_METHODS_NUMBERING_COLUMN,
    PAYMENT_METHODS_DEFAULT_COLUMN,
    PAYMENT_METHODS_TYPE_COLUMN,
    PAYMENT_METHODS_VISIBLE_COLUMN,
    PAYMENT_METHODS_ACTIVABLE_COLUMN,
    PAYMENT_METHODS_POINTER_COLUMN,
    NUM_PAYMENT_METHODS_COLUMNS,
};

#define GSB_PAYMENT_NEUTRAL 0
#define GSB_PAYMENT_DEBIT 1
#define GSB_PAYMENT_CREDIT 2

/* START_INCLUDE_H */
#include "gsb_payment_method.h"
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gchar *gsb_payment_method_automatic_numbering_get_new_number ( gint payment_number,
							       gint account_number );
gboolean gsb_payment_method_changed_callback ( GtkWidget *combo_box,
					       gpointer null );
gboolean gsb_payment_method_create_combo_list ( GtkWidget *combo_box,
						gint sign,
						gint account_number );
gchar *gsb_payment_method_get_automatic_current_number ( struct struct_type_ope * method_ptr );
gboolean gsb_payment_method_get_automatic_number ( gint payment_number,
						   gint account_number );
gint gsb_payment_method_get_combo_sign ( GtkWidget *combo_box );
gint gsb_payment_method_get_payment_location ( GtkWidget *combo_box,
					       gint payment_number );
gint gsb_payment_method_get_selected_number ( GtkWidget *combo_box );
gboolean gsb_payment_method_get_show_entry ( gint payment_number,
					     gint account_number );
/* END_DECLARATION */
#endif
