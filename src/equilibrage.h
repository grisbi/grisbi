GtkWidget *creation_fenetre_equilibrage ( void );
void equilibrage ( void );
void sortie_entree_date_equilibrage ( GtkWidget *entree );
void modif_entree_solde_init_equilibrage ( void );
void modif_entree_solde_final_equilibrage ( void );
void annuler_equilibrage ( GtkWidget *bouton_ann,
			   gpointer data);
void pointe_equilibrage ( int p_ligne );
void fin_equilibrage ( GtkWidget *bouton_ok,
		       gpointer data );
gint recherche_no_rapprochement_par_nom ( struct struct_no_rapprochement *rapprochement,
					  gchar *no_rap );
gint recherche_no_rapprochement_par_no ( struct struct_no_rapprochement *rapprochement,
					 gint *no_rap );
void calcule_total_pointe_compte ( gint no_compte );
GtkWidget * tab_display_reconciliation ( void );
void souris_equilibrage ( GtkWidget *entree,
			  GdkEventButton *event );
gboolean clavier_equilibrage ( GtkWidget *widget,
			       GdkEventKey *event );

