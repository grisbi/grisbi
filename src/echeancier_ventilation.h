#ifndef _ECHEANCIER_VENTILATION_H
#define _ECHEANCIER_VENTILATION_H (1)


/* Constantes qui dÃ©finisent le numÃ©ro de widget dans le formulaire
   de ventilation des opÃ©rations planifiÃ©es */
# define SCHEDULER_BREAKDOWN_FORM_CATEGORY 0
# define SCHEDULER_BREAKDOWN_FORM_NOTES 1
# define SCHEDULER_BREAKDOWN_FORM_DEBIT 2
# define SCHEDULER_BREAKDOWN_FORM_CREDIT 3
# define SCHEDULER_BREAKDOWN_FORM_BUDGETARY 4
# define SCHEDULER_BREAKDOWN_FORM_CONTRA 5
# define SCHEDULER_BREAKDOWN_FORM_EXERCICE 6
# define SCHEDULER_BREAKDOWN_FORM_VOUCHER 7
# define SCHEDULER_BREAKDOWN_FORM_TOTAL_WIDGET 8	 /* must be the last of the list */

/* START_INCLUDE_H */
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *creation_fenetre_ventilation_echeances ( void );
GtkWidget *creation_formulaire_ventilation_echeances ( void );
GSList *creation_liste_ope_de_ventil_echeances ( struct operation_echeance *operation );
void validation_ope_de_ventilation_echeances ( struct operation_echeance *operation );
void ventiler_operation_echeances ( gdouble montant );
/* END_DECLARATION */
#endif
