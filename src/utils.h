/*START_DECLARATION*/
void demarrage_idle ( void );
void desensitive_widget ( GtkWidget *widget );
gboolean lance_navigateur_web ( const gchar *url );
gboolean met_en_normal ( GtkWidget *event_box,
			 GdkEventMotion *event,
			 gpointer pointeur );
gboolean met_en_prelight ( GtkWidget *event_box,
			   GdkEventMotion *event,
			   gpointer pointeur );
GtkWidget *new_paddingbox_with_title (GtkWidget * parent, gboolean fill, gchar * title);
GtkWidget *new_vbox_with_title_and_icon ( gchar * title,
					  gchar * image_filename);
void sens_desensitive_pointeur ( GtkWidget *bouton,
				 GtkWidget *widget );
void sensitive_widget ( GtkWidget *widget );
gboolean termine_idle ( void );
void update_ecran ( void );
gpointer **verification_p_tab ( gchar *fonction_appelante );
gboolean assert_account_loaded ();


/*END_DECLARATION*/

