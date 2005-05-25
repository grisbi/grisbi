#ifndef _ETATS_CONFIG_H
#define _ETATS_CONFIG_H (1)

#define TEXT_NORMAL	(0<<0)
#define TEXT_BOLD	(1<<0)
#define TEXT_ITALIC	(1<<1)
#define TEXT_HUGE	(1<<2)
#define TEXT_LARGE	(1<<3)
#define TEXT_SMALL	(1<<4)


/* START_INCLUDE_H */
#include "etats_config.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void personnalisation_etat (void);
void remplissage_liste_categ_etats ( void );
void remplissage_liste_comptes_etats ( void );
void remplissage_liste_ib_etats ( void );
void remplissage_liste_tiers_etats ( void );
void selectionne_devise_categ_etat_courant ( void );
void selectionne_devise_ib_etat_courant ( void );
void selectionne_devise_tiers_etat_courant ( void );
void selectionne_liste_categ_etat_courant ( void );
void selectionne_liste_comptes_etat_courant ( void );
void selectionne_liste_tiers_etat_courant ( void );
/* END_DECLARATION */
#endif
