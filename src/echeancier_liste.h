/*START_DECLARATION*/
gboolean affichage_traits_liste_echeances ( void );
void affiche_cache_commentaire_echeancier( void );
void click_sur_saisir_echeance ( void );
GtkWidget *creation_liste_echeances ( void );
void new_scheduled_transaction ( void );
void edition_echeance ( void );
gboolean modification_affichage_echeances ( gint *origine, GtkWidget * widget );
void remplissage_liste_echeance ( void );
void selectionne_echeance ( struct operation_echeance *echeance );
void supprime_echeance ( struct operation_echeance *echeance );
void verification_echeances_a_terme ( void );
/*END_DECLARATION*/

