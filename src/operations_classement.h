gint classement_liste_par_no_ope_ventil ( GtkWidget *liste,
					  GtkCListRow *ligne_1,
					  GtkCListRow *ligne_2 );
gint classement_liste_equilibrage ( GtkWidget *liste,
				    GtkCListRow *ligne_1,
				    GtkCListRow *ligne_2 );
gint classement_sliste_par_date ( struct structure_operation *operation_1,
				  struct structure_operation *operation_2 );
gint classement_sliste_par_no ( struct structure_operation *operation_1,
				struct structure_operation *operation_2 );
gint classement_sliste_par_pointage ( struct structure_operation *operation_1,
				     struct structure_operation *operation_2 );
gint classement_sliste_par_debit ( struct structure_operation *operation_1,
				   struct structure_operation *operation_2 );
gint classement_sliste_par_credit ( struct structure_operation *operation_1,
				    struct structure_operation *operation_2 );
gint classement_sliste_par_montant ( struct structure_operation *operation_1,
				     struct structure_operation *operation_2 );
gint classement_sliste_par_devise ( struct structure_operation *operation_1,
				     struct structure_operation *operation_2 );
gint classement_sliste_par_tiers ( struct structure_operation *operation_1,
				     struct structure_operation *operation_2 );
gint classement_sliste_par_categories ( struct structure_operation *operation_1,
					struct structure_operation *operation_2 );
gint classement_sliste_par_imputation ( struct structure_operation *operation_1,
					struct structure_operation *operation_2 );
gint classement_sliste_par_notes ( struct structure_operation *operation_1,
				   struct structure_operation *operation_2 );
gint classement_sliste_par_auto_man ( struct structure_operation *operation_1,
				      struct structure_operation *operation_2 );
gint classement_sliste_par_no_rapprochement ( struct structure_operation *operation_1,
					      struct structure_operation *operation_2 );
gint classement_sliste_par_exercice ( struct structure_operation *operation_1,
				      struct structure_operation *operation_2 );
gint classement_sliste_par_pc ( struct structure_operation *operation_1,
				struct structure_operation *operation_2 );
gint classement_sliste_par_ibg ( struct structure_operation *operation_1,
				 struct structure_operation *operation_2 );
gint classement_sliste_par_type_ope ( struct structure_operation *operation_1,
				      struct structure_operation *operation_2 );
gint classe_liste_alphabetique ( gchar *string_1,
				 gchar *string_2 );

