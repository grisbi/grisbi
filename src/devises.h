void creation_devises_de_base ( void );
GtkWidget *creation_option_menu_devises ( gint devise_cachee,
					  GSList *liste_tmp );
void ajout_devise ( GtkWidget *bouton,
		    GtkWidget *widget );
gint bloque_echap_choix_devise ( GtkWidget *dialog,
				 GdkEventKey *key,
				 gpointer null );
gboolean nom_nouvelle_devise_defini ( GtkWidget *entree,
				      GtkWidget *label );
void retrait_devise ( GtkWidget *bouton,
		      GtkWidget *liste );
gint recherche_devise_par_nom ( struct struct_devise *devise,
				gchar *nom );
gint recherche_devise_par_code_iso ( struct struct_devise *devise,
				     gchar *nom );
gint recherche_devise_par_no ( struct struct_devise *devise,
			       gint *no_devise );
gint selection_devise ( gchar *nom_du_compte );
gboolean passe_a_l_euro ( GtkWidget *toggle_bouton,
			  GtkWidget *hbox );
void demande_taux_de_change ( struct struct_devise *devise_compte,
			      struct struct_devise *devise,
			      gint une_devise_compte_egale_x_devise_ope,
			      gdouble taux_change,
			      gdouble frais_change,
			      gboolean force );
gboolean devise_selectionnee ( GtkWidget *menu_shell,
			       gint origine );
GtkWidget *onglet_devises ( void );
gboolean selection_ligne_devise ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev,
				  GtkWidget *frame );
gboolean deselection_ligne_devise ( GtkWidget *liste,
				    gint ligne,
				    gint colonne,
				    GdkEventButton *ev,
				    GtkWidget *frame );
gboolean change_passera_euro ( GtkWidget *bouton,
			       GtkWidget *liste );
gboolean changement_devise_associee ( GtkWidget *menu_devises,
				      GtkWidget *liste );
gboolean changement_nom_entree_devise ( void );
gboolean changement_code_entree_devise ( void );
gboolean changement_iso_code_entree_devise ( void );
gdouble calcule_montant_devise_renvoi ( gdouble montant_init,
					gint no_devise_renvoi,
					gint no_devise_montant,
					gint une_devise_compte_egale_x_devise_ope,
					gdouble taux_change,
					gdouble frais_change );
gchar * devise_name (struct struct_devise * devise );
void create_change_menus (struct struct_devise *devise);
void update_exchange_rate_cache ( struct struct_devise * currency1, 
				  struct struct_devise * currency2,
				  gdouble change, gdouble fees );
struct cached_exchange_rate * cached_exchange_rate ( struct struct_devise * currency1, 
						     struct struct_devise * currency2 );
gboolean is_euro ( struct struct_devise * currency );
