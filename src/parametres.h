void preferences ( gint page );
GtkWidget *onglet_messages_and_warnings ( void );
GtkWidget *onglet_fichier ( void );
GtkWidget *onglet_applet ( void );
void changement_choix_backup ( GtkWidget *bouton,
			       gpointer pointeur );
void changement_preferences ( GtkWidget *fenetre_preferences,
			      gint page,
			      gpointer data );
void fermeture_preferences ( GtkWidget *, gint, gpointer);
gboolean selectionne_liste_preference ( GtkTreeSelection *selection,
					GtkTreeModel *model );
GtkWidget * create_preferences_tree ( );
gboolean preference_selectable_func (GtkTreeSelection *selection,
				     GtkTreeModel *model, GtkTreePath *path,
				     gboolean path_currently_selected,
				     gpointer data);
GtkWidget *onglet_programmes (void);
gboolean changement_nom_navigateur_web ( GtkWidget *entree );


/* Preference pages */
enum preference_page  {
    NOT_A_PAGE = -1,
    FILES_PAGE,
    IMPORT_PAGE,
    SOFTWARE_PAGE,
    FONTS_AND_LOGO_PAGE,
    MESSAGES_AND_WARNINGS_PAGE,
    ADDRESSES_AND_TITLES_PAGE,
    TRANSACTION_FORM_PAGE,
    TRANSACTIONS_LIST_PAGE,
    TRANSACTIONS_PAGE,
    RECONCILIATION_PAGE,
    TOTALS_PAGE,
    CURRENCIES_PAGE,
    BANKS_PAGE,
    FINANCIAL_YEARS_PAGE,
    METHODS_OF_PAYMENT_PAGE,
    NUM_PREFERENCES_PAGES
};
