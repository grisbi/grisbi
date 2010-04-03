#ifndef _DATA_PAYMENT_H
#define _DATA_PAYMENT_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */

#define GSB_PAYMENT_NEUTRAL 0
#define GSB_PAYMENT_DEBIT 1
#define GSB_PAYMENT_CREDIT 2



/* START_DECLARATION */
gint gsb_data_payment_create_default  ( gint account_number );
gint gsb_data_payment_get_account_number ( gint payment_number );
gint gsb_data_payment_get_automatic_numbering ( gint payment_number );
const gchar *gsb_data_payment_get_last_number ( gint payment_number );
gint gsb_data_payment_get_last_number_to_int ( gint payment_number );
GSList *gsb_data_payment_get_list_for_account ( gint account_number );
const gchar *gsb_data_payment_get_name ( gint payment_number );
gint gsb_data_payment_get_number ( gpointer payment_ptr );
gint gsb_data_payment_get_number_by_name ( const gchar *name,
						    gint account_number );
GSList *gsb_data_payment_get_payments_list ( void );
gint gsb_data_payment_get_show_entry ( gint payment_number );
gint gsb_data_payment_get_sign ( gint payment_number );
gint gsb_data_payment_get_similar ( gint origin_payment,
				    gint target_account_number );
gint gsb_data_payment_get_transfer_payment_number ( gint account_number );
gchar *gsb_data_payment_incremente_last_number ( gint payment_number,
                    gint increment );
gboolean gsb_data_payment_init_variables ( void );
gint gsb_data_payment_new ( const gchar *name );
gboolean gsb_data_payment_remove ( gint payment_number );
gint gsb_data_payment_search_number_other_account_by_name ( gint payment_number,
                                gint account_number );
gboolean gsb_data_payment_set_account_number ( gint payment_number,
					       gint account_number );
gboolean gsb_data_payment_set_automatic_numbering ( gint payment_number,
						    gint automatic_numbering );
gboolean gsb_data_payment_set_last_number ( gint payment_number,
					    const gchar *last_number );
gboolean gsb_data_payment_set_last_number_from_int ( gint payment_number,
					    gint last_number );
gboolean gsb_data_payment_set_name ( gint payment_number,
				     const gchar *name );
gint gsb_data_payment_set_new_number ( gint payment_number,
				       gint new_no_payment );
gboolean gsb_data_payment_set_show_entry ( gint payment_number,
					   gint show_entry );
gboolean gsb_data_payment_set_sign ( gint payment_number,
				     gint sign );
/* END_DECLARATION */
#endif
