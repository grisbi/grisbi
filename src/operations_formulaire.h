GtkWidget *creation_formulaire ( void );
void echap_formulaire ( void );
gboolean entree_prend_focus ( GtkWidget *entree );
gboolean entree_perd_focus ( GtkWidget *entree,
			 GdkEventFocus *ev,
			 gint *no_origine );
gboolean clique_champ_formulaire ( GtkWidget *entree,
			       GdkEventButton *ev,
			       gint *no_origine );
gboolean touches_champ_formulaire ( GtkWidget *widget,
				GdkEventKey *ev,
				gint *no_origine );
gboolean modifie_date ( GtkWidget *entree );
void  completion_operation_par_tiers ( void );
void fin_edition ( void );
void ajout_operation ( struct structure_operation *operation );
gint verification_validation_operation ( struct structure_operation *operation );
void recuperation_donnees_generales_formulaire ( struct structure_operation *operation );
void validation_virement_operation ( struct structure_operation *operation,
				     gint modification,
				     gchar *nom_compte_vire );
void recuperation_categorie_formulaire ( struct structure_operation *operation,
					 gint modification );
gint comparaison_date_list_ope ( struct structure_operation *ope_1,
				 struct structure_operation *ope_2);
void formulaire_a_zero (void);
void affiche_cache_le_formulaire ( void );
void affiche_cache_commentaire_echeancier( void );
void allocation_taille_formulaire ( GtkWidget *widget,
				    gpointer null );
void efface_formulaire ( GtkWidget *widget,
			 gpointer null );
void basculer_vers_ventilation ( GtkWidget *bouton,
				 gpointer null );
void click_sur_bouton_voir_change ( void );
void degrise_formulaire_operations ( void );
void increment_decrement_champ ( GtkWidget *entry, gint increment );
gint place_type_choix_type ( GtkWidget *option_menu,
			     gint type );
