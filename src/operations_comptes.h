GtkWidget *creation_liste_comptes (void);
GtkWidget *comptes_appel ( gint no_de_compte );
void changement_compte_par_menu ( GtkWidget *menu,
				  gint *compte );
gboolean changement_compte ( gint *compte);
void reaffiche_liste_comptes ( void );
