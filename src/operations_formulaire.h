#ifndef _OPERATIONS_FORMULAIRE_H
#define _OPERATIONS_FORMULAIRE_H (1)

#define TRANSACTION_FORM_DATE 1
#define TRANSACTION_FORM_DEBIT 2
#define TRANSACTION_FORM_CREDIT 3
#define TRANSACTION_FORM_VALUE_DATE 4
#define TRANSACTION_FORM_EXERCICE 5
#define TRANSACTION_FORM_PARTY 6
#define TRANSACTION_FORM_CATEGORY 7
#define TRANSACTION_FORM_FREE 8  /* free for the moment, to be fill with a new thing */
#define TRANSACTION_FORM_BUDGET 9
#define TRANSACTION_FORM_NOTES 10
#define TRANSACTION_FORM_TYPE 11
#define TRANSACTION_FORM_CHEQUE 12
#define TRANSACTION_FORM_DEVISE 13
#define TRANSACTION_FORM_CHANGE 14
#define TRANSACTION_FORM_VOUCHER 15
#define TRANSACTION_FORM_BANK 16
#define TRANSACTION_FORM_CONTRA 17
#define TRANSACTION_FORM_OP_NB 18
#define TRANSACTION_FORM_MODE 19
#define TRANSACTION_FORM_WIDGET_NB 19

/* Constantes pour style_entree_formulaire[] */
#define ENCLAIR 0
#define ENGRIS 1

/* START_INCLUDE_H */
#include "structures.h"
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
gboolean gsb_form_validate_transfer ( struct structure_operation *transaction,
				      gint new_transaction,
				      gchar *name_transfer_account );
gboolean gsb_transactions_append_transaction ( struct structure_operation *transaction,
					       gint no_account );
gboolean gsb_transactions_list_append_new_transaction ( struct structure_operation *transaction );
void place_type_formulaire ( gint no_type,
			     gint no_option_menu,
			     gchar *contenu );
gint recherche_element_suivant_formulaire ( gint element_courant,
					    gint sens_deplacement );
void remplissage_formulaire ( gint no_compte );
struct organisation_formulaire *renvoie_organisation_formulaire ( void );
void verification_bouton_change_devise ( void );
GtkWidget *widget_formulaire_par_element ( gint no_element );
/* END_DECLARATION */
#endif
