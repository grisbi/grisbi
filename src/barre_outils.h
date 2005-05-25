#ifndef _BARRE_OUTILS_H
#define _BARRE_OUTILS_H (1)
/* START_INCLUDE_H */
#include "barre_outils.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean change_aspect_liste ( gint demande );
GtkWidget *creation_barre_outils ( void );
GtkWidget *creation_barre_outils_echeancier ( void );
void mise_a_jour_boutons_caract_liste ( gint no_compte );
/* END_DECLARATION */
#endif
