#define START_DECLARATION
gint cherche_no_menu_type ( gint demande );
gint cherche_no_menu_type_associe ( gint demande,
				    gint origine );
gint cherche_no_menu_type_echeancier ( gint demande );
GtkWidget *creation_menu_types ( gint demande,
				 gint compte,
				 gint origine );
GtkWidget *onglet_types_operations ( void );
gchar *type_ope_name_by_no ( gint no_type_ope,
			     gint no_de_compte );
struct struct_type_ope *type_ope_par_no ( gint no_type_ope,
					  gint no_compte );
#define END_DECLARATION

