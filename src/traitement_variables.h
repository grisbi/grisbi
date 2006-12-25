#ifndef _TRAITEMENT_VARIABLES_H
#define _TRAITEMENT_VARIABLES_H (1)

/* initialisation des couleurs */

#define COULEUR1_RED  55000
#define COULEUR1_GREEN  55000
#define COULEUR1_BLUE  65535
#define COULEUR2_RED  65535
#define COULEUR2_GREEN  65535
#define COULEUR2_BLUE  65535
#define COULEUR_SELECTION_RED  63000
#define COULEUR_SELECTION_GREEN  40000
#define COULEUR_SELECTION_BLUE  40000
#define COULEUR_ROUGE_RED  65535
#define COULEUR_ROUGE_GREEN  0
#define COULEUR_ROUGE_BLUE  0
#define COULEUR_NOIRE_RED 0
#define COULEUR_NOIRE_GREEN 0
#define COULEUR_NOIRE_BLUE 0
#define COULEUR_GRISE_RED 50000
#define COULEUR_GRISE_GREEN 50000
#define COULEUR_GRISE_BLUE 50000


/* START_INCLUDE_H */
#include "structures.h"
#include "traitement_variables.h"
/* END_INCLUDE_H */


/*START_DECLARATION*/
void init_variables ( void );
void initialisation_couleurs_listes ( void );
void menus_sensitifs ( gboolean sensitif );
void modification_fichier ( gboolean modif );
/*END_DECLARATION*/


#endif

