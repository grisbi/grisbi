/* define the no of column in the transactions list */

#define TRANSACTION_COL_NB_IS_EXPANDER 0
#define TRANSACTION_COL_NB_CHECK 1
#define TRANSACTION_COL_NB_DATE 2
#define TRANSACTION_COL_NB_PARTY 3
#define TRANSACTION_COL_NB_PR 4
#define TRANSACTION_COL_NB_DEBIT 5
#define TRANSACTION_COL_NB_CREDIT 6
#define TRANSACTION_COL_NB_BALANCE 7

#define TRANSACTION_LIST_COL_NB 8	 /*< must be the last of the list */

/* define the columns in the store
 * as the data are filled above, the number here
 * begin at TRANSACTION_LIST_COL_NB */

#define TRANSACTION_COL_NB_BACKGROUND 8		/*< color of the background */
#define TRANSACTION_COL_NB_AMOUNT_COLOR 9	/*< color of the amount */
#define TRANSACTION_COL_NB_TRANSACTION_ADDRESS 10
#define TRANSACTION_COL_NB_SAVE_BACKGROUND 11	/*< when selection, save of the normal color of background */
#define TRANSACTION_COL_NB_FONT 12		/*< PangoFontDescription if used */
#define TRANSACTION_COL_NB_TRANSACTION_LINE 13	/*< the line in the transaction (1, 2, 3 or 4) */
#define TRANSACTION_COL_NB_VISIBLE 14		/*< whether that transaction is visible or not */
#define TRANSACTION_COL_NB_PIXBUF_CLOSED 15
#define TRANSACTION_COL_NB_PIXBUF_OPEN 16
#define TRANSACTION_COL_NB_IS_EXPANDED 17
#define TRANSACTION_COL_NB_IS_NOT_BREAKDOWN 18

#define TRANSACTION_COL_NB_TOTAL 19


#define TRANSACTION_LIST_ROWS_NB 4

/* constantes qui dÃ©finissent le no associÃ© au paramÃštre
 * dans la liste d'opÃ© */
# define TRANSACTION_LIST_DATE 1
# define TRANSACTION_LIST_VALUE_DATE 2
# define TRANSACTION_LIST_PARTY 3
# define TRANSACTION_LIST_BUDGET 4
# define TRANSACTION_LIST_CREDIT 5
# define TRANSACTION_LIST_DEBIT 6
# define TRANSACTION_LIST_BALANCE 7
# define TRANSACTION_LIST_AMOUNT 8
# define TRANSACTION_LIST_TYPE 9
# define TRANSACTION_LIST_RECONCILE_NB 10
# define TRANSACTION_LIST_EXERCICE 11
# define TRANSACTION_LIST_CATEGORY 12
# define TRANSACTION_LIST_MARK 13
# define TRANSACTION_LIST_VOUCHER 14 
# define TRANSACTION_LIST_NOTES 15
# define TRANSACTION_LIST_BANK 16
# define TRANSACTION_LIST_NO 17
# define TRANSACTION_LIST_CHQ 18

/* Constantes pour la fonction gtk_tree_view_column_set_alignment() (GTK2) */
#define COLUMN_LEFT 0.0
#define COLUMN_CENTER 0.5
#define COLUMN_RIGHT 1.0


