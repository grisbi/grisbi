#ifndef _UTILS_REAL_H
#define _UTILS_REAL_H

#include <glib.h>
#include "gsb_real.h"

gchar *gsb_real_get_string ( gsb_real number );

gchar *gsb_real_get_string_with_currency ( gsb_real number,
                        gint currency_number,
                        gboolean show_symbol );

#endif
