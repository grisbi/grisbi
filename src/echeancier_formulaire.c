/* ************************************************************************** */
/* Ce fichier s'occupe de la gestion du formulaire de saisie des échéances    */
/*			echeances_formulaire.c                                */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Alain Portal (dionysos@grisbi.org)	      */
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
#include "operations_formulaire.h"
#include "categories_onglet.h"
#include "comptes_traitements.h"
#include "type_operations.h"
#include "devises.h"
#include "dialog.h"
#include "calendar.h"
#include "utils.h"
#include "gtk_combofix.h"
#include "imputation_budgetaire.h"
#include "traitement_variables.h"
#include "echeancier_liste.h"
#include "tiers_onglet.h"
#include "ventilation.h"
#include "echeancier_ventilation.h"
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
static void cree_contre_operation_echeance ( struct structure_operation *operation,
				      gint compte_virement,
				      gint contre_type_ope );
static void echap_formulaire_echeancier ( void );
static gboolean entree_perd_focus_echeancier ( GtkWidget *entree,
					GdkEventFocus *ev,
					gint *no_origine );
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
extern struct struct_devise *devise_compte;
extern struct operation_echeance *echeance_selectionnnee;
extern gint enregistre_ope_au_retour_echeances;
extern GtkWidget *fleche_bas_echeancier;
extern GtkWidget *fleche_haut_echeancier;
extern GtkWidget *formulaire;
extern GtkWidget *formulaire_echeancier;
extern GtkWidget *formulaire_echeancier;
extern GtkWidget *frame_etat_echeances_finies;
extern GtkWidget *frame_formulaire_echeancier;
extern GSList *liste_categories_combofix;
extern GSList *liste_imputations_combofix;
extern GSList *liste_struct_devises;
extern GSList *liste_struct_echeances;
extern GSList *liste_tiers_combofix_echeancier;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint nb_comptes;
extern gint nb_echeances;
extern gint no_derniere_echeance;
extern FILE * out;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern GtkStyle *style_entree_formulaire[2];
extern gdouble taux_de_change[2];
extern GtkWidget *tree_view_liste_echeances;
/*END_EXTERN*/


/******************************************************************************/
/*  Routine qui crée le formulaire et le renvoie */
/******************************************************************************/
GtkWidget *creation_formulaire_echeancier ( void )
{
    GtkWidget *menu;
    GtkWidget *item;
    GtkTooltips *tips;
    GtkWidget *bouton;
    GtkWidget *table;


    /* on crée le tooltips */

    tips = gtk_tooltips_new ();


    formulaire_echeancier = gtk_vbox_new ( FALSE,
					   5 );
    gtk_widget_show ( formulaire_echeancier );


    /*   création de la table */

    table = gtk_table_new ( 6,
			    4,
			    FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER ( table ),
				     10 );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ),
				 5 );
    gtk_widget_set_usize ( table,
			   1,
			   FALSE );
    gtk_box_pack_start ( GTK_BOX ( formulaire_echeancier ),
			 table,
			 TRUE,
			 TRUE,
			 0 );
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
									     50 );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( pression_touche_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_PARTY ) );
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY]) -> entry),
				"focus-in-event",
				GTK_SIGNAL_FUNC ( entree_prend_focus ),
				GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ),
			 "focus-out-event",
			 GTK_SIGNAL_FUNC ( entree_perd_focus_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_PARTY ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
			 GINT_TO_POINTER ( SCHEDULER_FORM_PARTY ) );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> arrow ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( clique_champ_formulaire_echeancier ),
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
										       0 );
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

    if ( ( menu = creation_menu_types ( 1,
					recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
					1 )))
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				   menu );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				      cherche_no_menu_type_echeancier ( TYPE_DEFAUT_DEBIT ) );
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
			 GTK_SIGNAL_FUNC ( fin_edition_echeance ),
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

		    if ( ( menu = creation_menu_types ( 1,
							recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
							1 )))
		    {
			p_tab_nom_de_compte_variable = p_tab_nom_de_compte + recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
						   menu );
			gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
						      cherche_no_menu_type_echeancier ( TYPE_DEFAUT_DEBIT ) );
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

		    if ( ( menu = creation_menu_types ( 2,
							recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
							1  )))
		    {
			p_tab_nom_de_compte_variable = p_tab_nom_de_compte + recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
						   menu );
			gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
						      cherche_no_menu_type_echeancier ( TYPE_DEFAUT_CREDIT ) );
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
void affiche_cache_le_formulaire_echeancier ( void )
{
    if ( etat.formulaire_echeancier_toujours_affiche )
    {
	gtk_widget_hide ( fleche_bas_echeancier );
	gtk_widget_show ( fleche_haut_echeancier );

	gtk_widget_hide ( frame_formulaire_echeancier );
	etat.formulaire_echeancier_toujours_affiche = 0;
    }
    else
    {
	gtk_widget_hide ( fleche_haut_echeancier );
	gtk_widget_show ( fleche_bas_echeancier );

	gtk_widget_show ( frame_formulaire_echeancier );
	etat.formulaire_echeancier_toujours_affiche = 1;
    }

    gtk_widget_grab_focus ( tree_view_liste_echeances );
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
		fin_edition_echeance ();
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
		    fin_edition_echeance ();
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

/******************************************************************************/
/* Fonction fin_edition_echeance */
/* appelée pour soit valider une modif d'échéance ( ou nouvelle ), */
/* ou pour saisir l'échéance */
/******************************************************************************/
void fin_edition_echeance ( void )
{
    struct operation_echeance *echeance;
    gchar **tableau_char;
    gchar *pointeur_char;
    GSList *pointeur_liste;

    gint compte_virement;
    compte_virement = 0;

    /* on vérifie que les date et date_limite sont correctes */

    if ( !modifie_date ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ))
    {
	dialogue_error ( PRESPACIFY(_("Invalid date")) );
	gtk_widget_grab_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );
	gtk_entry_select_region ( GTK_ENTRY (  widget_formulaire_echeancier[SCHEDULER_FORM_DATE]),
				  0,
				  -1);
	return;
    }

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ) == style_entree_formulaire[ENCLAIR]
	 &&
	 strcmp ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ))),
		  _("None") ))
	if ( !modifie_date ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ))
	{
	    dialogue_error ( PRESPACIFY(_("Invalid limit date")) );
	    gtk_widget_grab_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
	    gtk_entry_select_region ( GTK_ENTRY (  widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE]),
				      0,
				      -1);
	    return;
	}

    /* vérifie  si c'est une opération ventilée, */
    /* si c'est le cas, si la liste des ventilation existe (soit adr de liste, soit -1), on va l'enregistrer plus tard */
    /* sinon on va ventiler tout de suite */

    if ( !strcmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ))),
		   _("Breakdown of transaction") )
	 &&
	 !gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
				"liste_adr_ventilation" ))
    {
	enregistre_ope_au_retour_echeances = 1;

	if ( gtk_widget_get_style ( widget_formulaire_echeancier[TRANSACTION_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	    ventiler_operation_echeances ( -my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ))),
							NULL ));
	else
	    ventiler_operation_echeances ( my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ))),
						       NULL ));

	return;
    }


    /* vérification que ce n'est pas un virement sur lui-même */

    if ( !g_strcasecmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ))),
			 g_strconcat ( COLON(_("Transfer")),
				       COMPTE_ECHEANCE,
				       NULL )))
    {
	dialogue_error ( _("Can't issue a transfer its own account.") );
	return;
    }

    /* si c'est un virement, on vérifie que le compte existe  */


    pointeur_char = g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] )));

    if ( !g_strncasecmp ( pointeur_char, _("Transfer"), 8 ))
    {
	gint i;

	tableau_char = g_strsplit ( pointeur_char, ":", 2 );

	/* S'il n'y a rien après "Transfer", alors : */
	if ( !tableau_char[1] ||
	     !strlen ( tableau_char[1] ) )
	{
	    dialogue_error ( _("There is no associated account for this transfer.") );
	    return;
	}

	if ( tableau_char[1] )
	{
	    tableau_char[1] = g_strstrip ( tableau_char[1] );

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	    compte_virement = -1;

	    for ( i = 0 ; i < nb_comptes ; i++ )
	    {
		if ( !g_strcasecmp ( NOM_DU_COMPTE,
				     tableau_char[1] ) )
		    compte_virement = i;
		p_tab_nom_de_compte_variable++;
	    }

	    if ( compte_virement == -1 )
	    {
		dialogue_warning ( _("Associated account of this transfer is invalid.") );
		return;
	    }
	}
	g_strfreev ( tableau_char );
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
	dialogue ( PRESPACIFY(_("Impossible to create or entry an automatic scheduled transaction\n with a cheque or a method of payment with an automatic incremental number.")) );
	return;
    }

    /* récupération de l'opération : soit l'adr de la struct, soit NULL si nouvelle */

    echeance = gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
				     "adr_echeance" );

    /* on sépare ici en 2 parties : si le label label_saisie_modif contient Modification, c'est une modif ou une nvelle échéance, */
    /* s'il contient Saisie, on enregistre l'opé */

    if ( strcmp ( GTK_LABEL ( label_saisie_modif ) -> label,
		  _("Input") ) )
    {
	/*       on commence ici la partie modification / nouvelle échéance */

	/* si c'est une nouvelle échéance, on la crée */
	/* et on lui met son numéro tout de suite */

	if ( !echeance )
	{
	    echeance = calloc ( 1,
				sizeof ( struct operation_echeance ));
	    echeance -> no_operation = ++no_derniere_echeance;
	    nb_echeances++;
	}

	/* récupère la date */


	pointeur_char = g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY (widget_formulaire_echeancier[SCHEDULER_FORM_DATE] )));

	tableau_char = g_strsplit ( pointeur_char,
				    "/",
				    3 );


	echeance -> jour = my_strtod ( tableau_char[0],
				       NULL );
	echeance -> mois = my_strtod ( tableau_char[1],
				       NULL );
	echeance -> annee = my_strtod (tableau_char[2],
				       NULL );

	echeance ->date = g_date_new_dmy ( echeance ->jour,
					   echeance ->mois,
					   echeance ->annee);


	/* récupération du tiers, s'il n'existe pas, on le crée */

	if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	{
	    pointeur_char = gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ));

	    echeance -> tiers  = tiers_par_nom ( pointeur_char,
						 1 ) -> no_tiers;
	}

	/* récupération du montant */

	if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	    echeance -> montant = -my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ))),
					       NULL );
	else
	    echeance -> montant = my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ))),
					      NULL );

	/* récupération de la devise */

	echeance -> devise = ((struct struct_devise *)( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ) -> menu_item ),
									      "adr_devise" ))) -> no_devise;

	/* récupération du no de compte */

	echeance -> compte = recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );


	/*   récupération des catégories / sous-catég, s'ils n'existent pas, on les crée */
	/* s'il n'y a pas de catég, ce n'es pas un virement non plus, donc on met compte_virement à -1 */

	if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	{
	    struct struct_categ *categ;

	    tableau_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ))),
					":",
					2 );

	    tableau_char[0] = g_strstrip ( tableau_char[0] );

	    if ( tableau_char[1] )
		tableau_char[1] = g_strstrip ( tableau_char[1] );


	    if ( strlen ( tableau_char[0] ) )
	    {
		if ( !strcmp ( tableau_char[0],
			       _("Transfer") )
		     && tableau_char[1]
		     && strlen ( tableau_char[1]) )
		{
		    /* c'est un virement, il n'y a donc aucune catégorie */

		    gint i;
		    echeance -> categorie = 0;
		    echeance -> sous_categorie = 0;
		    echeance -> operation_ventilee = 0;

		    /* recherche le no de compte du virement */

		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

		    for ( i = 0 ; i < nb_comptes ; i++ )
		    {
			if ( !g_strcasecmp ( NOM_DU_COMPTE,
					     tableau_char[1] ) )
			    echeance -> compte_virement = i;

			p_tab_nom_de_compte_variable++;
		    }
		}
		else
		{
		    /* 		    si c'est une opération ventilée, c'est ici que ça se passe ! */


		    if ( !strcmp ( tableau_char[0],
				   _("Breakdown of transaction") ))
		    {
			/* c'est une ventilation, il n'y a donc aucune catégorie */
			/* on va appeler la fonction validation_ope_de_ventilation */
			/* qui va créer les nouvelles opé*/

			validation_ope_de_ventilation_echeances ( echeance );
			echeance -> categorie = 0;
			echeance -> sous_categorie = 0;
			echeance -> operation_ventilee = 1;
			echeance -> compte_virement = 0;

		    }
		    else
		    {	
			struct struct_sous_categ *sous_categ;

			categ = categ_par_nom ( tableau_char[0],
						1,
						0,
						0 );

			if ( categ )
			{
			    echeance -> categorie = categ -> no_categ;

			    sous_categ = sous_categ_par_nom ( categ,
							      tableau_char[1],
							      1 );

			    if ( sous_categ )
				echeance -> sous_categorie = sous_categ -> no_sous_categ;
			}

			echeance -> compte_virement = 0;
			echeance -> operation_ventilee = 0;
		    }
		}
	    }
	    g_strfreev ( tableau_char );
	}
	else
	    echeance -> compte_virement = -1;

	/* récupération du type d'opération */

	if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ))
	{
	    echeance -> type_ope = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
									   "no_type" ));

	    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] )
		 &&
		 gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ) == style_entree_formulaire[ENCLAIR] )
		echeance -> contenu_type = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ))));
	}

	/* récupération du no d'exercice */

	echeance -> no_exercice = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ) -> menu_item ),
									  "no_exercice" ));

	/* récupération de l'imputation budgétaire */

	if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	{
	    struct struct_imputation *imputation;

	    pointeur_char = gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ));

	    tableau_char = g_strsplit ( pointeur_char,
					":",
					2 );

	    imputation = imputation_par_nom ( tableau_char[0],
					      1,
					      echeance -> montant < 0,
					      0 );

	    if ( imputation )
	    {
		struct struct_sous_imputation *sous_imputation;

		echeance -> imputation = imputation -> no_imputation;

		sous_imputation = sous_imputation_par_nom ( imputation,
							    tableau_char[1],
							    1 );

		if ( sous_imputation )
		    echeance -> sous_imputation = sous_imputation -> no_sous_imputation;
	    }

	    g_strfreev ( tableau_char );
	}

	/*       récupération de auto_man */

	echeance -> auto_man = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE]  ) -> menu_item ),
								       "auto_man" ) );

	/* récupération des notes */

	if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ) == style_entree_formulaire[ENCLAIR] )
	    echeance -> notes = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ))));


	/* récupération de la fréquence */

	echeance -> periodicite = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY]  ) -> menu_item ),
									  "periodicite" ) );

	if ( echeance -> periodicite == 4 )
	{
	    echeance -> intervalle_periodicite_personnalisee = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU]  ) -> menu_item ),
												       "intervalle_perso" ) );

	    echeance -> periodicite_personnalisee = my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] )),
								NULL );
	}

	if ( echeance -> periodicite
	     &&
	     gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ) == style_entree_formulaire[ENCLAIR] )
	{
	    /* traitement de la date limite */

	    tableau_char = g_strsplit ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ))),
					"/",
					3 );

	    echeance -> jour_limite = my_strtod ( tableau_char[0],
						  NULL );
	    echeance -> mois_limite = my_strtod ( tableau_char[1],
						  NULL );
	    echeance -> annee_limite = my_strtod (tableau_char[2],
						  NULL );

	    echeance->date_limite = g_date_new_dmy ( echeance->jour_limite,
						     echeance->mois_limite,
						     echeance->annee_limite);
	}


	/* si c'est une nouvelle opé,  on l'ajoute à la liste */

	if ( !gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
				    "adr_echeance" ) )
	    liste_struct_echeances = g_slist_insert_sorted ( liste_struct_echeances,
							     echeance,
							     (GCompareFunc) comparaison_date_echeance );
    }
    else
    {
	/* on commence ici la partie saisie de l'échéance */

	struct structure_operation *operation;
	gint virement;
	struct struct_devise *devise;

	/* crée l'opération */

	operation = calloc ( 1,
			     sizeof ( struct structure_operation ) ); 


	/* récupère la date */


	pointeur_char = g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY (widget_formulaire_echeancier[SCHEDULER_FORM_DATE] )));

	tableau_char = g_strsplit ( pointeur_char,
				    "/",
				    3 );


	operation -> jour = my_strtod ( tableau_char[0],
					NULL );
	operation -> mois = my_strtod ( tableau_char[1],
					NULL );
	operation -> annee = my_strtod (tableau_char[2],
					NULL );

	operation ->date = g_date_new_dmy ( operation ->jour,
					    operation ->mois,
					    operation ->annee);


	/* récupération du no de compte */

	operation -> no_compte = recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );


	/* récupération du tiers, s'il n'existe pas, on le crée */

	if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	{
	    pointeur_char = gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ));

	    operation -> tiers = tiers_par_nom ( pointeur_char,
						 1 ) -> no_tiers;
	}


	/* récupération du montant */

	if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	    operation -> montant = -my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ))),
						NULL );
	else
	    operation -> montant = my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ))),
					       NULL );

	/* récupération de la devise */

	devise = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ) -> menu_item),
				       "adr_devise" );


	/* si c'est la devise du compte ou si c'est un compte qui doit passer à l'euro ( la transfo se fait au niveau de l'affichage de la liste ) */
	/* ou si c'est un compte en euro et l'opé est dans une devise qui doit passer à l'euro -> ok */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	if ( !devise_compte
	     ||
	     devise_compte -> no_devise != DEVISE )
	    devise_compte = devise_par_no ( DEVISE );

	operation -> devise = devise -> no_devise;

	if ( !( operation -> no_operation
		||
		devise -> no_devise == DEVISE
		||
		( devise_compte -> passage_euro && !strcmp ( devise -> nom_devise, _("Euro") ))
		||
		( !strcmp ( devise_compte -> nom_devise, _("Euro") ) && devise -> passage_euro )))
	{
	    /* c'est une devise étrangère, on demande le taux de change et les frais de change */

	    demande_taux_de_change ( devise_compte, devise, 1,
				     (gdouble ) 0, (gdouble ) 0, FALSE );

	    operation -> taux_change = taux_de_change[0];
	    operation -> frais_change = taux_de_change[1];

	    if ( operation -> taux_change < 0 )
	    {
		operation -> taux_change = -operation -> taux_change;
		operation -> une_devise_compte_egale_x_devise_ope = 1;
	    }
	}


	/*   récupération des catégories / sous-catég, s'ils n'existent pas, on les crée */

	virement = 0;

	if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	{
	    struct struct_categ *categ;

	    tableau_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ))),
					":",
					2 );

	    tableau_char[0] = g_strstrip ( tableau_char[0] );

	    if ( tableau_char[1] )
		tableau_char[1] = g_strstrip ( tableau_char[1] );


	    if ( strlen ( tableau_char[0] ) )
	    {
		if ( !strcmp ( tableau_char[0],
			       _("Transfer") )
		     && tableau_char[1]
		     && strlen ( tableau_char[1]) )
		{
		    /* c'est un virement, il n'y a donc aucune catétorie */

		    operation -> categorie = 0;
		    operation -> sous_categorie = 0;
		    virement = 1;
		}
		else
		{
		    categ = categ_par_nom ( tableau_char[0],
					    1,
					    0,
					    0 );

		    if ( categ )
		    {
			struct struct_sous_categ *sous_categ;

			operation -> categorie = categ -> no_categ;

			sous_categ = sous_categ_par_nom ( categ,
							  tableau_char[1],
							  1 );
			if ( sous_categ )
			    operation -> sous_categorie = sous_categ -> no_sous_categ;
		    }
		}
	    }
	    g_strfreev ( tableau_char );
	}

	/* récupération du type d'opération */

	if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ))
	{
	    operation -> type_ope = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
									    "no_type" ));

	    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] )
		 &&
		 gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ) == style_entree_formulaire[ENCLAIR] )
	    {
		struct struct_type_ope *type;

		type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
					     "adr_type" );

		operation -> contenu_type = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ))));

		if ( type -> numerotation_auto )
		    type -> no_en_cours = ( my_atoi ( operation -> contenu_type ));
	    }
	}


	/* récupÃ©ration du no d'exercice */

	if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ) -> menu_item ),
						     "no_exercice" )) == -2 )
	    operation -> no_exercice = recherche_exo_correspondant ( operation -> date );
	else
	    operation -> no_exercice = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ) -> menu_item ),
									       "no_exercice" ));


	/* récupération de l'imputation budgétaire */

	if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
	{
	    struct struct_imputation *imputation;

	    pointeur_char = gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ));

	    tableau_char = g_strsplit ( pointeur_char,
					":",
					2 );

	    imputation = imputation_par_nom ( tableau_char[0],
					      1,
					      operation -> montant < 0,
					      0 );

	    if ( imputation )
	    {
		struct struct_sous_imputation *sous_imputation;

		operation -> imputation = imputation -> no_imputation;

		sous_imputation = sous_imputation_par_nom ( imputation,
							    tableau_char[1],
							    1 );

		if ( sous_imputation )
		    operation -> sous_imputation = sous_imputation -> no_sous_imputation;
	    }
	    else
		operation -> sous_imputation = 0;

	    g_strfreev ( tableau_char );
	}

	/*       récupération de auto_man */

	operation -> auto_man = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE]  ) -> menu_item ),
									"auto_man" ) );

	/* récupération des notes */

	if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ) == style_entree_formulaire[ENCLAIR] )
	    operation -> notes = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ))));


	/*   on a fini de remplir l'opé, on peut l'ajouter à la liste */
	/* 	c'est forcemment une nouvelle opé, donc on utilise ajout_operation */

	ajout_operation ( operation );

	/* si c'est un virement, on crée la contre opération et met les relations */
	/* FIXME : ça serait bien de faire apparaitre un bouton de contre type ope dans le formulaire qd virement */

	if ( virement )
	    cree_contre_operation_echeance ( operation,
					     compte_virement,
					     operation -> type_ope );

	/* 	si c'était une échéance ventilée, c'est ici qu'on fait joujou */

	pointeur_liste = gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
					       "liste_adr_ventilation" );
	while ( pointeur_liste
		&&
		pointeur_liste != GINT_TO_POINTER ( -1 ))
	{
	    struct struct_ope_ventil *ope_ventil;
	    struct structure_operation *operation_fille;

	    /* 	    la mère est donc une ventil */

	    operation -> operation_ventilee = 1;

	    ope_ventil = pointeur_liste -> data;
	    operation_fille = calloc ( 1,
				       sizeof ( struct structure_operation ));

	    operation_fille -> montant = ope_ventil -> montant;
	    operation_fille -> categorie = ope_ventil -> categorie;
	    operation_fille -> sous_categorie = ope_ventil -> sous_categorie;

	    if ( ope_ventil -> notes )
		operation_fille -> notes = g_strdup ( ope_ventil -> notes );

	    operation_fille -> imputation = ope_ventil -> imputation;
	    operation_fille -> sous_imputation = ope_ventil -> sous_imputation;

	    if ( ope_ventil -> no_piece_comptable )
		operation_fille -> no_piece_comptable = g_strdup ( ope_ventil -> no_piece_comptable);

	    operation_fille -> no_exercice = ope_ventil -> no_exercice;

	    /* 	    le reste est identique à la mère */

	    operation_fille -> jour = operation -> jour;
	    operation_fille -> mois = operation -> mois;
	    operation_fille -> annee = operation -> annee;
	    operation_fille -> date = g_date_new_dmy ( operation_fille -> jour,
						       operation_fille -> mois,
						       operation_fille -> annee );

	    if ( operation -> jour_bancaire )
	    {
		operation_fille -> jour_bancaire = operation -> jour_bancaire;
		operation_fille -> mois_bancaire = operation -> mois_bancaire;
		operation_fille -> annee_bancaire = operation -> annee_bancaire;
		operation_fille -> date_bancaire = g_date_new_dmy ( operation_fille -> jour_bancaire,
								    operation_fille -> mois_bancaire,
								    operation_fille -> annee_bancaire );
	    }

	    operation_fille -> no_compte = operation -> no_compte;
	    operation_fille -> devise = operation -> devise;
	    operation_fille -> une_devise_compte_egale_x_devise_ope = operation -> une_devise_compte_egale_x_devise_ope;
	    operation_fille -> taux_change = operation -> taux_change;
	    operation_fille -> frais_change = operation -> frais_change;
	    operation_fille -> tiers = operation -> tiers;
	    operation_fille -> pointe = operation -> pointe;
	    operation_fille -> auto_man = operation -> auto_man;
	    operation_fille -> no_operation_ventilee_associee = operation -> no_operation;


	    /*   on a fini de remplir l'opé, on peut l'ajouter à la liste */
	    /* 	    comme c'est une opé de ventilation, elle ne sera pas affiché et ne changera */
	    /* 		rien au solde, donc on l'ajoute juste à la sliste */

	    insere_operation_dans_liste ( operation_fille );

	    /* 	    on vérifie maintenant si c'est un virement */

	    if ( ope_ventil -> relation_no_operation == -1 )
	    {
		/* cette opé de ventil est un virement */

		cree_contre_operation_echeance ( operation_fille,
						 ope_ventil -> relation_no_compte,
						 ope_ventil -> no_type_associe );
	    }

	    pointeur_liste = pointeur_liste -> next;
	}


	/* passe l'échéance à la prochaine date */

	incrementation_echeance ( echeance );

	/* réaffiche les option menu de la périodicité, des banques et de l'automatisme, effacés pour une saisie d'opé */

	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] );
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] );
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] );

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
/******************************************************************************/




/******************************************************************************/
void cree_contre_operation_echeance ( struct structure_operation *operation,
				      gint compte_virement,
				      gint contre_type_ope )
{
    struct structure_operation *contre_operation;
    
    /*     on crée d'abord la contre opération et l'ajoute dans la liste des opés */

    contre_operation = ajoute_contre_operation_echeance_dans_liste ( operation,
								     compte_virement,
								     contre_type_ope );


    /*     on ajoute maintenant cette opé dans la list_store des opés */
    /* 	comme cette opé a déjà un no, elle ne va pas être réajoutée à la sliste */

    ajout_operation ( contre_operation );

}
/******************************************************************************/


/******************************************************************************/
/* cette fonction crée et ajoute la contre opération dans la slist */
/* mais ne s'occupe pas du tout des mises à jour graphiques */
/* elle renvoie l'opération nouvellement créé */
/******************************************************************************/

struct structure_operation *ajoute_contre_operation_echeance_dans_liste ( struct structure_operation *operation,
									  gint compte_virement,
									  gint contre_type_ope )
{
    /*   si c'Ã©tait un virement, on crée une copie de l'opé, on l'ajout à la liste puis on remplit les relations */

    struct structure_operation *contre_operation;
    struct struct_devise *contre_devise;
    struct struct_devise *devise;


    contre_operation = calloc ( 1,
				sizeof ( struct structure_operation ) );

    contre_operation -> no_compte = compte_virement;


    /* remplit la nouvelle opé */

    contre_operation -> jour = operation -> jour;
    contre_operation -> mois = operation -> mois;
    contre_operation -> annee = operation -> annee;
    contre_operation ->date = g_date_new_dmy ( contre_operation->jour,
					       contre_operation->mois,
					       contre_operation->annee);
    contre_operation -> montant = -operation -> montant;

    /* si c'est la devise du compte ou si c'est un compte qui doit passer à l'euro ( la transfo se fait au niveau */
    /* de l'affichage de la liste ) ou si c'est un compte en euro et l'opé est dans une devise qui doit passer à l'euro -> ok */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_virement;

    contre_devise = devise_par_no ( DEVISE );

    contre_operation -> devise = operation -> devise;

    /* récupération de la devise */

    devise = devise_par_no ( operation -> devise );

    if ( !( contre_operation-> no_operation
	    ||
	    devise -> no_devise == DEVISE
	    ||
	    ( contre_devise -> passage_euro && !strcmp ( devise -> nom_devise, _("Euro") ))
	    ||
	    ( !strcmp ( contre_devise -> nom_devise, _("Euro") ) && devise -> passage_euro )))
    {
	/* c'est une devise étrangère, on demande le taux de change et les frais de change */

	demande_taux_de_change ( contre_devise, devise, 1,
				 (gdouble ) 0, (gdouble ) 0, FALSE );

	contre_operation -> taux_change = taux_de_change[0];
	contre_operation -> frais_change = taux_de_change[1];

	if ( contre_operation -> taux_change < 0 )
	{
	    contre_operation -> taux_change = -contre_operation -> taux_change;
	    contre_operation -> une_devise_compte_egale_x_devise_ope = 1;
	}
    }
    else
    {
	contre_operation -> taux_change = 0;
	contre_operation -> frais_change = 0;
    }

    contre_operation -> tiers = operation -> tiers;
    contre_operation -> categorie = 0;
    contre_operation -> sous_categorie = 0;

    if ( operation -> notes )
	contre_operation -> notes = g_strdup ( operation -> notes);

    contre_operation -> auto_man = operation -> auto_man;
    contre_operation -> type_ope = contre_type_ope;

    if ( operation -> contenu_type )
	contre_operation -> contenu_type = g_strdup ( operation -> contenu_type );

    contre_operation -> no_exercice = operation -> no_exercice;
    contre_operation -> imputation = operation -> imputation;
    contre_operation -> sous_imputation = operation -> sous_imputation;

    /*   on a fini de remplir l'opé, on peut l'ajouter à la liste */

    insere_operation_dans_liste ( contre_operation );

    /* on met maintenant les relations entre les différentes opé */

    operation -> relation_no_operation = contre_operation -> no_operation;
    operation -> relation_no_compte = contre_operation -> no_compte;
    contre_operation -> relation_no_operation = operation -> no_operation;
    contre_operation -> relation_no_compte = operation -> no_compte;

    return ( contre_operation );
}
/******************************************************************************/





/******************************************************************************/
/* cette procédure compare 2 struct d'échéances entre elles au niveau de la date */
/* pour le classement */
/******************************************************************************/
gint comparaison_date_echeance (  struct operation_echeance *echeance_1,
				  struct operation_echeance *echeance_2)
{
    return ( g_date_compare ( echeance_1 -> date,
			      echeance_2 -> date ));
}
/******************************************************************************/

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

/******************************************************************************/
void incrementation_echeance ( struct operation_echeance *echeance )
{
    GDate *date_suivante;

    /* périodicité hebdomadaire */
    if ( echeance -> periodicite == 1 )
    {
	g_date_add_days ( echeance -> date,
			  7 );

	/* magouille car il semble y avoir un bug dans g_date_add_days qui ne fait pas l'addition si on ne met pas la ligne suivante */
	g_date_add_months ( echeance -> date,
			    0 );
    }
    else
	/* périodicité mensuelle */
	if ( echeance -> periodicite == 2 )
	    g_date_add_months ( echeance -> date,
				1 );
	else
	    /* périodicité annuelle */
	    if ( echeance -> periodicite == 3 )
		g_date_add_years ( echeance -> date,
				   1 );
	    else
		/* périodicité perso */
		if ( !echeance -> intervalle_periodicite_personnalisee )
		{
		    g_date_add_days ( echeance -> date,
				      echeance -> periodicite_personnalisee );

		    /* magouille car il semble y avoir un bug dans g_date_add_days qui ne fait pas l'addition si on ne met pas la ligne suivante */
		    g_date_add_months ( echeance -> date,
					0 );
		}
		else
		    if ( echeance -> intervalle_periodicite_personnalisee == 1 )
			g_date_add_months ( echeance -> date,
					    echeance -> periodicite_personnalisee );
		    else
			g_date_add_years ( echeance -> date,
					   echeance -> periodicite_personnalisee );

    /* on recommence l'incrémentation sur la copie de la date pour avoir la date suivante */
    /* permet de voir si c'était la dernière incrémentation */

    date_suivante = g_date_new_dmy ( echeance -> date -> day,
				     echeance -> date -> month,
				     echeance -> date -> year ); 


    /* périodicité hebdomadaire */

    if ( echeance -> periodicite == 1 )
    {
	g_date_add_days ( date_suivante,
			  7 );
	/* magouille car il semble y avoir un bug dans g_date_add_days qui ne fait pas l'addition si on ne met pas la ligne suivante */
	g_date_add_months ( echeance -> date,
			    0 );
    }
    else
	/* périodicité mensuelle */
	if ( echeance -> periodicite == 2 )
	    g_date_add_months ( date_suivante,
				1 );
	else
	    /* périodicité annuelle */
	    if ( echeance -> periodicite == 3 )
		g_date_add_years ( date_suivante,
				   1 );
	    else
		/* périodicité perso */
		if ( !echeance -> intervalle_periodicite_personnalisee )
		{
		    /* magouille car il semble y avoir un bug dans g_date_add_days qui ne fait pas l'addition si on ne met pas la ligne suivante */
		    g_date_add_months ( echeance -> date,
					0 );
		    g_date_add_days ( date_suivante,
				      echeance -> periodicite_personnalisee );
		}
		else
		    if ( echeance -> intervalle_periodicite_personnalisee == 1 )
			g_date_add_months ( date_suivante,
					    echeance -> periodicite_personnalisee );
		    else
			g_date_add_years ( date_suivante,
					   echeance -> periodicite_personnalisee );


    /* si l'échéance est finie, on la vire, sinon on met à jour les var jour, mois et année */

    if ( !echeance -> periodicite 
	 ||
	 (
	  echeance -> date_limite
	  &&
	  g_date_compare ( echeance -> date,
			   echeance -> date_limite ) > 0  ))
    {
	GtkWidget *label;

	if ( GTK_BIN ( frame_etat_echeances_finies ) -> child )
	{
	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + echeance ->compte;

	    if ( echeance -> montant >= 0 )
		label = gtk_label_new ( g_strdup_printf (PRESPACIFY(_("%4.2f %s credit on %s")),
							 echeance ->montant,
							 devise_code_by_no ( echeance -> devise ),
							 NOM_DU_COMPTE ));
	    else
		label = gtk_label_new ( g_strdup_printf (PRESPACIFY(_("%4.2f %s debit on %s")),
							 -echeance ->montant,
							 devise_code_by_no ( echeance -> devise ),
							 NOM_DU_COMPTE ));


	    gtk_misc_set_alignment ( GTK_MISC ( label ),
				     0,
				     0.5);
	    gtk_box_pack_start ( GTK_BOX ( GTK_BIN ( frame_etat_echeances_finies ) -> child ),
				 label,
				 FALSE,
				 TRUE,
				 5 );
	    gtk_widget_show (  label );


	}
	else
	{
	    GtkWidget *vbox;

	    vbox = gtk_vbox_new ( FALSE,
				  5 );
	    gtk_container_add ( GTK_CONTAINER ( frame_etat_echeances_finies ),
				vbox );
	    gtk_widget_show ( vbox );

	    label = gtk_label_new ("");
	    gtk_box_pack_start ( GTK_BOX ( vbox ),
				 label,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( label );

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + echeance ->compte;

	    if ( echeance -> montant >= 0 )
		label = gtk_label_new ( g_strdup_printf (PRESPACIFY(_("%4.2f %s credit on %s")),
							 echeance ->montant,
							 devise_code_by_no ( echeance -> devise ),
							 NOM_DU_COMPTE ));
	    else
		label = gtk_label_new ( g_strdup_printf (PRESPACIFY(_("%4.2f %s debit on %s")),
							 -echeance ->montant,
							 devise_code_by_no ( echeance -> devise ),
							 NOM_DU_COMPTE ));


	    gtk_misc_set_alignment ( GTK_MISC ( label ),
				     0,
				     0.5);
	    gtk_box_pack_start ( GTK_BOX ( vbox ),
				 label,
				 FALSE,
				 TRUE,
				 5 );
	    gtk_widget_show (  label );


	    gtk_widget_show ( frame_etat_echeances_finies );
	    /*  gtk_widget_show ( separateur_ech_finies_soldes_mini ); */
	}

	liste_struct_echeances = g_slist_remove ( liste_struct_echeances, 
						  echeance );
	free ( echeance );
	nb_echeances--;

	if ( ( echeance_selectionnnee = echeance ))
	    echeance_selectionnnee = gtk_clist_get_row_data ( GTK_CLIST ( tree_view_liste_echeances ),
							      gtk_clist_find_row_from_data ( GTK_CLIST ( tree_view_liste_echeances ),
											     echeance ) + 1);

    }
    else
    {
	echeance -> jour = echeance -> date -> day;
	echeance -> mois = echeance -> date -> month;
	echeance -> annee = echeance -> date -> year;
    }

    g_date_free ( date_suivante );

}
/******************************************************************************/

/******************************************************************************/
/* Fonction completion_operation_par_tiers_echeancier */
/* appelée lorsque le tiers perd le focus */
/* récupère le tiers, et recherche la dernière opé associée à ce tiers */
/******************************************************************************/
void completion_operation_par_tiers_echeancier ( void )
{
    struct struct_tiers *tiers;
    struct structure_operation *operation;
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
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

    operation = NULL;
    pointeur_ope = LISTE_OPERATIONS;

    while ( pointeur_ope )
    {
	struct structure_operation *ope_test;

	ope_test = pointeur_ope -> data;

	if ( ope_test -> tiers == tiers -> no_tiers )
	{
	    if ( operation )
	    {
		if ( g_date_compare ( ope_test -> date,
				      operation -> date ) >= 0 )
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

	gint i;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    if ( i != no_compte )
	    {
		pointeur_ope = LISTE_OPERATIONS;

		while ( pointeur_ope )
		{
		    struct structure_operation *ope_test;

		    ope_test = pointeur_ope -> data;

		    if ( ope_test -> tiers == tiers -> no_tiers )
		    {
			operation = ope_test;
			pointeur_ope = NULL;
			i = nb_comptes;
		    }
		    else
			pointeur_ope = pointeur_ope -> next;
		}
	    }
	    p_tab_nom_de_compte_variable++;
	}
    }

    /* si on n'a trouvé aucune opération, on se tire */

    if ( !operation )
	return;

    /* remplit les différentes entrées du formulaire */

    /* remplit les montant */

    if ( operation -> montant < 0 )
    {
	GtkWidget *menu;

	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
			     g_strdup_printf ( "%4.2f",
					       -operation -> montant ));
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
					       operation -> montant ));
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
						  devise_par_no ( operation -> devise )));

    /* mise en forme des catégories */

    /* vérifie si c'est un virement */

    if ( operation -> relation_no_operation )
    {
	/* c'est un virement, on l'affiche */

	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY]);

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
				g_strconcat ( COLON(_("Transfer")),
					      NOM_DU_COMPTE,
					      NULL ));
    }
    else
    {
	char_tmp = nom_categ_par_no ( operation -> categorie,
				      operation -> sous_categorie );
	
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

	place_type = cherche_no_menu_type_echeancier ( operation -> type_ope );

	/*       si la place est trouvée, on la met, sinon on met à la place par défaut */

	if ( place_type != -1 )
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					  place_type );
	else
	{
	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	    if ( operation -> montant < 0 )
		place_type = cherche_no_menu_type_echeancier ( TYPE_DEFAUT_DEBIT );
	    else
		place_type = cherche_no_menu_type_echeancier ( TYPE_DEFAUT_CREDIT );

	    if ( place_type != -1 )
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					      place_type );
	    else
	    {
		struct struct_type_ope *type;

		gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					      0 );

		/*  on met ce type par défaut, vu que celui par défaut marche plus ... */

		if ( operation -> montant < 0 )
		    TYPE_DEFAUT_DEBIT = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
										"no_type" ));
		else
		    TYPE_DEFAUT_CREDIT = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu_item ),
										 "no_type" ));

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
				   cherche_no_menu_exercice ( operation -> no_exercice,
							      widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ));

    /* met en place l'imputation budgétaire */


    char_tmp = nom_imputation_par_no ( operation -> imputation,
				       operation -> sous_imputation );

   if ( char_tmp )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY]);
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
				char_tmp );
    }


    /*   remplit les notes */

    if ( operation -> notes )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			     operation -> notes );
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


