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
    gdouble amount;
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
    gint day;
    gint month;
    gint year;

    GDate *value_date;
    gint value_day;
    gint value_month;
    gint value_year;

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


/* START_DECLARATION */
gboolean gsb_transaction_data_init_variables ( void );
/* END_DECLARATION */


#endif

