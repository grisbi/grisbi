#ifndef _OPERATIONS_FORMULAIRE_H
#define _OPERATIONS_FORMULAIRE_H (1)


/* START_INCLUDE_H */
#include "operations_formulaire.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void affiche_cache_le_formulaire ( void );
void click_sur_bouton_voir_change ( void );
gboolean completion_operation_par_tiers ( GtkWidget *entree );
gboolean gsb_form_finish_edition ( void );
gint gsb_form_validate_transfer ( gint transaction_number,
				  gint new_transaction,
				  gchar *name_transfer_account );
gboolean gsb_transactions_list_append_new_transaction ( gint transaction_number );
gboolean gsb_transactions_list_update_transaction ( gpointer transaction );
gint gsb_transactions_look_for_last_party ( gint no_party,
					    gint no_new_transaction,
					    gint account_number );
void place_type_formulaire ( gint no_type,
			     gint no_option_menu,
			     gchar *contenu );
void verification_bouton_change_devise ( void );
void verifie_champs_dates ( gint origine );
void widget_grab_focus_formulaire ( gint no_element );
/* END_DECLARATION */
#endif
