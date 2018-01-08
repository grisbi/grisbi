#ifndef _GSB_LOCALE_H
#define _GSB_LOCALE_H

#include <locale.h>
#include <glib.h>

const gchar *	gsb_locale_get_language 			(void);
struct lconv *	gsb_locale_get_locale 				(void);
gchar *			gsb_locale_get_mon_decimal_point 	(void);
gchar *			gsb_locale_get_mon_thousands_sep 	(void);
gchar *			gsb_locale_get_print_locale_var		(void);
void          	gsb_locale_init_language       		(const gchar *language);
void          	gsb_locale_init_lconv_struct	    (void);
void          	gsb_locale_set_mon_decimal_point 	(const gchar *decimal_point);
void          	gsb_locale_set_mon_thousands_sep 	(const gchar *thousands_sep);
void          	gsb_locale_shutdown   				(void);
#endif
