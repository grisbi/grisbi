#include "account_constants.h"

/** \struct
 * describe an account
 * */

struct struct_account
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
    GtkWidget *account_button;        /**< the button in the list of accounts on the transactions page */
    gint update_list;                /**< 1 when the list need to be updated when showed */

    /** @name remaining of the balances */
    gdouble init_balance;
    gdouble mini_balance_wanted;
    gdouble mini_balance_authorized;
    gdouble current_balance;
    gdouble marked_balance;

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
    GtkWidget *transactions_tree_view;
    GtkWidget *transactions_scrolled_window;
    GtkListStore *transactions_store;
    gdouble transactions_adjustment_value; 
    GtkTreeViewColumn *transactions_column[TRANSACTION_LIST_COL_NB];

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
};


/* START_DECLARATION */
gdouble gsb_account_get_current_balance ( gint no_account );
gchar *gsb_account_get_id ( gint no_account );
gdouble gsb_account_get_init_balance ( gint no_account );
kind_account gsb_account_get_kind ( gint no_account );
gdouble gsb_account_get_marked_balance ( gint no_account );
gdouble gsb_account_get_mini_balance_authorized ( gint no_account );
gdouble gsb_account_get_mini_balance_wanted ( gint no_account );
gchar *gsb_account_get_name ( gint no_account );
gint gsb_account_get_nb_rows ( gint no_account );
gboolean gsb_account_get_r ( gint no_account );
gboolean gsb_account_set_current_balance ( gint no_account,
					   gdouble balance );
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
gboolean gsb_account_set_mini_balance_wanted ( gint no_account,
					       gdouble balance );
gboolean gsb_account_set_name ( gint no_account,
				gchar *name );
gboolean gsb_account_set_nb_rows ( gint no_account,
				   gint nb_rows );
gboolean gsb_account_set_r ( gint no_account,
			     gboolean show_r );
/* END_DECLARATION */
