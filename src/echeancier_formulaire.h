GtkWidget *creation_formulaire_echeancier ( void );
void echap_formulaire_echeancier ( void );
gboolean entree_perd_focus_echeancier ( GtkWidget *entree,
					GdkEventFocus *ev,
					gint *no_origine );
void affiche_cache_le_formulaire_echeancier ( void );
gboolean clique_champ_formulaire_echeancier ( GtkWidget *entree,
					      GdkEventButton *ev,
					      gint *no_origine );
gboolean pression_touche_formulaire_echeancier ( GtkWidget *widget,
						 GdkEventKey *ev,
						 gint no_widget );
void affiche_date_limite_echeancier ( void );
void cache_date_limite_echeancier ( void );
void affiche_personnalisation_echeancier ( void );
void cache_personnalisation_echeancier ( void );
void fin_edition_echeance ( void );
void cree_contre_operation_echeance ( struct structure_operation *operation,
				      gint compte_virement,
				      gint contre_type_ope );
struct structure_operation *ajoute_contre_operation_echeance_dans_liste ( struct structure_operation *operation,
									  gint compte_virement,
									  gint contre_type_ope );
gint comparaison_date_echeance (  struct operation_echeance *echeance_1,
				  struct operation_echeance *echeance_2);
void formulaire_echeancier_a_zero ( void );
void incrementation_echeance ( struct operation_echeance *echeance );
void date_selectionnee_echeancier ( GtkCalendar *calendrier,
				    GtkWidget *popup );
void date_limite_selectionnee_echeancier ( GtkCalendar *calendrier,
					   GtkWidget *popup );
void completion_operation_par_tiers_echeancier ( void );
void degrise_formulaire_echeancier ( void );
void basculer_vers_ventilation_echeances ( void );

