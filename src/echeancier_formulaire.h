#define START_DECLARATION
void affiche_cache_le_formulaire_echeancier ( void );
struct structure_operation *ajoute_contre_operation_echeance_dans_liste ( struct structure_operation *operation,
									  gint compte_virement,
									  gint contre_type_ope );
gint comparaison_date_echeance (  struct operation_echeance *echeance_1,
				  struct operation_echeance *echeance_2);
GtkWidget *creation_formulaire_echeancier ( void );
void degrise_formulaire_echeancier ( void );
void fin_edition_echeance ( void );
void formulaire_echeancier_a_zero ( void );
void incrementation_echeance ( struct operation_echeance *echeance );
#define END_DECLARATION

