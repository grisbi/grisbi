GtkWidget *onglet_devises ( void );
void ajout_devise ( GtkWidget *widget );
struct struct_devise *devise_par_no ( gint no_devise );
struct struct_devise *devise_par_nom ( gchar *nom_devise );
struct struct_devise *devise_par_code_iso ( gchar *code_iso );
gchar *devise_code (struct struct_devise * devise );
gchar *devise_code_by_no ( gint no_devise );
gchar *devise_name (struct struct_devise * devise );

gdouble calcule_montant_devise_renvoi ( gdouble montant_init,
					gint no_devise_renvoi,
					gint no_devise_montant,
					gint une_devise_compte_egale_x_devise_ope,
					gdouble taux_change,
					gdouble frais_change );
void demande_taux_de_change ( struct struct_devise *devise_compte,
			      struct struct_devise *devise,
			      gint une_devise_compte_egale_x_devise_ope,
			      gdouble taux_change,
			      gdouble frais_change,
			      gboolean force );
GtkWidget *creation_option_menu_devises ( gint devise_cachee,
					  GSList *liste_tmp );
gboolean is_euro ( struct struct_devise * currency );
struct struct_devise * create_currency ( gchar * nom_devise, gchar * code_devise, 
					 gchar * code_iso4217_devise );
struct struct_devise * find_currency_from_iso4217_list ( gchar * currency_name );

