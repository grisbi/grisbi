/*START_DECLARATION*/
GtkWidget *creation_liste_comptes (void);
gboolean gsb_account_list_gui_change_current_account ( gint *no_account );
GtkWidget *gsb_account_list_gui_create_account_button ( gint no_account,
							gint group,
							gpointer callback );
gboolean gsb_account_list_gui_create_list ( void );
void mise_a_jour_taille_formulaire ( gint largeur_formulaire );
/*END_DECLARATION*/

