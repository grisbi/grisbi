/*START_DECLARATION*/
gdouble calcule_montant_devise_renvoi ( gdouble montant_init,
					gint no_devise_renvoi,
					gint no_devise_montant,
					gint une_devise_compte_egale_x_devise_ope,
					gdouble taux_change,
					gdouble frais_change );
gchar * devise_code ( struct struct_devise * devise );
gchar * devise_code_by_no ( gint no_devise );
gchar * devise_name ( struct struct_devise * devise );
struct struct_devise *devise_par_code_iso ( gchar *code_iso );
struct struct_devise *devise_par_no ( gint no_devise );
struct struct_devise *devise_par_nom ( gchar *nom_devise );
gboolean is_euro ( struct struct_devise * currency );
/*END_DECLARATION*/

