/*START_DECLARATION*/
gint cherche_no_menu_exercice ( gint no_demande,
				GtkWidget *option_menu );
gchar *exercice_name_by_no ( gint no_exo );
struct struct_exercice *exercice_par_no ( gint no_exo );
struct struct_exercice *exercice_par_nom ( gchar *nom_exo );
gint recherche_exo_correspondant ( GDate *date );
/*END_DECLARATION*/

