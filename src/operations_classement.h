gint classement_liste_par_date ( GtkWidget *liste,
				 GtkCListRow *ligne_1,
				 GtkCListRow *ligne_2 );
gint classement_liste_par_no_ope ( GtkWidget *liste,
				   GtkCListRow *ligne_1,
				   GtkCListRow *ligne_2 );
gint classement_liste_par_no_ope_ventil ( GtkWidget *liste,
					  GtkCListRow *ligne_1,
					  GtkCListRow *ligne_2 );
gint classement_liste_par_tri_courant ( GtkWidget *liste,
					GtkCListRow *ligne_1,
					GtkCListRow *ligne_2 );
gint classement_sliste ( struct structure_operation *operation_1,
			 struct structure_operation *operation_2 );
gint classement_sliste_par_date ( struct structure_operation *operation_1,
				  struct structure_operation *operation_2 );
gint classement_sliste_par_tri_courant ( struct structure_operation *operation_1,
					 struct structure_operation *operation_2 );
gint classe_liste_alphabetique ( gchar *string_1,
				 gchar *string_2 );
gint classement_sliste_par_date_rp ( struct structure_operation *pTransaction1,
				     struct structure_operation *pTransaction2 );

