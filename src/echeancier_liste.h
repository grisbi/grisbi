#define START_DECLARATION
void affiche_cache_commentaire_echeancier( void );
void click_sur_saisir_echeance ( void );
GtkWidget *creation_liste_echeances ( void );
GtkWidget *creation_partie_gauche_echeancier ( void );
void edition_echeance ( void );
void remplissage_liste_echeance ( void );
void selectionne_echeance ( void );
void supprime_echeance ( struct operation_echeance *echeance );
void verification_echeances_a_terme ( void );
#define END_DECLARATION

