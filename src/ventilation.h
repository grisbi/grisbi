GtkWidget *creation_fenetre_ventilation ( void );
GtkWidget *creation_verification_ventilation ( void );
GtkWidget *creation_formulaire_ventilation ( void );
void ventiler_operation ( gdouble montant );
void annuler_ventilation ( void );
void validation_ope_de_ventilation ( struct structure_operation *operation );
GSList *creation_liste_ope_de_ventil ( struct structure_operation *operation );

