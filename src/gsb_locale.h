#ifndef _GSB_LOCALE_H
#define _GSB_LOCALE_H

#include <locale.h>
#include <glib.h>

void          gsb_locale_init       ( void );
void          gsb_locale_shutdown   ( void );
struct lconv *gsb_locale_get_locale ( void );
gchar        *gsb_locale_get_mon_decimal_point ( void );
void          gsb_locale_set_mon_decimal_point ( const gchar *decimal_point );
gchar        *gsb_locale_get_mon_thousands_sep ( void );
void          gsb_locale_set_mon_thousands_sep ( const gchar *thousands_sep );

gchar *gsb_locale_get_print_locale_var ( void );
const gchar *gsb_locale_get_langue (void);

#endif
