#ifndef _OPERATIONS_COMPTES_H
#define _OPERATIONS_COMPTES_H (1)
/* START_INCLUDE_H */
#include "operations_comptes.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void changement_no_compte_par_menu ( gpointer null,
				     gint no_account_plus_un );
gboolean gsb_account_list_gui_change_current_account ( gint *no_account );
void mise_a_jour_taille_formulaire ( gint largeur_formulaire );
/* END_DECLARATION */
#endif
