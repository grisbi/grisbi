GtkWidget *creation_fenetre_ventilation ( void );
GtkWidget *creation_verification_ventilation ( void );
GtkWidget *creation_formulaire_ventilation ( void );
gboolean clique_champ_formulaire_ventilation ( void );
gboolean entree_ventilation_perd_focus ( GtkWidget *entree, GdkEventFocus *ev,
					 gint *no_origine );
gboolean traitement_clavier_liste_ventilation ( GtkCList *liste,
						GdkEventKey *evenement,
						gpointer null );
void ventiler_operation ( gdouble montant );
void changement_taille_liste_ventilation ( GtkWidget *clist,
					   GtkAllocation *allocation,
					   gpointer null );
void selectionne_ligne_souris_ventilation ( GtkCList *liste,
					    GdkEventButton *evenement,
					    gpointer null );
gboolean appui_touche_ventilation ( GtkWidget *entree,
				    GdkEventKey *evenement,
				    gint *no_origine );
void echap_formulaire_ventilation ( void );
void fin_edition_ventilation ( void );
void edition_operation_ventilation ( void );
void supprime_operation_ventilation ( void );
void affiche_liste_ventilation ( void );
void ajoute_ope_sur_liste_ventilation ( struct struct_ope_ventil *operation );
void mise_a_jour_couleurs_liste_ventilation ( void );
void selectionne_ligne_ventilation ( void );
void calcule_montant_ventilation ( void );
void mise_a_jour_labels_ventilation ( void );
void valider_ventilation ( void );
void annuler_ventilation ( void );
void quitter_ventilation ( void );
GSList *creation_liste_ope_de_ventil ( struct structure_operation *operation );
void validation_ope_de_ventilation ( struct structure_operation *operation );
