#ifndef _ECHEANCIER_INFOS_H
#define _ECHEANCIER_INFOS_H (1)


/* START_INCLUDE_H */
#include "././gsb_scheduler_list.h"
/* END_INCLUDE_H */

/* when a global variable contains a definition like here, need to put it with the definition,
 * else mk_include cannot find the .h file... */
enum periodicity_units affichage_echeances_perso_j_m_a;



/* START_DECLARATION */
GtkWidget *creation_partie_gauche_echeancier ( void );
void mise_a_jour_calendrier ( void );
/* END_DECLARATION */
#endif
