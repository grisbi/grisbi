void nouveau_compte (void);
gint initialisation_nouveau_compte ( gint type_de_compte );
void supprimer_compte ( void );
void compte_choisi_destruction ( GtkWidget *item_list, GdkEvent *evenement, 
				 GtkWidget *liste_comptes);
GtkWidget * creation_option_menu_comptes ( GtkSignalFunc func, 
					   gboolean activate_currrent,
					   gboolean include_closed );
void changement_choix_compte_echeancier ( void );
void creation_types_par_defaut ( gint no_compte, gulong dernier_cheque );
gint demande_type_nouveau_compte ( void );
gint recherche_compte_dans_option_menu ( GtkWidget *option_menu,
					 gint no_compte );
void update_options_menus_comptes ( void );

