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
#include "utils_editables.h"
#include "utils_categories.h"
#include "comptes_traitements.h"
#include "utils_exercices.h"
#include "type_operations.h"
#include "classement_echeances.h"
#include "devises.h"
#include "utils_devises.h"
#include "dialog.h"
#include "operations_formulaire.h"
#include "gsb_account.h"
#include "calendar.h"
#include "utils_dates.h"
#include "accueil.h"
#include "echeancier_liste.h"
#include "gsb_transaction_data.h"
#include "gtk_combofix.h"
#include "utils_ib.h"
#include "main.h"
#include "categories_onglet.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils_comptes.h"
#include "operations_liste.h"
#include "utils_tiers.h"
#include "utils_operations.h"
#include "echeancier_ventilation.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void affiche_date_limite_echeancier ( void );
static void affiche_personnalisation_echeancier ( void );
static void basculer_vers_ventilation_echeances ( void );
static void cache_date_limite_echeancier ( void );
static void cache_personnalisation_echeancier ( void );
static gboolean clique_champ_formulaire_echeancier ( GtkWidget *entree,
					      GdkEventButton *ev,
					      gint *no_origine );
static void completion_operation_par_tiers_echeancier ( void );
static void echap_formulaire_echeancier ( void );
static gboolean entree_perd_focus_echeancier ( GtkWidget *entree,
					GdkEventFocus *ev,
					gint *no_origine );
static gboolean gsb_scheduler_check_form ( void );
static gint gsb_scheduler_create_transaction_from_scheduled_form ( void );
static gboolean gsb_scheduler_get_category_for_transaction_from_form ( gint transaction_number );
static gboolean gsb_scheduler_get_category_for_transaction_from_transaction ( gint transaction_number,
								       struct operation_echeance *scheduled_transaction );
static gboolean gsb_scheduler_increase_date ( struct operation_echeance *scheduled_transaction,
				       GDate *date );
static gboolean pression_touche_formulaire_echeancier ( GtkWidget *widget,
						 GdkEventKey *ev,
						 gint no_widget );
/*END_STATIC*/



GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
GtkWidget *label_saisie_modif;
GSList *liste_categories_ventilation_combofix;        /*  liste des noms des categ et sous categ pour le combofix */
GtkWidget *separateur_formulaire_echeancier;
GtkWidget *hbox_valider_annuler_echeance;


/*START_EXTERN*/
extern gint enregistre_ope_au_retour_echeances;
extern GtkWidget *formulaire;
extern GtkWidget *formulaire_echeancier;
extern GtkWidget *formulaire_echeancier;
extern GtkWidget *frame_formulaire_echeancier;
extern GSList *liste_categories_combofix;
extern GSList *liste_imputations_combofix;
extern GSList *liste_struct_devises;
extern GSList *liste_struct_echeances;
extern GSList *liste_tiers_combofix_echeancier;
extern GtkWidget *main_page_finished_scheduled_transactions_part;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint nb_echeances;
extern gint no_derniere_echeance;
extern FILE * out;
extern GtkStyle *style_entree_formulaire[2];
extern GtkWidget *tree_view_liste_echeances;
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
			 GTK_SIGNAL_FUNC ( entree_prend_focus ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			 "focus-out-event",
			 GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			 NULL );
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );

    /* création du combofix des tiers */
    widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] = gtk_combofix_new (  liste_tiers_combofix_echeancier,
									     FALSE,
									     TRUE,
									     TRUE,
									     30 );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_PARTY ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_PARTY ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> arrow ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_PARTY ) );
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY]) -> entry),
				"focus-in-event",
				GTK_SIGNAL_FUNC ( entree_prend_focus ),
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
			 GTK_SIGNAL_FUNC ( entree_prend_focus ),
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
			 GTK_SIGNAL_FUNC ( entree_prend_focus ),
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
    widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE],
			   _("Choose currency"),
			   _("Choose currency") );
    menu = creation_option_menu_devises ( -1,
					  liste_struct_devises );
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ),
			       menu );
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
    widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] = gtk_combofix_new_complex ( liste_categories_combofix,
										       FALSE,
										       TRUE,
										       TRUE,
										       30 );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CATEGORY ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CATEGORY ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> arrow ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_CATEGORY ) );
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ),
				"focus-in-event",
				GTK_SIGNAL_FUNC ( entree_prend_focus ),
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
			 GTK_SIGNAL_FUNC ( entree_prend_focus ),
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

    if ( ( menu = creation_menu_types ( 1,
					no_compte,
					1 )))
    {
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				   menu );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				      cherche_no_menu_type_echeancier ( gsb_account_get_default_debit (no_compte) ) );
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

    widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] = gtk_combofix_new_complex ( liste_imputations_combofix,
											FALSE,
											TRUE,
											TRUE ,
											0);
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
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> arrow ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_BUDGETARY ) );
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry ),
				"focus-in-event",
				GTK_SIGNAL_FUNC ( entree_prend_focus ),
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
       GTK_SIGNAL_FUNC ( entree_prend_focus ),
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
			 GTK_SIGNAL_FUNC ( entree_prend_focus ),
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
    gtk_signal_connect ( GTK_OBJECT  ( item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( cache_date_limite_echeancier ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT  ( item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( cache_personnalisation_echeancier ),
			 NULL );
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Weekly"));
    gtk_object_set_data ( GTK_OBJECT  ( item ),
			  "periodicite",
			  GINT_TO_POINTER ( 1 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
    gtk_signal_connect ( GTK_OBJECT  ( item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( affiche_date_limite_echeancier ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT  ( item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( cache_personnalisation_echeancier ),
			 NULL );
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Monthly"));
    gtk_object_set_data ( GTK_OBJECT  ( item ),
			  "periodicite",
			  GINT_TO_POINTER ( 2 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
    gtk_signal_connect ( GTK_OBJECT  ( item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( affiche_date_limite_echeancier ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT  ( item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( cache_personnalisation_echeancier ),
			 NULL );
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Yearly"));
    gtk_object_set_data ( GTK_OBJECT  ( item ),
			  "periodicite",
			  GINT_TO_POINTER ( 3 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
    gtk_signal_connect ( GTK_OBJECT  ( item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( affiche_date_limite_echeancier ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT  ( item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( cache_personnalisation_echeancier ),
			 NULL );
    gtk_widget_show ( item );

    item = gtk_menu_item_new_with_label ( _("Custom"));
    gtk_object_set_data ( GTK_OBJECT  ( item ),
			  "periodicite",
			  GINT_TO_POINTER ( 4 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      item );
    gtk_signal_connect ( GTK_OBJECT  ( item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( affiche_date_limite_echeancier ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT  ( item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( affiche_personnalisation_echeancier ),
			 NULL );
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
			 GTK_SIGNAL_FUNC ( entree_prend_focus ),
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
			  "adr_echeance",
			  NULL );

    formulaire_echeancier_a_zero ();

    return ( formulaire_echeancier);
}
/******************************************************************************/

/******************************************************************************/
void echap_formulaire_echeancier ( void )
{
    formulaire_echeancier_a_zero();

    gtk_widget_grab_focus ( tree_view_liste_echeances );

    if ( !etat.formulaire_echeancier_toujours_affiche )
	gtk_widget_hide ( frame_formulaire_echeancier );
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
		modifie_date ( entree );

		/* on ne change l'exercice que si c'est une nouvelle échéance */

		if ( !gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
					    "adr_echeance" ))
		    affiche_exercice_par_date( widget_formulaire_echeancier[SCHEDULER_FORM_DATE],
					       widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] );
	    }
	    else
		texte = _("Date");
	    break;

	    /* on sort du tiers : soit vide soit complète le reste de l'opé */

	case SCHEDULER_FORM_PARTY :
	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		completion_operation_par_tiers_echeancier ();
	    else
		texte = _("Third party");
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
		    GtkWidget *menu;
		    gint no_compte;

		    no_compte = recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

		    if ( ( menu = creation_menu_types ( 1,
							no_compte,
							1 )))
		    {
			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
						   menu );
			gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
						      cherche_no_menu_type_echeancier ( gsb_account_get_default_debit (no_compte) ) );
			gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
		    }
		    else
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
		    GtkWidget *menu;
		    gint no_compte;

		    no_compte = recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

		    if ( ( menu = creation_menu_types ( 2,
							no_compte,
							1  )))
		    {
			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
						   menu );
			gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
						      cherche_no_menu_type_echeancier ( gsb_account_get_default_credit (no_compte) ) );
			gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
		    }
		    else
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
		modifie_date ( entree );
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

    degrise_formulaire_echeancier ();

    /* si l'entrée de la date et grise, on met la date courante */

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ) == style_entree_formulaire[ENGRIS] )
    {
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			     gsb_today() );

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
	    gtk_widget_grab_focus ( tree_view_liste_echeances );

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

		    gtk_widget_grab_focus ( tree_view_liste_echeances );
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

/******************************************************************************/
void affiche_date_limite_echeancier ( void )
{
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
}
/******************************************************************************/

/******************************************************************************/
void cache_date_limite_echeancier ( void )
{
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
}
/******************************************************************************/

/******************************************************************************/
void affiche_personnalisation_echeancier ( void )
{
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );
    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] );
}
/******************************************************************************/

/******************************************************************************/
void cache_personnalisation_echeancier ( void )
{
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] );
}
/******************************************************************************/


/** we come here when we validate a scheduled transaction
 * either modify or create a new scheduled transaction
 * either create a new transaction from the scheduled transaction
 * \param
 * \return
 * */
void gsb_scheduler_validate_form ( void )
{
    struct operation_echeance *scheduled_transaction;
    gchar **tab_char;
    gchar *char_ptr;

    gint compte_virement;
    compte_virement = 0;

    if ( !gsb_scheduler_check_form ())
	return;

    /* get the scheduled_transaction, will be NULL if it's a new one */

    scheduled_transaction = gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
						  "adr_echeance" );

    /* if the label label_saisie_modif is Input, it's a modification or new scheduled_transaction,
     * else we want to create a transaction from that scheduled transaction */

    if ( strcmp ( GTK_LABEL ( label_saisie_modif ) -> label,
		  _("Input") ) )
    {
	/* it's a modification/new scheduled transaction */

	/* si c'est une nouvelle échéance, on la crée */
	/* et on lui met son numéro tout de suite */

	if ( !scheduled_transaction )
	{
	    scheduled_transaction = calloc ( 1,
					     sizeof ( struct operation_echeance ));
	    scheduled_transaction -> no_operation = ++no_derniere_echeance;
	    nb_echeances++;
	}

	/* récupère la date */

	char_ptr = g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY (widget_formulaire_echeancier[SCHEDULER_FORM_DATE] )));

	tab_char = g_strsplit ( char_ptr,
				"/",
				3 );

	scheduled_transaction -> jour = my_strtod ( tab_char[0],
						    NULL );
	scheduled_transaction -> mois = my_strtod ( tab_char[1],
						    NULL );
	scheduled_transaction -> annee = my_strtod (tab_char[2],
						    NULL );

	scheduled_transaction ->date = g_date_new_dmy ( scheduled_transaction ->jour,
							scheduled_transaction ->mois,
							scheduled_transaction ->annee);

	/* récupération du tiers, s'il n'existe pas, on le crée */

	if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	{
	    char_ptr = gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ));

	    scheduled_transaction -> tiers  = tiers_par_nom ( char_ptr,
							      1 ) -> no_tiers;
	}

	/* récupération du montant */

	if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	    scheduled_transaction -> montant = -calcule_total_entree (widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] );
	else
	    scheduled_transaction -> montant = calcule_total_entree (widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] );

	/* récupération de la devise */

	scheduled_transaction -> devise = ((struct struct_devise *)( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ) -> menu_item ),
											   "adr_devise" ))) -> no_devise;

	/* récupération du no de compte */

	scheduled_transaction -> compte = recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );


	/*   récupération des catégories / sous-catég, s'ils n'existent pas, on les crée */
	/* s'il n'y a pas de catég, ce n'es pas un virement non plus, donc on met compte_virement à -1 */

	if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	{
	    struct struct_categ *categ;

	    tab_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ))),
				    ":",
				    2 );

	    tab_char[0] = g_strstrip ( tab_char[0] );

	    if ( tab_char[1] )
		tab_char[1] = g_strstrip ( tab_char[1] );


	    if ( strlen ( tab_char[0] ) )
	    {
		if ( !strcmp ( tab_char[0],
			       _("Transfer") )
		     && tab_char[1]
		     && strlen ( tab_char[1]) )
		{
		    /* c'est un virement, il n'y a donc aucune catégorie */

		    scheduled_transaction -> categorie = 0;
		    scheduled_transaction -> sous_categorie = 0;
		    scheduled_transaction -> operation_ventilee = 0;

		    /* recherche le no de compte du virement */

		    scheduled_transaction -> compte_virement = gsb_account_get_no_account_by_name ( tab_char[1] );
		}
		else
		{
		    /* 		    si c'est une opération ventilée, c'est ici que ça se passe ! */

		    if ( !strcmp ( tab_char[0],
				   _("Breakdown of transaction") ))
		    {
			/* c'est une ventilation, il n'y a donc aucune catégorie */
			/* on va appeler la fonction validation_ope_de_ventilation */
			/* qui va créer les nouvelles opé*/

			validation_ope_de_ventilation_echeances ( scheduled_transaction );
			scheduled_transaction -> categorie = 0;
			scheduled_transaction -> sous_categorie = 0;
			scheduled_transaction -> operation_ventilee = 1;
			scheduled_transaction -> compte_virement = 0;

		    }
		    else
		    {	
			struct struct_sous_categ *sous_categ;

			categ = categ_par_nom ( tab_char[0],
						1,
						0,
						0 );

			if ( categ )
			{
			    scheduled_transaction -> categorie = categ -> no_categ;

			    sous_categ = sous_categ_par_nom ( categ,
							      tab_char[1],
							      1 );

			    if ( sous_categ )
				scheduled_transaction -> sous_categorie = sous_categ -> no_sous_categ;
			}

			scheduled_transaction -> compte_virement = 0;
			scheduled_transaction -> operation_ventilee = 0;
		    }
		}
	    }
	    g_strfreev ( tab_char );
	}
	else
	    scheduled_transaction -> compte_virement = -1;

	/* récupération du type d'opération */

	if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ))
	{
	    scheduled_transaction -> type_ope = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
											"no_type" ));

	    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] )
		 &&
		 gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ) == style_entree_formulaire[ENCLAIR] )
		scheduled_transaction -> contenu_type = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ))));
	}

	/* récupération du no d'exercice */

	scheduled_transaction -> no_exercice = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ) -> menu_item ),
										       "no_exercice" ));

	/* récupération de l'imputation budgétaire */

	if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	{
	    struct struct_imputation *imputation;

	    char_ptr = gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ));

	    tab_char = g_strsplit ( char_ptr,
				    ":",
				    2 );

	    imputation = imputation_par_nom ( tab_char[0],
					      1,
					      scheduled_transaction -> montant < 0,
					      0 );

	    if ( imputation )
	    {
		struct struct_sous_imputation *sous_imputation;

		scheduled_transaction -> imputation = imputation -> no_imputation;

		sous_imputation = sous_imputation_par_nom ( imputation,
							    tab_char[1],
							    1 );

		if ( sous_imputation )
		    scheduled_transaction -> sous_imputation = sous_imputation -> no_sous_imputation;
	    }

	    g_strfreev ( tab_char );
	}

	/*       récupération de auto_man */

	scheduled_transaction -> auto_man = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE]  ) -> menu_item ),
										    "auto_man" ) );

	/* récupération des notes */

	if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ) == style_entree_formulaire[ENCLAIR] )
	    scheduled_transaction -> notes = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ))));


	/* récupération de la fréquence */

	scheduled_transaction -> periodicite = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY]  ) -> menu_item ),
										       "periodicite" ) );

	if ( scheduled_transaction -> periodicite == 4 )
	{
	    scheduled_transaction -> intervalle_periodicite_personnalisee = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU]  ) -> menu_item ),
														    "intervalle_perso" ) );

	    scheduled_transaction -> periodicite_personnalisee = my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] )),
									     NULL );
	}

	if ( scheduled_transaction -> periodicite
	     &&
	     gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ) == style_entree_formulaire[ENCLAIR] )
	{
	    /* traitement de la date limite */

	    tab_char = g_strsplit ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ))),
				    "/",
				    3 );

	    scheduled_transaction -> jour_limite = my_strtod ( tab_char[0],
							       NULL );
	    scheduled_transaction -> mois_limite = my_strtod ( tab_char[1],
							       NULL );
	    scheduled_transaction -> annee_limite = my_strtod (tab_char[2],
							       NULL );

	    scheduled_transaction->date_limite = g_date_new_dmy ( scheduled_transaction->jour_limite,
								  scheduled_transaction->mois_limite,
								  scheduled_transaction->annee_limite);
	}


	/* si c'est une nouvelle opé,  on l'ajoute à la liste */

	if ( !gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
				    "adr_echeance" ) )
	    liste_struct_echeances = g_slist_insert_sorted ( liste_struct_echeances,
							     scheduled_transaction,
							     (GCompareFunc) classement_sliste_echeance_par_date );
    }
    else
    {
	gint transaction_number;

	transaction_number = gsb_scheduler_create_transaction_from_scheduled_form ();

	/* the scheduled transaction goes to the next date */

	gsb_scheduler_increase_scheduled_transaction ( scheduled_transaction );

	/* réaffiche les option menu de la périodicité, des banques et de l'automatisme, effacés pour une saisie d'opé */

	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] );
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] );
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] );

	update_transaction_in_trees (gsb_transaction_data_get_pointer_to_transaction (transaction_number));    
    }

    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ ();

    formulaire_echeancier_a_zero ();

    if ( !etat.formulaire_echeancier_toujours_affiche )
	gtk_widget_hide ( frame_formulaire_echeancier );

    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_liste_echeances_auto_accueil = 1;
    remplissage_liste_echeance ();
    gtk_widget_grab_focus ( tree_view_liste_echeances );

    modification_fichier ( TRUE );
}



/** check if the scheduled transaction in the schedule's form is ok to continue
 * \param
 * \return TRUE if ok, FALSE if pb
 * */
gboolean gsb_scheduler_check_form ( void )
{
    /* check the date */

    if ( !modifie_date ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ))
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
	 !modifie_date ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ))
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

    /* if it's a breakdown, check the children */

    if ( !strcmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ))),
		   _("Breakdown of transaction") )
	 &&
	 !gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
				"liste_adr_ventilation" ))
    {
	GSList *slist_ptr;

	enregistre_ope_au_retour_echeances = 1;

	/* We check if one of the subtransactions of the breakdown of
	   transaction is a transfer on itself. */

	slist_ptr = gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
					  "liste_adr_ventilation" );
	while ( slist_ptr
		&&
		slist_ptr != GINT_TO_POINTER ( -1 ))
	{
	    struct struct_ope_ventil *scheduled_transaction;

	    scheduled_transaction = slist_ptr -> data;

	    if ( scheduled_transaction -> relation_no_compte != -1
		 &&
		 scheduled_transaction -> relation_no_compte
		 == 
		 recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ))
	    {
		dialogue_error_hint ( _("This breakdown of transaction has a transfer on itself.  Either change the sub transaction to transfer on another account or change account of the transaction itself."),
				      _("Invalid breakdown of transaction."));
		return FALSE;
	    }
	    slist_ptr = slist_ptr -> next;
	}

	if ( gtk_widget_get_style ( widget_formulaire_echeancier[TRANSACTION_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	    ventiler_operation_echeances ( -my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ))),
							NULL ));
	else
	    ventiler_operation_echeances ( my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ))),
						       NULL ));

	return FALSE;
    }

    /* check if it's not a transfer on itself */

    if ( !g_strcasecmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ))),
			 g_strconcat ( COLON(_("Transfer")),
				       COMPTE_ECHEANCE,
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
	     gsb_account_get_no_account_by_name ( g_strstrip (tab_char[1])) == -1 )
	{
	    dialogue_error_hint ( _("Associated account of this transfer does not exist.  Please associate a valid account with this transfer."),
				  _("Invalid transfer.") );
	    g_strfreev ( tab_char );
	    return FALSE;
	}
	g_strfreev ( tab_char );
    }

    /* vérification si c'est une échéance auto que ce n'est pas un chèque */

    if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE]  ) -> menu_item ),
						 "auto_man" )) == 1
	 &&
	 GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] )
	 &&
	 ((struct struct_type_ope  *)( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
							     "adr_type" )))->numerotation_auto )
    {
	dialogue_error_hint ( _("Impossible to create or entry an automatic scheduled transaction\n with a cheque or a method of payment with an automatic incremental number."),
			      _("Invalid scheduled transaction.") );
	return FALSE;
    }


    return TRUE;
}

/** create a new transaction and fill it from the scheduled form
 * \param 
 * \return the number of the new transaction
 * */
gint gsb_scheduler_create_transaction_from_scheduled_form ( void )
{
    gint transaction_number;
    gint day, month, year;

    transaction_number = gsb_transaction_data_new_transaction ( recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ));

    /* take the date */

    sscanf ( gtk_entry_get_text ( GTK_ENTRY (GTK_ENTRY (widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ))),
	     "%d/%d/%d",
	     &day,
	     &month,
	     &year );

    gsb_transaction_data_set_date ( transaction_number,
				    g_date_new_dmy ( day,
						     month,
						     year  ));

    /* take the party */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	gsb_transaction_data_set_party_number ( transaction_number,
						tiers_par_nom ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] )),
								1 ) -> no_tiers );

    /* get the amount */

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	gsb_transaction_data_set_amount ( transaction_number,
					  -calcule_total_entree (widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT]));
    else
	gsb_transaction_data_set_amount ( transaction_number,
					  calcule_total_entree (widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT]));

    /* get the currency */

    gsb_transaction_data_set_currency_number ( transaction_number,
					       gsb_currency_get_option_menu_currency (widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE]));

    gsb_currency_check_for_change ( transaction_number );

    /* get the method of payment of the transaction */

    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ))
    {
	gsb_transaction_data_set_method_of_payment_number ( transaction_number,
							    GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
												    "no_type" )) );

	if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] )
	     &&
	     gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ) == style_entree_formulaire[ENCLAIR] )
	{
	    struct struct_type_ope *type;

	    type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
					 "adr_type" );

	    gsb_transaction_data_set_method_of_payment_content ( transaction_number,
								 g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] )))) );

	    if ( type -> numerotation_auto )
		type -> no_en_cours = my_atoi ( gsb_transaction_data_get_method_of_payment_content (transaction_number));
	}
    }

    /* get the financial year */
    /* if no_exercice is -2 : it's automatic
     * if -1 : it's not showed */

    switch ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ) -> menu_item ),
						     "no_exercice" )))
    {
	case -2:
	    gsb_transaction_data_set_financial_year_number ( transaction_number,
							     recherche_exo_correspondant ( gsb_transaction_data_get_date (transaction_number)));

	    break;

	case -1:
	    gsb_transaction_data_set_financial_year_number ( transaction_number,
							     0);

	    break;

	default:
	    gsb_transaction_data_set_financial_year_number ( transaction_number,
							     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ) -> menu_item ),
												     "no_exercice" )));
    }

    /* get the budget */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	gchar **tab_char;
	struct struct_imputation *budgetary;

	tab_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] )),
				":",
				2 );

	budgetary = imputation_par_nom ( tab_char[0],
					 1,
					 gsb_transaction_data_get_amount (transaction_number)< 0,
					 0 );

	if ( budgetary )
	{
	    struct struct_sous_imputation *sub_budgetary;

	    gsb_transaction_data_set_budgetary_number ( transaction_number,
							budgetary -> no_imputation);

	    sub_budgetary = sous_imputation_par_nom ( budgetary,
						      tab_char[1],
						      1 );

	    if ( sub_budgetary )
		gsb_transaction_data_set_sub_budgetary_number ( transaction_number,
								sub_budgetary -> no_sous_imputation);
	}
	g_strfreev ( tab_char );
    }

    /* get the automatic mode */

    gsb_transaction_data_set_automatic_transaction ( transaction_number,
						     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE]  ) -> menu_item ),
											     "auto_man" )));
    /* get the notes */

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ) == style_entree_formulaire[ENCLAIR] )
	gsb_transaction_data_set_notes ( transaction_number,
					 g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] )))));


    /* get the category */

    gsb_scheduler_get_category_for_transaction_from_form ( transaction_number );

    /* we show the new transaction in the tree view */

    gsb_transactions_list_append_new_transaction (gsb_transaction_data_get_pointer_to_transaction(transaction_number));

    return transaction_number;
}


/* used to catch a transaction from the scheduled form
 * take the category, check if it's a transfer or a breakdown and
 * do the necessary (create contra-transaction, take the children's transactions
 * \param transaction_number
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_scheduler_get_category_for_transaction_from_form ( gint transaction_number )
{
    GtkWidget *category_entry;
    gchar *char_ptr;

    category_entry = GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry;

    if ( gtk_widget_get_style ( category_entry ) == style_entree_formulaire[ENCLAIR] )
    {
	char_ptr = g_strstrip ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( category_entry ))));

	/* we check first if it's a breakdown, next a transfer and after a normal category
	 * each time we needn't to set the differents variables to 0 because here it's always
	 * a new transaction, so all is at 0 */

	if ( !strcmp ( char_ptr,
		       _("Breakdown of transaction") ))
	{
	    /* it's a breakdown of transaction */

	    GSList *breakdown_list;

	    gsb_transaction_data_set_breakdown_of_transaction ( transaction_number,
								1 );

	    breakdown_list = gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
						   "liste_adr_ventilation" );
	    while ( breakdown_list
		    &&
		    breakdown_list != GINT_TO_POINTER ( -1 ))
	    {
		struct struct_ope_ventil *scheduled_transaction;
		gint breakdown_transaction_number;

		/* we create the daughter transaction */

		breakdown_transaction_number = gsb_transaction_data_new_transaction ( gsb_transaction_data_get_account_number (transaction_number));

		/* most of that transaction is the same of the mother */

		gsb_transaction_data_copy_transaction ( transaction_number,
							breakdown_transaction_number );

		/* set the mother */

		gsb_transaction_data_set_mother_transaction_number ( breakdown_transaction_number,
								     transaction_number );

		/* now we had to change what is specific to the daughter */

		scheduled_transaction = breakdown_list -> data;

		gsb_transaction_data_set_amount ( breakdown_transaction_number,
						  scheduled_transaction -> montant );
		gsb_transaction_data_set_category_number ( breakdown_transaction_number,
							   scheduled_transaction -> categorie );
		gsb_transaction_data_set_sub_category_number ( breakdown_transaction_number,
							       scheduled_transaction -> sous_categorie );
		gsb_transaction_data_set_notes ( breakdown_transaction_number,
						 scheduled_transaction -> notes );
		gsb_transaction_data_set_budgetary_number ( breakdown_transaction_number,
							    scheduled_transaction -> imputation);
		gsb_transaction_data_set_sub_budgetary_number ( breakdown_transaction_number,
								scheduled_transaction -> sous_imputation);
		gsb_transaction_data_set_voucher ( breakdown_transaction_number,
						   scheduled_transaction -> no_piece_comptable);
		gsb_transaction_data_set_financial_year_number ( breakdown_transaction_number,
								 scheduled_transaction -> no_exercice);

		/* if it's a transfer, set the contra transaction */

		if ( scheduled_transaction -> relation_no_operation == -1 )
		{
		    gint contra_transaction_number;

		    contra_transaction_number = gsb_form_validate_transfer ( breakdown_transaction_number,
									     TRUE,
									     gsb_account_get_name ( scheduled_transaction -> relation_no_compte ));
		    gsb_transaction_data_set_method_of_payment_number ( contra_transaction_number,
									scheduled_transaction -> no_type_associe );
		}

		update_transaction_in_trees (gsb_transaction_data_get_pointer_to_transaction(breakdown_transaction_number));

		breakdown_list = breakdown_list -> next;
	    }
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

		    gsb_form_validate_transfer ( transaction_number,
						 1,
						 tab_char[1] );
		}
		else
		{
		    /* it's a normal category */

		    struct struct_categ *categ;

		    categ = categ_par_nom ( tab_char[0],
					    1,
					    gsb_transaction_data_get_amount (transaction_number)< 0,
					    0 );

		    if ( categ )
		    {
			struct struct_sous_categ *sous_categ;

			gsb_transaction_data_set_category_number ( transaction_number,
								   categ -> no_categ );

			sous_categ = sous_categ_par_nom ( categ,
							  tab_char[1],
							  1 );

			if ( sous_categ )
			    gsb_transaction_data_set_sub_category_number ( transaction_number,
									   sous_categ -> no_sous_categ );
		    }
		}
	    }
	    g_strfreev ( tab_char );
	}
    }
    return TRUE;
}


/** create a new transaction and fill it from a scheduled transaction
 * \param scheduled_transaction the transaction we use to fill the new transaction
 * \return the number of the new transaction
 * */
gint gsb_scheduler_create_transaction_from_scheduled_transaction ( struct operation_echeance *scheduled_transaction )
{
    gint transaction_number;

    transaction_number = gsb_transaction_data_new_transaction (scheduled_transaction -> compte);

    /* begin to fill the new transaction */

    gsb_transaction_data_set_date ( transaction_number,
				    gsb_date_copy (scheduled_transaction -> date));
    gsb_transaction_data_set_party_number ( transaction_number,
					    scheduled_transaction -> tiers );
    gsb_transaction_data_set_amount ( transaction_number,
				      scheduled_transaction -> montant);
    gsb_transaction_data_set_currency_number ( transaction_number,
					       scheduled_transaction -> devise );
    gsb_currency_check_for_change ( transaction_number );
    gsb_transaction_data_set_method_of_payment_number ( transaction_number,
							scheduled_transaction -> type_ope );
    gsb_transaction_data_set_notes ( transaction_number,
				     scheduled_transaction -> notes );
    gsb_transaction_data_set_method_of_payment_content ( transaction_number,
							 scheduled_transaction -> contenu_type );
    gsb_transaction_data_set_automatic_transaction ( transaction_number,
						     scheduled_transaction -> auto_man );
    gsb_transaction_data_set_budgetary_number ( transaction_number,
						scheduled_transaction -> imputation);
    gsb_transaction_data_set_sub_budgetary_number ( transaction_number,
						    scheduled_transaction -> sous_imputation);

    /* if the financial year is automatic, we set it here */

    if ( scheduled_transaction -> no_exercice == -2 )
	gsb_transaction_data_set_financial_year_number ( transaction_number,
							 recherche_exo_correspondant ( gsb_transaction_data_get_date (transaction_number)));
    else
	gsb_transaction_data_set_financial_year_number ( transaction_number,
							 scheduled_transaction -> no_exercice);


    /* get the category */

    gsb_scheduler_get_category_for_transaction_from_transaction ( transaction_number,
								  scheduled_transaction );

    /* we show the new transaction in the tree view */

    gsb_transactions_list_append_new_transaction (gsb_transaction_data_get_pointer_to_transaction(transaction_number));

    return transaction_number;
}


/* used to catch a transaction from a scheduled transaction
 * take the category, check if it's a transfer or a breakdown and
 * do the necessary (create contra-transaction, take the children's transactions
 * \param transaction_number
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_scheduler_get_category_for_transaction_from_transaction ( gint transaction_number,
								       struct operation_echeance *scheduled_transaction )
{
    /* if category is set, it's a normal category */

    if ( scheduled_transaction -> categorie )
    {
	/* it's a normal category */

	gsb_transaction_data_set_category_number ( transaction_number,
						   scheduled_transaction -> categorie );
	gsb_transaction_data_set_sub_category_number ( transaction_number,
						       scheduled_transaction -> sous_categorie );
	return TRUE;
    }

    if ( scheduled_transaction -> operation_ventilee )
    {
	/* it's a breakdown of transaction,
	 * we will check all the scheduled transactions to find all the children */

	GSList *scheduled_transactions_list;

	gsb_transaction_data_set_breakdown_of_transaction ( transaction_number,
							    1 );

	scheduled_transactions_list = liste_struct_echeances;

	while (scheduled_transactions_list)
	{
	    struct operation_echeance *scheduled_transaction_child;

	    scheduled_transaction_child = scheduled_transactions_list -> data;

	    if ( scheduled_transaction_child -> no_operation_ventilee_associee == scheduled_transaction -> no_operation )
	    {
		/* the scheduled_transaction_child is a child of scheduled_transaction */

		gint breakdown_transaction_number;

		/* we create the daughter transaction */

		breakdown_transaction_number = gsb_transaction_data_new_transaction ( gsb_transaction_data_get_account_number (transaction_number));

		/* most of that transaction is the same of the mother */

		gsb_transaction_data_copy_transaction ( transaction_number,
							breakdown_transaction_number );

		/* set the mother */

		gsb_transaction_data_set_mother_transaction_number ( breakdown_transaction_number,
								     transaction_number );

		/* now we had to change what is specific to the daughter */

		gsb_transaction_data_set_amount ( breakdown_transaction_number,
						  scheduled_transaction -> montant );
		gsb_transaction_data_set_category_number ( breakdown_transaction_number,
							   scheduled_transaction -> categorie );
		gsb_transaction_data_set_sub_category_number ( breakdown_transaction_number,
							       scheduled_transaction -> sous_categorie );
		gsb_transaction_data_set_notes ( breakdown_transaction_number,
						 scheduled_transaction -> notes );
		gsb_transaction_data_set_budgetary_number ( breakdown_transaction_number,
							    scheduled_transaction -> imputation);
		gsb_transaction_data_set_sub_budgetary_number ( breakdown_transaction_number,
								scheduled_transaction -> sous_imputation);
		gsb_transaction_data_set_financial_year_number ( breakdown_transaction_number,
								 scheduled_transaction -> no_exercice);

		/* if it's a transfer, set the contra transaction */

		if ( scheduled_transaction -> compte_virement != -1 )
		{
		    gint contra_transaction_number;

		    contra_transaction_number = gsb_form_validate_transfer ( breakdown_transaction_number,
									     TRUE,
									     gsb_account_get_name ( scheduled_transaction -> compte_virement ));
		    gsb_transaction_data_set_method_of_payment_number ( contra_transaction_number,
									scheduled_transaction -> type_contre_ope );

		}
		update_transaction_in_trees (gsb_transaction_data_get_pointer_to_transaction(breakdown_transaction_number));
	    }
	    scheduled_transactions_list = scheduled_transactions_list -> next;
	}
    }
    else
    {
	/* it's not a breakdown of transaction and not a normal category 
	 * so it's a transfer, except if the target account is -1 then it's a
	 * transaction with no category */

	if ( scheduled_transaction -> compte_virement != -1 )
	{
	    gint contra_transaction_number;

	    contra_transaction_number = gsb_form_validate_transfer ( transaction_number,
								     TRUE,
								     gsb_account_get_name ( scheduled_transaction -> compte_virement ));
	    gsb_transaction_data_set_method_of_payment_number ( contra_transaction_number,
								scheduled_transaction -> type_contre_ope );

	}
    }

    return TRUE;
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
			    _("Third party") );
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

    /* on associe au formulaire l'adr de l'échéance courante */

    gtk_object_set_data ( GTK_OBJECT ( formulaire_echeancier ),
			  "adr_echeance",
			  NULL );
}
/******************************************************************************/

/** set the next date in the scheduled transaction
 * if it's above the limit date, that transaction is deleted
 * \param scheduled_transaction the transaction we want to increase
 * \return FALSE if the scheduled transaction is finished, TRUE else
 * */
gboolean gsb_scheduler_increase_scheduled_transaction ( struct operation_echeance *scheduled_transaction )
{
    /* increase the date of the scheduled_transaction */

    gsb_scheduler_increase_date ( scheduled_transaction,
				  scheduled_transaction -> date );

    /* si l'échéance est finie, on la vire, sinon on met à jour les var jour, mois et année */

    if ( main_page_finished_scheduled_transactions_part
	 && 
	 ( !scheduled_transaction -> periodicite 
	   ||
	   (
	    scheduled_transaction -> date_limite
	    &&
	    g_date_compare ( scheduled_transaction -> date,
			     scheduled_transaction -> date_limite ) > 0  )) )
    {
	/* update the main page */

	gsb_main_page_update_finished_scheduled_transactions (scheduled_transaction);
	gsb_scheduler_delete_scheduled_transaction (scheduled_transaction);
	return FALSE;
    }
    else
    {
	scheduled_transaction -> jour = scheduled_transaction -> date -> day;
	scheduled_transaction -> mois = scheduled_transaction -> date -> month;
	scheduled_transaction -> annee = scheduled_transaction -> date -> year;
    }
    return TRUE;
}


/** increase the date given in param with what is describe
 * in the scheduled transaction (it doen't do any copy of the date !)
 * \param scheduled_transaction it contains the parameters to increase the date
 * \param date a GDate which will be increase
 * \return FALSE
 * */
gboolean gsb_scheduler_increase_date ( struct operation_echeance *scheduled_transaction,
				       GDate *date )
{
    switch ( scheduled_transaction -> periodicite )
    {
	case 1:
	    /* périodicité hebdomadaire */
	    g_date_add_days ( date,
			      7 );

	    /* magouille car il semble y avoir un bug dans g_date_add_days qui ne fait pas l'addition si on ne met pas la ligne suivante */
	    /* FIXME check if the bug in g_date_add_days is still present */

	    g_date_add_months ( date,
				0 );
	    break;

	case 2:
	    /* périodicité mensuelle */
	    g_date_add_months ( date,
				1 );
	    break;

	case 3:
	    /* périodicité annuelle */
	    g_date_add_years ( date,
			       1 );

	case 4:
	    /* périodicité perso */

	    switch ( scheduled_transaction -> intervalle_periodicite_personnalisee )
	    {
		case 0:
		    g_date_add_days ( date,
				      scheduled_transaction -> periodicite_personnalisee );

		    /* magouille car il semble y avoir un bug dans g_date_add_days qui ne fait pas l'addition si on ne met pas la ligne suivante */
		    /* FIXME check if the bug in g_date_add_days is still present */
		    g_date_add_months ( date,
					0 );
		    break;

		case 1:
		    g_date_add_months ( date,
					scheduled_transaction -> periodicite_personnalisee );
		    break;

		case 2:
		    g_date_add_years ( date,
				       scheduled_transaction -> periodicite_personnalisee );
		    break;
	    }    
    }

    return FALSE;
}


/******************************************************************************/
/* Fonction completion_operation_par_tiers_echeancier */
/* appelée lorsque le tiers perd le focus */
/* récupère le tiers, et recherche la dernière opé associée à ce tiers */
/******************************************************************************/
void completion_operation_par_tiers_echeancier ( void )
{
    struct struct_tiers *tiers;
    gpointer operation;
    gint no_compte;
    GSList *pointeur_ope;
    gchar *char_tmp;

    /* s'il y a quelque chose dans les crédit ou débit ou catégories, on se barre */

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR]
	 ||
	 gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ) == style_entree_formulaire[ENCLAIR]
	 ||
	 gtk_widget_get_style ( GTK_COMBOFIX (widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY])->entry ) == style_entree_formulaire[ENCLAIR] )
	return;


    tiers = tiers_par_nom ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY])),
			    0 );

    /*   si nouveau tiers,  on s'en va simplement */

    if ( !tiers )
	return;

    /* on fait d'abord le tour du compte courant pour recherche une opé avec ce tiers */
    /* s'il n'y a aucune opé correspondante, on fait le tour de tous les comptes */

    no_compte = recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

    operation = NULL;
    pointeur_ope = gsb_account_get_transactions_list (no_compte);

    while ( pointeur_ope )
    {
	gpointer ope_test;

	ope_test = pointeur_ope -> data;

	if ( gsb_transaction_data_get_party_number ( gsb_transaction_data_get_transaction_number (ope_test ))== tiers -> no_tiers )
	{
	    if ( operation )
	    {
		if ( g_date_compare ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (ope_test)),
				      gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation))) >= 0 )
		    operation = ope_test;
	    }
	    else
		operation = ope_test;
	}

	pointeur_ope = pointeur_ope -> next;
    }

    if ( !operation )
    {
	/* aucune opération correspondant à ce tiers n'a été trouvée dans le compte courant */
	/*       on recherche dans les autres comptes, la première trouvée fera l'affaire */

	GSList *list_tmp;

	list_tmp = gsb_account_get_list_accounts ();

	while ( list_tmp )
	{
	    gint i;

	    i = gsb_account_get_no_account ( list_tmp -> data );

	    if ( i != no_compte )
	    {
		pointeur_ope = gsb_account_get_transactions_list (i);

		while ( pointeur_ope )
		{
		    gpointer ope_test;

		    ope_test = pointeur_ope -> data;

		    if ( gsb_transaction_data_get_party_number ( gsb_transaction_data_get_transaction_number (ope_test ))== tiers -> no_tiers )
		    {
			operation = ope_test;
			pointeur_ope = NULL;
			i = gsb_account_get_accounts_amount ();
		    }
		    else
			pointeur_ope = pointeur_ope -> next;
		}
	    }

	    list_tmp = list_tmp -> next;
	}
    }

    /* si on n'a trouvé aucune opération, on se tire */

    if ( !operation )
	return;

    /* remplit les différentes entrées du formulaire */

    /* remplit les montant */

    if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (operation ))< 0 )
    {
	GtkWidget *menu;

	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
			     g_strdup_printf ( "%4.2f",
					       -gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (operation ))));
	/* met le menu des types débits */


	if ( (menu = creation_menu_types ( 1,
					   recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
					   1 )))
	{
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				       menu );
	    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	else
	    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
    }
    else
    {
	GtkWidget *menu;
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
			     g_strdup_printf ( "%4.2f",
					       gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (operation ))));
	/* met le menu des types crédits */


	if ( (menu = creation_menu_types ( 2,
					   recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
					   1 )))
	{
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				       menu );
	    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	else
	    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );

    }

    /* met la devise */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ),
				  g_slist_index ( liste_struct_devises,
						  devise_par_no ( gsb_transaction_data_get_currency_number ( gsb_transaction_data_get_transaction_number (operation )))));

    /* mise en forme des catégories */

    /* vérifie si c'est un virement */

    if ( gsb_transaction_data_get_transaction_number_transfer ( gsb_transaction_data_get_transaction_number (operation ))
	 &&
	 gsb_transaction_data_get_account_number_transfer ( gsb_transaction_data_get_transaction_number (operation ))!= -1 )
    {
	/* c'est un virement, on l'affiche */

	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY]);

	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
				g_strconcat ( COLON(_("Transfer")),
					      gsb_account_get_name (gsb_transaction_data_get_account_number_transfer ( gsb_transaction_data_get_transaction_number (operation ))),
					      NULL ));
    }
    else
    {
	char_tmp = nom_categ_par_no ( gsb_transaction_data_get_category_number ( gsb_transaction_data_get_transaction_number (operation)),
				      gsb_transaction_data_get_sub_category_number ( gsb_transaction_data_get_transaction_number (operation)));
	
	if ( char_tmp )
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY]);
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
				    char_tmp );
	}
    }

    /* met l'option menu du type d'opé */

    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ))
    {
	gint place_type;

	place_type = cherche_no_menu_type_echeancier ( gsb_transaction_data_get_method_of_payment_number ( gsb_transaction_data_get_transaction_number (operation )));

	/*       si la place est trouvée, on la met, sinon on met à la place par défaut */

	if ( place_type != -1 )
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					  place_type );
	else
	{
	    if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (operation ))< 0 )
		place_type = cherche_no_menu_type_echeancier ( gsb_account_get_default_debit (no_compte) );
	    else
		place_type = cherche_no_menu_type_echeancier ( gsb_account_get_default_credit (no_compte) );

	    if ( place_type != -1 )
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					      place_type );
	    else
	    {
		struct struct_type_ope *type;

		gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					      0 );

		/*  on met ce type par défaut, vu que celui par défaut marche plus ... */

		if ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (operation ))< 0 )
		    gsb_account_set_default_debit ( no_compte,
						    GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
											    "no_type" )) );
		else
		    gsb_account_set_default_credit ( no_compte,
						     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
											     "no_type" )));

		/* récupère l'adr du type pour afficher l'entrée si nécessaire */

		type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
					     "adr_type" );

		if ( type -> affiche_entree )
		    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );
	    }
	}
    }

    /* met en place l'exercice */

    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ),
				   cherche_no_menu_exercice ( gsb_transaction_data_get_financial_year_number ( gsb_transaction_data_get_transaction_number (operation )),
							      widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ));

    /* met en place l'imputation budgétaire */


    char_tmp = nom_imputation_par_no ( gsb_transaction_data_get_budgetary_number ( gsb_transaction_data_get_transaction_number (operation )),
				       gsb_transaction_data_get_sub_budgetary_number ( gsb_transaction_data_get_transaction_number (operation )));

   if ( char_tmp )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY]);
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
				char_tmp );
    }


    /*   remplit les notes */

    if ( gsb_transaction_data_get_notes ( gsb_transaction_data_get_transaction_number (operation )))
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			     gsb_transaction_data_get_notes ( gsb_transaction_data_get_transaction_number (operation )));
    }
}
/******************************************************************************/

/******************************************************************************/
void degrise_formulaire_echeancier ( void )
{
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ), TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ), TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ), TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ), TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] ), TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] ), TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] ), TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( hbox_valider_annuler_echeance ), TRUE );
    gtk_widget_show ( label_saisie_modif );
}
/******************************************************************************/




/******************************************************************************/
/* Fonction basculer_vers_ventilation                                         */
/* appelée par l'appui du bouton Ventilation...                               */
/* permet de voir les opés ventilées d'une ventilation                        */
/******************************************************************************/
void basculer_vers_ventilation_echeances ( void )
{
    enregistre_ope_au_retour_echeances = 0;

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	ventiler_operation_echeances  ( -my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ))),
						     NULL ));
    else
	ventiler_operation_echeances  ( my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ))),
						    NULL ));
}
/******************************************************************************/



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
