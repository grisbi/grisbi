#ifndef _COMPTES_TRAITEMENTS_H
#define _COMPTES_TRAITEMENTS_H (1)
/* START_INCLUDE_H */
#include "comptes_traitements.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void changement_choix_compte_echeancier ( void );
GtkWidget * creation_option_menu_comptes ( GtkSignalFunc func, 
					   gboolean activate_currrent,
					   gboolean include_closed );
void creation_types_par_defaut ( gint no_compte,
				 gulong dernier_cheque );
gboolean delete_account ( void );
gint demande_type_nouveau_compte ( void );
gboolean gsb_account_create_name_tree_model ( GtkWidget *combo_box,
					      GCallback func, 
					      gboolean include_closed );
gboolean new_account ( void );
/* END_DECLARATION */
#endif
