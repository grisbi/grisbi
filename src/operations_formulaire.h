#define START_DECLARATION
void affiche_cache_le_formulaire ( void );
void ajout_operation ( struct structure_operation *operation );
void basculer_vers_ventilation ( GtkWidget *bouton,
				 gpointer null );
gboolean clique_champ_formulaire ( GtkWidget *entree,
				   GdkEventButton *ev,
				   gint *no_origine );
GtkWidget *creation_formulaire ( void );
void degrise_formulaire_operations ( void );
void echap_formulaire ( void );
gboolean entree_perd_focus ( GtkWidget *entree,
			     GdkEventFocus *ev,
			     gint *no_origine );
gboolean entree_prend_focus ( GtkWidget *entree );
gboolean fin_edition ( void );
void formulaire_a_zero (void);
void insere_operation_dans_liste ( struct structure_operation *operation );
void place_type_formulaire ( gint no_type,
			     gint no_option_menu,
			     gchar *contenu );
void remplissage_formulaire ( gint no_compte );
struct organisation_formulaire *renvoie_organisation_formulaire ( void );
void validation_virement_operation ( struct structure_operation *operation,
				     gint modification,
				     gchar *nom_compte_vire );
void verification_bouton_change_devise ( void );
GtkWidget *widget_formulaire_par_element ( gint no_element );
#define END_DECLARATION

