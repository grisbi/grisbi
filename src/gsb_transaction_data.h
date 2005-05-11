#ifndef _GSB_TRANSACTION_DATA_H
#define _GSB_TRANSACTION_DATA_H (1)


/** \struct
 * describe a transaction 
 * */

typedef struct
{
    /** @name general stuff */
    gint transaction_number;
    gchar *transaction_id;				/**< filled by ofx */
    gint account_number;
    gdouble transaction_amount;
    gint party_number;
    gchar *notes;
    gint marked_transaction;				/**<  0=nothing, 1=P, 2=T, 3=R */
    gshort automatic_transaction;			/**< 0=manual, 1=automatic (scheduled transaction) */
    gint reconcile_number;
    guint financial_year_number;
    gchar *voucher;
    gchar *bank_references;

    /** @name dates of the transaction */

    GDate *date;
    GDate *value_date;

    /** @name currency stuff */
    gint currency_number;
    gint change_between_account_and_transaction;	/**< if 1 : 1 account_currency = (exchange_rate * amount) transaction_currency */
    gdouble exchange_rate;
    gdouble exchange_fees;

    /** @name category stuff */
    gint category_number;
    gint sub_category_number;
    gint budgetary_number;
    gint sub_budgetary_number;
    gint transaction_number_transfer;
    gint account_number_transfer;			/**< -1 for a deleted account */
    gint breakdown_of_transaction;			/**< 1 if it's a breakdown of transaction */
    gint mother_transaction_number;			/**< for a breakdown, the mother's transaction number */

    /** @name method of payment */
    gint method_of_payment_number;
    gchar *method_of_payment_content;

} struct_transaction;


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_transaction_data_copy_transaction ( gint source_transaction_number,
						 gint target_transaction_number );
gint gsb_transaction_data_get_account_number ( gint no_transaction );
gint gsb_transaction_data_get_account_number_transfer ( gint no_transaction );
gdouble gsb_transaction_data_get_adjusted_amount ( gint no_transaction );
gdouble gsb_transaction_data_get_adjusted_amount_for_currency ( gint no_transaction,
								gint no_currency_for_return );
gdouble gsb_transaction_data_get_amount ( gint no_transaction );
gint gsb_transaction_data_get_automatic_transaction ( gint no_transaction );
gchar *gsb_transaction_data_get_bank_references ( gint no_transaction );
gint gsb_transaction_data_get_breakdown_of_transaction ( gint no_transaction );
gint gsb_transaction_data_get_budgetary_number ( gint no_transaction );
gint gsb_transaction_data_get_category_number ( gint no_transaction );
gint gsb_transaction_data_get_change_between ( gint no_transaction );
gint gsb_transaction_data_get_currency_number ( gint no_transaction );
GDate *gsb_transaction_data_get_date ( gint no_transaction );
gdouble gsb_transaction_data_get_exchange_fees ( gint no_transaction );
gdouble gsb_transaction_data_get_exchange_rate ( gint no_transaction );
gint gsb_transaction_data_get_financial_year_number ( gint no_transaction );
gint gsb_transaction_data_get_last_number (void);
gint gsb_transaction_data_get_marked_transaction ( gint no_transaction );
gchar *gsb_transaction_data_get_method_of_payment_content ( gint no_transaction );
gint gsb_transaction_data_get_method_of_payment_number ( gint no_transaction );
gint gsb_transaction_data_get_mother_transaction_number ( gint no_transaction );
gchar *gsb_transaction_data_get_notes ( gint no_transaction );
gint gsb_transaction_data_get_party_number ( gint no_transaction );
gpointer gsb_transaction_data_get_pointer_to_transaction ( gint transaction_number );
gint gsb_transaction_data_get_reconcile_number ( gint no_transaction );
gint gsb_transaction_data_get_sub_budgetary_number ( gint no_transaction );
gint gsb_transaction_data_get_sub_category_number ( gint no_transaction );
gchar *gsb_transaction_data_get_transaction_id ( gint no_transaction );
gint gsb_transaction_data_get_transaction_number ( gpointer transaction_pointer );
gint gsb_transaction_data_get_transaction_number_transfer ( gint no_transaction );
GDate *gsb_transaction_data_get_value_date ( gint no_transaction );
gchar *gsb_transaction_data_get_voucher ( gint no_transaction );
gboolean gsb_transaction_data_init_variables ( void );
gint gsb_transaction_data_new_transaction ( gint no_account );
gboolean gsb_transaction_data_set_account_number ( gint no_transaction,
						   gint no_account );
gboolean gsb_transaction_data_set_account_number_transfer ( gint no_transaction,
							    gint account_number_transfer );
gboolean gsb_transaction_data_set_amount ( gint no_transaction,
					   gdouble amount );
gboolean gsb_transaction_data_set_automatic_transaction ( gint no_transaction,
							  gint automatic_transaction );
gboolean gsb_transaction_data_set_bank_references ( gint no_transaction,
						    gchar *bank_references );
gboolean gsb_transaction_data_set_breakdown_of_transaction ( gint no_transaction,
							     gint is_breakdown );
gboolean gsb_transaction_data_set_budgetary_number ( gint no_transaction,
						     gint budgetary_number );
gboolean gsb_transaction_data_set_category_number ( gint no_transaction,
						    gint no_category );
gboolean gsb_transaction_data_set_change_between ( gint no_transaction,
						   gint value );
gboolean gsb_transaction_data_set_currency_number ( gint no_transaction,
						    gint no_currency );
gboolean gsb_transaction_data_set_date ( gint no_transaction,
					 GDate *date );
gboolean gsb_transaction_data_set_exchange_fees ( gint no_transaction,
						  gdouble rate );
gboolean gsb_transaction_data_set_exchange_rate ( gint no_transaction,
						  gdouble rate );
gboolean gsb_transaction_data_set_financial_year_number ( gint no_transaction,
							  gint financial_year_number );
gboolean gsb_transaction_data_set_marked_transaction ( gint no_transaction,
						       gint marked_transaction );
gboolean gsb_transaction_data_set_method_of_payment_content ( gint no_transaction,
							      gchar *method_of_payment_content );
gboolean gsb_transaction_data_set_method_of_payment_number ( gint no_transaction,
							     gint number );
gboolean gsb_transaction_data_set_mother_transaction_number ( gint no_transaction,
							      gint mother_transaction_number );
gboolean gsb_transaction_data_set_notes ( gint no_transaction,
					  gchar *notes );
gboolean gsb_transaction_data_set_party_number ( gint no_transaction,
						 gint no_party );
gboolean gsb_transaction_data_set_reconcile_number ( gint no_transaction,
						     gint reconcile_number );
gboolean gsb_transaction_data_set_sub_budgetary_number ( gint no_transaction,
							 gint sub_budgetary_number );
gboolean gsb_transaction_data_set_sub_category_number ( gint no_transaction,
							gint no_sub_category );
gboolean gsb_transaction_data_set_transaction_id ( gint no_transaction,
						   gchar *transaction_id );
gboolean gsb_transaction_data_set_transaction_number ( gpointer transaction_pointer,
						       gint no_transaction );
gboolean gsb_transaction_data_set_transaction_number_transfer ( gint no_transaction,
							 gint transaction_number_transfer );
gboolean gsb_transaction_data_set_value_date ( gint no_transaction,
					       GDate *date );
gboolean gsb_transaction_data_set_voucher ( gint no_transaction,
					    gchar *voucher );
/* END_DECLARATION */


#endif

