
/* Fichier echeancier_ventilation.c */
/* s'occupe de tout ce qui concerne les ventilation des échéances */


/*     Copyright (C) 2000-2004  Cédric Auger */
/* 			cedric@grisbi.org */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "echeancier_ventilation.h"
#include "echeancier_liste.h"
#include "operations_classement.h"
#include "operations_formulaire.h"
#include "exercice.h"
#include "type_operations.h"
#include "dialog.h"
#include "search_glist.h"
#include "categories_onglet.h"
#include "utils.h"
#include "imputation_budgetaire.h"
#include "echeancier_formulaire.h"
#include "constants.h"





GtkWidget *liste_echeances_ventilees;

gint ancienne_largeur_ventilation_echeances;          /* magouille utilisée pour bloquer un signal size-allocate qui s'emballe */
GtkWidget *widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_TOTAL_WIDGET];
GtkWidget *separateur_formulaire_ventilations_echeances;
GtkWidget *hbox_valider_annuler_ventil_echeances;
GtkWidget *label_somme_ventilee_echeances_echeances;                       /*  label correspondant */
GtkWidget *label_non_affecte_echeances;                       /*  label correspondant */
GtkWidget *label_montant_operation_ventilee_echeances_echeances;                       /*  label correspondant */
struct struct_ope_ventil *ligne_selectionnee_ventilation_echeances;              /* struct ope ligne sélectionnée */
gdouble montant_operation_ventilee_echeances;
gdouble somme_ventilee_echeances;
gint enregistre_ope_au_retour_echeances;            /* à 1 si au click du bouton valider on enregistre l'opé */

extern GSList *liste_categories_ventilation_combofix; 
extern GtkWidget *formulaire_echeancier;
extern GSList *gsliste_echeances; 
extern gint nb_echeances;
extern gint no_derniere_echeance;
extern GtkWidget *notebook_calendrier_ventilations;
extern GtkWidget *notebook_formulaire_echeances;
extern GtkWidget *notebook_liste_ventil_echeances;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];




/*******************************************************************************************/
/* Fonction creation_fenetre_ventilation_echeances */
/* crée la fenetre qui contient la liste des ventilation */
/*******************************************************************************************/

GtkWidget *creation_fenetre_ventilation_echeances ( void )
{
    GtkWidget *onglet;
    gchar *titres[] ={ _("Category"),
	_("Notes"),
	_("Amount") };

	/* création de la scrolled window  */

	onglet = gtk_scrolled_window_new ( NULL,
					   NULL);
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( onglet ),
					 GTK_POLICY_NEVER,
					 GTK_POLICY_AUTOMATIC);
	gtk_widget_show ( onglet );


	/* création de la liste */

	liste_echeances_ventilees = gtk_clist_new_with_titles ( 3,
								titres );
	gtk_widget_set_usize ( GTK_WIDGET ( liste_echeances_ventilees ),
			       1,
			       FALSE );

	/*   par défaut, le classement de la liste s'effectue par no des opérations */

	gtk_clist_set_compare_func ( GTK_CLIST ( liste_echeances_ventilees ),
				     (GtkCListCompareFunc) classement_liste_par_no_ope_ventil );

	gtk_signal_connect ( GTK_OBJECT ( liste_echeances_ventilees ),
			     "size-allocate",
			     GTK_SIGNAL_FUNC ( changement_taille_liste_ventilation_echeances ),
			     NULL );
	gtk_container_add ( GTK_CONTAINER ( onglet ),
			    liste_echeances_ventilees );
	gtk_widget_show ( liste_echeances_ventilees );


	/* On annule la fonction bouton des titres */

	gtk_clist_column_titles_passive ( GTK_CLIST ( liste_echeances_ventilees ));

	gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_echeances_ventilees ),
					  0,
					  FALSE );
	gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_echeances_ventilees ),
					  1,
					  FALSE );
	gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_echeances_ventilees ),
					  2,
					  FALSE );


	/* justification du contenu des cellules */


	gtk_clist_set_column_justification ( GTK_CLIST ( liste_echeances_ventilees ),
					     0,
					     GTK_JUSTIFY_LEFT);
	gtk_clist_set_column_justification ( GTK_CLIST ( liste_echeances_ventilees ),
					     1,
					     GTK_JUSTIFY_LEFT);
	gtk_clist_set_column_justification ( GTK_CLIST ( liste_echeances_ventilees ),
					     2,
					     GTK_JUSTIFY_RIGHT);


	/* vérifie le simple ou double click */

	gtk_signal_connect ( GTK_OBJECT ( liste_echeances_ventilees ),
			     "button_press_event",
			     GTK_SIGNAL_FUNC ( selectionne_ligne_souris_ventilation_echeances ),
			     NULL );


	/*   vérifie la touche entrée, haut et bas */

	gtk_signal_connect ( GTK_OBJECT ( liste_echeances_ventilees ),
			     "key_press_event",
			     GTK_SIGNAL_FUNC ( traitement_clavier_liste_ventilation_echeances ),
			     NULL );



	return ( onglet );
}
/*******************************************************************************************/



/*******************************************************************************************/
/* Fonction  creation_verification_ventilation_echeances*/
/* crée la fenetre à la place de la liste des comptes qui contient les boutons et l'état de la ventilation */
/*******************************************************************************************/

GtkWidget *creation_verification_ventilation_echeances ( void )
{
    GtkWidget *onglet;
    GtkWidget *label;
    GtkWidget *frame;
    GtkWidget *tableau;
    GtkWidget *separateur;
    GtkWidget *hbox;
    GtkWidget *bouton;


    /* création de la vbox */

    onglet = gtk_vbox_new ( FALSE,
			    10 );
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				     10 );
    gtk_widget_show ( onglet );


    /* création du titre "opération ventilée" */

    frame = gtk_frame_new ( NULL );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    label = gtk_label_new ( _("Breakdown of transaction") );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			label );
    gtk_widget_show ( label );


    /* création du tableau */

    tableau = gtk_table_new ( 4,
			      2,
			      FALSE );
    gtk_table_set_row_spacings ( GTK_TABLE ( tableau ),
				 10 );
    gtk_table_set_col_spacings ( GTK_TABLE ( tableau ),
				 10 );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 tableau,
			 FALSE,
			 FALSE,
			 20 );
    gtk_widget_show ( tableau );


    label = gtk_label_new ( COLON(_("Break down amount")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_table_attach ( GTK_TABLE ( tableau ),
		       label,
		       0, 1,
		       0, 1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( label );

    label_somme_ventilee_echeances_echeances = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_somme_ventilee_echeances_echeances ),
			     1,
			     0.5 );
    gtk_table_attach ( GTK_TABLE ( tableau ),
		       label_somme_ventilee_echeances_echeances,
		       1, 2,
		       0, 1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( label_somme_ventilee_echeances_echeances );


    label = gtk_label_new ( COLON(_("Not assigned")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_table_attach ( GTK_TABLE ( tableau ),
		       label,
		       0, 1,
		       1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( label );

    label_non_affecte_echeances = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_non_affecte_echeances ),
			     1,
			     0.5 );
    gtk_table_attach ( GTK_TABLE ( tableau ),
		       label_non_affecte_echeances,
		       1, 2,
		       1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( label_non_affecte_echeances );


    separateur = gtk_hseparator_new ();
    gtk_table_attach ( GTK_TABLE ( tableau ),
		       separateur,
		       0, 2,
		       2, 3,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( separateur );



    label = gtk_label_new ( COLON(_("Amount")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_table_attach ( GTK_TABLE ( tableau ),
		       label,
		       0, 1,
		       3, 4,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( label );

    label_montant_operation_ventilee_echeances_echeances = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_montant_operation_ventilee_echeances_echeances ),
			     1,
			     0.5 );
    gtk_table_attach ( GTK_TABLE ( tableau ),
		       label_montant_operation_ventilee_echeances_echeances,
		       1, 2,
		       3, 4,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( label_montant_operation_ventilee_echeances_echeances );



    /* mise en place des boutons */

    hbox = gtk_hbox_new ( FALSE,
			  10 );
    gtk_box_pack_end ( GTK_BOX ( onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       10 );
    gtk_widget_show ( hbox );


    bouton = gtk_button_new_from_stock    (GTK_STOCK_CANCEL);
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( annuler_ventilation_echeances ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    bouton = gtk_button_new_from_stock (GTK_STOCK_OK);
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( valider_ventilation_echeances ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    separateur = gtk_hseparator_new ();
    gtk_box_pack_end ( GTK_BOX ( onglet ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( separateur );

    return ( onglet );
}
/*******************************************************************************************/


/*******************************************************************************************/
/* Fonction creation_formulaire_ventilation_echeances */
/* crée la fenetre qui contient e formulaire_echeancier pour la ventilation */
/*******************************************************************************************/

GtkWidget *creation_formulaire_ventilation_echeances ( void )
{
    GtkWidget *onglet;
    GtkTooltips *tips;
    GtkWidget *table;
    GtkWidget *bouton;
    GtkWidget *menu;

    /* on crée le tooltips */

    tips = gtk_tooltips_new ();

    /* création du formulaire_echeancier */

    onglet = gtk_vbox_new ( FALSE,
			    5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				     10);
    gtk_widget_show ( onglet );

    /* mise en place de la table */

    table = gtk_table_new ( 2,
			    5,
			    FALSE);
    gtk_table_set_col_spacings ( GTK_TABLE ( table ),
				 10 );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 table,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( table );

    /* mise en place des catégories */

    widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] = gtk_combofix_new_complex ( liste_categories_ventilation_combofix,
									    FALSE,
									    TRUE,
									    TRUE,
									    0 );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ) -> entry ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( appui_touche_ventilation_echeances ),
			 GINT_TO_POINTER ( SCHEDULER_BREAKDOWN_FORM_CATEGORY ) );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY]) -> entry),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation_echeances),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY]) -> arrow),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation_echeances),
			 NULL );
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY]) -> entry ),
				"focus_in_event",
				GTK_SIGNAL_FUNC (entree_prend_focus),
				GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ) );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY]) -> entry),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus_echeances),
			 GINT_TO_POINTER ( SCHEDULER_BREAKDOWN_FORM_CATEGORY ) );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY],
		       0, 1, 0,1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_widget_show ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] );

    /* mise en place des notes */

    widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES] = gtk_entry_new ();
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES] ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( appui_touche_ventilation_echeances ),
			 GINT_TO_POINTER ( SCHEDULER_BREAKDOWN_FORM_NOTES ) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES]),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation_echeances ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES]),
			 "focus_in_event",
			 GTK_SIGNAL_FUNC (entree_prend_focus),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES]),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus_echeances),
			 GINT_TO_POINTER ( SCHEDULER_BREAKDOWN_FORM_NOTES ) );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES],
		       1, 3, 0,1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_widget_show ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES] );



    /* mise en place du débit */

    widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] = gtk_entry_new ();
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( appui_touche_ventilation_echeances ),
			 GINT_TO_POINTER ( SCHEDULER_BREAKDOWN_FORM_DEBIT ) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT]),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation_echeances ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT]),
			 "focus_in_event",
			 GTK_SIGNAL_FUNC (entree_prend_focus),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT]),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus_echeances),
			 GINT_TO_POINTER ( SCHEDULER_BREAKDOWN_FORM_DEBIT ) );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT],
		       3, 4, 0,1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_widget_show ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] );


    /* mise en place du crédit */

    widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT] = gtk_entry_new ();
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT] ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( appui_touche_ventilation_echeances ),
			 GINT_TO_POINTER ( SCHEDULER_BREAKDOWN_FORM_CREDIT ) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT]),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation_echeances ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT]),
			 "focus_in_event",
			 GTK_SIGNAL_FUNC (entree_prend_focus),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT]),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus_echeances),
			 GINT_TO_POINTER ( SCHEDULER_BREAKDOWN_FORM_CREDIT ) );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT],
		       4, 5, 0,1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_widget_show ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT] );


    /*  Affiche l'imputation budgétaire */

    widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] = gtk_combofix_new_complex ( liste_imputations_combofix,
									    FALSE,
									    TRUE,
									    TRUE,
									    0 );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY],
		       0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY]) -> entry),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation_echeances),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY]) -> arrow),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation_echeances),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY]) -> entry),
			 "key_press_event",
			 GTK_SIGNAL_FUNC (appui_touche_ventilation_echeances),
			 GINT_TO_POINTER( SCHEDULER_BREAKDOWN_FORM_BUDGETARY ) );
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY]) -> entry ),
				"focus_in_event",
				GTK_SIGNAL_FUNC (entree_prend_focus),
				GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] ) );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY]) -> entry),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus_echeances),
			 GINT_TO_POINTER ( SCHEDULER_BREAKDOWN_FORM_BUDGETARY ) );

    gtk_widget_show (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY]);

    gtk_widget_set_sensitive ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY],
			       etat.utilise_imputation_budgetaire );


    /* mise en place du type de l'opé associée en cas de virement */
    /* non affiché au départ */

    widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA],
			   _("Associated method of payment"),
			   _("Associated method of payment") );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA]),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( appui_touche_ventilation_echeances ),
			 GINT_TO_POINTER( SCHEDULER_BREAKDOWN_FORM_CONTRA ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA],
		       1, 2, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);

    /* création du bouton de l'exo */

    widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE],
			   _("Choose the financial year"),
			   _("Choose the financial year") );
    menu = gtk_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE] ),
			       creation_menu_exercices (0) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE]),
			 "key_press_event",
			 GTK_SIGNAL_FUNC (appui_touche_ventilation_echeances),
			 GINT_TO_POINTER( SCHEDULER_BREAKDOWN_FORM_EXERCICE ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE],
		       2, 3, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_widget_show ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE] );

    gtk_widget_set_sensitive ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE],
			       etat.utilise_exercice );

    /*   création de l'entrée du no de pièce comptable */

    widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER] = gtk_entry_new();
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER],
		       3, 5, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER]),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation_echeances ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER]),
			 "key_press_event",
			 GTK_SIGNAL_FUNC (appui_touche_ventilation_echeances),
			 GINT_TO_POINTER( SCHEDULER_BREAKDOWN_FORM_VOUCHER ) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER]),
			 "focus_in_event",
			 GTK_SIGNAL_FUNC (entree_prend_focus),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER]),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus_echeances),
			 GINT_TO_POINTER ( SCHEDULER_BREAKDOWN_FORM_VOUCHER ) );
    gtk_widget_show ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER] );

    gtk_widget_set_sensitive ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER],
			       etat.utilise_piece_comptable );

    /* séparation d'avec les boutons */

    separateur_formulaire_ventilations_echeances = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 separateur_formulaire_ventilations_echeances,
			 FALSE,
			 FALSE,
			 0 );
    if ( etat.affiche_boutons_valider_annuler )
	gtk_widget_show ( separateur_formulaire_ventilations_echeances );

    /* mise en place des boutons */

    hbox_valider_annuler_ventil_echeances = gtk_hbox_new ( FALSE,
							   5 );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 hbox_valider_annuler_ventil_echeances,
			 FALSE,
			 FALSE,
			 0 );
    if ( etat.affiche_boutons_valider_annuler )
	gtk_widget_show ( hbox_valider_annuler_ventil_echeances );


    bouton = gtk_button_new_from_stock (GTK_STOCK_OK);
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( fin_edition_ventilation_echeances ),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox_valider_annuler_ventil_echeances ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( echap_formulaire_ventilation_echeances ),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox_valider_annuler_ventil_echeances ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( bouton );


    /*   met l'adr de l'opé dans le formulaire_echeancier à -1 */

    gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
			  "adr_struct_ope",
			  GINT_TO_POINTER ( -1 ) );

    return ( onglet );
}
/*******************************************************************************************/





/***********************************************************************************************************/
gboolean clique_champ_formulaire_ventilation_echeances ( void )
{

    /* on rend sensitif tout ce qui ne l'était pas sur le formulaire_echeancier */

    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
			       TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE] ),
			       TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( hbox_valider_annuler_ventil_echeances ),
			       TRUE );

    return FALSE;
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction appelée quand une entry perd le focus */
/* si elle ne contient rien, on remet la fonction en gris */
/***********************************************************************************************************/

gboolean entree_ventilation_perd_focus_echeances ( GtkWidget *entree, GdkEventFocus *ev,
						   gint *no_origine )
{
    gchar *texte;

    texte = NULL;

    switch ( GPOINTER_TO_INT ( no_origine ))
    {
	/* on sort des catégories */
	case SCHEDULER_BREAKDOWN_FORM_CATEGORY :
	    if ( strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		/* si c'est un virement, on met le menu des types de l'autre compte */
		/* si ce menu n'est pas déjà affiché */

		gchar **tableau_char;

		tableau_char = g_strsplit ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree )),
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
			/* c'est un virement : on recherche le compte associé et on affiche les types de paiement */

			gint i;

			if ( strcmp ( tableau_char[1],
				      _("Deleted account") ) )
			{
			    /* recherche le no de compte du virement */

			    gint compte_virement;

			    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

			    compte_virement = -1;

			    for ( i = 0 ; i < nb_comptes ; i++ )
			    {
				if ( !g_strcasecmp ( NOM_DU_COMPTE,
						     tableau_char[1] ) )
				    compte_virement = i;
				p_tab_nom_de_compte_variable++;
			    }

			    /* si on a touvé un compte de virement, que celui ci n'est pas le compte */
			    /* courant et que son menu des types n'est pas encore affiché, on crée le menu */

			    if ( compte_virement != -1
				 &&
				 compte_virement != compte_courant )
			    {
				/* si le menu affiché est déjà celui du compte de virement, on n'y touche pas */

				if ( !GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] )
				     ||
				     ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ) -> menu ),
									       "compte_virement" ))
				       !=
				       compte_virement ))
				{
				    /* vérifie quel est le montant entré, affiche les types opposés de l'autre compte */

				    GtkWidget *menu;

				    if ( gtk_widget_get_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] ) == style_entree_formulaire[ENCLAIR] )
					/* il y a un montant dans le crédit */
					menu = creation_menu_types ( 1, compte_virement, 2  );
				    else
					/* il y a un montant dans le débit ou défaut */
					menu = creation_menu_types ( 2, compte_virement, 2  );

				    /* si un menu à été créé, on l'affiche */

				    if ( menu )
				    {
					gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
								   menu );
					gtk_widget_show ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] );
				    }

				    /* on associe le no de compte de virement au formulaire_echeancier pour le retrouver */
				    /* rapidement s'il y a un chgt débit/crédit */

				    gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
							  "compte_virement",
							  GINT_TO_POINTER ( compte_virement ));
				}
			    }
			    else
				gtk_widget_hide ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] );
			}
			else
			    gtk_widget_hide ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] );
		    }
		    else
			gtk_widget_hide ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] );
		}
		else
		    gtk_widget_hide ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] );

		g_strfreev ( tableau_char );
	    }
	    else
		texte = _("Categories : Sub-categories");
	    break;

	    /* sort des notes */

	case SCHEDULER_BREAKDOWN_FORM_NOTES :
	    if ( !strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Notes");
	    break;

	    /* sort du débit */
	    /*   soit vide, soit change le menu des types s'il ne correspond pas */

	case SCHEDULER_BREAKDOWN_FORM_DEBIT :

	    if ( strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree ))))
		 &&
		 gtk_widget_get_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	    {
		/* on  commence par virer ce qu'il y avait dans les crédits */

		if ( gtk_widget_get_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT] ) == style_entree_formulaire[ENCLAIR] )
		{
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT] ),
					 "" );
		    gtk_widget_set_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT],
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT]),
					 _("Credit") );
		}

		/* comme il y a eu un changement de signe, on change aussi le type de l'opé associée */
		/* s'il est affiché */

		if ( GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] )
		     &&
		     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ) -> menu ),
							     "signe_menu" ))
		     ==
		     1 )
		{
		    GtkWidget *menu;

		    menu = creation_menu_types ( 2,
						 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
											 "compte_virement" )),
						 2  );

		    if ( menu )
			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
						   menu );
		    else
			gtk_option_menu_remove_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ));
		}
	    }
	    else
		texte = _("Debit");
	    break;

	    /* sort du crédit */

	case SCHEDULER_BREAKDOWN_FORM_CREDIT :
	    if ( strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree ))))
		 &&
		 gtk_widget_get_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT] ) == style_entree_formulaire[ENCLAIR])
	    {
		/* on  commence par virer ce qu'il y avait dans les débits */

		if ( gtk_widget_get_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
		{
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] ),
					 "" );
		    gtk_widget_set_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT],
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT]),
					 _("Debit") );
		}

		/* comme il y a eu un changement de signe, on change aussi le type de l'opé associée */
		/* s'il est affiché */

		if ( GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] )
		     &&
		     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ) -> menu ),
							     "signe_menu" ))
		     ==
		     2 )
		{
		    GtkWidget *menu;

		    menu = creation_menu_types ( 1,
						 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
											 "compte_virement" )),
						 2  );

		    if ( menu )
			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
						   menu );
		    else
			gtk_option_menu_remove_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ));
		}
	    }
	    else
		texte = _("Credit");
	    break;

	    /* sort de l'ib */

	case SCHEDULER_BREAKDOWN_FORM_BUDGETARY :
	    if ( !strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Budgetary line");
	    break;

	    /* sort de la pièce comptable */

	case SCHEDULER_BREAKDOWN_FORM_VOUCHER :
	    if ( !strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Voucher");
	    break;

    }


    /* l'entrée était vide, on remet le défaut */

    if ( texte )
    {
	gtk_widget_set_style ( entree,
			       style_entree_formulaire[ENGRIS] );
	gtk_entry_set_text ( GTK_ENTRY ( entree ),
			     texte );
    }

    return FALSE;
}
/*******************************************************************************************/




/*******************************************************************************************/
/* Fonction ventiler_operation_echeances */
/* appelée lorsque la catégorie est Ventilation lors de l'enregistrement d'une opé */
/* ou lors d'une modif d'une opé ventilée */
/* Arguments : montant de l'opé */
/*******************************************************************************************/

void ventiler_operation_echeances ( gdouble montant )
{
    /*   si liste_ope est NULL, c'est une nouvelle opé, les ventils ne peuvent être associées */
    /* à un no d'opé, on les met donc à -1 */
    /* si c'est != NULL, c'est que c'était déjà une ventilation, et on a appuyé sur voir pour */
    /* arriver ici */

    /* on commence par mettre la taille au formulaire_echeancier et à la liste */

    ancienne_largeur_ventilation_echeances = 0;
    changement_taille_liste_ventilation_echeances  ( liste_echeances_ventilees,
						     &liste_echeances_ventilees->allocation,
						     NULL );

    montant_operation_ventilee_echeances = montant;

    ligne_selectionnee_ventilation_echeances = NULL;

    /* affiche la liste */

    affiche_liste_ventilation_echeances ();

    ligne_selectionnee_ventilation_echeances = GINT_TO_POINTER ( -1 );

    gtk_clist_select_row ( GTK_CLIST ( liste_echeances_ventilees ),
			   gtk_clist_find_row_from_data ( GTK_CLIST ( liste_echeances_ventilees ),
							  ligne_selectionnee_ventilation_echeances ),
			   0 );

    /* met à jour les labels */

    gtk_label_set_text ( GTK_LABEL ( label_somme_ventilee_echeances_echeances ),
			 g_strdup_printf ( "%4.2f",
					   somme_ventilee_echeances ) );


    /*   s'il n'y a pas de montant total, celui ci = la somme ventilée */

    if ( montant_operation_ventilee_echeances )
    {
	gtk_label_set_text ( GTK_LABEL ( label_non_affecte_echeances ),
			     g_strdup_printf ( "%4.2f",
					       montant_operation_ventilee_echeances - somme_ventilee_echeances ));
	gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee_echeances_echeances ),
			     g_strdup_printf ( "%4.2f",
					       montant_operation_ventilee_echeances ));
    }
    else
    {
	gtk_label_set_text ( GTK_LABEL ( label_non_affecte_echeances ),
			     "0.00" );
	gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee_echeances_echeances ),
			     g_strdup_printf ( "%4.2f",
					       somme_ventilee_echeances ));
    }


    gtk_widget_grab_focus ( liste_echeances_ventilees );


    /* bloque les fonctions */
    /* qui modifient la position dans la liste des opés */

    /*     gtk_signal_handler_block_by_func ( GTK_OBJECT ( frame_droite_bas ), */
    /* 				       GTK_SIGNAL_FUNC(allocation_taille_formulaire), */
    /* 				       NULL ); */


    /*     FIXME */
    /*     gtk_signal_handler_block_by_func ( GTK_OBJECT ( frame_droite_bas ), */
    /* 				       GTK_SIGNAL_FUNC(efface_formulaire), */
    /* 				       NULL ); */


    /* affiche les pages de ventilation */

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_liste_ventil_echeances ),
			    1 );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_calendrier_ventilations ),
			    1 );
    /*     on chache le formulaire auparavant pour que la taille s'adapte au formulaire des ventil qui est plus petit */
    gtk_widget_hide ( formulaire_echeancier );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_formulaire_echeances ),
			    1 );

    /* on grise tout ce qu'il faut */

    echap_formulaire_ventilation_echeances ();

    /* on donne le focus directement aux categ */

    clique_champ_formulaire_ventilation_echeances ();
    gtk_window_set_focus ( GTK_WINDOW ( window ),
			   GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ) -> entry );

    /* FIXME: magic numbers sucks, enum rulez */
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), 2 );
}
/*******************************************************************************************/





/* ***************************************************************************************************** */
/* Fonction changement_taille_liste_ventilation_echeances  */
/* appelée dès que la taille de la clist a changé */
/* pour mettre la taille des différentes colonnes */
/* ***************************************************************************************************** */

void changement_taille_liste_ventilation_echeances  ( GtkWidget *clist,
						      GtkAllocation *allocation,
						      gpointer null )
{
    gint categorie, description, montant;
    gint largeur;
    gint col0, col1, col2;

    /*   si la largeur de grisbi est < LOWEST_RESOLUTION, on fait rien */

    if ( window -> allocation.width < LOWEST_RESOLUTION )
	return;

    largeur = allocation->width;

    if ( largeur == ancienne_largeur_ventilation_echeances )
	return;

    ancienne_largeur_ventilation_echeances = largeur;

    categorie = ( 40 * largeur) / 100;
    description = ( 35 * largeur) / 100;
    montant = ( 10 * largeur) / 100;

    gtk_clist_set_column_width ( GTK_CLIST ( clist ),
				 0,
				 categorie );
    gtk_clist_set_column_width ( GTK_CLIST ( clist ),
				 1,
				 description );
    gtk_clist_set_column_width ( GTK_CLIST ( clist ),
				 2,
				 montant );

    /* met les entrées du formulaire_echeancier à la même taille */

    col0 = largeur * 32  / 100;
    col1 = largeur * 32  / 100;
    col2 = largeur * 15  / 100;

    /* 1ère ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
			   col0,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES] ),
			   col1,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] ),
			   col2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT] ),
			   col2,
			   FALSE );

    /* 2ème ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] ),
			   col0,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
			   col1 / 2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE] ),
			   col1/2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER] ),
			   col2,
			   FALSE );


}
/* ***************************************************************************************************** */



/***************************************************************************************************/
/* Fonction traitement_clavier_liste */
/* gère le clavier sur la clist */
/***************************************************************************************************/

gboolean traitement_clavier_liste_ventilation_echeances ( GtkCList *liste,
							  GdkEventKey *evenement,
							  gpointer null )
{
    gint ligne;


    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste ),
				   "key_press_event");

    switch ( evenement->keyval )
    {
	/* entrée */
	case GDK_KP_Enter:
	case GDK_Return:

	    edition_operation_ventilation_echeances ();
	    break;

	case GDK_Up :		/* touches flèche haut */
	case GDK_KP_Up :

	    ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste ),
						   ligne_selectionnee_ventilation_echeances );
	    if ( ligne )
	    {
		ligne--;

		ligne_selectionnee_ventilation_echeances = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
										    ligne );
		gtk_clist_unselect_all ( GTK_CLIST ( liste ) );
		gtk_clist_select_row ( GTK_CLIST ( liste ), ligne, 0 );

		if ( gtk_clist_row_is_visible ( GTK_CLIST ( liste ),
						ligne) != GTK_VISIBILITY_FULL )
		    gtk_clist_moveto ( GTK_CLIST ( liste ),
				       ligne, 0, 0, 0 );
	    }
	    break;


	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :

	    if ( ligne_selectionnee_ventilation_echeances != GINT_TO_POINTER ( -1 ) )
	    {
		ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste ),
						       ligne_selectionnee_ventilation_echeances );

		ligne++;

		ligne_selectionnee_ventilation_echeances = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
										    ligne );
		gtk_clist_unselect_all ( GTK_CLIST ( liste ) );
		gtk_clist_select_row ( GTK_CLIST ( liste ),
				       ligne,
				       0 );


		if ( gtk_clist_row_is_visible ( GTK_CLIST ( liste ),
						ligne )
		     != GTK_VISIBILITY_FULL )
		    gtk_clist_moveto ( GTK_CLIST ( liste ),
				       ligne,
				       0,
				       1,
				       0 );
	    }
	    break;


	    /*  del  */
	case GDK_Delete:

	    supprime_operation_ventilation_echeances ();
	    break;


	default : 
	    return TRUE;
    }

    return FALSE;
}
/***************************************************************************************************/






/***************************************************************************************************/
/* Fonction selectionne_ligne_souris */
/* place la sélection sur l'opé clickée */
/***************************************************************************************************/

void selectionne_ligne_souris_ventilation_echeances ( GtkCList *liste,
						      GdkEventButton *evenement,
						      gpointer null )
{
    gint colonne, x, y;
    gint ligne;

    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste_echeances_ventilees ),
				   "button_press_event");


    /* Récupération des coordonnées de la souris */

    gdk_window_get_pointer ( GTK_CLIST ( liste ) -> clist_window,
			     &x,
			     &y,
			     FALSE );

    gtk_clist_get_selection_info ( GTK_CLIST ( liste ),
				   x,
				   y,
				   &ligne,
				   &colonne);

    if ( !gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				   ligne ) )
	return;

    /*   vire l'ancienne sélection */

    gtk_clist_unselect_all ( GTK_CLIST ( liste ) );

    /* on met l'adr de la struct dans OPERATION_SELECTIONNEE */

    ligne_selectionnee_ventilation_echeances = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
									ligne );

    gtk_clist_select_row ( GTK_CLIST ( liste ),
			   ligne,
			   colonne );

    gtk_widget_grab_focus ( GTK_WIDGET (liste) );

    if ( evenement -> type == GDK_2BUTTON_PRESS )
	edition_operation_ventilation_echeances ();

}
/***************************************************************************************************/



/***********************************************************************************************************/
/* Fonction appui_touche_ventilation_echeances  */
/* gére l'action du clavier sur les entrées du formulaire_echeancier de ventilation */
/***********************************************************************************************************/
gboolean appui_touche_ventilation_echeances ( GtkWidget *entree, GdkEventKey *evenement,
					      gint *no_origine )
{
    gint origine;

    origine = GPOINTER_TO_INT ( no_origine );

    /*   si etat.entree = 1, la touche entrée finit l'opération (
	 fonction par défaut ) sinon elle fait comme tab */

    if ( !etat.entree && ( evenement -> keyval == GDK_Return || evenement -> keyval == GDK_KP_Enter ))
	evenement->keyval = GDK_Tab ;

    switch (evenement->keyval)
    {
	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :
	case GDK_Up :		/* touches flèche haut */
	case GDK_KP_Up :

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entree ),
					   "key_press_event");
	    gtk_widget_grab_focus ( entree );
	    break;


	case GDK_Tab:

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entree ),
					   "key_press_event");

	    /* on efface la sélection en cours si c'est une entrée ou un combofix */

	    if ( GTK_IS_ENTRY ( entree ))
		gtk_entry_select_region ( GTK_ENTRY ( entree ), 0, 0);
	    else
		if ( GTK_IS_COMBOFIX ( entree ))
		    gtk_entry_select_region ( GTK_ENTRY(GTK_COMBOFIX(entree) -> entry),
					      0, 0);

	    /* on donne le focus au widget suivant */

	    origine = (origine + 1 ) % SCHEDULER_BREAKDOWN_FORM_TOTAL_WIDGET;

	    while ( !(GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation_echeances[origine] )
		      &&
		      GTK_WIDGET_SENSITIVE ( widget_formulaire_ventilation_echeances[origine] )
		      &&
		      ( GTK_IS_COMBOFIX (widget_formulaire_ventilation_echeances[origine] )
			||
			GTK_IS_ENTRY ( widget_formulaire_ventilation_echeances[origine] )
			||
			GTK_IS_BUTTON ( widget_formulaire_ventilation_echeances[origine] ) )))
		origine = (origine + 1 ) % SCHEDULER_BREAKDOWN_FORM_TOTAL_WIDGET;

	    /*       si on se retrouve sur les catég et que etat.entree = 0, on enregistre l'opérations */

	    if ( !origine && !etat.entree )
	    {
		fin_edition_ventilation_echeances();
		return(FALSE);
	    }

	    /* si on se retrouve sur le crédit et qu'il y a qque chose dans le débit, on passe au suivant */
	    /*       à ce niveau, il n'y a pas eu encore de focus out donc on peut tester par strlen */

	    if ( origine == SCHEDULER_BREAKDOWN_FORM_CREDIT
		 &&
		 strlen ( (char *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] ))))
		origine = (origine + 1 ) % SCHEDULER_BREAKDOWN_FORM_TOTAL_WIDGET;

	    /* on sélectionne le contenu de la nouvelle entrée */

	    if ( GTK_IS_COMBOFIX ( widget_formulaire_ventilation_echeances[origine] ) )
	    {
		gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[origine] ) -> entry );  
		gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[origine] ) -> entry ),
					  0,
					  -1 );
	    }
	    else
	    {
		if ( GTK_IS_ENTRY ( widget_formulaire_ventilation_echeances[origine] ) )
		    gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[origine] ),
					      0,
					      -1 );

		gtk_widget_grab_focus ( widget_formulaire_ventilation_echeances[origine]  );
	    }
	    break;


	case GDK_KP_Enter:
	case GDK_Return:

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entree ),
					   "key_press_event");
	    fin_edition_ventilation_echeances ();
	    break;


	case GDK_Escape :

	    echap_formulaire_ventilation_echeances ();
	    break;

	default:
	    return FALSE;
    }

    return TRUE;
}
/***********************************************************************************************************/





/***********************************************************************************************************/
void echap_formulaire_ventilation_echeances ( void )
{

    /* on met les styles des entrées au gris */

    gtk_widget_set_style ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] )->entry,
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] )->entry,
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER],
			   style_entree_formulaire[ENGRIS] );


    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
			    _("Categories : Sub-categories") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES]),
			 _("Notes") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT]),
			 _("Debit") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT]),
			 _("Credit") );
    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] ),
			    _("Budgetary line") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER]),
			 _("Voucher") );

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
				  0 );

    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
			       FALSE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE] ),
			       FALSE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( hbox_valider_annuler_ventil_echeances ),
			       FALSE );
    gtk_widget_set_sensitive ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY],
			       TRUE );
    gtk_widget_set_sensitive ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT],
			       TRUE );
    gtk_widget_set_sensitive ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT],
			       TRUE );

    gtk_widget_hide ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] );

    /*   met l'adr de l'opé dans le formulaire_echeancier à -1 */

    gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
			  "adr_struct_ope",
			  GINT_TO_POINTER ( -1 ) );

    gtk_widget_grab_focus ( liste_echeances_ventilees );
}
/***********************************************************************************************************/





/***********************************************************************************************************/
void fin_edition_ventilation_echeances ( void )
{
    struct struct_ope_ventil *operation;
    gint modification;
    gchar **tableau_char;
    gint compte_vire;
    gint perte_ligne_selectionnee;

    /* pour éviter les warnings lors de la compil */

    compte_vire = 0;
    tableau_char = NULL;

    /* on met le focus sur la liste des opés pour éventuellement faire perdre le focus aux entrées des */
    /* montants pour faire les modifs nécessaires automatiquement */

    gtk_window_set_focus ( GTK_WINDOW ( window ),
			   liste_echeances_ventilees );

    /* perte ligne sélectionnée sera à 1 s'il y a une magouille avec les virements et */
    /* qu'on recrée une opé au lieu de la modifier. dans ce cas on remettra la ligne */
    /* sélectionné sur la nouvelle opé */

    perte_ligne_selectionnee = 0;

    /*   dans cette fonction, on récupère les infos du formulaire_echeancier qu'on met dans une structure */
    /* de ventilation, et on ajoute cette structure à celle en cours (ou modifie si elle existait */
    /* déjà */

    /* on vérifie si c'est un virement que le compte est valide et que ce n'est pas un virement sur lui-même */


    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	/*       on split déjà les catég, sans libérer la variable, pour la récupérer ensuite pour les categ */

	tableau_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ))),
				    ":",
				    2 );
	if ( tableau_char[0]  )
	{
	    tableau_char[0] = g_strstrip ( tableau_char[0] );

	    if ( tableau_char[1] )
		tableau_char[1] = g_strstrip ( tableau_char[1] );


	    if ( !strcmp ( tableau_char[0],
			   _("Transfer")))
	    {
		gint i;

		if ( tableau_char[1] )
		{
		    compte_vire = -1;

		    for ( i = 0 ; i < nb_comptes ; i++ )
		    {
			p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

			if ( !strcmp ( NOM_DU_COMPTE,
				       tableau_char[1] ) )
			    compte_vire = i;
		    }

		    if ( compte_vire == -1 )
		    {
			dialogue_error ( _("The associated account for this transfer is invalid") );
			g_strfreev ( tableau_char );
			return;
		    }

		    if ( compte_vire == GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ) -> menu_item ),
										"no_compte" )) )
		    {
			dialogue_error ( _("It's impossible to transfer an account to itself") );
			g_strfreev ( tableau_char );
			return;
		    }
		}
		else
		{
		    dialogue_error ( _("No account associated with the transfer") );
		    g_strfreev ( tableau_char );
		    return;
		}
	    }
	}
    }


    /*   on récupère l'adresse de l'opération, soit c'est une modif, soit c'est une nouvelle (-1) */

    operation = gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
				      "adr_struct_ope" );


    if (operation == GINT_TO_POINTER ( -1 ))
    {
	operation = calloc ( 1,
			     sizeof ( struct struct_ope_ventil ));
	modification = 0;
    }
    else
	modification = 1;


    /*   récupération des catégories / sous-catég, s'ils n'existent pas, on les crée */
    /* la variable tableau_char est déjà initialisée lors des tests du virement */

    /*   il y a 3 possibilités en rapport avec les virements : */
    /* si l'ancienne opé était un virement, la nouvelle est : */
    /* soit virement vers le même compte */
    /* soit virement vers un autre compte */
    /* soit ce n'est plus un virement */
    /*     pour la 1ère, c'est une modif normale d'opé */
    /*     pour les 2nde et 3ème, on supprime cette opé et en recrée une nouvelle */

    /* il faut donc mettre la récup des catég en premier car il peut y avoir un changement au niveau des */
    /* modif avec suppression de l'ancienne et création d'une nouvelle ope */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	struct struct_categ *categ;

       tableau_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ))),
                                   ":",
                                   2 );

       tableau_char[0] = g_strstrip ( tableau_char[0] );

       if ( tableau_char[1] )
	   tableau_char[1] = g_strstrip ( tableau_char[1] );

	if ( strlen ( tableau_char[0] ) )
	{
	    /* on vérifie ici si c'est un virement */

	    if ( strcmp ( tableau_char[0],
			  _("Transfer") ) )
	    {
		/* ce n'est pas un virement, recherche les catég */

		GSList *pointeur_liste;

		/* si c'est une modif d'opé et que l'ancienne opé était un virement */
		/* on marque cette opé comme supprimée et on en fait une nouvelle */

		if ( modification
		     &&
		     operation -> relation_no_operation )
		{
		    operation -> supprime = 1;
		    operation = calloc ( 1,
					 sizeof ( struct struct_ope_ventil ));
		    modification = 0;
		    perte_ligne_selectionnee = 1;
		}

		/* recherche des catégories */

		pointeur_liste = g_slist_find_custom ( liste_struct_categories,
						       tableau_char[0],
						       ( GCompareFunc ) recherche_categorie_par_nom );

		if ( pointeur_liste )
		    categ = pointeur_liste -> data;
		else
		{
		    categ = ajoute_nouvelle_categorie ( tableau_char[0] );
		    if ( operation -> montant < 0 )
			categ -> type_categ = 1;
		    else
			categ -> type_categ = 0;
		}

		operation -> categorie = categ -> no_categ;

		if ( tableau_char[1] && strlen (tableau_char[1]) )
		{
		    struct struct_sous_categ *sous_categ;

		    pointeur_liste = g_slist_find_custom ( categ -> liste_sous_categ,
							   tableau_char[1],
							   ( GCompareFunc ) recherche_sous_categorie_par_nom );

		    if ( pointeur_liste )
			sous_categ = pointeur_liste -> data;
		    else
			sous_categ = ajoute_nouvelle_sous_categorie ( tableau_char[1],
								      categ );

		    operation -> sous_categorie = sous_categ -> no_sous_categ;
		}
		else
		    operation -> sous_categorie = 0;
	    }
	    else
	    {
		/* c'est un virement */

		/* si c'est une nouvelle opé, on est content et on prend juste le compte de virement */
		/* si c'est une modif d'opé et que l'ancienne n'était pas un virement, idem */
		/* si l'ancienne était un virement vers le même compte, idem */
		/* si l'ancienne était un virement vers un autre compte, c'est qu'on cherche les bugs ... */
		/* dans ce cas, on marque l'opé comme supprimée et on en recrée une nouvelle */

		/* le no de compte du virement est déjà dans compte_vire */

		if ( modification
		     &&
		     operation -> relation_no_operation != -1
		     &&
		     operation -> relation_no_compte != compte_vire )
		{
		    /* on supprime donc l'opé et en crée une nouvelle */

		    operation -> supprime = 1;
		    operation = calloc ( 1,
					 sizeof ( struct struct_ope_ventil ));
		    modification = 0;
		    perte_ligne_selectionnee = 1;
		}

		/* on met les no de categ à 0 */

		operation -> categorie = 0;
		operation -> sous_categorie = 0;

		/* on met le compte en relation si c'est une nouvelle opération */

		if ( !modification )
		    operation -> relation_no_operation = -1;

		operation -> relation_no_compte = compte_vire;
	    }
	}
	/*       on peut maintenant libérer la variable tableau_char, qui ne sera plus utilisée */

	g_strfreev ( tableau_char );
    }
    else
    {
	operation -> categorie = 0;
	operation -> sous_categorie = 0;
	operation -> relation_no_compte = -1;
    }

    /* récupération du type d'opé associée s'il est affiché */

    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ))
	operation -> no_type_associe = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ) -> menu_item ),
									       "no_type" ));

    /* récupération des notes */

    if ( gtk_widget_get_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES] ) == style_entree_formulaire[ENCLAIR] )
	operation -> notes = g_strdup ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES] ))));
    else
	operation -> notes = NULL;


    /* récupération du montant */

    if ( gtk_widget_get_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	/* c'est un débit */
	operation -> montant = -my_strtod ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] ))),
					    NULL );
    else
	operation -> montant = my_strtod ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT] ))),
					   NULL );



    /* récupération de l'imputation budgétaire */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	struct struct_imputation *imputation;
	gchar **tableau_char;
	GSList *pointeur_liste;

	tableau_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] )),
				    ":",
				    2 );

	tableau_char[0] = g_strstrip ( tableau_char[0] );

	if ( tableau_char[1] )
	    tableau_char[1] = g_strstrip ( tableau_char[1] );

	pointeur_liste = g_slist_find_custom ( liste_struct_imputation,
					       tableau_char[0],
					       ( GCompareFunc ) recherche_imputation_par_nom );

	if ( pointeur_liste )
	    imputation = pointeur_liste -> data;
	else
	{
	    imputation = ajoute_nouvelle_imputation ( tableau_char[0] );

	    if ( operation -> montant < 0 )
		imputation -> type_imputation = 1;
	    else
		imputation -> type_imputation = 0;
	}

	operation -> imputation = imputation -> no_imputation;

	if ( tableau_char[1] && strlen (tableau_char[1]) )
	{
	    struct struct_sous_imputation *sous_imputation;

	    pointeur_liste = g_slist_find_custom ( imputation -> liste_sous_imputation,
						   tableau_char[1],
						   ( GCompareFunc ) recherche_sous_imputation_par_nom );

	    if ( pointeur_liste )
		sous_imputation = pointeur_liste -> data;
	    else
		sous_imputation = ajoute_nouvelle_sous_imputation ( tableau_char[1],
								    imputation );

	    operation -> sous_imputation = sous_imputation -> no_sous_imputation;
	}
	else
	    operation -> sous_imputation = 0;

	g_strfreev ( tableau_char );
    }

    /* récupération de l'exercice */
    /* si l'exo est à -1, c'est que c'est sur non affiché */
    /* soit c'est une modif d'opé et on touche pas à l'exo */
    /* soit c'est une nouvelle opé et on met l'exo à 0 */

    if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE] ) -> menu_item ),
						 "no_exercice" )) == -1 )
    {
	if ( !operation -> no_operation )
	    operation -> no_exercice = 0;
    }
    else
	operation -> no_exercice = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE] ) -> menu_item ),
									   "no_exercice" ));


    /* récupération du no de pièce comptable */

    if ( gtk_widget_get_style ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER] ) == style_entree_formulaire[ENCLAIR] )
	operation -> no_piece_comptable = g_strdup ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER] ))));
    else
	operation -> no_piece_comptable = NULL;



    /* on a rempli l'opération, on l'ajoute à la liste */
    /* si c'est pas une modif */

    if ( !modification )
    {
	GSList *liste_struct_ventilations;

	/* récupération de la liste de ventilations */

	liste_struct_ventilations = gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
							  "liste_adr_ventilation" );

	/*   si cette liste est à -1 (ce qui veut dire qu'elle est nulle en réalité mais */
	/* qu'elle a déjà été éditée ), on la met à 0 */

	if ( liste_struct_ventilations == GINT_TO_POINTER ( -1 ))
	    liste_struct_ventilations = NULL;

	/* on ajoute l'opé */

	liste_struct_ventilations = g_slist_append ( liste_struct_ventilations,
						     operation );

	gtk_object_set_data ( GTK_OBJECT ( formulaire_echeancier ),
			      "liste_adr_ventilation",
			      liste_struct_ventilations );
    }


    /*   si perte_ligne_selectionnee = 1, c'est qu'au lieu de modifier une opé (virement), on l'a */
    /* effacé puis recréé une nouvelle. comme ça se fait que lors d'une modif d'opé, on remet */
    /* la selection sur cette nouvelle opé */

    if ( perte_ligne_selectionnee == 1 )
	ligne_selectionnee_ventilation_echeances = operation;


    mise_a_jour_categ ();
    mise_a_jour_imputation ();

    /* on met à jour la liste des ventilations */

    affiche_liste_ventilation_echeances ();

    /* efface le formulaire_echeancier et prépare l'opé suivante */

    echap_formulaire_ventilation_echeances ();

    if ( modification )
	gtk_widget_grab_focus ( liste_echeances_ventilees );
    else
    {
	clique_champ_formulaire_ventilation_echeances ();
	gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ) -> entry );
    }
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* Fonction edition_operation_ventilation_echeances */
/* appelé lors d'un double click ou entrée sur une opé de ventilation */
/***********************************************************************************************************/

void edition_operation_ventilation_echeances ( void )
{
    struct struct_ope_ventil *operation;
    GSList *liste_tmp;

    /* on récupère la struc de l'opé de ventil, ou -1 si c'est une nouvelle */

    operation = ligne_selectionnee_ventilation_echeances;

    echap_formulaire_ventilation_echeances ();

    /* dégrise ce qui est nécessaire */

    clique_champ_formulaire_ventilation_echeances ();

    gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
			  "adr_struct_ope",
			  operation );

    /* si l'opé est -1, c'est que c'est une nouvelle opé */

    if ( operation == GINT_TO_POINTER ( -1 ) )
    {
	gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ) -> entry );
	return;
    }


    /*   l'opé n'est pas -1, c'est une modif, on remplit les champs */

    gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
			  "adr_struct_ope",
			  operation );


    /* mise en forme du montant */


    if ( operation -> montant < 0 )
    {
	entree_prend_focus (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_DEBIT] ),
			     g_strdup_printf ( "%4.2f", -operation -> montant ));
    }
    else
    {
	entree_prend_focus (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CREDIT] ),
			     g_strdup_printf ( "%4.2f", operation -> montant ));
    }


    /* mise en forme des catégories */

    if ( operation -> relation_no_operation )
    {
	/* c'est un virement */

	GtkWidget *menu;

	entree_prend_focus (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] );

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
				g_strconcat ( COLON(_("Transfer")),
					      NOM_DU_COMPTE,
					      NULL ));

	/* on met le type de l'opé associée */

	if ( operation -> montant < 0 )
	    menu = creation_menu_types ( 2,
					 operation -> relation_no_compte,
					 2  );
	else
	    menu = creation_menu_types ( 1,
					 operation -> relation_no_compte,
					 2  );

	if ( menu )
	{
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
				       menu );

	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] ),
					  cherche_no_menu_type_associe ( operation -> no_type_associe,
									 1 ));
	    gtk_widget_show ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CONTRA] );

	}
    }
    else
    {
	liste_tmp = g_slist_find_custom ( liste_struct_categories,
					  GINT_TO_POINTER ( operation -> categorie ),
					  ( GCompareFunc ) recherche_categorie_par_no );

	if ( liste_tmp )
	{
	    GSList *liste_tmp_2;

	    entree_prend_focus (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] );

	    liste_tmp_2 = g_slist_find_custom ( (( struct struct_categ * )( liste_tmp -> data )) -> liste_sous_categ,
						GINT_TO_POINTER ( operation -> sous_categorie ),
						( GCompareFunc ) recherche_sous_categorie_par_no );
	    if ( liste_tmp_2 )
		gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
					g_strconcat ( (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ,
						      " : ",
						      (( struct struct_sous_categ * )( liste_tmp_2 -> data )) -> nom_sous_categ,
						      NULL ));
	    else
		gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ),
					(( struct struct_categ * )( liste_tmp -> data )) -> nom_categ );

	}
    }


    /* mise en forme des notes */

    if ( operation -> notes )
    {
	entree_prend_focus (widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES] ),
			     operation -> notes );
    }


    /* met en place l'imputation budgétaire */

    liste_tmp = g_slist_find_custom ( liste_struct_imputation,
				      GINT_TO_POINTER ( operation -> imputation ),
				      ( GCompareFunc ) recherche_imputation_par_no );

    if ( liste_tmp )
    {
	GSList *liste_tmp_2;

	entree_prend_focus ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] );

	liste_tmp_2 = g_slist_find_custom ( (( struct struct_imputation * )( liste_tmp -> data )) -> liste_sous_imputation,
					    GINT_TO_POINTER ( operation -> sous_imputation ),
					    ( GCompareFunc ) recherche_sous_imputation_par_no );
	if ( liste_tmp_2 )
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] ),
				    g_strconcat ( (( struct struct_imputation * )( liste_tmp -> data )) -> nom_imputation,
						  " : ",
						  (( struct struct_sous_imputation * )( liste_tmp_2 -> data )) -> nom_sous_imputation,
						  NULL ));
	else
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_BUDGETARY] ),
				    (( struct struct_imputation * )( liste_tmp -> data )) -> nom_imputation );
    }


    /* met en place l'exercice */

    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE] ),
				   cherche_no_menu_exercice ( operation -> no_exercice,
							      widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_EXERCICE] ));

    /* mise en place de la pièce comptable */

    if ( operation -> no_piece_comptable )
    {
	entree_prend_focus ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_VOUCHER] ),
			     operation -> no_piece_comptable );
    }


    /*   on a fini de remplir le formulaire, on donne le focus à la date */

    if ( GTK_WIDGET_SENSITIVE ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ))
    {
	gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ) -> entry ),
				  0,
				  -1);
	gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_CATEGORY] ) -> entry );
    }
    else
    {
	gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES] ),
				  0,
				  -1);
	gtk_widget_grab_focus ( widget_formulaire_ventilation_echeances[SCHEDULER_BREAKDOWN_FORM_NOTES] );
    }
    enregistre_ope_au_retour_echeances = 1 ;
}
/***********************************************************************************************************/





/***********************************************************************************************************/
void supprime_operation_ventilation_echeances ( void )
{
    struct struct_ope_ventil *operation;
    gint ligne;


    operation = ligne_selectionnee_ventilation_echeances;

    if ( operation == GINT_TO_POINTER ( -1 ) ||
	 ! ligne_selectionnee_ventilation_echeances )
	return;


    if ( operation -> no_operation )
	operation -> supprime = 1;
    else
    {

	GSList *liste_struct_ventilations;

	liste_struct_ventilations = gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
							  "liste_adr_ventilation" );

	liste_struct_ventilations = g_slist_remove ( liste_struct_ventilations,
						     operation );
	free ( operation );
	gtk_object_set_data ( GTK_OBJECT ( formulaire_echeancier ),
			      "liste_adr_ventilation",
			      liste_struct_ventilations );
    }
    /*   si la sélection est sur l'opé qu'on supprime, on met la sélection sur celle du dessous */

    ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste_echeances_ventilees ),
					   operation );
    ligne_selectionnee_ventilation_echeances = gtk_clist_get_row_data ( GTK_CLIST ( liste_echeances_ventilees ),
									ligne + 1 );

    /* supprime l'opération de la liste */

    gtk_clist_remove ( GTK_CLIST ( liste_echeances_ventilees ),
		       ligne );

    calcule_montant_ventilation_echeances();
    mise_a_jour_couleurs_liste_ventilation_echeances();
    selectionne_ligne_ventilation_echeances ();
    enregistre_ope_au_retour_echeances = 1 ;
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction affiche_liste_ventilation_echeances */
/* récupère la liste des struct d'opé de ventil sur le formulaire_echeancier et affiche ces opés */
/***********************************************************************************************************/

void affiche_liste_ventilation_echeances ( void )
{
    gchar *ligne[3];
    gint ligne_insertion;
    GSList *liste_tmp;


    somme_ventilee_echeances = 0;

    gtk_clist_freeze ( GTK_CLIST ( liste_echeances_ventilees ) );

    gtk_clist_clear ( GTK_CLIST ( liste_echeances_ventilees ) );

    /* récupère la liste des struct_ope_ventil */

    liste_tmp = gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
				      "liste_adr_ventilation" );

    while ( liste_tmp && GPOINTER_TO_INT ( liste_tmp ) != -1 )
    {
	ajoute_ope_sur_liste_ventilation_echeances ( liste_tmp -> data );

	liste_tmp = liste_tmp -> next;
    }


    /* ajoute la ligne blanche associee à -1 */

    ligne[0] = NULL;
    ligne[1] = NULL;
    ligne[2] = NULL;

    ligne_insertion = gtk_clist_append ( GTK_CLIST ( liste_echeances_ventilees ),
					 ligne );

    gtk_clist_set_row_data ( GTK_CLIST ( liste_echeances_ventilees ),
			     ligne_insertion,
			     GINT_TO_POINTER ( -1 ));



    /* on met la couleur */

    mise_a_jour_couleurs_liste_ventilation_echeances ();


    /* on sélectionne la ligne blanche */

    selectionne_ligne_ventilation_echeances ();

    gtk_clist_thaw ( GTK_CLIST ( liste_echeances_ventilees ));

    /* on met à jour les labels d'état */

    calcule_montant_ventilation_echeances ();
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* prend en argument une opé de ventil dont l'adr de la struct est donnée en argument */
/***********************************************************************************************************/

void ajoute_ope_sur_liste_ventilation_echeances ( struct struct_ope_ventil *operation )
{
    gchar *ligne[3];
    gint ligne_insertion;
    GSList *liste_tmp;

    /*   si cette opération a été supprimée, on ne l'affiche pas */

    if ( operation -> supprime )
	return;


    /* mise en forme des catégories */

    if ( operation -> relation_no_operation )
    {
	/* c'est un virement */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	ligne [0] = g_strconcat ( COLON(_("Transfer")),
				  NOM_DU_COMPTE,
				  NULL );
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
    }
    else
    {
	/* c'est des categ : sous categ */

	liste_tmp = g_slist_find_custom ( liste_struct_categories,
					  GINT_TO_POINTER ( operation -> categorie ),
					  ( GCompareFunc ) recherche_categorie_par_no );


	if ( liste_tmp )
	{
	    GSList *liste_tmp_2;

	    liste_tmp_2 = g_slist_find_custom ( (( struct struct_categ * )( liste_tmp -> data )) -> liste_sous_categ,
						GINT_TO_POINTER ( operation -> sous_categorie ),
						( GCompareFunc ) recherche_sous_categorie_par_no );
	    if ( liste_tmp_2 )
		ligne [0] = g_strconcat ( (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ,
					  " : ",
					  (( struct struct_sous_categ * )( liste_tmp_2 -> data )) -> nom_sous_categ,
					  NULL );
	    else
		ligne [0] = (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ;

	}
	else
	    ligne[0] = NULL;
    }


    /* mise en forme des notes */

    ligne[1] = operation -> notes;

    /* mise en forme du montant */

    ligne[2] = g_strdup_printf ( "%4.2f",
				 operation -> montant );


    ligne_insertion = gtk_clist_append ( GTK_CLIST ( liste_echeances_ventilees ),
					 ligne );

    /* on associe à cette ligne l'adr de la struct de l'opé */

    gtk_clist_set_row_data ( GTK_CLIST ( liste_echeances_ventilees ),
			     ligne_insertion,
			     operation);

    calcule_montant_ventilation_echeances ();
}
/***********************************************************************************************************/


/***********************************************************************************************************/
void calcule_montant_ventilation_echeances ( void )
{
    gint ligne;
    struct struct_ope_ventil *operation;

    /* fait le tour de la liste pour retrouver les ventil affichée pour calculer le montant */


    ligne = 0;
    somme_ventilee_echeances = 0;

    while ( ( operation = gtk_clist_get_row_data ( GTK_CLIST ( liste_echeances_ventilees ),
						   ligne )) != GINT_TO_POINTER ( -1 )
	    &&
	    operation )
    {
	somme_ventilee_echeances = somme_ventilee_echeances + operation -> montant;
	ligne++;
    }

    mise_a_jour_labels_ventilation_echeances ();
}
/***********************************************************************************************************/




/***********************************************************************************************************/
void mise_a_jour_labels_ventilation_echeances ( void )
{
    gtk_label_set_text ( GTK_LABEL ( label_somme_ventilee_echeances_echeances ),
			 g_strdup_printf ( "%4.2f",
					   somme_ventilee_echeances ));

    if ( montant_operation_ventilee_echeances )
    {
	gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee_echeances_echeances ),
			     g_strdup_printf ( "%4.2f",
					       montant_operation_ventilee_echeances ));

	gtk_label_set_text ( GTK_LABEL ( label_non_affecte_echeances ),
			     g_strdup_printf ( "%4.2f",
					       montant_operation_ventilee_echeances - somme_ventilee_echeances ));
    }
    else
    {
	gtk_label_set_text ( GTK_LABEL ( label_non_affecte_echeances ),
			     "0.00" );
	gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee_echeances_echeances ),
			     g_strdup_printf ( "%4.2f",
					       somme_ventilee_echeances ));
    }
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fait le tour le la liste de ventilation et met bien les couleurs */
/***********************************************************************************************************/

void mise_a_jour_couleurs_liste_ventilation_echeances ( void )
{
    gint i;

    i=-1;

    do
    {
	i++;

	gtk_clist_set_row_style ( GTK_CLIST ( liste_echeances_ventilees ),
				  i,
				  style_couleur [ i % 2 ] );
    }
    while ( gtk_clist_get_row_data ( GTK_CLIST ( liste_echeances_ventilees ),
				     i ) != GINT_TO_POINTER ( -1 ));
}
/***********************************************************************************************************/


/***********************************************************************************************************/
void selectionne_ligne_ventilation_echeances ( void )
{
    gint ligne_selectionnee;

    ligne_selectionnee = gtk_clist_find_row_from_data ( GTK_CLIST ( liste_echeances_ventilees ),
							ligne_selectionnee_ventilation_echeances );

    gtk_clist_select_row ( GTK_CLIST ( liste_echeances_ventilees ),
			   ligne_selectionnee,
			   0 );

    if ( gtk_clist_row_is_visible ( GTK_CLIST ( liste_echeances_ventilees ),
				    ligne_selectionnee )
	 != GTK_VISIBILITY_FULL )
	gtk_clist_moveto ( GTK_CLIST ( liste_echeances_ventilees ),
			   ligne_selectionnee,
			   0,
			   0.5,
			   0 );
}
/***********************************************************************************************************/


/* ************************************************************************** */
/* Fonction valider_ventilation_echeances                                               */
/* appelée par appui du bouton valider                                        */
/* ************************************************************************** */
void valider_ventilation_echeances ( void )
{
    /* Cette fonction est toute simple car la liste des structures des
       ventilations a été mise à jour au fur et à mesure et toujours associée
       au formulaire_echeancier des opérations. Donc, il faut juste réafficher ce qu'il faut
       et return. C'est la validation réelle de l'opération qui créera/supprimera
       toutes les opérations */

    /* Si par contre cette liste est null, on met -1 sur le formulaire_echeancier pour
       montrer qu'on est passé par là et qu'on veut une liste nulle */

    /* On associe l'adresse de la nouvelle liste des ventilation au formulaire,
       met -1 si la liste est vide */

    if ( !gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
				"liste_adr_ventilation" ))
	gtk_object_set_data ( GTK_OBJECT ( formulaire_echeancier ),
			      "liste_adr_ventilation",
			      GINT_TO_POINTER ( -1 ) );
    /*
       if ( gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ), "liste_adr_ventilation" ) == GINT_TO_POINTER ( -1 ) )
       dialogue("Liste nulle");
       */
    if ( fabs ( montant_operation_ventilee_echeances - somme_ventilee_echeances ) >= 0.000001 )
    {
	if ( ! question_yes_no_hint ( _("Incomplete breakdown"),
				      _("Transaction amount isn't fully broken down.\nProceed anyway?") ))
	    return;
    }

    quitter_ventilation_echeances ();

    if ( enregistre_ope_au_retour_echeances )
	fin_edition_echeance();
}
/* ************************************************************************** */


/* ************************************************************************** */
/* Fonction annuler_ventilation_echeances                                               */
/* appelée par appui du bouton annuler                                        */
/* ************************************************************************** */
void annuler_ventilation_echeances ( void )
{
    /* Cette fonction remet la liste des structures de ventilation par défaut
       en recherchant les opérations de ventilation dans la liste des opérations
       puis appelle valider ventilation */

    gtk_object_set_data ( GTK_OBJECT ( formulaire_echeancier ),
			  "liste_adr_ventilation",
			  creation_liste_ope_de_ventil_echeances ( gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
											 "adr_echeance" )));

    quitter_ventilation_echeances ();
}
/* ************************************************************************** */

/* ************************************************************************** */
/* Fonction quitter_ventilation_echeances                                               */
/* appelée valider_ventilation_echeances et quitter_ventilation_echeances                         */
/* ************************************************************************** */
void quitter_ventilation_echeances ( void )
{
    /* Cette fonction remet la liste des structures de ventilation par défaut
       en recherchant les opérations de ventilation dans la liste des opérations
       puis appelle valider ventilation */

    gtk_widget_show ( barre_outils );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_liste_ventil_echeances ),
			    0 );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_calendrier_ventilations ),
			    0 );
    /*     on réaffiche le formulaire qu'on avait caché */
    gtk_widget_show ( formulaire_echeancier );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_formulaire_echeances ),
			    0 );

    gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_echeancier [SCHEDULER_FORM_CATEGORY] ) -> entry );

    if ( !montant_operation_ventilee_echeances )
    {
	if ( somme_ventilee_echeances < 0 )
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
				 g_strdup_printf ( "%4.2f", fabs ( somme_ventilee_echeances ) ));
	}
	else
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
				 g_strdup_printf ( "%4.2f", somme_ventilee_echeances ));
	}
    }
}
/* ************************************************************************** */

/***********************************************************************************************************/
/* Cette fonction prend une échéance en argument et crée la liste des opés de ventil */
/* qui correspondent avec des struct struct_ope_ventil */
/* renvoie cette liste */
/***********************************************************************************************************/

GSList *creation_liste_ope_de_ventil_echeances ( struct operation_echeance *operation )
{
    GSList *liste_ventil;
    GSList *liste_operations;
    GSList *liste_tmp;

    liste_ventil = NULL;
    liste_operations = NULL;

    /* si c'est une nouvelle opé, il n'y a aucun opé de ventil associée */

    if ( !operation )
	return ( NULL );


    liste_tmp = gsliste_echeances;

    while ( liste_tmp )
    {
	struct operation_echeance *operation_2;

	operation_2 = liste_tmp -> data;

	/* si l'opération est une opé de ventil de l'opé demandée, on lui fait une struct struct_ope_ventil */

	if ( operation_2 -> no_operation_ventilee_associee == operation -> no_operation )
	{
	    struct struct_ope_ventil *ope_ventil;

	    ope_ventil = calloc ( 1,
				  sizeof ( struct struct_ope_ventil ));

	    ope_ventil -> no_operation = operation_2 -> no_operation;
	    ope_ventil -> montant = operation_2 -> montant;
	    ope_ventil -> categorie = operation_2 -> categorie;
	    ope_ventil -> sous_categorie = operation_2 -> sous_categorie;

	    if ( operation_2 -> notes )
		ope_ventil -> notes = g_strdup ( operation_2 -> notes );

	    ope_ventil -> imputation = operation_2 -> imputation;
	    ope_ventil -> sous_imputation = operation_2 -> sous_imputation;

	    ope_ventil -> no_exercice = operation_2 -> no_exercice;

	    ope_ventil -> relation_no_compte = operation_2 -> compte_virement;

	    /* 	    si c'est un virement, la categ, la sous categ sont à 0 */
	    /* 		et relation_no_compte != -1 (sinon c'est qu'il n'y a pas de categ) */

	    if ( ope_ventil -> relation_no_compte != -1
		 &&
		 !ope_ventil -> categorie
		 &&
		 !ope_ventil -> sous_categorie )
		ope_ventil -> relation_no_operation = -1;

	    ope_ventil -> no_type_associe = operation_2 -> type_contre_ope;

	    liste_ventil = g_slist_append ( liste_ventil,
					    ope_ventil );
	}
	liste_tmp = liste_tmp -> next;
    }
    return ( liste_ventil );
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* cette fonction est appelée lors de la validation d'une ventilation */
/* l'opération en argument a déjà son numéro d'opé */
/* ellse fait le tour des structures de ventil et crée/supprime/modifie */
/* les opérations nécessaires */
/***********************************************************************************************************/

void validation_ope_de_ventilation_echeances ( struct operation_echeance *operation )
{
    GSList *liste_struct_ventilations;

    /* récupération de la liste de ventilations */

    liste_struct_ventilations = gtk_object_get_data ( GTK_OBJECT ( formulaire_echeancier ),
						      "liste_adr_ventilation" );

    /*   si cette liste est à -1, c'est qu'elle est null, donc rien à faire */

    if ( liste_struct_ventilations == GINT_TO_POINTER ( -1 ))
	return;

    while ( liste_struct_ventilations )
    {
	struct struct_ope_ventil *ope_ventil;

	ope_ventil = liste_struct_ventilations -> data;

	/*       si cette opé est supprimée, c'est ici */
	/* cela sous entend qu'elle existait déjà */

	if ( ope_ventil -> supprime )
	{
	    /* petite protection quand même, normalement le texte ne devrait jamais apparaitre */

	    if ( !ope_ventil -> no_operation )
		dialogue_warning ( _("A breakdown line is to be deleted though it is not yet registered."));
	    else
	    {
		GSList *tmp;

		tmp = g_slist_find_custom ( gsliste_echeances,
					    GINT_TO_POINTER ( ope_ventil -> no_operation ),
					    (GCompareFunc) recherche_echeance_par_no );

		if ( tmp )
		    supprime_echeance  ( tmp -> data );
	    }
	}
	else
	{
	    /* l'opération ne doit pas être supprimée, c'est qu'elle doit être créée ou modifiée */
	    /* 	  on n'a pas à s'embêter avec des changements de virements ou autres trucs bizarres, dans */
	    /* ce cas il y aura eu une suppression puis une nouvelle opération */

	    if ( ope_ventil -> no_operation )
	    {
		/* c'est une modif d'opération */

		GSList *tmp;

		tmp = g_slist_find_custom ( gsliste_echeances,
					    GINT_TO_POINTER ( ope_ventil -> no_operation ),
					    (GCompareFunc) recherche_echeance_par_no );

		if ( tmp )
		{
		    struct operation_echeance *ope_modifiee;

		    ope_modifiee = tmp -> data;

		    /* on récupère d'abord les modifs de l'opé de ventil */

		    ope_modifiee -> montant = ope_ventil -> montant;
		    ope_modifiee -> categorie = ope_ventil -> categorie;
		    ope_modifiee -> sous_categorie = ope_ventil -> sous_categorie;

		    if ( ope_ventil -> notes )
			ope_modifiee -> notes = g_strdup ( ope_ventil -> notes );

		    ope_modifiee -> no_exercice = ope_ventil -> no_exercice;

		    ope_modifiee -> imputation = ope_ventil -> imputation;
		    ope_modifiee -> sous_imputation = ope_ventil -> sous_imputation;

		    ope_modifiee -> compte_virement = ope_ventil -> relation_no_compte;
		    ope_modifiee -> type_contre_ope = ope_ventil -> no_type_associe;

		    /* on récupère ensuite les modifs de la ventilation */

		    ope_modifiee -> jour = operation -> jour;
		    ope_modifiee -> mois = operation -> mois;
		    ope_modifiee -> annee = operation -> annee;

		    ope_modifiee -> date = g_date_new_dmy ( operation -> jour,
							    operation -> mois,
							    operation -> annee );

		    ope_modifiee -> compte = operation -> compte;
		    ope_modifiee -> devise = operation -> devise;
		    ope_modifiee -> tiers = operation -> tiers;
		    ope_modifiee -> type_ope = operation -> type_ope;
		    ope_modifiee -> auto_man = operation -> auto_man;

		    /* théoriquement, cette ligne n'est pas nécessaire vu que c'est une modif d'opé de ventil */

		    ope_modifiee -> no_operation_ventilee_associee = operation -> no_operation;
		    ope_modifiee -> operation_ventilee = 1;
		

		}
	    }
	    else
	    {
		/* c'est une nouvelle opération */
		/*  on la crée, l'ajoute et si c'est un virement on crée la contre opération */

		struct operation_echeance *nouvelle_ope;

		nouvelle_ope = calloc ( 1,
					sizeof ( struct operation_echeance ));

		/* on récupère d'abord les modifs de l'opé de ventil */

		nouvelle_ope -> montant = ope_ventil -> montant;
		nouvelle_ope -> categorie = ope_ventil -> categorie;
		nouvelle_ope -> sous_categorie = ope_ventil -> sous_categorie;

		if ( ope_ventil -> notes )
		    nouvelle_ope -> notes = g_strdup ( ope_ventil -> notes );

		nouvelle_ope -> imputation = ope_ventil -> imputation;
		nouvelle_ope -> sous_imputation = ope_ventil -> sous_imputation;

		nouvelle_ope -> no_exercice = ope_ventil -> no_exercice;

		nouvelle_ope -> compte_virement = ope_ventil -> relation_no_compte;
		nouvelle_ope -> type_contre_ope = ope_ventil -> no_type_associe;


		/* on récupère ensuite les modifs de la ventilation */

		nouvelle_ope -> jour = operation -> jour;
		nouvelle_ope -> mois = operation -> mois;
		nouvelle_ope -> annee = operation -> annee;

		nouvelle_ope -> date = g_date_new_dmy ( operation -> jour,
							operation -> mois,
							operation -> annee );

		nouvelle_ope -> compte = operation -> compte;
		nouvelle_ope -> devise = operation -> devise;
		nouvelle_ope -> tiers = operation -> tiers;
		nouvelle_ope -> type_ope = operation -> type_ope;
		nouvelle_ope -> auto_man = operation -> auto_man;
		nouvelle_ope -> no_operation_ventilee_associee = operation -> no_operation;
		nouvelle_ope -> operation_ventilee = 1;

		/* on ajoute cette opé à la liste */

		nouvelle_ope -> no_operation = ++no_derniere_echeance;
		nb_echeances++;
		gsliste_echeances = g_slist_insert_sorted ( gsliste_echeances,
							    nouvelle_ope,
							    (GCompareFunc) comparaison_date_echeance );
	    }
	}
	liste_struct_ventilations = liste_struct_ventilations -> next;
    }
}
/***********************************************************************************************************/
