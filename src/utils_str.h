/*START_DECLARATION*/
gint count_char_from_string ( gchar *search_char, gchar *string );
gchar *get_line_from_string ( gchar *string );
gchar *itoa ( gint integer );
gchar * latin2utf8 (char * inchar);
gchar *limit_string ( gchar *string,
		      gint length );
gint my_atoi ( gchar *chaine );
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
/*END_DECLARATION*/
