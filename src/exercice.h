GtkWidget *onglet_exercices ( void );
gboolean update_financial_year_menus ();
void ajout_exercice ( GtkWidget *bouton,
		      GtkWidget *clist );
void supprime_exercice ( GtkWidget *bouton,
			 GtkWidget *liste );
void selection_ligne_exercice ( GtkWidget *liste,
			      gint ligne,
			      gint colonne,
			      GdkEventButton *ev);
void deselection_ligne_exercice ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev);
GtkWidget *creation_menu_exercices ( gint origine );
gint cherche_no_menu_exercice ( gint no_demande,
				GtkWidget *option_menu );
void affiche_exercice_par_date ( GtkWidget *entree_date,
				 GtkWidget *option_menu_exercice );
void association_automatique ( void );
gint recherche_exo_correspondant ( GDate *date );

struct struct_exercice *exercice_par_no ( gint no_exo );
struct struct_exercice *exercice_par_nom ( gchar *nom_exo );
gchar *exercice_name_by_no ( gint no_exo );

