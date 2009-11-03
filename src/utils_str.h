#ifndef _UTILS_STR_H
#define _UTILS_STR_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gint gsb_strcasecmp ( gchar *string_1,
                        gchar *string_2 );
gchar *gsb_string_extract_int ( const gchar *chaine );
GSList *gsb_string_get_categ_budget_struct_list_from_string ( const gchar *string );
GSList *gsb_string_get_int_list_from_string ( const gchar *string,
                        gchar *delimiter );
GSList *gsb_string_get_string_list_from_string ( const gchar *string,
                        gchar *delimiter );
gboolean gsb_string_is_trouve ( const gchar *payee_name, const gchar *needle );
gchar * gsb_string_remplace_joker ( const gchar *chaine, gchar *new_str );
gchar * gsb_string_remplace_string ( gchar * str, gchar *old_str, gchar *new_str );
gchar * gsb_string_truncate ( gchar * string );
gchar *gsb_string_uniform_new_line ( const gchar *chaine, gint nbre_char );
G_MODULE_EXPORT gchar * latin2utf8 ( const gchar * inchar);
gchar *limit_string ( gchar *string,
                        gint length );
gint my_strcasecmp ( const gchar *string_1,
                        const gchar *string_2 );
gint my_strcmp ( gchar *string_1, gchar *string_2 );
gchar *my_strdelimit ( const gchar *string,
                        const gchar *delimiters,
                        const gchar *new_delimiters );
G_MODULE_EXPORT gchar *my_strdup ( const gchar *string );
gint my_strncasecmp ( gchar *string_1,
                        gchar *string_2,
                        gint longueur );
double my_strtod ( const char *nptr, const char **endptr );
G_MODULE_EXPORT gint utils_str_atoi ( const gchar *chaine );
gchar *utils_str_itoa ( gint integer );
gchar *utils_str_reduce_exponant_from_string ( const gchar *amount_string,
                        gint exponent );
/* END_DECLARATION */

typedef enum GSB_TITLE_NAME {
    GSB_ACCOUNTS_TITLE,
    GSB_ACCOUNT_HOLDER,
    GSB_ACCOUNTS_FILE,
} GsbTitleType;
#endif
