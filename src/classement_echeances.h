#ifndef _CLASSEMENT_ECHEANCES_H
#define _CLASSEMENT_ECHEANCES_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gint classement_sliste_echeance_par_date ( gint scheduled_number_1, 
                        gint scheduled_number_2,
                        gpointer manual );
gint classement_sliste_transactions_par_date ( gpointer transaction_pointer_1, 
                        gpointer transaction_pointer_2 );
gint classement_sliste_transactions_par_date_decroissante ( gpointer transaction_pointer_1, 
                        gpointer transaction_pointer_2 );
/* END_DECLARATION */
#endif
