GtkWidget *creation_liste_comptes (void);
GtkWidget *comptes_appel ( gint no_de_compte );
void changement_compte_par_menu ( gpointer null,
				  gint compte_plus_un );
gboolean changement_compte ( gint *compte);
void reaffiche_liste_comptes ( void );
gboolean changement_ordre_liste_comptes ( GtkWidget *bouton );

