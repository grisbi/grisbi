#ifndef _UTILS_REAL_H
#define _UTILS_REAL_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

gchar *		utils_real_get_string								(GsbReal number);
gchar *		utils_real_get_string_intl							(GsbReal number);

gchar *		utils_real_get_string_with_currency					(GsbReal number,
                        										 gint currency_number,
                        										 gboolean show_symbol);

gchar *		utils_real_get_string_with_currency_from_double		(gdouble number,
																 gint account_number);

GsbReal		utils_real_get_calculate_entry						(GtkWidget *entry);
GsbReal		utils_real_get_from_string							(const gchar *string);

#endif
