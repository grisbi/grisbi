GtkWidget *creation_formulaire ( void );
void affiche_cache_le_formulaire ( void );
void affiche_cache_commentaire_echeancier( void );
void formulaire_a_zero (void);
gboolean entree_prend_focus ( GtkWidget *entree );
gboolean entree_perd_focus ( GtkWidget *entree, GdkEventFocus *ev, gint *no_origine );
void ajout_operation ( struct structure_operation *operation );
void insere_operation_dans_liste ( struct structure_operation *operation );
void echap_formulaire ( void );
void degrise_formulaire_operations ( void );
void validation_virement_operation ( struct structure_operation *operation,
				     gint modification, gchar *nom_compte_vire );
gboolean fin_edition ( void );
void remplissage_formulaire ( gint no_compte );
GtkWidget *widget_formulaire_par_element ( gint no_element );
struct organisation_formulaire *renvoie_organisation_formulaire ( void );
void mise_a_jour_taille_formulaire ( gint largeur_formulaire );
void verification_bouton_change_devise ( void );
void place_type_formulaire ( gint no_type,
			     gint no_option_menu,
			     gchar *contenu );
gboolean clique_champ_formulaire ( GtkWidget *entree,
				   GdkEventButton *ev, 
				   gint *no_origine );

