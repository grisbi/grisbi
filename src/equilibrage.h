GtkWidget *creation_fenetre_equilibrage ( void );
void equilibrage ( void );
void sortie_entree_date_equilibrage ( GtkWidget *entree );
void modif_entree_solde_init_equilibrage ( void );
void modif_entree_solde_final_equilibrage ( void );
void annuler_equilibrage ( void );
void pointe_equilibrage ( int p_ligne );
void fin_equilibrage ( GtkWidget *bouton_ok,
		       gpointer data );
void calcule_total_pointe_compte ( gint no_compte );
GtkWidget * tab_display_reconciliation ( void );
void souris_equilibrage ( GtkWidget *entree,
			  GdkEventButton *event );
gboolean clavier_equilibrage ( GtkWidget *widget,
			       GdkEventKey *event );
gchar *rapprochement_name_by_no ( gint no_rapprochement );

