#include "account_constants.h"

/*START_DECLARATION*/
void changement_choix_compte_echeancier ( void );
GtkWidget * creation_option_menu_comptes ( GtkSignalFunc func, 
					   gboolean activate_currrent,
					   gboolean include_closed );
void creation_types_par_defaut ( gint no_compte,
				 gulong dernier_cheque );
gboolean delete_account ( void );
kind_account demande_type_nouveau_compte ( void );
gboolean new_account ( void );
/*END_DECLARATION*/

