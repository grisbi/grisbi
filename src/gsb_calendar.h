#ifndef _ECHEANCIER_INFOS_H
#define _ECHEANCIER_INFOS_H (1)


/* START_INCLUDE_H */
#include "gsb_scheduler_list.h"
/* END_INCLUDE_H */

/* when a global variable contains a definition like here, need to put it with the definition,
 * else mk_include cannot find the .h file...
 * Real declaration is in gsb_calendar.c */
extern enum periodicity_units affichage_echeances_perso_j_m_a;



/* START_DECLARATION */
GtkWidget *gsb_calendar_new ( void );
gboolean gsb_calendar_update ( void );
/* END_DECLARATION */
#endif
