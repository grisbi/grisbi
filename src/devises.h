/*START_DECLARATION*/
void ajout_devise ( GtkWidget *widget );
GtkWidget *creation_option_menu_devises ( gint devise_cachee, GSList *liste_tmp );
void demande_taux_de_change ( struct struct_devise *devise_compte,
			      struct struct_devise *devise ,
			      gint une_devise_compte_egale_x_devise_ope,
			      gdouble taux_change,
			      gdouble frais_change,
			      gboolean force );
struct struct_devise * find_currency_from_iso4217_list ( gchar * currency_name );
GtkWidget *onglet_devises ( void );
/*END_DECLARATION*/

