void nouveau_compte (void);
gint initialisation_nouveau_compte ( gint type_de_compte );
void supprimer_compte ( void );
void compte_choisi_destruction ( GtkWidget *item_list,
				   GdkEvent *evenement,
				   GtkWidget *liste_comptes);
gint cherche_compte_dans_echeances ( struct operation_echeance *echeance,
				     gint no_compte );
GtkWidget *creation_option_menu_comptes ( GtkSignalFunc func );
void changement_choix_compte_echeancier ( void );
void creation_types_par_defaut ( gint no_compte,
				 gulong dernier_cheque );
gint demande_type_nouveau_compte ( void );
