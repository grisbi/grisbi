/*START_DECLARATION*/
gchar * devise_code ( struct struct_devise * devise );
gchar * devise_code_by_no ( gint no_devise );
gchar * devise_name ( struct struct_devise * devise );
struct struct_devise *devise_par_code_iso ( gchar *code_iso );
struct struct_devise *devise_par_no ( gint no_devise );
struct struct_devise *devise_par_nom ( gchar *nom_devise );
gboolean is_euro ( struct struct_devise * currency );
/*END_DECLARATION*/

