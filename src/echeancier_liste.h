GtkWidget *creation_partie_gauche_echeancier ( void );
GtkWidget *creation_liste_echeances ( void );
gboolean traitement_clavier_liste_echeances ( GtkCList *liste_echeances,
					      GdkEventKey *evenement,
					      gpointer null );
void click_sur_saisir_echeance ( void );
void remplissage_liste_echeance ( void );
gboolean click_ligne_echeance ( GtkCList *liste, GdkEventButton *evenement, gpointer data );
void selectionne_echeance ( void );
void edition_echeance ( void );
void supprime_echeance ( struct operation_echeance *echeance );
void changement_taille_liste_echeances ( GtkWidget *clist,
					 GtkAllocation *allocation,
					 gpointer null );
void mise_a_jour_calendrier ( void );
void click_sur_jour_calendrier_echeance ( GtkWidget *calendrier,
					  gpointer null );
void verification_echeances_a_terme ( void );
void verifie_ligne_selectionnee_echeance_visible ( void );
gboolean modification_affichage_echeances ( gint *origine, GtkWidget * widget );
GDate *date_suivante_echeance ( struct operation_echeance *echeance,
			      GDate *date_courante );
gint classement_liste_echeances ( GtkWidget *liste,
				  GtkCListRow *ligne_1,
				  GtkCListRow *ligne_2 );
