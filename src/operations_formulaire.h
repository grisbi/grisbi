#ifndef _OPERATIONS_FORMULAIRE_H
#define _OPERATIONS_FORMULAIRE_H (1)


/* START_INCLUDE_H */
#include "operations_formulaire.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void affiche_cache_le_formulaire ( void );
gboolean clique_champ_formulaire ( GtkWidget *entree,
				   GdkEventButton *ev,
				   gint *no_origine );
GtkWidget *creation_formulaire ( void );
void degrise_formulaire_operations ( void );
void echap_formulaire ( void );
gboolean entree_perd_focus ( GtkWidget *entree,
			     GdkEventFocus *ev,
			     gint *no_origine );
gboolean entree_prend_focus ( GtkWidget *entree );
void formulaire_a_zero (void);
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
gint recherche_element_suivant_formulaire ( gint element_courant,
					    gint sens_deplacement );
void remplissage_formulaire ( gint account_number );
void verification_bouton_change_devise ( void );
GtkWidget *widget_formulaire_par_element ( gint no_element );
/* END_DECLARATION */
#endif
