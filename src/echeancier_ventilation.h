GtkWidget *creation_fenetre_ventilation_echeances ( void );
GtkWidget *creation_verification_ventilation_echeances  ( void );
GtkWidget *creation_formulaire_ventilation_echeances  ( void );
gboolean clique_champ_formulaire_ventilation_echeances  ( void );
gboolean entree_ventilation_perd_focus_echeances  ( GtkWidget *entree, GdkEventFocus *ev,
						   gint *no_origine );
gboolean traitement_clavier_liste_ventilation_echeances  ( GtkCList *liste,
							  GdkEventKey *evenement,
							  gpointer null );
void ventiler_operation_echeances  ( gdouble montant );
void changement_taille_liste_ventilation_echeances  ( GtkWidget *clist,
						      GtkAllocation *allocation,
						      gpointer null );
void selectionne_ligne_souris_ventilation_echeances  ( GtkCList *liste,
						       GdkEventButton *evenement,
						       gpointer null );
gboolean appui_touche_ventilation_echeances  ( GtkWidget *entree,
					      GdkEventKey *evenement,
					      gint *no_origine );
void echap_formulaire_ventilation_echeances  ( void );
void fin_edition_ventilation_echeances  ( void );
void edition_operation_ventilation_echeances  ( void );
void supprime_operation_ventilation_echeances  ( void );
void affiche_liste_ventilation_echeances  ( void );
void ajoute_ope_sur_liste_ventilation_echeances  ( struct struct_ope_ventil *operation );
void mise_a_jour_couleurs_liste_ventilation_echeances  ( void );
void selectionne_ligne_ventilation_echeances  ( void );
void calcule_montant_ventilation_echeances  ( void );
void mise_a_jour_labels_ventilation_echeances  ( void );
void valider_ventilation_echeances  ( void );
void annuler_ventilation_echeances  ( void );
void quitter_ventilation_echeances  ( void );
GSList *creation_liste_ope_de_ventil_echeances  ( struct operation_echeance *operation );
void validation_ope_de_ventilation_echeances  ( struct operation_echeance *operation );
