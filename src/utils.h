gboolean met_en_prelight ( GtkWidget *event_box,
		       GdkEventMotion *event,
		       gpointer pointeur );
gboolean met_en_normal ( GtkWidget *event_box,
		     GdkEventMotion *event,
		     gpointer pointeur );
gchar *gsb_today ( void ) ;
GDate *gdate_today ( void );
gboolean modifie_date ( GtkWidget *entree );
gboolean format_date ( GtkWidget *entree );
double gsb_strtod ( char *nptr, char **endptr );
void sens_desensitive_pointeur ( GtkWidget *boutton,
				 GtkWidget *widget );
void sensitive_widget ( GtkWidget *widget );
void desensitive_widget ( GtkWidget *widget );
gchar *itoa ( gint integer );
gint my_atoi ( gchar *chaine );
double my_strtod ( char *nptr, char **endptr );
gchar * latin2utf8 (char * inchar);
int myisolat1ToUTF8(unsigned char* out, int *outlen,
		const unsigned char* in, int *inlen);
GtkWidget *cree_bouton_url ( const gchar *adr,
			     const gchar *inscription );
gboolean lance_navigateur_web ( const gchar *url );
gchar *my_strdelimit ( gchar *string,
		       gchar *delimiters,
		       gchar *new_delimiters );
GtkWidget * new_paddingbox_with_title ( GtkWidget * parent, 
					gboolean fill, 
					gchar * title );
GtkWidget *new_vbox_with_title_and_icon ( gchar * title,
					  gchar * image_filename);
GtkWidget * new_text_entry ( gchar **, GCallback );
void entry_set_value ( GtkWidget * entry, gchar ** value );
GtkWidget * new_text_area ( gchar ** value, GCallback hook );
void text_area_set_value ( GtkWidget * text_view, gchar ** value );
gboolean set_text_from_area ( GtkTextBuffer *buffer, gpointer dummy );
gboolean set_text ( GtkEntry *, gchar *, gint, gint * );
GtkWidget * new_checkbox_with_title ( gchar *, guint *, GCallback );
GtkWidget * new_radiogroup_with_title ( GtkWidget *, gchar *, gchar *, gchar *, 
					guint *, GCallback );
void checkbox_set_value (GtkWidget *, guint *, gboolean);
gboolean set_boolean ( GtkWidget * checkbox, guint * dummy);
gboolean set_date (GtkEntry *entry, gchar *value, gint length, gint * position);
GtkWidget * new_date_entry ( gchar ** value, GCallback hook );
void date_set_value ( GtkWidget * hbox, GDate ** value, gboolean update );
gboolean popup_calendar ( GtkWidget * button, gpointer data );
void close_calendar_popup ( GtkWidget *popup );
GtkWidget * get_entry_from_date_entry (GtkWidget *);
GtkWidget * new_spin_button ( gdouble * value, 
			      gdouble lower, gdouble upper, 
			      gdouble step_increment, gdouble page_increment, 
			      gdouble page_size, 
			      gdouble climb_rate, guint digits,
			      GCallback hook );
void spin_button_set_value ( GtkWidget * spin, gdouble * value );
gboolean set_double ( GtkWidget * spin, gdouble * dummy );
GtkWidget * my_file_chooser ();
void browse_file ( GtkButton *button, gpointer data );
gint my_strcasecmp ( gchar *chaine_1,
		     gchar *chaine_2 );
gint my_strncasecmp ( gchar *chaine_1,
		     gchar *chaine_2,
		     gint longueur );
gchar* my_get_grisbirc_dir(void);
gchar* my_get_gsb_file_default_dir(void);

