/*START_DECLARATION*/
void checkbox_set_value ( GtkWidget * checkbox, guint * data, gboolean update );
GtkWidget *cree_bouton_url ( const gchar *adr,
			     const gchar *inscription );
void date_set_value ( GtkWidget * hbox, GDate ** value, gboolean update );
void demarrage_idle ( void );
void desensitive_widget ( GtkWidget *widget );
void entry_set_value ( GtkWidget * entry, gchar ** value );
gboolean format_date ( GtkWidget *entree );
GDate *gdate_today ( void );
GtkWidget * get_entry_from_date_entry (GtkWidget * hbox);
gint get_line_from_file ( FILE *fichier,
			  gchar **string );
gchar *get_line_from_string ( gchar *string );
gchar *gsb_today ( void );
void increment_decrement_champ ( GtkWidget *entry, gint increment );
gchar *itoa ( gint integer );
gboolean lance_navigateur_web ( const gchar *url );
gchar * latin2utf8 (char * inchar);
gchar *limit_string ( gchar *string,
		      gint length );
gboolean met_en_normal ( GtkWidget *event_box,
			 GdkEventMotion *event,
			 gpointer pointeur );
gboolean met_en_prelight ( GtkWidget *event_box,
			   GdkEventMotion *event,
			   gpointer pointeur );
gboolean modifie_date ( GtkWidget *entree );
gint my_atoi ( gchar *chaine );
GtkWidget * my_file_chooser ();
gchar* my_get_grisbirc_dir(void);
gchar* my_get_gsb_file_default_dir(void);
gint my_strcasecmp ( gchar *chaine_1,
		     gchar *chaine_2 );
gint my_strcmp ( gchar *chaine_1,
		 gchar *chaine_2 );
gchar *my_strdelimit ( gchar *string,
		       gchar *delimiters,
		       gchar *new_delimiters );
gint my_strncasecmp ( gchar *chaine_1,
		      gchar *chaine_2,
		      gint longueur );
double my_strtod ( char *nptr, char **endptr );
GtkWidget *new_checkbox_with_title ( gchar * label, guint * data, GCallback hook);
GtkWidget * new_date_entry ( gchar ** value, GCallback hook );
GtkWidget *new_paddingbox_with_title (GtkWidget * parent, gboolean fill, gchar * title);
GtkWidget *new_radiogroup_with_title (GtkWidget * parent,
				      gchar * title, gchar * choice1, gchar * choice2,
				      guint * data, GCallback hook);
GtkWidget * new_spin_button ( gint * value, 
			      gdouble lower, gdouble upper, 
			      gdouble step_increment, gdouble page_increment, 
			      gdouble page_size, 
			      gdouble climb_rate, guint digits,
			      GCallback hook );
GtkWidget * new_text_area ( gchar ** value, GCallback hook );
GtkWidget * new_text_entry ( gchar ** value, GCallback hook );
GtkWidget *new_vbox_with_title_and_icon ( gchar * title,
					  gchar * image_filename);
gchar *renvoie_date_formatee ( GDate *date );
void sens_desensitive_pointeur ( GtkWidget *bouton,
				 GtkWidget *widget );
void sensitive_widget ( GtkWidget *widget );
void spin_button_set_value ( GtkWidget * spin, gdouble * value );
gboolean termine_idle ( void );
void text_area_set_value ( GtkWidget * text_view, gchar ** value );
void update_ecran ( void );
gpointer **verification_p_tab ( gchar *fonction_appelante );
/*END_DECLARATION*/

