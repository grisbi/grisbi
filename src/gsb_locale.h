#ifndef _GSB_LOCALE_H
#define _GSB_LOCALE_H

#include <locale.h>
#include <glib.h>

void          gsb_locale_init       ( void );
void          gsb_locale_shutdown   ( void );
struct lconv *gsb_locale_get_locale ( void );

#endif
