#ifndef _ETATS_CALCULS_H
#define _ETATS_CALCULS_H (1)
/* START_INCLUDE_H */
#include "etats_calculs.h"
/* END_INCLUDE_H */

#include "etats_affiche.h"


/* START_DECLARATION */
void affichage_etat ( gint report_number, 
		      struct struct_etat_affichage * affichage );
void denote_struct_sous_jaccentes ( gint origine );
void impression_etat_courant ( );
void rafraichissement_etat ( gint report_number );
GSList *recupere_opes_etat ( gint report_number );
/* END_DECLARATION */
#endif
