#ifndef _GSB_REAL_H
#define _GSB_REAL_H (1)

/** \struct
 * describe an real 
 * */
/** FIXME : that structure is defined in include.h
 * because mk_include don't take it if i define it
 * here
 * i will have to adapt mk_include to accept the typedef struct */


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gsb_real gsb_real_abs ( gsb_real number );
gsb_real gsb_real_add ( gsb_real number_1,
			gsb_real number_2 );
gsb_real gsb_real_adjust_exponent ( gsb_real number,
				    gint return_exponent );
gint gsb_real_cmp ( gsb_real number_1,
		    gsb_real number_2 );
gsb_real gsb_real_div ( gsb_real number_1,
			gsb_real number_2 );
gsb_real gsb_real_double_to_real ( gdouble number );
gsb_real gsb_real_get_from_string ( const gchar *string );
gsb_real gsb_real_get_from_string_normalized ( const gchar *string, gint default_mantissa );
gchar *gsb_real_get_string ( gsb_real number );
gsb_real gsb_real_mul ( gsb_real number_1,
			gsb_real number_2 );
gsb_real gsb_real_opposite ( gsb_real number );
gsb_real gsb_real_sub ( gsb_real number_1,
			gsb_real number_2 );
/* END_DECLARATION */
#endif
