/*START_DECLARATION*/
void changement_choix_compte_echeancier ( void );
GtkWidget * creation_option_menu_comptes ( GtkSignalFunc func, 
					   gboolean activate_currrent,
					   gboolean include_closed );
gint demande_type_nouveau_compte ( void );
gint initialisation_nouveau_compte ( gint type_de_compte );
void  nouveau_compte ( void );
void supprimer_compte ( void );
/*END_DECLARATION*/

