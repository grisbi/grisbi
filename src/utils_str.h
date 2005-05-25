#ifndef _UTILS_STR_H
#define _UTILS_STR_H (1)
/* START_INCLUDE_H */
#include "utils_str.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gint count_char_from_string ( gchar *search_char, gchar *string );
gchar *get_line_from_string ( gchar *string );
gchar *utils_str_itoa ( gint integer );
gchar * latin2utf8 (char * inchar);
gchar *limit_string ( gchar *string,
		      gint length );
gint utils_str_atoi ( gchar *chaine );
gchar *my_strdelimit ( gchar *string,
		       gchar *delimiters,
		       gchar *new_delimiters );
gint my_strncasecmp ( gchar *chaine_1,
		      gchar *chaine_2,
		      gint longueur );
double my_strtod ( char *nptr, char **endptr );
/* END_DECLARATION */
#endif
