GtkWidget *creation_fenetre_equilibrage ( void );
void equilibrage ( void );
gboolean sortie_entree_date_equilibrage ( GtkWidget *entree );
gboolean modif_entree_solde_init_equilibrage ( void );
gboolean modif_entree_solde_final_equilibrage ( void );
gboolean annuler_equilibrage ( GtkWidget *bouton_ann,
			       gpointer data);
void pointe_equilibrage ( int p_ligne );
gboolean fin_equilibrage ( GtkWidget *bouton_ok,
			   gpointer data );
void calcule_total_pointe_compte ( gint no_compte );
GtkWidget * tab_display_reconciliation ( void );
gboolean souris_equilibrage ( GtkWidget *entree,
			      GdkEventButton *event );
gboolean clavier_equilibrage ( GtkWidget *widget,
			       GdkEventKey *event );

