struct struct_banque * ajout_banque ( GtkWidget *bouton, GtkWidget *clist );
void applique_modif_banque ( GtkWidget *liste );
void annuler_modif_banque ( GtkWidget *bouton, GtkWidget *liste );
void supprime_banque ( GtkWidget *bouton, GtkWidget *liste );
GtkWidget *creation_menu_banques ( void );
void affiche_detail_banque ( GtkWidget *bouton, gpointer null );
GtkWidget *onglet_banques ( void );
void selection_ligne_banque ( GtkWidget *liste, gint ligne, gint colonne,
			      GdkEventButton *ev, GtkWidget *frame );
void deselection_ligne_banque ( GtkWidget *liste, gint ligne, gint colonne,
				GdkEventButton *ev, GtkWidget *frame );
void modif_detail_banque ( GtkWidget *entree, gpointer null );

GtkWidget * bank_form ( GtkWidget * parent );
void update_bank_form ( struct struct_banque * bank, GtkWidget * frame );
void view_bank ( GtkWidget * button, gpointer data );
void edit_bank ( GtkWidget * button, struct struct_banque * bank );
gboolean update_bank_menu ();

