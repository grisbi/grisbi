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
void activer_bouton_appliquer ( void );
gboolean selectionne_liste_preference ( GtkTreeSelection *selection,
					GtkTreeModel *model );
/* Some utility functions */
GtkWidget * new_paddingbox_with_title ( GtkWidget * parent, 
					gboolean fill, 
					gchar * title );
GtkWidget *new_vbox_with_title_and_icon ( gchar * title,
					  gchar * image_filename);
gboolean update_homepage_title ( GtkEntry *, gchar *, gint, gint * );
/* Text functions */
GtkWidget * new_text_entry ( gchar **, GCallback );
void entry_set_value ( GtkWidget * entry, gchar ** value );
gboolean set_text ( GtkEntry *, gchar *, gint, gint * );
GtkWidget * new_text_area ( gchar ** value, GCallback hook );
gboolean set_text_from_area ( GtkTextBuffer *buffer, gpointer dummy );
/* Checkbox & radio group functions */
GtkWidget * new_checkbox_with_title ( gchar *, guint *, GCallback );
GtkWidget * new_radiogroup_with_title ( GtkWidget *, gchar *, gchar *, gchar *, 
					guint *, GCallback );
void checkbox_set_value (GtkWidget *, guint *, gboolean);
gboolean set_boolean ( GtkWidget * checkbox, guint * dummy);
/* Date functions */
GtkWidget * new_date_entry ( gchar ** value, GCallback hook );
GtkWidget * get_entry_from_date_entry (GtkWidget *);
void date_set_value ( GtkWidget * hbox, GDate ** value, gboolean update );
gboolean popup_calendar ( GtkWidget * button, gpointer data );
void close_calendar_popup ( GtkWidget *popup );
/* Spin buttons functions */
GtkWidget * new_spin_button ( gdouble * value, 
			      gdouble lower, gdouble upper, 
			      gdouble step_increment, gdouble page_increment, 
			      gdouble page_size, 
			      gdouble climb_rate, guint digits,
			      GCallback hook );
void spin_button_set_value ( GtkWidget * spin, gdouble * value );
gboolean set_double ( GtkWidget * spin, gdouble * dummy );

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
    CURRENCIES_PAGE,
    BANKS_PAGE,
    FINANCIAL_YEARS_PAGE,
    METHODS_OF_PAYMENT_PAGE,
    NUM_PREFERENCES_PAGES
};
