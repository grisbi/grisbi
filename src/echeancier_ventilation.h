#define START_DECLARATION
GtkWidget *creation_fenetre_ventilation_echeances ( void );
GtkWidget *creation_formulaire_ventilation_echeances ( void );
GSList *creation_liste_ope_de_ventil_echeances ( struct operation_echeance *operation );
GtkWidget *creation_verification_ventilation_echeances ( void );
void validation_ope_de_ventilation_echeances ( struct operation_echeance *operation );
void ventiler_operation_echeances ( gdouble montant );
#define END_DECLARATION

