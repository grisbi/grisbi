#ifndef _GSB_FORM_CONFIG_H
#define _GSB_FORM_CONFIG_H (1)

/* START_INCLUDE_H */
#include "gsb_form_config.h"
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *onglet_display_transaction_form ( void );
gboolean recherche_place_element_formulaire ( struct organisation_formulaire *structure_formulaire,
					      gint no_element,
					      gint *ligne,
					      gint *colonne );
gboolean verifie_element_formulaire_existe ( gint no_element );
/* END_DECLARATION */
#endif
