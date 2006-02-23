/* ************************************************************************** */
/* Ce fichier s'occupe de la gestion du formulaire de saisie des échéances    */
/*			echeances_formulaire.c                                */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
/*			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


#include "include.h"

/*START_INCLUDE*/
#include "echeancier_formulaire.h"
#include "exercice.h"
#include "comptes_traitements.h"
#include "erreur.h"
#include "dialog.h"
#include "calendar.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "gsb_form.h"
#include "gsb_form_transaction.h"
#include "accueil.h"
#include "gsb_payment_method.h"
#include "gsb_real.h"
#include "gsb_scheduler_list.h"
#include "utils_editables.h"
#include "gtk_combofix.h"
#include "main.h"
#include "categories_onglet.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils_exercices.h"
#include "utils_comptes.h"
#include "utils_types.h"
#include "utils_operations.h"
#include "structures.h"
#include "gsb_data_form.h"
#include "echeancier_infos.h"
#include "echeancier_formulaire.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void basculer_vers_ventilation_echeances ( void );
static GtkWidget *creation_formulaire_echeancier ( void );
static void echap_formulaire_echeancier ( void );
static gboolean gsb_scheduler_check_form ( void );
static gint gsb_scheduler_create_scheduled_transaction_from_scheduled_form ( gint scheduled_number );
static gint gsb_scheduler_create_transaction_from_scheduled_form ( void );
static gboolean gsb_scheduler_get_category_for_transaction_from_form ( gint transaction_number );
static gboolean gsb_scheduler_get_category_for_transaction_from_transaction ( gint transaction_number,
								       gint scheduled_number );
static gboolean gsb_scheduler_increase_date ( gint scheduled_number,
				       GDate *date );
/*END_STATIC*/



GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
GtkWidget *label_saisie_modif;
GtkWidget *separateur_formulaire_echeancier;
GtkWidget *hbox_valider_annuler_echeance;


/*START_EXTERN*/
extern GtkWidget *formulaire;
extern GtkWidget *formulaire_echeancier;
extern GtkWidget *frame_formulaire_echeancier;
extern GtkWidget *main_page_finished_scheduled_transactions_part;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern FILE * out;
extern GtkStyle *style_entree_formulaire[2];
/*END_EXTERN*/


/******************************************************************************/
/*  Routine qui crée le formulaire et le renvoie */
/******************************************************************************/
GtkWidget *creation_formulaire_echeancier ( void )
{
    GtkWidget *menu, *item, *bouton, *table;
    GtkTooltips *tips;
    gint no_compte;

    /* on crée le tooltips */
    tips = gtk_tooltips_new ();

    formulaire_echeancier = gtk_vbox_new ( FALSE, 0 );
    gtk_widget_show ( formulaire_echeancier );

    /*   création de la table */
    table = gtk_table_new ( 6, 4, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
    gtk_widget_set_usize ( table, 1, FALSE );
    gtk_box_pack_start ( GTK_BOX ( formulaire_echeancier ), table, TRUE, TRUE, 0 );
    gtk_widget_show ( table );

    /* création du label saisie / modif */

    label_saisie_modif = gtk_label_new ( "" );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label_saisie_modif,
		       0, 1,
		       0, 1,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0,0);


    /* création de l'entrée de la date */
    widget_formulaire_echeancier[SCHEDULER_FORM_DATE] = gtk_entry_new_with_max_length ( 10 );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_DATE],
		       1, 2,
		       0, 1,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_DATE ) );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			 "focus-in-event",
			 GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			 "focus-out-event",
			 GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			 NULL );
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );

    /* création du combofix des tiers */
    widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] = gtk_combofix_new (gsb_data_payee_get_name_list());
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_PARTY ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_PARTY ) );
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY]) -> entry),
				"focus-in-event",
				GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
				GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ),
			 "focus-out-event",
			 GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_PARTY ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_PARTY],
		       2, 3,
		       0, 1,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] );

    /* création de l'entrée des débits */
    widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] = gtk_entry_new ();
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_DEBIT ) );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_DEBIT ) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT]),
			 "focus-in-event",
			 GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
			 "focus-out-event",
			 GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_DEBIT ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT],
		       3, 4,
		       0, 1,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] );


    /* création de l'entrée des crédits */
    widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] = gtk_entry_new ();
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CREDIT ) );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CREDIT ) );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
			 "focus-in-event",
			 GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
			 "focus-out-event",
			 GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CREDIT ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT],
		       4, 5,
		       0, 1,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] );


    /* met l'option menu des devises */
    widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] = gsb_currency_make_combobox (TRUE);
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE],
			   _("Choose currency"),
			   _("Choose currency") );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_DEVISE ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE],
		       5, 7,
		       0, 1,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] );

    /* Mise en place du menu des comptes */
    widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT],
			   _("Choose the account"),
			   _("Choose the account") );

    menu = creation_option_menu_comptes ( GTK_SIGNAL_FUNC(changement_choix_compte_echeancier), TRUE, FALSE );
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
			       menu );
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
				  0);

    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_ACCOUNT ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT],
		       0, 2,
		       1, 2,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

    /* Affiche les catégories / sous-catégories */
    widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] = gtk_combofix_new_complex ( gsb_data_category_get_name_list (TRUE,
															TRUE,
															TRUE,
															TRUE ));
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CATEGORY ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CATEGORY ) );
/*     gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> arrow ), */
/* 			 "button-press-event", */
/* 			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ), */
/* 			 GINT_TO_POINTER ( SCHEDULER_FORM_CATEGORY ) ); */
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ),
				"focus-in-event",
				GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
				widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ),
			 "focus-out-event",
			 GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CATEGORY ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY],
		       2, 3,
		       1, 2,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] );

    /* Création de l'entrée du chèque, non affichée pour le moment
       à créer avant l'option menu du type d'opé */

    widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] = gtk_entry_new();
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER( SCHEDULER_FORM_CHEQUE ) );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CHEQUE ) );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
			 "focus-in-event",
			 GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
			 "focus-out-event",
			 GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CHEQUE ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE],
		       5, 7,
		       1, 2,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );

    /*  Affiche l'option menu des types */
    widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_echeancier[SCHEDULER_FORM_TYPE],
			   _("Choose the method of payment"),
			   _("Choose the method of payment") );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_TYPE ) );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_echeancier[SCHEDULER_FORM_TYPE],
		       3, 5,
		       1, 2,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);

    /* le menu par défaut est celui des débits */
    no_compte = recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

/*     if ( ( menu = creation_menu_types ( 1, */
/* 					no_compte, */
/* 					1 ))) */
    {
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				   menu );
/* 	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ), */
/* 				      cherche_no_menu_type_echeancier ( gsb_data_account_get_default_debit (no_compte) ) ); */
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
    }

    /* met l'option menu de l'exercice */

    widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE],
			   _("Choose the financial year"),
			   _("Choose the financial year") );
    menu = gtk_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ),
			       creation_menu_exercices (1) );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER( SCHEDULER_FORM_EXERCICE ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE],
		       0, 2,
		       2, 3,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] );

    /*  Affiche l'imputation budgétaire */

    widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] = gtk_combofix_new_complex ( gsb_data_budget_get_name_list (TRUE, TRUE));
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY],
		       2, 3,
		       2, 3,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER( SCHEDULER_FORM_BUDGETARY ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_BUDGETARY ) );
/*     gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> arrow ), */
/* 			 "button-press-event", */
/* 			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ), */
/* 			 GINT_TO_POINTER ( SCHEDULER_FORM_BUDGETARY ) ); */
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry ),
				"focus-in-event",
				GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
				widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry ),
			 "focus-out-event",
			 GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_BUDGETARY ) );

    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] );

    /* bouton ventilation, pour accéder à la fenetre de ventil de l'échéance */

    widget_formulaire_echeancier[SCHEDULER_FORM_BREAKDOWN] = gtk_button_new_with_label ( _("Breakdown"));
    gtk_button_set_relief ( GTK_BUTTON ( widget_formulaire_echeancier[SCHEDULER_FORM_BREAKDOWN] ),
			    GTK_RELIEF_NONE );

    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_BREAKDOWN],
		       3, 5, 2, 3,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_BREAKDOWN] ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( basculer_vers_ventilation_echeances ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_BREAKDOWN]),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_BREAKDOWN ) );


    /* création de l'entrée du no de pièce comptable
       à ne pas mettre, mais on réserve encore le widget n°
       ne pas l'effacer pour respecter les tabulations */

    widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER] = gtk_entry_new();

    /*
       gtk_table_attach ( GTK_TABLE ( table ),
       widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER],
       5, 7, 2, 3,
       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
       GTK_SHRINK | GTK_FILL,
       0, 0);
       gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER] ),
       "button-press-event",
       GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
       GINT_TO_POINTER ( SCHEDULER_FORM_VOUCHER ) );
       gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER]),
       "key-press-event",
       GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
       GINT_TO_POINTER ( SCHEDULER_FORM_VOUCHER ) );
       gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER]),
       "focus-in-event",
       GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
       NULL );
       gtk_signal_connect_after ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER]),
       "focus-out-event",
       GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
       GINT_TO_POINTER ( SCHEDULER_FORM_VOUCHER ) );

       if ( etat.utilise_piece_comptable )
       gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER] );
       */

    /* Mise en place du menu automatique/manuel */

    widget_formulaire_echeancier[SCHEDULER_FORM_MODE] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_echeancier[SCHEDULER_FORM_MODE],
			   _("Automatic/manual scheduled transaction"),
			   _("Automatic/manual scheduled transaction") );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_MODE ) );

    menu = gtk_menu_new ();

    item = gtk_menu_item_new_with_label ( _("Manual"));
    gtk_object_set_data ( GTK_OBJECT ( item ),
			  "auto_man",
			  GINT_TO_POINTER ( 0 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Automatic"));
    gtk_object_set_data ( GTK_OBJECT ( item ),
			  "auto_man",
			  GINT_TO_POINTER ( 1 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
    gtk_widget_show ( item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] ),
			       menu );

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] ),
				  0);
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_MODE],
		       0, 2,
		       3, 4,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] );

    /*  Affiche les notes */

    widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] = gtk_entry_new ();
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_NOTES ) );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_NOTES ) );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			 "focus-in-event",
			 GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
			 NULL );
    gtk_signal_connect_after ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			       "focus-out-event",
			       GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			       GINT_TO_POINTER ( SCHEDULER_FORM_NOTES ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_NOTES],
		       2, 3,
		       3, 4,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] );

    /* Mise en place du menu de la fréquence */

    widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY],
			   _("Frequency"),
			   _("Frequency") );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_FREQUENCY ) );

    menu = gtk_menu_new ();

    item = gtk_menu_item_new_with_label ( _("Once"));
    gtk_object_set_data ( GTK_OBJECT  ( item ),
			  "periodicite",
			  GINT_TO_POINTER ( 0 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
/*     gtk_signal_connect ( GTK_OBJECT  ( item ), */
/* 			 "activate", */
/* 			 GTK_SIGNAL_FUNC ( cache_date_limite_echeancier ), */
/* 			 NULL ); */
/*     gtk_signal_connect ( GTK_OBJECT  ( item ), */
/* 			 "activate", */
/* 			 GTK_SIGNAL_FUNC ( cache_personnalisation_echeancier ), */
/* 			 NULL ); */
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Weekly"));
    gtk_object_set_data ( GTK_OBJECT  ( item ),
			  "periodicite",
			  GINT_TO_POINTER ( 1 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
/*     gtk_signal_connect ( GTK_OBJECT  ( item ), */
/* 			 "activate", */
/* 			 GTK_SIGNAL_FUNC ( affiche_date_limite_echeancier ), */
/* 			 NULL ); */
/*     gtk_signal_connect ( GTK_OBJECT  ( item ), */
/* 			 "activate", */
/* 			 GTK_SIGNAL_FUNC ( cache_personnalisation_echeancier ), */
/* 			 NULL ); */
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Monthly"));
    gtk_object_set_data ( GTK_OBJECT  ( item ),
			  "periodicite",
			  GINT_TO_POINTER ( 2 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
/*     gtk_signal_connect ( GTK_OBJECT  ( item ), */
/* 			 "activate", */
/* 			 GTK_SIGNAL_FUNC ( affiche_date_limite_echeancier ), */
/* 			 NULL ); */
/*     gtk_signal_connect ( GTK_OBJECT  ( item ), */
/* 			 "activate", */
/* 			 GTK_SIGNAL_FUNC ( cache_personnalisation_echeancier ), */
/* 			 NULL ); */
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Yearly"));
    gtk_object_set_data ( GTK_OBJECT  ( item ),
			  "periodicite",
			  GINT_TO_POINTER ( 3 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
/*     gtk_signal_connect ( GTK_OBJECT  ( item ), */
/* 			 "activate", */
/* 			 GTK_SIGNAL_FUNC ( affiche_date_limite_echeancier ), */
/* 			 NULL ); */
/*     gtk_signal_connect ( GTK_OBJECT  ( item ), */
/* 			 "activate", */
/* 			 GTK_SIGNAL_FUNC ( cache_personnalisation_echeancier ), */
/* 			 NULL ); */
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Custom"));
    gtk_object_set_data ( GTK_OBJECT  ( item ),
			  "periodicite",
			  GINT_TO_POINTER ( 4 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
/*     gtk_signal_connect ( GTK_OBJECT  ( item ), */
/* 			 "activate", */
/* 			 GTK_SIGNAL_FUNC ( affiche_date_limite_echeancier ), */
/* 			 NULL ); */
/*     gtk_signal_connect ( GTK_OBJECT  ( item ), */
/* 			 "activate", */
/* 			 GTK_SIGNAL_FUNC ( affiche_personnalisation_echeancier ), */
/* 			 NULL ); */
    gtk_widget_show ( item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] ),
			       menu );

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] ),
				  0);

    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY],
		       3, 4,
		       3, 4,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] );

    /* entrée de la date limite, non affichée au départ */

    widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] = gtk_entry_new_with_max_length (11);
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_FINAL_DATE ) );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_FINAL_DATE ) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE]),
			 "focus-in-event",
			 GTK_SIGNAL_FUNC ( gsb_form_entry_get_focus ),
			 NULL );
    gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE]),
			       "focus-out-event",
			       GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			       GINT_TO_POINTER ( SCHEDULER_FORM_FINAL_DATE ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE],
		       4, 5,
		       3, 4,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );

    /* et la gtk entry quand la fréquence est personnalisée, non affiché au départ */

    widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] = gtk_entry_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB],
			   _("Custom frequency"),
			   _("Custom frequency") );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_FREQ_CUSTOM_NB ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB],
		       5, 6,
		       3, 4,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );

    /* le menu jour / mois / année */

    widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU],
			   _("Custom frequency"),
			   _("Custom frequency") );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_FREQ_CUSTOM_MENU ) );

    menu = gtk_menu_new ();

    item = gtk_menu_item_new_with_label ( _("Days") );
    gtk_object_set_data ( GTK_OBJECT ( item ),
			  "intervalle_perso",
			  GINT_TO_POINTER ( 0 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Months") );
    gtk_object_set_data ( GTK_OBJECT ( item ),
			  "intervalle_perso",
			  GINT_TO_POINTER ( 1 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Years") );
    gtk_object_set_data ( GTK_OBJECT ( item ),
			  "intervalle_perso",
			  GINT_TO_POINTER ( 2 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
    gtk_widget_show ( item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] ),
			       menu );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU],
		       6, 7,
		       3, 4,
		       GTK_SHRINK | GTK_FILL | GTK_EXPAND,
		       GTK_SHRINK | GTK_FILL,
		       0, 0);
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] );

    /* séparation d'avec les boutons */

    separateur_formulaire_echeancier = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( formulaire_echeancier ),
			 separateur_formulaire_echeancier,
			 FALSE,
			 FALSE,
			 0 );
    if ( etat.affiche_boutons_valider_annuler )
	gtk_widget_show ( separateur_formulaire_echeancier );

    /* mise en place des boutons */

    hbox_valider_annuler_echeance = gtk_hbox_new ( FALSE,
						   5 );
    gtk_box_pack_start ( GTK_BOX ( formulaire_echeancier ),
			 hbox_valider_annuler_echeance,
			 FALSE,
			 FALSE,
			 0 );
    if ( etat.affiche_boutons_valider_annuler )
	gtk_widget_show ( hbox_valider_annuler_echeance );

    bouton = gtk_button_new_from_stock ( GTK_STOCK_OK );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ), GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC (gsb_scheduler_validate_form),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox_valider_annuler_echeance ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gtk_button_set_relief ( GTK_BUTTON ( bouton ), GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( echap_formulaire_echeancier ),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox_valider_annuler_echeance ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( bouton );

    /* on associe au formulaire l'adr de l'échéance courante */

    gtk_object_set_data ( GTK_OBJECT ( formulaire_echeancier ),
			  "scheduled_number",
			  NULL );

    formulaire_echeancier_a_zero ();

    return ( formulaire_echeancier);
}
/******************************************************************************/

/******************************************************************************/
void echap_formulaire_echeancier ( void )
{
    formulaire_echeancier_a_zero();

    gtk_widget_grab_focus ( gsb_scheduler_list_get_tree_view () );

    if ( !etat.formulaire_echeancier_toujours_affiche )
	gtk_expander_set_expanded ( GTK_EXPANDER(frame_formulaire_echeancier), FALSE );
}
/******************************************************************************/

/******************************************************************************/
/* Fonction appelée quand une entry perd le focus */
/* si elle ne contient rien, on remet la fonction en gris */
/******************************************************************************/
gboolean entree_perd_focus_echeancier ( GtkWidget *entree,
					GdkEventFocus *ev,
					gint *no_origine )
{
    gchar *texte;
    texte = NULL;

    /* !!!!!!!mettre p_tab... en fonction du compte sélectionné */


    switch ( GPOINTER_TO_INT ( no_origine ))
    {
	/* on sort de la date, soit c'est vide, soit on la vérifie, la complète si nécessaire et met à jour l'exercice */
	case SCHEDULER_FORM_DATE :
	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		gsb_date_check_and_complete_entry ( entree );

		/* on ne change l'exercice que si c'est une nouvelle échéance */

		if ( !gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
					    "scheduled_number" ))
		    affiche_exercice_par_date( widget_formulaire_echeancier[SCHEDULER_FORM_DATE],
					       widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] );
	    }
	    else
		texte = _("Date");
	    break;

	    /* on sort du tiers : soit vide soit complète le reste de l'opé */

	case SCHEDULER_FORM_PARTY :
/* 	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree ))))) */
/* 		completion_operation_par_tiers_echeancier (); */
/* 	    else */
/* 		texte = _("Payee"); */
	    break;

	    /* on sort du débit : soit vide, soit change le menu des types
	       s'il ne correspond pas */

	case SCHEDULER_FORM_DEBIT :

	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		/* on commence par virer ce qu'il y avait dans les crédits */

		if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ) == style_entree_formulaire[ENCLAIR] )
		{
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
					 "" );
		    entree_perd_focus_echeancier ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT],
						   NULL,
						   GINT_TO_POINTER ( SCHEDULER_FORM_CREDIT ));
		}

		if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) &&
		     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu ),
							     "signe_menu" )) == 2 )
		{
/* 		    GtkWidget *menu; */
		    gint no_compte;

		    no_compte = recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

/* 		    if ( ( menu = creation_menu_types ( 1, */
/* 							no_compte, */
/* 							1 ))) */
		    {
/* 			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ), */
/* 						   menu ); */
/* 			gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ), */
/* 						      cherche_no_menu_type_echeancier ( gsb_data_account_get_default_debit (no_compte) ) ); */
			gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
		    }
/* 		    else */
			gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
		}
	    }
	    else
		texte = _("Debit");
	    break;

	    /* on sort du crédit : soit vide, soit change le menu des types
	       s'il n'y a aucun tiers ( <=> nouveau tiers ) */

	case SCHEDULER_FORM_CREDIT :
	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		/* on commence par virer ce qu'il y avait dans les débits */

		if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
		{
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
					 "" );
		    entree_perd_focus_echeancier ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT],
						   NULL,
						   GINT_TO_POINTER ( SCHEDULER_FORM_DEBIT ));
		}


		if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) &&
		     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu ),
							     "signe_menu" )) == 1 )
		{
/* 		    GtkWidget *menu; */
		    gint no_compte;

		    no_compte = recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

/* 		    if ( ( menu = creation_menu_types ( 2, */
/* 							no_compte, */
/* 							1  ))) */
		    {
/* 			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ), */
/* 						   menu ); */
/* 			gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ), */
/* 						      cherche_no_menu_type_echeancier ( gsb_data_account_get_default_credit (no_compte) ) ); */
			gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
		    }
/* 		    else */
			gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
		}
	    }
	    else
		texte = _("Credit");
	    break;

	    /*       sort des catégories */

	case SCHEDULER_FORM_CATEGORY :
	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Categories : Sub-categories");
	    else
		if ( !strcmp ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree ))),
			       _("Breakdown of transaction") ))
		{
		    if ( !etat.formulaire_echeance_dans_fenetre )
			gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_BREAKDOWN] );
		    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE],
					       FALSE );
		    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY],
					       FALSE );
		}


	    break;

	case SCHEDULER_FORM_CHEQUE :
	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Transfer reference");
	    break;

	case SCHEDULER_FORM_BUDGETARY :
	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Budgetary line");
	    break;

	case SCHEDULER_FORM_BREAKDOWN :
	    break;

	case SCHEDULER_FORM_VOUCHER :
	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Voucher");

	    break;

	case SCHEDULER_FORM_NOTES :
	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Notes");
	    break;

	    /* on sort de la date limite, soit c'est vide, soit on la vérifie,
	       la complète si nécessaire */
	case SCHEDULER_FORM_FINAL_DATE :
	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		gsb_date_check_and_complete_entry ( entree );
	    else
		texte = _("Limit date");
	    break;
    }

    /* l'entrée était vide, on remet le défaut
       si l'origine était un combofix, il faut remettre le texte 
       avec le gtk_combofix (sinon risque de complétion), donc utiliser l'origine */

    if ( texte )
    {
	switch ( GPOINTER_TO_INT ( no_origine ) )
	{
	    case SCHEDULER_FORM_PARTY :
	    case SCHEDULER_FORM_CATEGORY :
	    case SCHEDULER_FORM_BUDGETARY :
		gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[GPOINTER_TO_INT ( no_origine )] ),
					texte );
		break;

	    default:
		gtk_entry_set_text ( GTK_ENTRY ( entree ), texte );
		break;
	}
	gtk_widget_set_style ( entree, style_entree_formulaire[ENGRIS] );
    }

    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
gboolean clique_champ_formulaire_echeancier ( GtkWidget *entree,
					      GdkEventButton *ev,
					      gint *no_origine )
{
    GtkWidget *popup_cal;
    /* on rend sensitif tout ce qui ne l'était pas sur le formulaire */

    gsb_scheduler_form_set_sensitive (FALSE);

    /* si l'entrée de la date et grise, on met la date courante */

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ) == style_entree_formulaire[ENGRIS] )
    {
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			     gsb_date_today() );

	gtk_widget_set_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE],
			       style_entree_formulaire[ENCLAIR] );
    }

    /* si ev est null ( càd que ça ne vient pas d'un click mais appelé par ex
       à la fin de fin_edition ), on se barre */

    if ( !ev )
	return FALSE;

    /* énumération suivant l'entrée où on clique */

    switch ( GPOINTER_TO_INT ( no_origine ) )
    {
	case SCHEDULER_FORM_DATE :
	case SCHEDULER_FORM_FINAL_DATE :
	    /* click sur l'entrée de la date ou date limite */

	    /* si double click, on popup le calendrier */

	    if ( ev -> type == GDK_2BUTTON_PRESS )
	    {
		popup_cal = gsb_calendar_new ( entree );
	    }
	    break;

	default :

	    break;
    }
    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
gboolean pression_touche_formulaire_echeancier ( GtkWidget *widget,
						 GdkEventKey *ev,
						 gint no_widget )
{
    GtkWidget *popup_cal;

    /* si etat.entree = 1, la touche entrée finit l'opération ( fonction par défaut ) */
    /* sinon elle fait comme tab */

    if ( !etat.entree && ( ev -> keyval == GDK_Return || ev -> keyval == GDK_KP_Enter ))
	ev->keyval = GDK_Tab ;


    switch ( ev -> keyval )
    {

	case GDK_Escape :		/* échap */

	    echap_formulaire_echeancier();
	    return FALSE;

	case GDK_Tab :

	    if ( GTK_IS_ENTRY ( widget ) )
		gtk_entry_select_region ( GTK_ENTRY ( widget ), 0, 0);
	    else
		if ( GTK_IS_COMBOFIX ( widget ) )
		    gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( widget ) -> entry ), 0, 0);

	    /* on fait perdre le focus au widget courant pour faire
	       les changements automatiques si nécessaire */
	    gtk_widget_grab_focus ( gsb_scheduler_list_get_tree_view () );

	    /* on donne le focus au widget suivant */
	    no_widget = ( no_widget + 1 ) % SCHEDULER_FORM_TOTAL_WIDGET;

	    while ( !( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[no_widget] ) &&
		       GTK_WIDGET_SENSITIVE ( widget_formulaire_echeancier[no_widget] ) &&
		       ( GTK_IS_COMBOFIX ( widget_formulaire_echeancier[no_widget] ) ||
			 GTK_IS_ENTRY ( widget_formulaire_echeancier[no_widget] ) ||
			 GTK_IS_BUTTON ( widget_formulaire_echeancier[no_widget] ) )))
		no_widget = ( no_widget + 1 ) % SCHEDULER_FORM_TOTAL_WIDGET;

	    /* si on se retrouve sur la date et que etat.entree = 0,
	       on enregistre l'opération */

	    if ( no_widget == SCHEDULER_FORM_DATE &&
		 etat.entree == 0 )
	    {
		gsb_scheduler_validate_form ();
		return TRUE;
	    }

	    /* si le prochain est le débit, on vérifie s'il n'y a rien dans
	       cette entrée et s'il y a quelque chose dans l'entrée du
	       crédit */

	    if ( no_widget == SCHEDULER_FORM_DEBIT )
	    {
		/* si le débit est gris et le crédit est noir, on met sur le crédit */

		if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENGRIS] &&
		     gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ) == style_entree_formulaire[ENCLAIR] )
		    no_widget = SCHEDULER_FORM_CREDIT;
	    }

	    /* si le prochain est le crédit et que le débit a été rempli,
	       on se met sur la devise et on efface le crédit */

	    if ( no_widget == SCHEDULER_FORM_CREDIT )
		if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
		{
		    no_widget = SCHEDULER_FORM_DEVISE;
		    gtk_widget_set_style (widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT],
					  style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT]),
					 _("Credit") );
		}

	    /* on sélectionne le contenu de la nouvelle entrée */
	    if ( GTK_IS_COMBOFIX ( widget_formulaire_echeancier[no_widget] ) )
	    {
		gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_echeancier[no_widget] ) -> entry );
		gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( widget_formulaire_echeancier[no_widget] ) -> entry ), 0, -1 );
	    }
	    else
	    {
		if ( GTK_IS_ENTRY ( widget_formulaire_echeancier[no_widget] ) )
		    gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_echeancier[no_widget] ), 0, -1 );
		gtk_widget_grab_focus ( widget_formulaire_echeancier[no_widget]  );
	    }
	    return TRUE;

	case GDK_Return :		/* touches entrée */
	case GDK_KP_Enter :

	    if ( !etat.formulaire_echeance_dans_fenetre )
	    {
		/*	 gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
			 "key-press-event");
			 */
		/* si la touche CTRL est elle aussi active, alors c'est que l'on est
		   probablement sur un champ de date et que l'on souhaite ouvrir
		   un calendrier */
		if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
		{
		    switch ( no_widget )
		    {
			case SCHEDULER_FORM_DATE :
			case SCHEDULER_FORM_FINAL_DATE :

			    popup_cal = gsb_calendar_new ( widget );
			    break ;

			default :
			    break ;
		    }
		}
		/* si la touche CTRL n'est pas active, alors on valide simplement
		   la saisie de l'échéance */
		else
		{
		    /* on fait perdre le focus au widget courant pour faire
		       les changements automatiques si nécessaire */

		    gtk_widget_grab_focus ( gsb_scheduler_list_get_tree_view () );
		    gsb_scheduler_validate_form ();
		}
	    }
	    return FALSE;
	    break;

	case GDK_plus :		/* touches + */
	case GDK_KP_Add :

	    /* si on est dans une entree de date, on augmente d'un jour
	       (d'une semaine) la date */

	    if ( no_widget == SCHEDULER_FORM_DATE ||
		 no_widget == SCHEDULER_FORM_FINAL_DATE )
	    {
		gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					       "key-press-event");
		if ( ( ev -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
		    inc_dec_date ( widget_formulaire_echeancier[no_widget], ONE_DAY );
		else
		    inc_dec_date ( widget_formulaire_echeancier[no_widget], ONE_WEEK );
		return TRUE;
	    }
	    return FALSE;

	case GDK_minus :		/* touches - */
	case GDK_KP_Subtract :

	    /* si on est dans une entree de date, on diminue d'un jour
	       (d'une semaine) la date */

	    if ( no_widget == SCHEDULER_FORM_DATE ||
		 no_widget == SCHEDULER_FORM_FINAL_DATE )
	    {
		gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					       "key-press-event");
		if ( ( ev -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
		    inc_dec_date ( widget_formulaire_echeancier[no_widget], - ONE_DAY );
		else
		    inc_dec_date ( widget_formulaire_echeancier[no_widget], - ONE_WEEK );
		return TRUE;
	    }
	    return FALSE;

	case GDK_Page_Up :		/* touche PgUp */
	case GDK_KP_Page_Up :

	    /* si on est dans une entree de date,
	       on augmente d'un mois (d'un an) la date */

	    if ( no_widget == SCHEDULER_FORM_DATE ||
		 no_widget == SCHEDULER_FORM_FINAL_DATE )
	    {
		gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					       "key-press-event");
		if ( ( ev -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
		    inc_dec_date ( widget_formulaire_echeancier[no_widget], ONE_MONTH );
		else
		    inc_dec_date ( widget_formulaire_echeancier[no_widget], ONE_YEAR );
		return TRUE;
	    }
	    return FALSE;

	case GDK_Page_Down :		/* touche PgDown */
	case GDK_KP_Page_Down :

	    /* si on est dans une entree de date,
	       on augmente d'un mois (d'un an) la date */

	    if ( no_widget == SCHEDULER_FORM_DATE ||
		 no_widget == SCHEDULER_FORM_FINAL_DATE )
	    {
		gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					       "key-press-event");
		if ( ( ev -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
		    inc_dec_date ( widget_formulaire_echeancier[no_widget], - ONE_MONTH );
		else
		    inc_dec_date ( widget_formulaire_echeancier[no_widget], - ONE_YEAR );
		return TRUE;
	    }
	    return FALSE;

	default:
	    /* Reverting to default handler */
	    return FALSE;
    }
    return TRUE;
}
/******************************************************************************/

/**
 * we come here when we validate a scheduled transaction
 * either modify or create a new scheduled transaction
 * either create a new transaction from the scheduled transaction
 * 
 * \param
 * 
 * \return
 * */
void gsb_scheduler_validate_form ( void )
{
    gint scheduled_number;

    devel_debug ( "gsb_scheduler_validate_form" );

    if ( !gsb_scheduler_check_form ())
    {
	notice_debug ( "gsb_scheduler_check_form didn't validate the form" );
	return;
    }

    /* get the scheduled transaction, will be 0 if the user entered directly in the form
     * and < 0 for a white line */

    scheduled_number = GPOINTER_TO_INT (gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
							      "scheduled_number" ));
    if ( !scheduled_number )
	scheduled_number = -1;

    /* split if we execute a scheduled transaction or just edit/modify it */

    if ( g_object_get_data ( G_OBJECT ( formulaire_echeancier ),
				       "execute_transaction"))
    {
	/* we execute a scheduled transaction */
	gint transaction_number;

	/* check that we are not in child or white line,
	 * normally impossible because the button execute transaction should be unsensitive,
	 * but never now... */

	if ( scheduled_number < 0
	     ||
	     gsb_data_scheduled_get_mother_scheduled_number (scheduled_number))
	{
	    warning_debug ( "gsb_scheduler_validate_form has been called for a white line or a child of breakdown.\nThis shouldn't happen. Please find how you get that message and tell it to the grisbi team.\nThanks." );
	    return;
	}

	/* create the transaction */
	
	transaction_number = gsb_scheduler_create_transaction_from_scheduled_form ();

	/* if the transaction is a breakdown, we'll append the children now */

	if ( gsb_data_transaction_get_breakdown_of_transaction ( transaction_number ))
	    gsb_scheduler_execute_children_of_scheduled_transaction ( scheduled_number,
								      transaction_number );

	/* the scheduled transaction goes to the next date */

	if ( !gsb_scheduler_increase_scheduled_transaction (scheduled_number))
	{
	    /* that scheduled transaction is finished, we set the number to 0 */

	    gsb_scheduler_list_remove_transaction_from_list ( scheduled_number );
	    scheduled_number = 0;
	}

	/* réaffiche les option menu de la périodicité, des banques et de l'automatisme, effacés pour une saisie d'opé */

	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] );
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] );
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] );
	update_transaction_in_trees (transaction_number);    
    }
    else
    {
	/* it's a modification/new scheduled transaction */

	scheduled_number = gsb_scheduler_create_scheduled_transaction_from_scheduled_form (scheduled_number);
    }

    /* now scheduled_number contains
     * either the number of the modified scheduled transaction
     * either the number of the new scheduled transaction
     * either 0 if it was an execution and the scheduled transaction is finished */
    
    if ( scheduled_number )
	gsb_scheduler_list_update_transaction_in_list (scheduled_number);

    gsb_scheduler_list_set_background_color (gsb_scheduler_list_get_tree_view ());

    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ ();

    formulaire_echeancier_a_zero ();

    if ( !etat.formulaire_echeancier_toujours_affiche )
	gtk_expander_set_expanded ( GTK_EXPANDER(frame_formulaire_echeancier), FALSE );

    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_liste_echeances_auto_accueil = 1;

    gtk_widget_grab_focus ( gsb_scheduler_list_get_tree_view () );

    mise_a_jour_accueil(FALSE);
    modification_fichier ( TRUE );
}


/**
 * check if the scheduled transaction in the schedule's form is ok to continue
 * 
 * \param
 * 
 * \return TRUE if ok, FALSE if pb
 * */
gboolean gsb_scheduler_check_form ( void )
{
    devel_debug ("gsb_scheduler_check_form");

    /* check the date */

    if ( !gsb_date_check_and_complete_entry ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ))
    {
	dialogue_error_hint ( _("Grisbi can't parse date.  For a list of date formats that Grisbi can use, refer to Grisbi manual."),
			      g_strdup_printf ( _("Invalid date '%s'"), 
						gtk_entry_get_text ( GTK_ENTRY(widget_formulaire_echeancier[SCHEDULER_FORM_DATE]) ) ) );
	gtk_widget_grab_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );
	gtk_entry_select_region ( GTK_ENTRY (  widget_formulaire_echeancier[SCHEDULER_FORM_DATE]),
				  0,
				  -1);
	return FALSE;
    }

    /* check the limit date */

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ) == style_entree_formulaire[ENCLAIR]
	 &&
	 !gsb_date_check_and_complete_entry ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ))
    {
	dialogue_error_hint ( _("Grisbi can't parse date.  For a list of date formats that Grisbi can use, refer to Grisbi manual."),
			      g_strdup_printf ( _("Invalid final date '%s'"), 
						gtk_entry_get_text ( GTK_ENTRY(widget_formulaire_echeancier[SCHEDULER_FORM_DATE]) ) ) );
	gtk_widget_grab_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
	gtk_entry_select_region ( GTK_ENTRY (  widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE]),
				  0,
				  -1);
	return FALSE;
    }

    /* check if it's not a transfer on itself */

    if ( !g_strcasecmp ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] )),
			 g_strconcat ( COLON(_("Transfer")),
				       GTK_LABEL (GTK_BIN ( widget_formulaire_echeancier[5]  )  -> child ) -> label,
				       NULL )))
    {
	dialogue_error_hint ( _("Transfers can't be associated with their own account.  Please associate another account with this transfer."),
			      _("Invalid transfer."));
	return FALSE;
    }

    /* if it's a transfer, check that the account exists */

    if ( !g_strncasecmp ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] )),
			  _("Transfer"),
			  8 ))
    {
	gchar **tab_char;

	tab_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] )),
				":",
				2 );

	/* check if there is an account */

	if ( !tab_char[1]
	     ||
	     !strlen ( tab_char[1] ) )
	{
	    dialogue_error_hint ( _("Transfer is not associated with an account.  Please associate a valid account with this transfer."),
				  _("Invalid transfer.") );
	    return FALSE;
	}

	/* check if the account exists */

	if ( tab_char[1]
	     &&
	     gsb_data_account_get_no_account_by_name ( g_strstrip (tab_char[1])) == -1 )
	{
	    dialogue_error_hint ( _("Associated account of this transfer does not exist.  Please associate a valid account with this transfer."),
				  _("Invalid transfer.") );
	    g_strfreev ( tab_char );
	    return FALSE;
	}
	g_strfreev ( tab_char );
    }

    return TRUE;
}



/** 
 * create a new transaction and fill it from the scheduled form
 * don't mind if the transaction is a breakdown, the children will be appened later
 * 
 * \param 
 * 
 * \return the number of the new transaction
 * */
gint gsb_scheduler_create_transaction_from_scheduled_form ( void )
{
    gint transaction_number;

    transaction_number = gsb_data_transaction_new_transaction ( recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT]));

    /* get the date */
    gsb_data_transaction_set_date ( transaction_number,
				    gsb_parse_date_string ( gtk_entry_get_text ( GTK_ENTRY (GTK_ENTRY (widget_formulaire_echeancier[SCHEDULER_FORM_DATE])))));

    /* get the payee */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	gsb_data_transaction_set_party_number ( transaction_number,
						gsb_data_payee_get_number_by_name ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] )),
										    TRUE ));

    /* get the amount */

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	gsb_data_transaction_set_amount ( transaction_number,
					  gsb_real_opposite (gsb_utils_edit_calculate_entry (widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT])));
    else
	gsb_data_transaction_set_amount ( transaction_number,
					  gsb_utils_edit_calculate_entry (widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT]));

    /* get the currency */

    gsb_data_transaction_set_currency_number ( transaction_number,
					       gsb_currency_get_currency_from_combobox (widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE]));

    gsb_currency_check_for_change ( transaction_number );

    /* get the method of payment of the transaction */

    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ))
    {
	gsb_data_transaction_set_method_of_payment_number ( transaction_number,
							    gsb_payment_method_get_selected_number (widget_formulaire_echeancier[SCHEDULER_FORM_TYPE]));

	if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] )
	     &&
	     gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ) == style_entree_formulaire[ENCLAIR] )
	{
	    struct struct_type_ope *type;

	    type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
					 "adr_type" );

	    gsb_data_transaction_set_method_of_payment_content ( transaction_number,
								 gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE])));
	    if ( type -> numerotation_auto )
		type -> no_en_cours = utils_str_atoi ( gsb_data_transaction_get_method_of_payment_content (transaction_number));
	}
    }

    /* get the financial year */
    /* if no_exercice is -2 : it's automatic
     * if -1 : it's not showed */

    switch (gsb_financial_year_get_number_from_option_menu (widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE]))
    {
	case -2:
	    gsb_data_transaction_set_financial_year_number ( transaction_number,
							     recherche_exo_correspondant ( gsb_data_transaction_get_date (transaction_number)));

	    break;

	case -1:
	    gsb_data_transaction_set_financial_year_number ( transaction_number,
							     0 );

	    break;

	default:
	    gsb_data_transaction_set_financial_year_number ( transaction_number,
							     gsb_financial_year_get_number_from_option_menu (widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE]));
    }

    /* get the budget */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	gchar **tab_char;

	tab_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] )),
				":",
				2 );

	tab_char[0] = g_strstrip ( tab_char[0] );

	if ( tab_char[1] )
	    tab_char[1] = g_strstrip ( tab_char[1] );

	if ( strlen ( tab_char[0] ) )
	{
	    gint budget_number;

	    budget_number = gsb_data_budget_get_number_by_name ( tab_char[0],
								 TRUE,
								 gsb_data_transaction_get_amount (transaction_number).mantissa < 0 );
	    gsb_data_transaction_set_budgetary_number ( transaction_number,
							budget_number );
	    gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
							    gsb_data_budget_get_sub_budget_number_by_name ( budget_number,
													    tab_char[1],
													    TRUE ));
	}
	g_strfreev ( tab_char );
    }

    /* get the automatic mode */

    gsb_data_transaction_set_automatic_transaction ( transaction_number,
						     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE]  ) -> menu_item ),
											     "auto_man" )));
    /* get the notes */

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ) == style_entree_formulaire[ENCLAIR] )
	gsb_data_transaction_set_notes ( transaction_number,
					 gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] )));

    /* get the category */

    gsb_scheduler_get_category_for_transaction_from_form ( transaction_number );

    /* we show the new transaction in the tree view */

    gsb_transactions_list_append_new_transaction (transaction_number);

    return transaction_number;
}


/**
 * create or modify the scheduled transaction from the scheduled form
 *
 * \param scheduled_number can be <0 if it's a white line, and <-1 for breakdown white line
 *
 * \return the scheduled number, the new one if created, or the same as given in param
 * */
gint gsb_scheduler_create_scheduled_transaction_from_scheduled_form ( gint scheduled_number )
{
    gchar **tab_char;

    /* if it's a new scheduled transaction, create it and append the mother
     * if it's a breakdown child */

    if ( scheduled_number < 0 )
    {
	gint mother_number;

	mother_number = gsb_data_scheduled_get_mother_scheduled_number (scheduled_number);
	scheduled_number = gsb_data_scheduled_new_scheduled ();
	gsb_data_scheduled_set_mother_scheduled_number ( scheduled_number,
							 mother_number );
    }

    /* get the date */

    gsb_data_scheduled_set_date ( scheduled_number,
				  gsb_parse_date_string (gtk_entry_get_text ( GTK_ENTRY (widget_formulaire_echeancier[SCHEDULER_FORM_DATE]))));

    /* get the payee */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	gsb_data_scheduled_set_party_number ( scheduled_number,
					      gsb_data_payee_get_number_by_name ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY])),
										  TRUE ));

    /* get the amount */

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	gsb_data_scheduled_set_amount ( scheduled_number,
					gsb_real_opposite (gsb_utils_edit_calculate_entry (widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT])));
    else
	gsb_data_scheduled_set_amount ( scheduled_number,
					gsb_utils_edit_calculate_entry (widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT]));

    /* get the currency */

    gsb_data_scheduled_set_currency_number ( scheduled_number,
					     gsb_currency_get_currency_from_combobox (widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE]));

    /* get the account number */

    gsb_data_scheduled_set_account_number ( scheduled_number,
					    recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT]));

    /* get the method of payment */

    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ))
    {
	gsb_data_scheduled_set_method_of_payment_number ( scheduled_number,
							  gsb_payment_method_get_selected_number (widget_formulaire_echeancier[SCHEDULER_FORM_TYPE]));

	if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] )
	     &&
	     gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ) == style_entree_formulaire[ENCLAIR] )
	    gsb_data_scheduled_set_method_of_payment_content ( scheduled_number,
							       gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] )));
    }

    /* get the financial year */

    gsb_data_scheduled_set_financial_year_number ( scheduled_number,
						   gsb_financial_year_get_number_from_option_menu (widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE]));

    /* get the budget */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	tab_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] )),
				":",
				2 );

	tab_char[0] = g_strstrip ( tab_char[0] );

	if ( tab_char[1] )
	    tab_char[1] = g_strstrip ( tab_char[1] );

	if ( strlen ( tab_char[0] ) )
	{
	    gint budget_number;

	    budget_number = gsb_data_budget_get_number_by_name ( tab_char[0],
								 TRUE,
								 gsb_data_scheduled_get_amount (scheduled_number).mantissa < 0 );
	    gsb_data_scheduled_set_budgetary_number ( scheduled_number,
						      budget_number );
	    gsb_data_scheduled_set_sub_budgetary_number ( scheduled_number,
							  gsb_data_budget_get_sub_budget_number_by_name ( budget_number,
													  tab_char[1],
													  TRUE ));
	}
	g_strfreev ( tab_char );
    }

    /* get the automatic mode */

    gsb_data_scheduled_set_automatic_scheduled ( scheduled_number,
						 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE]  ) -> menu_item ),
											 "auto_man" )));

    /* get the notes */

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ) == style_entree_formulaire[ENCLAIR] )
	gsb_data_scheduled_set_notes ( scheduled_number,
				       gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] )));


    /* get the categories,
     * if no categories, neither breakdown and not a transfer, set the account_transfer to -1 */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	tab_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] )),
				":",
				2 );

	tab_char[0] = g_strstrip ( tab_char[0] );

	if ( strlen ( tab_char[0] ) )
	{
	    if ( tab_char[1] )
		tab_char[1] = g_strstrip ( tab_char[1] );

	    if ( !strcmp ( tab_char[0],
			   _("Transfer") )
		 && tab_char[1]
		 && strlen ( tab_char[1]) )
	    {
		/* it's a transfer,
		 * so no category and no breakdown */

		gsb_data_scheduled_set_category_number ( scheduled_number,
							 0 );
		gsb_data_scheduled_set_sub_category_number ( scheduled_number,
							     0 );
		gsb_data_scheduled_set_breakdown_of_scheduled ( scheduled_number,
								0 );

		/* set the account transfer number */

		gsb_data_scheduled_set_account_number_transfer ( scheduled_number,
								 gsb_data_account_get_no_account_by_name ( tab_char[1] ));
	    }
	    else
	    {
		/* not a transfer, check for breakdown */

		if ( !strcmp ( tab_char[0],
			       _("Breakdown of transaction") ))
		{
		    /* it's a breakdown */

		    gsb_data_scheduled_set_category_number ( scheduled_number,
							     0 );
		    gsb_data_scheduled_set_sub_category_number ( scheduled_number,
								 0 );
		    gsb_data_scheduled_set_breakdown_of_scheduled ( scheduled_number,
								    1 );
		    gsb_data_scheduled_set_account_number_transfer ( scheduled_number,
								     0 );
		}
		else
		{
		    /* it's a normal category */

		    gint category_number;

		    category_number = gsb_data_category_get_number_by_name ( tab_char[0],
									     TRUE,
									     gsb_data_scheduled_get_amount (scheduled_number).mantissa < 0);
		    gsb_data_scheduled_set_category_number ( scheduled_number,
							     category_number);
		    gsb_data_scheduled_set_sub_category_number ( scheduled_number,
								 gsb_data_category_get_sub_category_number_by_name ( category_number,
														     tab_char[1],
														     TRUE ));
		    gsb_data_scheduled_set_breakdown_of_scheduled ( scheduled_number,
								    0 );
		    gsb_data_scheduled_set_account_number_transfer ( scheduled_number,
								     0 );
		}
	    }
	}
	g_strfreev ( tab_char );
    }
    else
	gsb_data_scheduled_set_account_number_transfer ( scheduled_number,
							 -1 );

    /* get the frequency */

    gsb_data_scheduled_set_frequency ( scheduled_number,
				       GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY]  ) -> menu_item ),
									       "periodicite" )));

    if ( gsb_data_scheduled_get_frequency (scheduled_number) == 4 )
    {
	gsb_data_scheduled_set_user_interval ( scheduled_number,
					       GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU]  ) -> menu_item ),
										       "intervalle_perso" )));
	gsb_data_scheduled_set_user_entry ( scheduled_number,
					    my_strtod (gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] )),
						       NULL ));
    }

    if ( gsb_data_scheduled_get_frequency (scheduled_number)
	 &&
	 gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ) == style_entree_formulaire[ENCLAIR] )
    {
	/* set the limit date */

	gsb_data_scheduled_set_limit_date ( scheduled_number,
					    gsb_parse_date_string ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE]))));
    }
    
    return scheduled_number;
}



/**
 * used to catch a transaction from the scheduled form
 * take the category, check if it's a transfer or a breakdown and
 * do the necessary (create contra-transaction, take the children's transactions
 * 
 * \param transaction_number
 * 
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_scheduler_get_category_for_transaction_from_form ( gint transaction_number )
{
    GtkWidget *category_entry;
    gchar *char_ptr;

    category_entry = GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry;

    if ( gtk_widget_get_style ( category_entry ) == style_entree_formulaire[ENCLAIR] )
    {
	char_ptr = g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( category_entry ))));

	/* we check first if it's a breakdown, next a transfer and after a normal category
	 * each time we needn't to set the differents variables to 0 because here it's always
	 * a new transaction, so all is at 0 */

	if ( !strcmp ( char_ptr,
		       _("Breakdown of transaction") ))
	{
	    /* it's a breakdown of transaction,
	     * we don't get the children now, it will be done later */

	    gsb_data_transaction_set_breakdown_of_transaction ( transaction_number,
								1 );
	}
	else
	{
	    /* it's not a breakdown of transaction */
	    /* check if it's a transfer or a normal category */

	    gchar **tab_char;

	    tab_char = g_strsplit ( char_ptr,
				    ":",
				    2 );

	    tab_char[0] = g_strstrip ( tab_char[0] );

	    if ( tab_char[1] )
		tab_char[1] = g_strstrip ( tab_char[1] );

	    if ( strlen ( tab_char[0] ) )
	    {
		if ( !strcmp ( tab_char[0],
			       _("Transfer") )
		     &&
		     tab_char[1]
		     &&
		     strlen ( tab_char[1] ) )
		{
		    /* it's a transfert */

/* 		    gsb_form_validate_transfer ( transaction_number, */
/* 						 1, */
/* 						 tab_char[1] ); */
		}
		else
		{
		    /* it's a normal category */

		    gint category_number;

		    category_number = gsb_data_category_get_number_by_name ( tab_char[0],
									     TRUE,
									     gsb_data_transaction_get_amount (transaction_number).mantissa < 0 );
		    gsb_data_transaction_set_category_number ( transaction_number,
							       category_number );

		    gsb_data_transaction_set_sub_category_number ( transaction_number,
								   gsb_data_category_get_sub_category_number_by_name ( category_number,
														       tab_char[1],
														       TRUE ));
		}
	    }
	    g_strfreev ( tab_char );
	}
    }
    return TRUE;
}


/**
 * create a new transaction and fill it from a scheduled transaction
 * if it's a child of breakdown, append it automatickly to the mother
 * 
 * \param scheduled_number the transaction we use to fill the new transaction
 * \param transaction_mother the number of the mother if it's a breakdown child, 0 else
 *
 * \return the number of the new transaction
 * */
gint gsb_scheduler_create_transaction_from_scheduled_transaction ( gint scheduled_number,
								   gint transaction_mother )
{
    gint transaction_number, payment_method;

    transaction_number = gsb_data_transaction_new_transaction (gsb_data_scheduled_get_account_number (scheduled_number));

    /* begin to fill the new transaction */

    gsb_data_transaction_set_date ( transaction_number,
				    gsb_date_copy (gsb_data_scheduled_get_date (scheduled_number)));
    gsb_data_transaction_set_party_number ( transaction_number,
					    gsb_data_scheduled_get_party_number (scheduled_number));
    gsb_data_transaction_set_amount ( transaction_number,
				      gsb_data_scheduled_get_amount (scheduled_number));
    gsb_data_transaction_set_currency_number ( transaction_number,
					       gsb_data_scheduled_get_currency_number (scheduled_number));
    gsb_currency_check_for_change ( transaction_number );
    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
							gsb_data_scheduled_get_method_of_payment_number (scheduled_number));
    gsb_data_transaction_set_notes ( transaction_number,
				     gsb_data_scheduled_get_notes (scheduled_number));

    payment_method = gsb_data_scheduled_get_method_of_payment_number (scheduled_number);
    if ( payment_method )
    {
	struct struct_type_ope *type_ope = type_ope_par_no ( payment_method,
							     gsb_data_scheduled_get_account_number (scheduled_number));
	if ( type_ope -> affiche_entree )
	{
	    if ( type_ope -> numerotation_auto )
	    {
/* 		gsb_data_transaction_set_method_of_payment_content ( transaction_number, */
/* 								     automatic_numbering_get_new_number ( type_ope )); */
		type_ope -> no_en_cours ++;
	    }
	    else
		gsb_data_transaction_set_method_of_payment_content ( transaction_number,
								     gsb_data_scheduled_get_method_of_payment_content (scheduled_number) );
	}
    }
    else
    {
	gsb_data_transaction_set_method_of_payment_content ( transaction_number,
							     gsb_data_scheduled_get_method_of_payment_content (scheduled_number) );
    }
    gsb_data_transaction_set_automatic_transaction ( transaction_number,
						     gsb_data_scheduled_get_automatic_scheduled (scheduled_number));
    gsb_data_transaction_set_budgetary_number ( transaction_number,
						gsb_data_scheduled_get_budgetary_number (scheduled_number));
    gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
						    gsb_data_scheduled_get_sub_budgetary_number (scheduled_number));

    /* if the financial year is automatic, we set it here */

    if ( gsb_data_scheduled_get_financial_year_number (scheduled_number) == -2 )
	gsb_data_transaction_set_financial_year_number ( transaction_number,
							 recherche_exo_correspondant ( gsb_data_transaction_get_date (transaction_number)));
    else
	gsb_data_transaction_set_financial_year_number ( transaction_number,
							 gsb_data_scheduled_get_financial_year_number (scheduled_number));

    /* get the category */

    gsb_scheduler_get_category_for_transaction_from_transaction ( transaction_number,
								  scheduled_number );

     /* set the mother breakdown if exists */

    gsb_data_transaction_set_mother_transaction_number ( transaction_number,
							 transaction_mother );

    /* we show the new transaction in the tree view */

    gsb_transactions_list_append_new_transaction (transaction_number);

    return transaction_number;
}


/**
 * used to catch a transaction from a scheduled transaction
 * take the category, check if it's a transfer or a breakdown and
 * do the necessary (create contra-transaction)
 * don't execute the children if it's a breakdown, need to call 
 * gsb_scheduler_execute_children_of_scheduled_transaction later
 * 
 * \param transaction_number
 * \param scheduled_number
 *
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_scheduler_get_category_for_transaction_from_transaction ( gint transaction_number,
								       gint scheduled_number )
{
    /* if category is set, it's a normal category */

    if ( gsb_data_scheduled_get_category_number (scheduled_number))
    {
	/* it's a normal category */

	gsb_data_transaction_set_category_number ( transaction_number,
						   gsb_data_scheduled_get_category_number (scheduled_number));
	gsb_data_transaction_set_sub_category_number ( transaction_number,
						       gsb_data_scheduled_get_sub_category_number (scheduled_number));
	return TRUE;
    }

    if ( gsb_data_scheduled_get_breakdown_of_scheduled (scheduled_number))
    {
	/* it's a breakdown of transaction,
	 * we don't append the children here, we need to call later
	 * the function gsb_scheduler_execute_children_of_scheduled_transaction */

	gsb_data_transaction_set_breakdown_of_transaction ( transaction_number,
							    1 );
    }
    else
    {
	/* it's not a breakdown of transaction and not a normal category so it's a transfer
	 * except if the target account is -1 then it's a
	 * transaction with no category */

	if ( gsb_data_scheduled_get_account_number_transfer (scheduled_number) != -1 )
	{
	    gint contra_transaction_number;

	    contra_transaction_number = gsb_form_validate_transfer ( transaction_number,
								     TRUE,
								     gsb_data_scheduled_get_account_number_transfer (scheduled_number));
	    gsb_data_transaction_set_method_of_payment_number ( contra_transaction_number,
								gsb_data_scheduled_get_contra_method_of_payment_number (scheduled_number));
	}
    }
    return TRUE;
}


/**
 * get the children of a breakdown scheduled transaction,
 * make the transactions from them and append them to the transactions list
 *
 * \param scheduled_number the number of the mother scheduled transaction (the breakdown)
 * \param transaction_number the number of the transaction created from that scheduled (so, the future mother of the children)
 *
 * \return FALSE
 * 
 * */
gboolean gsb_scheduler_execute_children_of_scheduled_transaction ( gint scheduled_number,
								   gint transaction_number )
{
    GSList *children_numbers_list;

    children_numbers_list = gsb_data_scheduled_get_children ( scheduled_number );

    while ( children_numbers_list )
    {
	gint child_number;

	child_number = GPOINTER_TO_INT ( children_numbers_list -> data );

	gsb_scheduler_create_transaction_from_scheduled_transaction ( child_number,
								      transaction_number );

	children_numbers_list = children_numbers_list -> next;
    }
    g_slist_free (children_numbers_list);
    return FALSE;
}




/******************************************************************************/
void formulaire_echeancier_a_zero ( void )
{
    /* on met les styles des entrées au gris */

    gtk_widget_set_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry,
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry,
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry,
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE],
			   style_entree_formulaire[ENGRIS] );

    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			 _("Date") );
    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ),
			    _("Payee") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
			 _("Debit") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
			 _("Credit") );
    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
			    _("Categories : Sub-categories") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
			 _("Transfer reference") );
    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
			    _("Budgetary line") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER] ),
			 _("Voucher") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			 _("Notes") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ),
			 _("Limit date") );

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ),
				  0 );
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ),
				  0 );
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] ),
				  0 );
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] ),
				  2 );
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] ),
				  0 );
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
				  0 );
    changement_choix_compte_echeancier ();


    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_BREAKDOWN] );
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] );


    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE],
			       FALSE );
    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT],
			       FALSE );
    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE],
			       FALSE );
    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE],
			       FALSE );
    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE],
			       FALSE );
    gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY],
			       FALSE );
    gtk_widget_set_sensitive ( hbox_valider_annuler_echeance,
			       FALSE );

    gtk_widget_hide ( label_saisie_modif );
    gtk_label_set_text ( GTK_LABEL ( label_saisie_modif ),
			 _("Modification") );

    /* réaffiche les boutons effacés pour une saisie */

    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] );
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] );

    /* set the scheduled_number of the form to NULL */

    g_object_set_data ( G_OBJECT ( formulaire_echeancier ),
			"scheduled_number",
			NULL );
    
    /* by default, it's an edition of the scheduled transaction */

    g_object_set_data ( G_OBJECT ( formulaire_echeancier ),
			"execute_transaction",
			NULL );
}
/******************************************************************************/

/**
 * set the next date in the scheduled transaction
 * if it's above the limit date, that transaction is deleted
 * if it's a breakdown, the children are updated too
 * 
 * \param scheduled_number the scheduled transaction we want to increase
 * 
 * \return FALSE if the scheduled transaction is finished, TRUE else
 * */
gboolean gsb_scheduler_increase_scheduled_transaction ( gint scheduled_number )
{
    g_return_val_if_fail ( g_date_valid (gsb_data_scheduled_get_date (scheduled_number)), TRUE );

    /* increase the date of the scheduled_transaction */

    gsb_scheduler_increase_date ( scheduled_number,
				  gsb_data_scheduled_get_date (scheduled_number));

    if ( gsb_data_scheduled_get_breakdown_of_scheduled ( scheduled_number ))
    {
	GSList *children_numbers_list;

	children_numbers_list = gsb_data_scheduled_get_children ( scheduled_number );

	while ( children_numbers_list )
	{
	    gint child_number;

	    child_number = GPOINTER_TO_INT ( children_numbers_list -> data );

	    gsb_data_scheduled_set_date ( child_number,
					  gsb_date_copy (gsb_data_scheduled_get_date (scheduled_number)));

	    children_numbers_list = children_numbers_list -> next;
	}
	g_slist_free (children_numbers_list);
    }

    /* if the scheduled transaction is over, we remove it */

    if ( main_page_finished_scheduled_transactions_part
	 && 
	 ( !gsb_data_scheduled_get_frequency (scheduled_number)
	   ||
	   (
	    gsb_data_scheduled_get_limit_date (scheduled_number)
	    &&
	    g_date_compare ( gsb_data_scheduled_get_date (scheduled_number),
			     gsb_data_scheduled_get_limit_date (scheduled_number)) > 0  )) )
    {
	/* update the main page */

	gsb_main_page_update_finished_scheduled_transactions (scheduled_number);
	gsb_scheduler_list_delete_scheduled_transaction (scheduled_number);
	return FALSE;
    }
    return TRUE;
}


/**
 * increase the date given in param with what is describe
 * in the scheduled transaction (it doen't do any copy of the date !)
 * 
 * \param scheduled_number it contains the parameters to increase the date
 * \param date a GDate which will be increase
 * 
 * \return FALSE
 * */
gboolean gsb_scheduler_increase_date ( gint scheduled_number,
				       GDate *date )
{
    switch ( gsb_data_scheduled_get_frequency (scheduled_number))
    {
	case SCHEDULER_PERIODICITY_WEEK_VIEW:
	    g_date_add_days ( date,
			      7 );

	    /* need to add 0 month to work... */
	    /* FIXME check if the bug in g_date_add_days is still present */

	    g_date_add_months ( date,
				0 );
	    break;

	case SCHEDULER_PERIODICITY_MONTH_VIEW:
	    g_date_add_months ( date,
				1 );
	    break;

	case SCHEDULER_PERIODICITY_TWO_MONTHS_VIEW:
	    g_date_add_months ( date,
				2 );
	    break;

	case SCHEDULER_PERIODICITY_TRIMESTER_VIEW:
	    g_date_add_months ( date,
				3 );
	    break;

	case SCHEDULER_PERIODICITY_YEAR_VIEW:
	    g_date_add_years ( date,
			       1 );

	case SCHEDULER_PERIODICITY_CUSTOM_VIEW:
	    /* set default here because sometimes the periodicity can be more than 4... 
	     * FIXME to check why... if it was a bug, change that
	     * here it's a personnal periodicity of 3 monthes which have here 6 instead of 4 */
	    /* c'est corrigé lors de l'import mais ça serait pas mal de retrouver le bug... */
	default:

	    switch ( gsb_data_scheduled_get_user_interval (scheduled_number) )
	    {
		case PERIODICITY_DAYS:
		    g_date_add_days ( date,
				      gsb_data_scheduled_get_user_entry (scheduled_number));

		    /* magouille car il semble y avoir un bug dans g_date_add_days qui ne fait pas l'addition si on ne met pas la ligne suivante */
		    /* FIXME check if the bug in g_date_add_days is still present */
		    g_date_add_months ( date,
					0 );
		    break;

		case PERIODICITY_WEEKS:
		    g_date_add_days ( date,
				      gsb_data_scheduled_get_user_entry (scheduled_number) * 7);
		    g_date_add_months ( date,
					0 );
		    break;

		case PERIODICITY_MONTHS:
		    g_date_add_months ( date,
					gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;

		case PERIODICITY_YEARS:
		    g_date_add_years ( date,
				       gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;
	    }    
    }
    return FALSE;
}



/**
 * set the form sensitive, depend if it's a breakdown child or normal scheduled transaction
 *
 * \param breakdown_child
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_form_set_sensitive ( gboolean breakdown_child )
{
    if ( breakdown_child )
    {
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU],
				   FALSE );
    }
    else
    {
	/* here it's only for real transactions, not children */
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE],
				   TRUE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY],
				   TRUE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE],
				   TRUE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT],
				   TRUE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE],
				   TRUE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE],
				   TRUE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE],
				   TRUE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY],
				   TRUE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE],
				   TRUE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB],
				   TRUE );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU],
				   TRUE );
    }

    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ), TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( hbox_valider_annuler_echeance ), TRUE );
    gtk_widget_show ( label_saisie_modif );

    return FALSE;
}




/******************************************************************************/
/* Fonction basculer_vers_ventilation                                         */
/* appelée par l'appui du bouton Ventilation...                               */
/* permet de voir les opés ventilées d'une ventilation                        */
/******************************************************************************/
void basculer_vers_ventilation_echeances ( void )
{
    /* FIXME : to remove */
}
/******************************************************************************/



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
