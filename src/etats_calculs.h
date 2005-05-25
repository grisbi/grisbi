#ifndef _ETATS_CALCULS_H
#define _ETATS_CALCULS_H (1)
/* START_INCLUDE_H */
#include "etats_calculs.h"
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void affichage_etat ( struct struct_etat *etat, 
		      struct struct_etat_affichage * affichage );
void denote_struct_sous_jaccentes ( gint origine );
void impression_etat_courant ( );
void rafraichissement_etat ( struct struct_etat *etat );
GSList *recupere_opes_etat ( struct struct_etat *etat );
/* END_DECLARATION */
#endif
