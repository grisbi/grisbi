/*START_DECLARATION*/
gchar *compte_name_by_no ( gint no_compte );
gint no_compte_by_name ( gchar *name );
gint recherche_compte_dans_option_menu ( GtkWidget *option_menu,
					 gint no_compte );
gint recupere_no_compte ( GtkWidget *option_menu );
void update_options_menus_comptes ( void );
/*END_DECLARATION*/

