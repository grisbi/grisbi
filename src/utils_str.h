#ifndef _UTILS_STR_H
#define _UTILS_STR_H (1)

#include <glib.h>


#ifdef G_OS_WIN32
#define NEW_LINE "\r\n"
#else
#define NEW_LINE "\n"
#endif

/* START_DECLARATION */
gchar *		gsb_string_extract_int 									(const gchar *chaine);
GSList *	gsb_string_get_categ_budget_struct_list_from_string 	(const gchar *string);
GSList *	gsb_string_get_int_list_from_string 					(const gchar *string,
						                                             gchar *delimiter);
GSList *	gsb_string_get_string_list_from_string 					(const gchar *string,
						                                             gchar *delimiter);
gboolean 	gsb_string_is_trouve 									(const gchar *payee_name,
									                                 const gchar *needle);
gchar * 	gsb_string_remplace_joker 								(const gchar *chaine,
								                                     gchar *new_str);
gchar *		gsb_string_remplace_string 								(const gchar *str,
																	 gchar *old_str,
																	 gchar *new_str);
gchar * 	gsb_string_truncate 									(gchar * string);
gchar *		gsb_string_uniform_new_line 							(const gchar *chaine,
																	 gint nbre_char);
gchar * 	latin2utf8 												(const gchar * inchar);
gchar *		limit_string 											(gchar *string,
												                     gint length);
gint 		my_strcasecmp 										    (const gchar *string_1,
																	 const gchar *string_2);
gint 		my_strcmp 												(gchar *string_1,
																	 gchar *string_2);
gchar *		my_strdelimit 											(const gchar *string,
																	 const gchar *delimiters,
																	 const gchar *new_delimiters);
gchar *		my_strdup 												(const gchar *string);
gint 		my_strncasecmp 											(gchar *string_1,
																	 gchar *string_2,
												                     gint longueur);
gint 		utils_str_atoi 											(const gchar *chaine);
gchar *		utils_str_break_filename 								(const gchar *string,
									                                 gint trunc);
gint 		utils_str_get_nbre_motifs 								(const gchar *chaine,
																	 const gchar *motif);
gchar *		utils_str_itoa 											(gint integer);
gchar *		utils_str_dtostr 										(gdouble number,
																	 gint nbre_decimal,
																	 gboolean canonical);
gchar *		utils_str_incremente_number_from_str 					(const gchar *str_number,
																	 gint increment);
gchar *		utils_str_localise_decimal_point_from_string 			(const gchar *string);
gdouble 	utils_str_safe_strtod 									(const gchar *str_number,
																	 gchar **endptr);
gdouble 	utils_str_strtod 										(const gchar *str_number,
																	 gchar **endptr);
/* END_DECLARATION */

#endif
