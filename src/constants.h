/* Fichier de définition de constantes constant.h */

/* Constantes qui définisent le numéro de colonne dans la liste des opérations
   dans l'onglet opérations */
#define TRANSACTION_COL_NB_CHECK 0
#define TRANSACTION_COL_NB_DATE 1
#define TRANSACTION_COL_NB_PARTY 2
#define TRANSACTION_COL_NB_PR 3
#define TRANSACTION_COL_NB_DEBIT 4
#define TRANSACTION_COL_NB_CREDIT 5
#define TRANSACTION_COL_NB_BALANCE 6
#define TRANSACTION_LIST_COL_NB 7	 /* must be the last of the list, defini aussi dans structures.h */

#define TRANSACTION_LIST_ROWS_NB 4

/* Constantes qui définisent le numéro de widget dans le formulaire
   de saisie des opérations */
# define TRANSACTION_FORM_OP_NB 0
# define TRANSACTION_FORM_DATE 1
# define TRANSACTION_FORM_PARTY 2
# define TRANSACTION_FORM_DEBIT 3
# define TRANSACTION_FORM_CREDIT 4
# define TRANSACTION_FORM_DEVISE 5
# define TRANSACTION_FORM_CHANGE 6
# define TRANSACTION_FORM_VALUE_DATE 7
# define TRANSACTION_FORM_CATEGORY 8
# define TRANSACTION_FORM_TYPE 9
# define TRANSACTION_FORM_CHEQUE 10
# define TRANSACTION_FORM_EXERCICE 11
# define TRANSACTION_FORM_BUDGET 12
# define TRANSACTION_FORM_CONTRA 13
# define TRANSACTION_FORM_VOUCHER 14
# define TRANSACTION_FORM_BREAKDOWN 15
# define TRANSACTION_FORM_NOTES 16
# define TRANSACTION_FORM_BANK 17
# define TRANSACTION_FORM_MODE 18
# define TRANSACTION_FORM_WIDGET_NB 19	 /* must be the last of the list */

/* Constantes qui définisent le numéro de widget dans le formulaire
   de saisie des échéances */
# define SCHEDULER_FORM_DATE 0
# define SCHEDULER_FORM_PARTY 1
# define SCHEDULER_FORM_DEBIT 2
# define SCHEDULER_FORM_CREDIT 3
# define SCHEDULER_FORM_DEVISE 4
# define SCHEDULER_FORM_ACCOUNT 5
# define SCHEDULER_FORM_CATEGORY 6
# define SCHEDULER_FORM_TYPE 7
# define SCHEDULER_FORM_CHEQUE 8
# define SCHEDULER_FORM_EXERCICE 9
# define SCHEDULER_FORM_BUDGETARY 10
# define SCHEDULER_FORM_BREAKDOWN 11
# define SCHEDULER_FORM_VOUCHER 12
# define SCHEDULER_FORM_MODE 13
# define SCHEDULER_FORM_NOTES 14
# define SCHEDULER_FORM_FREQUENCY 15
# define SCHEDULER_FORM_FINAL_DATE 16
# define SCHEDULER_FORM_FREQ_CUSTOM_NB 17
# define SCHEDULER_FORM_FREQ_CUSTOM_MENU 18
# define SCHEDULER_FORM_TOTAL_WIDGET 19	 /* must be the last of the list */

/* Constantes pour style_entree_formulaire[] */
# define ENCLAIR 0
# define ENGRIS 1

/* Constantes pour la fonction gtk_misc_set_alignment() (GTK1) */
#define MISC_TOP 0
#define MISC_VERT_CENTER 0.5
#define MISC_BOTTOM 1
#define MISC_LEFT 0
#define MISC_HORIZ_CENTER 0.5
#define MISC_RIGHT 1

/* Constantes pour la fonction gtk_tree_view_column_set_alignment() (GTK2) */
#define COLUMN_LEFT 0.0
#define COLUMN_CENTER 0.5
#define COLUMN_RIGHT 1.0
