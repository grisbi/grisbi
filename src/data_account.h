#ifndef _DATA_ACCOUNT_H
#define _DATA_ACCOUNT_H (1)

#include "account_constants.h"

/** \struct
 * describe an account
 * */

typedef struct
{
    /** @name general stuff */
    gint account_number;
    gchar *account_id;                       /**< for ofx import, invisible for the user */
    kind_account account_kind;
    gchar *account_name;
    gint currency;
    gint closed_account;                     /**< if 1 => closed */
    gchar *comment;
    gchar *holder_name;
    gchar *holder_address;

    /** @name method of payment */
    GSList *method_payment_list;         /**< list of method of payment struct */
    gint default_debit;
    gint default_credit;

    /** @name showed list stuff */
    gint show_r;                      /**< 1 : reconciled transactions are showed */
    gint nb_rows_by_transaction;      /**< 1, 2, 3, 4  */
    gpointer account_button;        /**< the button in the list of accounts on the transactions page */
    gint update_list;                /**< 1 when the list need to be updated when showed */

    /** @name remaining of the balances */
    gdouble init_balance;
    gdouble mini_balance_wanted;
    gdouble mini_balance_authorized;
    gdouble current_balance;
    gdouble marked_balance;

    /** @name remaining of the minimun balance message */
    gint mini_balance_wanted_message;
    gint mini_balance_authorized_message;

    /** @name transactions list : contains a list of transactions structures */
    GSList *transactions_list;

    /** @name pointer of the transaction selectionned, or -1 for the white line */
    struct structure_operation *current_transaction;

    /** @name bank stuff */
    gint bank_number;
    gchar *bank_branch_code;
    gchar *bank_account_number;
    gchar *bank_account_key;

    /** @name reconcile stuff */
    GDate *reconcile_date;
    gdouble reconcile_balance;
    gint reconcile_last_number;

    /** @name reconcile sort */
    gint sort_type;                           /**< 1 : sort by method of payment ; 0 : sort by date */
    GSList *sort_list;                        /**< the method of payment sorted in a list */
    gint split_neutral_payment;               /**< if 1 : neutral payments are splitted into debits/credits */

    /** @name tree_view stuff */
    gpointer transactions_tree_view;
    gpointer transactions_scrolled_window;
    gpointer transactions_store;
    gdouble transactions_adjustment_value; 
    gpointer transactions_column[TRANSACTION_LIST_COL_NB];

    /** @name tree_view sort stuff */
    gint (*current_sort) ( struct structure_operation *operation_1,
				 struct structure_operation *operation_2 );      /**< current sort function */
    gint ascending_sort;          /**< GTK_SORT_DESCENDING / GTK_SORT_ASCENDING */
    gint sort_number;             /**< number of sort (TRANSACTION_LIST_...)*/
    gint sort_column;             /**< used to hide the arrow when change the column */

    /** @name stuff for filling the transactions list */
    GSList *last_transaction;              /**< the slist on the last transaction added or -1 if finished */
    gint finished_background_color;        /**< 1 when the background color is finish */
    gint finished_balance_showed;          /**< 1 when the transactions balance is written */
    gint finished_selection_transaction;   /**< 1 once the transaction is selectionned */

    /** @name struct of the form's organization */
    struct organisation_formulaire *form_organization;
} struct_account;


/* START_DECLARATION */
gdouble gsb_account_get_adjustment_value ( gint no_account );
gpointer gsb_account_get_column ( gint no_account,
				  gint no_column );
gdouble gsb_account_get_current_balance ( gint no_account );
GDate *gsb_account_get_current_reconcile_date ( gint no_account );
gpointer gsb_account_get_current_transaction ( gint no_account );
gchar *gsb_account_get_id ( gint no_account );
gdouble gsb_account_get_init_balance ( gint no_account );
kind_account gsb_account_get_kind ( gint no_account );
gdouble gsb_account_get_marked_balance ( gint no_account );
gdouble gsb_account_get_mini_balance_authorized ( gint no_account );
gboolean gsb_account_get_mini_balance_authorized_message ( gint no_account );
gdouble gsb_account_get_mini_balance_wanted ( gint no_account );
gboolean gsb_account_get_mini_balance_wanted_message ( gint no_account );
gchar *gsb_account_get_name ( gint no_account );
gint gsb_account_get_nb_rows ( gint no_account );
gboolean gsb_account_get_r ( gint no_account );
gdouble gsb_account_get_reconcile_balance ( gint no_account );
gint gsb_account_get_reconcile_last_number ( gint no_account );
gpointer gsb_account_get_scrolled_window ( gint no_account );
gpointer gsb_account_get_store ( gint no_account );
GSList *gsb_account_get_transactions_list ( gint no_account );
gpointer gsb_account_get_tree_view ( gint no_account );
gboolean gsb_account_set_adjustment_value ( gint no_account,
					    gdouble value );
gboolean gsb_account_set_column ( gint no_account,
				  gint no_column,
				  gpointer column );
gboolean gsb_account_set_current_balance ( gint no_account,
					   gdouble balance );
gboolean gsb_account_set_current_reconcile_date ( gint no_account,
						  GDate *date );
gboolean gsb_account_set_current_transaction ( gint no_account,
					       gpointer transaction );
gboolean gsb_account_set_id ( gint no_account,
			      gchar *id );
gboolean gsb_account_set_init_balance ( gint no_account,
					gdouble balance );
gboolean gsb_account_set_kind ( gint no_account,
				kind_account account_kind );
gboolean gsb_account_set_marked_balance ( gint no_account,
					  gdouble balance );
gboolean gsb_account_set_mini_balance_authorized ( gint no_account,
						   gdouble balance );
gboolean gsb_account_set_mini_balance_authorized_message ( gint no_account,
							   gboolean value );
gboolean gsb_account_set_mini_balance_wanted ( gint no_account,
					       gdouble balance );
gboolean gsb_account_set_mini_balance_wanted_message ( gint no_account,
						       gboolean value );
gboolean gsb_account_set_name ( gint no_account,
				gchar *name );
gboolean gsb_account_set_nb_rows ( gint no_account,
				   gint nb_rows );
gboolean gsb_account_set_r ( gint no_account,
			     gboolean show_r );
gboolean gsb_account_set_reconcile_balance ( gint no_account,
					     gdouble balance );
gboolean gsb_account_set_reconcile_last_number ( gint no_account,
						 gint number );
gboolean gsb_account_set_scrolled_window ( gint no_account,
					   gpointer scrolled_window );
gboolean gsb_account_set_store ( gint no_account,
				 gpointer store );
gboolean gsb_account_set_transactions_list ( gint no_account,
					     GSList *list );
gboolean gsb_account_set_tree_view ( gint no_account,
				     gpointer tree );
/* END_DECLARATION */


#endif
