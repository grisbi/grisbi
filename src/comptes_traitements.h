#include "account_constants.h"

/*START_DECLARATION*/
void changement_choix_compte_echeancier ( void );
GtkWidget * creation_option_menu_comptes ( GtkSignalFunc func, 
					   gboolean activate_currrent,
					   gboolean include_closed );
void creation_types_par_defaut ( gint no_compte,
				 gulong dernier_cheque );
kind_account demande_type_nouveau_compte ( void );
gint initialisation_nouveau_compte ( kind_account type_de_compte );
void  nouveau_compte ( void );
void supprimer_compte ( void );
/*END_DECLARATION*/

