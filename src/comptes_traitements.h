void nouveau_compte (void);
void supprimer_compte ( void );

void changement_choix_compte_echeancier ( void );
void update_options_menus_comptes ( void );

gint initialisation_nouveau_compte ( gint type_de_compte );
void creation_types_par_defaut ( gint no_compte, gulong dernier_cheque );
gint demande_type_nouveau_compte ( void );


GtkWidget * creation_option_menu_comptes ( GtkSignalFunc func, 
					   gboolean activate_currrent,
					   gboolean include_closed );
gint recherche_compte_dans_option_menu ( GtkWidget *option_menu,
					 gint no_compte );
gint recupere_no_compte ( GtkWidget *option_menu );

