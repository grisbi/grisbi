#ifndef _GSB_REAL_H
#define _GSB_REAL_H (1)

#include <locale.h>
#include <glib.h>

#define EXPONENT_MAX 15
#define ERROR_REAL_STRING "###ERR###"

/* structure describe a real number. */
typedef struct _GsbReal		GsbReal;
struct _GsbReal
{
    gint64 mantissa;
    gint exponent;
};

static const GsbReal null_real = { 0 , 0 };
static const GsbReal error_real = { G_MININT64, 0 };


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
GsbReal		gsb_real_abs					(GsbReal number);
GsbReal		gsb_real_add					(GsbReal number_1,
                        					 GsbReal number_2);
GsbReal		gsb_real_adjust_exponent		(GsbReal number,
                        					 gint return_exponent);
gint		gsb_real_cmp					(GsbReal number_1,
                        					 GsbReal number_2);
GsbReal		gsb_real_div					(GsbReal number_1,
                        					 GsbReal number_2);
GsbReal		gsb_real_double_to_real			(gdouble number);
GsbReal		gsb_real_mul					(GsbReal number_1,
                        					 GsbReal number_2);
GsbReal		gsb_real_new					(gint64 mantissa,
											 gint exponent);
gboolean	gsb_real_normalize				(GsbReal *number_1,
											 GsbReal *number_2);
GsbReal		gsb_real_opposite				(GsbReal number);
gchar *		gsb_real_raw_format_string		(GsbReal number,
                        					 struct lconv *locale,
                        					 const gchar *currency_symbol);
GsbReal		gsb_real_raw_get_from_string	(const gchar *string,
                                        	 const gchar *mon_thousands_sep,
                                        	 const gchar *mon_decimal_point);
gdouble		gsb_real_real_to_double			(GsbReal number);
GsbReal		gsb_real_safe_real_from_string	(const gchar *string);
gchar *		gsb_real_safe_real_to_string 	(GsbReal number,
											 gint default_exponent);
GsbReal		gsb_real_sub					(GsbReal number_1,
                        					 GsbReal number_2);
/* END_DECLARATION */
#endif
