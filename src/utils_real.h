#ifndef _UTILS_REAL_H
#define _UTILS_REAL_H

#include <glib.h>
#include "gsb_real.h"

gchar *utils_real_get_string ( gsb_real number );

gchar *utils_real_get_string_with_currency ( gsb_real number,
                        gint currency_number,
                        gboolean show_symbol );

gchar *utils_real_get_string_with_currency_from_double ( gdouble number,
                        gint account_number );

gsb_real utils_real_get_from_string ( const gchar *string );

#endif
