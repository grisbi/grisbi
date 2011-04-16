#ifndef _GSB_FORM_WIDGET_H
#define _GSB_FORM_WIDGET_H (1)

/**
 * \struct
 * Associate an element number to its widget,
 * used to be appended in the widgets list
 * */
typedef struct
{
    gint element_number;
    GtkWidget *element_widget;
} struct_element;


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean gsb_form_widget_amount_entry_changed ( GtkWidget *entry,
                        gpointer null );
gboolean gsb_form_widget_amount_entry_validate ( gint element_number );
gboolean gsb_form_widget_check_empty ( GtkWidget *entry );
GtkWidget *gsb_form_widget_create ( gint element_number,
                        gint account_number );
gboolean gsb_form_widget_entry_get_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gint *ptr_origin );
gboolean gsb_form_widget_free_list ( void );
GSList *gsb_form_widget_get_list ( void );
gchar *gsb_form_widget_get_name ( gint element_number );
gchar *gsb_form_widget_get_old_credit ( void );
gint gsb_form_widget_get_old_credit_payment_number ( void );
const gchar *gsb_form_widget_get_old_credit_payment_content ( void );
gchar *gsb_form_widget_get_old_debit ( void );
gint gsb_form_widget_get_old_debit_payment_number ( void );
const gchar *gsb_form_widget_get_old_debit_payment_content ( void );
gboolean gsb_form_widget_get_valide_amout_entry ( const gchar *string );
GtkWidget *gsb_form_widget_get_widget ( gint element_number );
gint gsb_form_widget_next_element ( gint account_number,
                        gint element_number,
                        gint direction );
void gsb_form_widget_set_empty ( GtkWidget *entry,
                    gboolean empty );
void gsb_form_widget_set_focus ( gint element_number );
/* END_DECLARATION */
#endif
