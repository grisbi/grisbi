GtkWidget *creation_onglet_etats ( void );
GtkWidget *creation_liste_etats ( void );
GtkWidget *creation_barre_boutons_etats ( void );
void remplissage_liste_etats ( void );
gboolean ajout_etat ( void );
void change_choix_nouvel_etat ( GtkWidget *menu_item,
				GtkWidget *label_description );
void efface_etat ( void );
void changement_etat ( GtkWidget *bouton,
		       struct struct_etat *etat );
void exporter_etat ( void );
void importer_etat ( void );
void dupliquer_etat ( void );
