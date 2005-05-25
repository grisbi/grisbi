#ifndef _UTILS_EXERCICES_H
#define _UTILS_EXERCICES_H (1)
/* START_INCLUDE_H */
#include "utils_exercices.h"
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gint cherche_no_menu_exercice ( gint no_demande,
				GtkWidget *option_menu );
gchar *exercice_name_by_no ( gint no_exo );
struct struct_exercice *exercice_par_no ( gint no_exo );
struct struct_exercice *exercice_par_nom ( gchar *nom_exo );
GDate *gsb_financial_year_get_begining_date ( gint no_exo );
gint recherche_exo_correspondant ( GDate *date );
/* END_DECLARATION */
#endif
