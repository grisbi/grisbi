#ifndef _ECHEANCIER_FORMULAIRE_H
#define _ECHEANCIER_FORMULAIRE_H (1)

/* Constantes qui dÃ©finisent le numÃ©ro de widget dans le formulaire
   de saisie des Ã©chÃ©ances */
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

/* START_INCLUDE_H */
#include "echeancier_formulaire.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *creation_formulaire_echeancier ( void );
void formulaire_echeancier_a_zero ( void );
gint gsb_scheduler_create_transaction_from_scheduled_transaction ( gint scheduled_number );
gboolean gsb_scheduler_form_set_sensitive ( gboolean breakdown_child );
gboolean gsb_scheduler_increase_scheduled_transaction ( gint scheduled_number );
void gsb_scheduler_validate_form ( void );
/* END_DECLARATION */
#endif
