GtkWidget *creation_onglet_accueil ( void );
void change_temps ( GtkWidget *label_temps );
gboolean saisie_echeance_accueil ( GtkWidget *event_box,
			       GdkEventButton *event,
			       struct operation_echeance *echeance );
void update_liste_comptes_accueil ( void );
void update_liste_echeances_manuelles_accueil ( void );
void update_liste_echeances_auto_accueil ( void );
void mise_a_jour_soldes_minimaux ( void );
void mise_a_jour_fin_comptes_passifs ( void );
gboolean select_expired_scheduled_transaction ( GtkWidget * event_box, GdkEventButton *event,
						struct structure_operation * operation );

