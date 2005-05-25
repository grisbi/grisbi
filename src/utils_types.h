#ifndef _UTILS_TYPES_H
#define _UTILS_TYPES_H (1)
/* START_INCLUDE_H */
#include "utils_types.h"
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gchar *type_ope_name_by_no ( gint no_type_ope,
			     gint no_de_compte );
struct struct_type_ope *type_ope_par_no ( gint no_type_ope,
					  gint no_compte );
/* END_DECLARATION */
#endif
