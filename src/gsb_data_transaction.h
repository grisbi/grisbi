#ifndef _GSB_TRANSACTION_DATA_H
#define _GSB_TRANSACTION_DATA_H (1)


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */


/** Etat de rapprochement d'une opération */
enum operation_etat_rapprochement {
  OPERATION_NORMALE = 0,
  OPERATION_POINTEE,
  OPERATION_TELERAPPROCHEE,
  OPERATION_RAPPROCHEE,
};


/* START_DECLARATION */
gboolean gsb_data_transaction_add_archived_to_list ( gint transaction_number );
gint gsb_data_transaction_check_content_payment ( gint payment_number,
                        const gchar *number );
gboolean gsb_data_transaction_copy_transaction ( gint source_transaction_number,
                        gint target_transaction_number,
                        gboolean reset_mark );
gint gsb_data_transaction_find_by_id ( gchar *id, gint account_number );
gint gsb_data_transaction_find_by_payment_content ( const gchar *string,
                        gint account_number );
gint gsb_data_transaction_get_account_number ( gint transaction_number );
gsb_real gsb_data_transaction_get_adjusted_amount ( gint transaction_number,
                        gint return_exponent );
gsb_real gsb_data_transaction_get_adjusted_amount_for_currency ( gint transaction_number,
                        gint return_currency_number,
                        gint return_exponent );
gsb_real gsb_data_transaction_get_amount ( gint transaction_number );
gint gsb_data_transaction_get_archive_number ( gint transaction_number );
gint gsb_data_transaction_get_automatic_transaction ( gint transaction_number );
const gchar *gsb_data_transaction_get_bank_references ( gint transaction_number );
gint gsb_data_transaction_get_budgetary_number ( gint transaction_number );
gint gsb_data_transaction_get_category_number ( gint transaction_number );
gint gsb_data_transaction_get_change_between ( gint transaction_number );
GSList *gsb_data_transaction_get_children ( gint transaction_number,
                        gboolean return_number);
GSList *gsb_data_transaction_get_complete_transactions_list ( void );
gint gsb_data_transaction_get_contra_transaction_account ( gint transaction_number );
gint gsb_data_transaction_get_contra_transaction_number ( gint transaction_number );
gint gsb_data_transaction_get_currency_number ( gint transaction_number );
gint gsb_data_transaction_get_currency_floating_point ( gint transaction_number );
const GDate *gsb_data_transaction_get_date ( gint transaction_number );
gsb_real gsb_data_transaction_get_exchange_fees ( gint transaction_number );
gsb_real gsb_data_transaction_get_exchange_rate ( gint transaction_number );
gint gsb_data_transaction_get_financial_year_number ( gint transaction_number );
const gchar *gsb_data_transaction_get_id ( gint transaction_number );
gint gsb_data_transaction_get_last_number (void);
gsb_real gsb_data_transaction_get_last_transaction_with_div_sub_div (
                        gint account_number,
                        gint div_number,
                        gint sub_div_nb,
                        gint type_div );
gint gsb_data_transaction_get_marked_transaction ( gint transaction_number );
const gchar *gsb_data_transaction_get_method_of_payment_content ( gint transaction_number );
gint gsb_data_transaction_get_method_of_payment_number ( gint transaction_number );
gint gsb_data_transaction_get_mother_transaction_number ( gint transaction_number );
const gchar *gsb_data_transaction_get_notes ( gint transaction_number );
gint gsb_data_transaction_get_party_number ( gint transaction_number );
gpointer gsb_data_transaction_get_pointer_of_transaction ( gint transaction_number );
gint gsb_data_transaction_get_reconcile_number ( gint transaction_number );
gint gsb_data_transaction_get_split_of_transaction ( gint transaction_number );
gint gsb_data_transaction_get_sub_budgetary_number ( gint transaction_number );
gint gsb_data_transaction_get_sub_category_number ( gint transaction_number );
const gchar *gsb_data_transaction_get_transaction_id ( gint transaction_number );
gint gsb_data_transaction_get_transaction_number ( gpointer transaction_pointer );
GSList *gsb_data_transaction_get_transactions_list ( void );
GSList *gsb_data_transaction_get_transactions_list_by_date ( void );
const GDate *gsb_data_transaction_get_value_date ( gint transaction_number );
const GDate *gsb_data_transaction_get_value_date_or_date ( gint transaction_number );
const gchar *gsb_data_transaction_get_voucher ( gint transaction_number );
gint gsb_data_transaction_get_white_line ( gint transaction_number );
gboolean gsb_data_transaction_init_variables ( void );
gint gsb_data_transaction_new_transaction ( gint no_account );
gint gsb_data_transaction_new_transaction_with_number ( gint no_account,
                        gint transaction_number );
gint gsb_data_transaction_new_white_line ( gint mother_transaction_number);
gboolean gsb_data_transaction_remove_transaction ( gint transaction_number );
gboolean gsb_data_transaction_remove_transaction_without_check ( gint transaction_number );
gboolean gsb_data_transaction_set_account_number ( gint transaction_number,
                        gint no_account );
gboolean gsb_data_transaction_set_amount ( gint transaction_number,
                        gsb_real amount );
gboolean gsb_data_transaction_set_archive_number ( gint transaction_number,
                        gint archive_number );
gboolean gsb_data_transaction_set_automatic_transaction ( gint transaction_number,
                        gint automatic_transaction );
gboolean gsb_data_transaction_set_bank_references ( gint transaction_number,
                        const gchar *bank_references );
gboolean gsb_data_transaction_set_budgetary_number ( gint transaction_number,
                        gint budgetary_number );
gboolean gsb_data_transaction_set_category_number ( gint transaction_number,
                        gint no_category );
gboolean gsb_data_transaction_set_change_between ( gint transaction_number,
                        gint value );
gboolean gsb_data_transaction_set_contra_transaction_number ( gint transaction_number,
                        gint transaction_number_transfer );
gboolean gsb_data_transaction_set_currency_number ( gint transaction_number,
                        gint no_currency );
gboolean gsb_data_transaction_set_date ( gint transaction_number,
                        const GDate *date );
gboolean gsb_data_transaction_set_exchange_fees ( gint transaction_number,
                        gsb_real rate );
gboolean gsb_data_transaction_set_exchange_rate ( gint transaction_number,
                        gsb_real rate );
gboolean gsb_data_transaction_set_financial_year_number ( gint transaction_number,
                        gint financial_year_number );
gboolean gsb_data_transaction_set_marked_transaction ( gint transaction_number,
                        gint marked_transaction );
gboolean gsb_data_transaction_set_method_of_payment_content ( gint transaction_number,
                        const gchar *method_of_payment_content );
gboolean gsb_data_transaction_set_method_of_payment_number ( gint transaction_number,
                        gint number );
gboolean gsb_data_transaction_set_mother_transaction_number ( gint transaction_number,
                        gint mother_transaction_number );
gboolean gsb_data_transaction_set_notes ( gint transaction_number,
                        const gchar *notes );
gboolean gsb_data_transaction_set_party_number ( gint transaction_number,
                        gint no_party );
gboolean gsb_data_transaction_set_reconcile_number ( gint transaction_number,
                        gint reconcile_number );
gboolean gsb_data_transaction_set_split_of_transaction ( gint transaction_number,
                        gint is_split );
gboolean gsb_data_transaction_set_sub_budgetary_number ( gint transaction_number,
                        gint sub_budgetary_number );
gboolean gsb_data_transaction_set_sub_category_number ( gint transaction_number,
                        gint no_sub_category );
gboolean gsb_data_transaction_set_transaction_id ( gint transaction_number,
                        const gchar *transaction_id );
gboolean gsb_data_transaction_set_value_date ( gint transaction_number,
                        const GDate *date );
gboolean gsb_data_transaction_set_voucher ( gint transaction_number,
                        const gchar *voucher );
/* END_DECLARATION */


#endif

