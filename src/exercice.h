/*START_DECLARATION*/
void affiche_exercice_par_date ( GtkWidget *entree_date,
				 GtkWidget *option_menu_exercice );
gint cherche_no_menu_exercice ( gint no_demande,
				GtkWidget *option_menu );
GtkWidget *creation_menu_exercices ( gint origine );
gchar *exercice_name_by_no ( gint no_exo );
struct struct_exercice *exercice_par_no ( gint no_exo );
struct struct_exercice *exercice_par_nom ( gchar *nom_exo );
GtkWidget *onglet_exercices ( void );
gint recherche_exo_correspondant ( GDate *date );
/*END_DECLARATION*/

