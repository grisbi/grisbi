/*START_DECLARATION*/
gint cherche_no_menu_type ( gint demande );
gint cherche_no_menu_type_associe ( gint demande,
				    gint origine );
gint cherche_no_menu_type_echeancier ( gint demande );
GtkWidget *creation_menu_types ( gint demande,
				 gint compte,
				 gint origine );
GtkWidget *onglet_types_operations ( void );
gchar * automatic_numbering_get_new_number ( struct struct_type_ope * type );
/*END_DECLARATION*/
