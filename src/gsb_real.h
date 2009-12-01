#ifndef _GSB_REAL_H
#define _GSB_REAL_H (1)


/** \struct frac
 *  Describes a fractionary number
 *	*/
typedef struct
{
	gint32 num;
	gint32 denom;
} frac;

/** \struct
 *  Describe a real number.
 * */
typedef struct 
{
    gint32 ent;
	frac f;
} gsb_real;

/* START MACROS */
// TRUE if r is valid (denom not null)
#define GSB_REAL_VALID(r) (((r).f.denom)!=0)
// TRUE if ent is null and num is null and denom not null
#define GSB_REAL_NULL(r) (gsb_real_sign(r) == 0)
// Returns -1 if integer is negative, 0 if null, and +1 if positive
#define INT_SIGN(i) ((i) > 0 ? 1 : ((i)< 0 ? -1 : 0 ))
// Returns -1 if gsb_real is negative, 0 if null, and +1 if positive
#define GSB_REAL_SIGN(r) (gsb_real_sign(r))
/* END MACROS */

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
static const gsb_real null_real = {0, {0, 1}};
static const gsb_real error_real = {G_MAXINT32, {0, 0}};

gsb_real gsb_real_abs ( gsb_real number );
gint gsb_real_sign ( gsb_real number );
G_MODULE_EXPORT gsb_real gsb_real_opposite ( gsb_real number );
G_MODULE_EXPORT gint gsb_real_cmp ( gsb_real number_1,
                        gsb_real number_2 );

G_MODULE_EXPORT gsb_real gsb_real_add ( gsb_real number_1,
                        gsb_real number_2 );
gsb_real gsb_real_sub ( gsb_real number_1,
                        gsb_real number_2 );
gsb_real gsb_real_mul ( gsb_real number_1,
                        gsb_real number_2 );
gsb_real gsb_real_div ( gsb_real number_1,
                        gsb_real number_2 );

//gsb_real gsb_str_to_real ( const gchar * str );
gsb_real gsb_real_get_from_string ( const gchar *string );

gchar *gsb_real_get_string ( gsb_real number );
gchar *gsb_real_get_string_with_currency ( gsb_real number,
                        gint currency_number,
                        gboolean show_symbol );
gchar *gsb_real_raw_format_string (gsb_real number,
                        struct lconv *conv,
                        const gchar *currency_symbol,
                        gint floating_point );

G_MODULE_EXPORT gsb_real gsb_real_double_to_real ( gdouble number );

gsb_real gsb_real_adjust_exponent ( gsb_real number,
                        gint return_exponent );
gboolean gsb_real_normalize ( gsb_real *number_1, gsb_real *number_2 );
/* END_DECLARATION */
#endif
