#ifndef _GSB_ACCOUNT_H
#define _GSB_ACCOUNT_H (1)


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */


/** \struct
 * kind of the account
 * */

enum _kind_account
{
    GSB_TYPE_BANK        = 0,
    GSB_TYPE_CASH        = 1,
    GSB_TYPE_LIABILITIES = 2,
    GSB_TYPE_ASSET       = 3
};
typedef enum _kind_account	kind_account;


/* START_DECLARATION */
gboolean gsb_data_account_bet_update_initial_date_if_necessary ( gint account_number );
gsb_real gsb_data_account_calculate_current_and_marked_balances ( gint account_number );
gsb_real gsb_data_account_calculate_current_day_balance ( gint account_number,
                        GDate *day );
gsb_real gsb_data_account_calculate_waiting_marked_balance ( gint account_number );
void gsb_data_account_change_account_icon ( GtkWidget *button, gpointer data );
void gsb_data_account_colorize_current_balance ( gint account_number );
gint gsb_data_account_compare_position ( gint account_number_1,
                        gint account_number_2 );
gboolean gsb_data_account_delete ( gint account_number );
gint gsb_data_account_first_number ( void );
gint gsb_data_account_get_account_by_id ( const gchar *account_id );
GtkWidget *gsb_data_account_get_account_icon_image ( gint account_number );
GdkPixbuf *gsb_data_account_get_account_icon_pixbuf ( gint account_number );
GdkPixbuf *gsb_data_account_get_account_standard_pixbuf ( kind_account account_kind );
gint gsb_data_account_get_accounts_amount ( void );
gint gsb_data_account_get_bank ( gint account_number );
gchar *gsb_data_account_get_bank_account_iban (gint account_number);
gchar *gsb_data_account_get_bank_account_key ( gint account_number );
gchar *gsb_data_account_get_bank_account_number ( gint account_number );
gchar *gsb_data_account_get_bank_branch_code ( gint account_number );
gboolean gsb_data_account_get_bet_auto_inc_month ( gint account_number );
gdouble gsb_data_account_get_bet_finance_capital ( gint account_number );
gdouble gsb_data_account_get_bet_finance_frais ( gint account_number );
gdouble gsb_data_account_get_bet_finance_taux_annuel ( gint account_number );
gint gsb_data_account_get_bet_finance_type_taux ( gint account_number );
gint gsb_data_account_get_bet_hist_data ( gint account_number );
gint gsb_data_account_get_bet_hist_fyear ( gint account_number );
gint gsb_data_account_get_bet_maj ( gint account_number );
gint gsb_data_account_get_bet_months ( gint account_number );
gint gsb_data_account_get_bet_select_label ( gint account_number, gint origine );
gint gsb_data_account_get_bet_spin_range ( gint account_number );
GDate *gsb_data_account_get_bet_start_date ( gint account_number );
gint gsb_data_account_get_bet_use_budget ( gint account_number );
gint gsb_data_account_get_closed_account ( gint account_number );
gchar *gsb_data_account_get_comment ( gint account_number );
gint gsb_data_account_get_currency ( gint account_number );
gint gsb_data_account_get_currency_floating_point ( gint account_number );
gsb_real gsb_data_account_get_current_balance ( gint account_number );
gint gsb_data_account_get_current_transaction_number ( gint account_number );
gint gsb_data_account_get_default_credit ( gint account_number );
gint gsb_data_account_get_default_debit ( gint account_number );
gint gsb_data_account_get_element_sort ( gint account_number,
                        gint no_column );
gpointer gsb_data_account_get_form_organization ( gint account_number );
gchar *gsb_data_account_get_holder_address ( gint account_number );
gchar *gsb_data_account_get_holder_name ( gint account_number );
gchar *gsb_data_account_get_id ( gint account_number );
gsb_real gsb_data_account_get_init_balance ( gint account_number,
                        gint floating_point );
kind_account gsb_data_account_get_kind ( gint account_number );
gboolean gsb_data_account_get_l ( gint account_number );
GSList *gsb_data_account_get_list_accounts ( void );
gsb_real gsb_data_account_get_marked_balance ( gint account_number );
gsb_real gsb_data_account_get_mini_balance_authorized ( gint account_number );
gboolean gsb_data_account_get_mini_balance_authorized_message ( gint account_number );
gsb_real gsb_data_account_get_mini_balance_wanted ( gint account_number );
gboolean gsb_data_account_get_mini_balance_wanted_message ( gint account_number );
gchar *gsb_data_account_get_name ( gint account_number );
gchar *gsb_data_account_get_name_icon (gint account_number);
gint gsb_data_account_get_nb_rows ( gint account_number );
gint gsb_data_account_get_no_account ( gpointer account_ptr );
gint gsb_data_account_get_no_account_by_name ( const gchar *account_name );
gboolean gsb_data_account_get_r ( gint account_number );
gint gsb_data_account_get_reconcile_sort_type ( gint account_number );
gfloat gsb_data_account_get_row_align ( gint account_number );
gint gsb_data_account_get_sort_column ( gint account_number );
GSList *gsb_data_account_get_sort_list ( gint account_number );
gint gsb_data_account_get_sort_type ( gint account_number );
gint gsb_data_account_get_split_neutral_payment ( gint account_number );
gboolean gsb_data_account_init_variables ( void );
gboolean gsb_data_account_move_account ( gint account_number,
                        gint dest_account_number );
gint gsb_data_account_new ( kind_account account_kind );
gboolean gsb_data_account_reorder ( GSList *new_order );
gboolean gsb_data_account_set_account_icon_pixbuf ( gint account_number,
                        GdkPixbuf * pixbuf );
gint gsb_data_account_set_account_number ( gint account_number,
                        gint new_no );
gboolean gsb_data_account_set_balances_are_dirty ( gint account_number );
gboolean gsb_data_account_set_bank ( gint account_number,
                        gint bank );
gboolean gsb_data_account_set_bank_account_iban ( gint account_number, const gchar *iban );
gboolean gsb_data_account_set_bank_account_key ( gint account_number,
                        const gchar *bank_account_key );
gboolean gsb_data_account_set_bank_account_number ( gint account_number,
                        const gchar *bank_account_number );
gboolean gsb_data_account_set_bank_branch_code ( gint account_number,
                        const gchar *bank_branch_code );
gboolean gsb_data_account_set_bet_auto_inc_month ( gint account_number,
                        gboolean auto_inc_month );
gboolean gsb_data_account_set_bet_finance_capital ( gint account_number, gdouble capital );
gboolean gsb_data_account_set_bet_finance_frais ( gint account_number, gdouble frais );
gboolean gsb_data_account_set_bet_finance_taux_annuel ( gint account_number, gdouble taux_annuel );
gboolean gsb_data_account_set_bet_finance_type_taux ( gint account_number, gint type_taux );
gboolean gsb_data_account_set_bet_hist_data ( gint account_number, gint hist_data );
gboolean gsb_data_account_set_bet_hist_fyear ( gint account_number, gint hist_fyear );
gboolean gsb_data_account_set_bet_maj ( gint account_number, gint type_maj );
gboolean gsb_data_account_set_bet_months ( gint account_number, gint months );
gboolean gsb_data_account_set_bet_select_label ( gint account_number,
                        gint origine,
                        gint type );
gboolean gsb_data_account_set_bet_spin_range ( gint account_number, gint spin_range );
gboolean gsb_data_account_set_bet_start_date ( gint account_number, const GDate *date );
gboolean gsb_data_account_set_bet_use_budget ( gint account_number, gint value );
gboolean gsb_data_account_set_closed_account ( gint account_number,
                        gint closed_account );
gboolean gsb_data_account_set_comment ( gint account_number,
                        const gchar *comment );
gboolean gsb_data_account_set_currency ( gint account_number,
                        gint currency );
gboolean gsb_data_account_set_current_transaction_number ( gint account_number,
                        gint transaction_number );
gboolean gsb_data_account_set_default_credit ( gint account_number,
                        gint default_credit );
gboolean gsb_data_account_set_default_debit ( gint account_number,
                        gint default_debit );
gboolean gsb_data_account_set_element_sort ( gint account_number,
                        gint no_column,
                        gint element_number );
gboolean gsb_data_account_set_form_organization ( gint account_number,
                        gpointer form_organization );
gboolean gsb_data_account_set_holder_address ( gint account_number,
                        const gchar *holder_address );
gboolean gsb_data_account_set_holder_name ( gint account_number,
                        const gchar *holder_name );
gboolean gsb_data_account_set_id ( gint account_number,
                        const gchar *id );
gboolean gsb_data_account_set_init_balance ( gint account_number,
                        gsb_real balance );
gboolean gsb_data_account_set_kind ( gint account_number,
                        kind_account account_kind );
gboolean gsb_data_account_set_l ( gint account_number,
                        gboolean show_l );
gboolean gsb_data_account_set_mini_balance_authorized ( gint account_number,
                        gsb_real balance );
gboolean gsb_data_account_set_mini_balance_authorized_message ( gint account_number,
                        gboolean value );
gboolean gsb_data_account_set_mini_balance_wanted ( gint account_number,
                        gsb_real balance );
gboolean gsb_data_account_set_mini_balance_wanted_message ( gint account_number,
                        gboolean value );
gboolean gsb_data_account_set_name ( gint account_number,
                        const gchar *name );
gboolean gsb_data_account_set_name_icon ( gint account_number,
                        const gchar *filename );
gboolean gsb_data_account_set_nb_rows ( gint account_number,
                        gint nb_rows );
gboolean gsb_data_account_set_r ( gint account_number,
                        gboolean show_r );
gboolean gsb_data_account_set_reconcile_sort_type ( gint account_number,
                        gint sort_type );
gboolean gsb_data_account_set_row_align ( gint account_number,
                        gfloat row_align );
gboolean gsb_data_account_set_sort_column ( gint account_number,
                        gint sort_column );
gboolean gsb_data_account_set_sort_list ( gint account_number,
                        GSList *list );
gboolean gsb_data_account_set_sort_type ( gint account_number,
                        gint sort_type );
gboolean gsb_data_account_set_split_neutral_payment ( gint account_number,
                        gint split_neutral_payment );
gboolean gsb_data_account_sort_list_add ( gint account_number,
                        gint payment_number );
gboolean gsb_data_account_sort_list_free ( gint account_number );
gboolean gsb_data_account_sort_list_remove ( gint account_number,
                        gint payment_number );
/* END_DECLARATION */


#endif
