#ifndef _GSB_REAL_H
#define _GSB_REAL_H (1)

#define EXPONENT_MAX 10
#define ERROR_REAL_STRING "###ERR###"

/** \struct
 *  Describe a real number.
 * */
typedef struct
{
    gint64 mantissa;
    gint exponent;
} gsb_real;


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gsb_real gsb_real_abs ( gsb_real number );
G_MODULE_EXPORT gsb_real gsb_real_add ( gsb_real number_1,
                        gsb_real number_2 );
gsb_real gsb_real_adjust_exponent ( gsb_real number,
                        gint return_exponent );
G_MODULE_EXPORT gint gsb_real_cmp ( gsb_real number_1,
                        gsb_real number_2 );
gsb_real gsb_real_div ( gsb_real number_1,
                        gsb_real number_2 );
G_MODULE_EXPORT gsb_real gsb_real_double_to_real ( gdouble number );
gchar *gsb_real_get_decimal_point ( void );
gsb_real gsb_real_get_from_string ( const gchar *string );
gchar *gsb_real_get_string ( gsb_real number );
gchar *gsb_real_get_thousands_sep ( void );
gsb_real gsb_real_mul ( gsb_real number_1,
                        gsb_real number_2 );
gsb_real gsb_real_new ( gint64 mantissa, gint exponent );
gboolean gsb_real_normalize ( gsb_real *number_1, gsb_real *number_2 );
G_MODULE_EXPORT gsb_real gsb_real_opposite ( gsb_real number );
gchar *gsb_real_raw_format_string (gsb_real number,
                        struct lconv *conv,
                        const gchar *currency_symbol );
gsb_real gsb_real_raw_get_from_string ( const gchar *string,
                                        const gchar *mon_thousands_sep,
                                        const gchar *mon_decimal_point );
gdouble gsb_real_real_to_double ( gsb_real number );
gsb_real gsb_real_safe_real_from_string ( const gchar *string );
gchar *gsb_real_safe_real_to_string ( gsb_real number, gint default_exponent );
void gsb_real_set_decimal_point ( const gchar *decimal_point );
void gsb_real_set_thousands_sep ( const gchar *thousands_sep );
gsb_real gsb_real_sub ( gsb_real number_1,
                        gsb_real number_2 );
/* END_DECLARATION */
#endif
