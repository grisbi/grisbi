#ifndef _UTILS_DEVISES_H
#define _UTILS_DEVISES_H (1)
/* START_INCLUDE_H */
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gchar * devise_code ( struct struct_devise * devise );
gchar * devise_code_by_no ( gint no_devise );
gchar * devise_name ( struct struct_devise * devise );
struct struct_devise *devise_par_code_iso ( gchar *code_iso );
struct struct_devise *devise_par_no ( gint no_devise );
struct struct_devise *devise_par_nom ( gchar *nom_devise );
/* END_DECLARATION */
#endif
