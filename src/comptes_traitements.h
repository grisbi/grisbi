/*START_DECLARATION*/
void changement_choix_compte_echeancier ( void );
gchar *compte_name_by_no ( gint no_compte );
GtkWidget * creation_option_menu_comptes ( GtkSignalFunc func, 
					   gboolean activate_currrent,
					   gboolean include_closed );
gint demande_type_nouveau_compte ( void );
gint initialisation_nouveau_compte ( gint type_de_compte );
void  nouveau_compte ( void );
gint recherche_compte_dans_option_menu ( GtkWidget *option_menu,
					 gint no_compte );
gint recupere_no_compte ( GtkWidget *option_menu );
void supprimer_compte ( void );
void update_options_menus_comptes ( void );
/*END_DECLARATION*/

