/*START_DECLARATION*/
void affichage_traits_liste_ventilation ( void );
void annuler_ventilation ( void );
gboolean changement_taille_liste_ope_ventilation ( GtkWidget *tree_view,
						   GtkAllocation *allocation );
GtkWidget *creation_formulaire_ventilation ( void );
GSList *creation_liste_ope_de_ventil ( struct structure_operation *operation );
GtkWidget *creation_verification_ventilation ( void );
gboolean selectionne_ligne_souris_ventilation ( GtkWidget *tree_view,
						GdkEventButton *evenement );
gboolean traitement_clavier_liste_ventilation ( GtkWidget *widget_variable,
						GdkEventKey *evenement );
void validation_ope_de_ventilation ( struct structure_operation *operation );
void ventiler_operation ( gdouble montant );
/*END_DECLARATION*/

