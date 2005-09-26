#ifndef _SEARCH_GLIST_H
#define _SEARCH_GLIST_H (1)
/* START_INCLUDE_H */
#include "search_glist.h"
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gint cherche_string_equivalente_dans_slist ( gchar *string_list,
					     gchar *string_cmp );
gint recherche_banque_par_no ( struct struct_banque *banque,
			       gint *no_banque );
gint recherche_devise_par_code_iso ( struct struct_devise *devise,
				     gchar *nom );
gint recherche_devise_par_no ( struct struct_devise *devise,
			       gint *no_devise );
gint recherche_devise_par_nom ( struct struct_devise *devise,
				gchar *nom );
gint recherche_echeance_par_no ( struct operation_echeance *echeance,
				 gint no_echeance );
gint recherche_echeance_par_no_compte ( struct operation_echeance *echeance,
					gint no_compte );
gint recherche_exercice_par_no ( struct struct_exercice *exercice,
				 gint *no_exercice );
gint recherche_operation_par_cheque ( gpointer operation,
				      gint *no_chq );
gint recherche_operation_par_id ( gpointer operation,
				  gchar *id_recherchee );
gint recherche_rapprochement_par_no ( struct struct_no_rapprochement *rapprochement,
				      gint *no_rap );
gint recherche_rapprochement_par_nom ( struct struct_no_rapprochement *rapprochement,
				       gchar *no_rap );
gint recherche_type_ope_par_no ( struct struct_type_ope *type_ope,
				 gint *no_type );
/* END_DECLARATION */
#endif
