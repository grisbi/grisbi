/* Fichier ventilation.c */
/* s'occupe de tout ce qui concerne les ventilation des opérations */


/*     Copyright (C) 2000-2003  Cédric Auger */
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




/*START_INCLUDE*/
#include "ventilation.h"
#include "operations_formulaire.h"
#include "categories_onglet.h"
#include "operations_liste.h"
#include "exercice.h"
#include "type_operations.h"
#include "devises.h"
#include "dialog.h"
#include "gtk_combofix.h"
#include "imputation_budgetaire.h"
#include "utils.h"
#include "operations_onglet.h"
#include "comptes_traitements.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajoute_ope_sur_liste_ventilation ( struct struct_ope_ventil *operation );
static void ajuste_scrolling_liste_ventilations_a_selection ( void );
static gboolean appui_touche_ventilation ( GtkWidget *entree,
				    GdkEventKey *evenement,
				    gint *no_origine );
static void calcule_montant_ventilation ( void );
static gint cherche_ligne_from_operation_ventilee ( struct struct_ope_ventil *operation );
static struct struct_ope_ventil *cherche_operation_ventilee_from_ligne ( gint no_ligne );
static gboolean clique_champ_formulaire_ventilation ( void );
static void echap_formulaire_ventilation ( void );
static void edition_operation_ventilation ( void );
static gboolean entree_ventilation_perd_focus ( GtkWidget *entree, GdkEventFocus *ev,
					 gint *no_origine );
static void fin_edition_ventilation ( void );
static void mise_a_jour_couleurs_liste_ventilation ( void );
static void mise_a_jour_labels_ventilation ( void );
static void quitter_ventilation ( void );
static void remplit_liste_ventilation ( void );
static void selectionne_ligne_ventilation ( gint nouvelle_ligne );
static void supprime_operation_ventilation ( void );
static void valider_ventilation ( void );
/*END_STATIC*/




/* list strore des ventilations, créé à l'appel de la ventil */
/* et détruit ensuite */

GtkListStore *list_store_ventils = NULL;

/* contient le no de la ligne sélectionnée en cours */
/* mise à -1 lors de la création du list_store */

gint ligne_selectionnee_ventilation;

/* magouille utilisée pour bloquer un signal size-allocate qui s'emballe */

gint ancienne_largeur_ventilation;

GtkWidget *widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_TOTAL_WIDGET];
GtkWidget *separateur_formulaire_ventilations;
GtkWidget *hbox_valider_annuler_ventil;
gdouble montant_operation_ventilee;
gdouble somme_ventilee;


/* adresses des labels de montants à gauche */

GtkWidget *label_somme_ventilee;
GtkWidget *label_non_affecte;
GtkWidget *label_montant_operation_ventilee;

/* à 1 si au click du bouton valider on enregistre l'opé */

gint enregistre_ope_au_retour;



/*START_EXTERN*/
extern GtkWidget *barre_outils;
extern GtkTreeViewColumn *colonnes_liste_ventils[3];
extern gint compte_courant;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_selection;
extern GtkWidget *formulaire;
extern GtkWidget *frame_droite_bas;
extern GdkGC *gc_separateur_operation;
extern gint hauteur_ligne_liste_opes;
extern GSList *liste_categories_ventilation_combofix;
extern GSList *liste_imputations_combofix;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint nb_comptes;
extern GtkWidget *notebook_comptes_equilibrage;
extern GtkWidget *notebook_formulaire;
extern FILE * out;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern PangoFontDescription *pango_desc_fonte_liste;
extern GtkWidget *scrolled_window_liste_ventilations;
extern GtkTreeSelection * selection;
extern GtkStyle *style_entree_formulaire[2];
extern gdouble taux_de_change[2];
extern GtkWidget *tree_view;
extern GtkWidget *tree_view_liste_ventilations;
/*END_EXTERN*/



/*******************************************************************************************/
/* Fonction  creation_verification_ventilation*/
/* crée la fenetre à la place de la liste des comptes qui contient les boutons et l'état de la ventilation */
/*******************************************************************************************/

GtkWidget *creation_verification_ventilation ( void )
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
    gtk_signal_connect ( GTK_OBJECT ( onglet ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( traitement_clavier_liste_ventilation ),
			 NULL );
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

    label_somme_ventilee = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_somme_ventilee ),
			     1,
			     0.5 );
    gtk_table_attach ( GTK_TABLE ( tableau ),
		       label_somme_ventilee,
		       1, 2,
		       0, 1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( label_somme_ventilee );


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

    label_non_affecte = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_non_affecte ),
			     1,
			     0.5 );
    gtk_table_attach ( GTK_TABLE ( tableau ),
		       label_non_affecte,
		       1, 2,
		       1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( label_non_affecte );


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

    label_montant_operation_ventilee = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_montant_operation_ventilee ),
			     1,
			     0.5 );
    gtk_table_attach ( GTK_TABLE ( tableau ),
		       label_montant_operation_ventilee,
		       1, 2,
		       3, 4,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( label_montant_operation_ventilee );



    /* mise en place des boutons */

    hbox = gtk_hbox_new ( FALSE,
			  10 );
    gtk_box_pack_end ( GTK_BOX ( onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       10 );
    gtk_widget_show ( hbox );


    bouton = gtk_button_new_from_stock (GTK_STOCK_OK);

    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( valider_ventilation ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_from_stock    (GTK_STOCK_CANCEL);
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( annuler_ventilation ),
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
/* Fonction creation_formulaire_ventilation */
/* crée la fenetre qui contient e formulaire pour la ventilation */
/*******************************************************************************************/

GtkWidget *creation_formulaire_ventilation ( void )
{
    GtkWidget *onglet;
    GtkTooltips *tips;
    GtkWidget *table;
    GtkWidget *bouton;
    GtkWidget *menu;

    /* on crée le tooltips */

    tips = gtk_tooltips_new ();

    /* création du formulaire */

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

    widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] = gtk_combofix_new_complex ( liste_categories_ventilation_combofix,
								  FALSE,
								  TRUE,
								  TRUE,
								  0 );
    gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ) -> entry ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( appui_touche_ventilation ),
			 GINT_TO_POINTER ( TRANSACTION_BREAKDOWN_FORM_CATEGORY ) );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY]) -> entry),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY]) -> arrow),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation),
			 NULL );
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY]) -> entry ),
				"focus_in_event",
				GTK_SIGNAL_FUNC (entree_prend_focus),
				GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ) );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY]) -> entry),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
			 GINT_TO_POINTER ( TRANSACTION_BREAKDOWN_FORM_CATEGORY ) );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY],
		       0, 1, 0,1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_widget_show ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] );

    /* mise en place des notes */

    widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES] = gtk_entry_new ();
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES] ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( appui_touche_ventilation ),
			 GINT_TO_POINTER ( TRANSACTION_BREAKDOWN_FORM_NOTES ) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES]),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES]),
			 "focus_in_event",
			 GTK_SIGNAL_FUNC (entree_prend_focus),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES]),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
			 GINT_TO_POINTER ( TRANSACTION_BREAKDOWN_FORM_NOTES ) );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES],
		       1, 3, 0,1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_widget_show ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES] );



    /* mise en place du débit */

    widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] = gtk_entry_new ();
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( appui_touche_ventilation ),
			 GINT_TO_POINTER ( TRANSACTION_BREAKDOWN_FORM_DEBIT ) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT]),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT]),
			 "focus_in_event",
			 GTK_SIGNAL_FUNC (entree_prend_focus),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT]),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
			 GINT_TO_POINTER ( TRANSACTION_BREAKDOWN_FORM_DEBIT ) );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT],
		       3, 4, 0,1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_widget_show ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] );


    /* mise en place du crédit */

    widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] = gtk_entry_new ();
    gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( appui_touche_ventilation ),
			 GINT_TO_POINTER ( TRANSACTION_BREAKDOWN_FORM_CREDIT ) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT]),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT]),
			 "focus_in_event",
			 GTK_SIGNAL_FUNC (entree_prend_focus),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT]),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
			 GINT_TO_POINTER ( TRANSACTION_BREAKDOWN_FORM_CREDIT ) );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT],
		       4, 5, 0,1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_widget_show ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] );


    /*  Affiche l'imputation budgétaire */

    widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] = gtk_combofix_new_complex ( liste_imputations_combofix,
								  FALSE,
								  TRUE,
								  TRUE,
								  0 );
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY],
		       0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY]) -> entry),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY]) -> arrow),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY]) -> entry),
			 "key_press_event",
			 GTK_SIGNAL_FUNC (appui_touche_ventilation),
			 GINT_TO_POINTER( TRANSACTION_BREAKDOWN_FORM_BUDGETARY ) );
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY]) -> entry ),
				"focus_in_event",
				GTK_SIGNAL_FUNC (entree_prend_focus),
				GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] ) );
    gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY]) -> entry),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
			 GINT_TO_POINTER ( TRANSACTION_BREAKDOWN_FORM_BUDGETARY ) );

    gtk_widget_show (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY]);


    /* mise en place du type de l'opé associée en cas de virement */
    /* non affiché au départ */

    widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA],
			   _("Associated method of payment"),
			   _("Associated method of payment") );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA]),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( appui_touche_ventilation ),
			 GINT_TO_POINTER( TRANSACTION_BREAKDOWN_FORM_CONTRA ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA],
		       1, 2, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);

    /* création du bouton de l'exo */

    widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE] = gtk_option_menu_new ();
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE],
			   _("Choose the financial year"),
			   _("Choose the financial year") );
    menu = gtk_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE] ),
			       creation_menu_exercices (0) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE]),
			 "key_press_event",
			 GTK_SIGNAL_FUNC (appui_touche_ventilation),
			 GINT_TO_POINTER( TRANSACTION_BREAKDOWN_FORM_EXERCICE ) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE],
		       2, 3, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_widget_show ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE] );

    /*   création de l'entrée du no de pièce comptable */

    widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER] = gtk_entry_new();
    gtk_table_attach ( GTK_TABLE (table),
		       widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER],
		       3, 5, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0,0);
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER]),
			 "button_press_event",
			 GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER]),
			 "key_press_event",
			 GTK_SIGNAL_FUNC (appui_touche_ventilation),
			 GINT_TO_POINTER( TRANSACTION_BREAKDOWN_FORM_VOUCHER ) );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER]),
			 "focus_in_event",
			 GTK_SIGNAL_FUNC (entree_prend_focus),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER]),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
			 GINT_TO_POINTER ( TRANSACTION_BREAKDOWN_FORM_VOUCHER ) );
    gtk_widget_show ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER] );

    /* séparation d'avec les boutons */

    separateur_formulaire_ventilations = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 separateur_formulaire_ventilations,
			 FALSE,
			 FALSE,
			 0 );
    if ( etat.affiche_boutons_valider_annuler )
	gtk_widget_show ( separateur_formulaire_ventilations );

    /* mise en place des boutons */

    hbox_valider_annuler_ventil = gtk_hbox_new ( FALSE,
						 5 );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 hbox_valider_annuler_ventil,
			 FALSE,
			 FALSE,
			 0 );
    if ( etat.affiche_boutons_valider_annuler )
	gtk_widget_show ( hbox_valider_annuler_ventil );

    bouton = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( echap_formulaire_ventilation ),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox_valider_annuler_ventil ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_from_stock (GTK_STOCK_OK);
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( fin_edition_ventilation ),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox_valider_annuler_ventil ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( bouton );


    /*   met l'adr de l'opé dans le formulaire à -1 */

    gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
			  "adr_struct_ope",
			  GINT_TO_POINTER ( -1 ) );

    return ( onglet );
}
/*******************************************************************************************/





/***********************************************************************************************************/
gboolean clique_champ_formulaire_ventilation ( void )
{

    /* on rend sensitif tout ce qui ne l'était pas sur le formulaire */

    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
			       TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE] ),
			       TRUE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( hbox_valider_annuler_ventil ),
			       TRUE );

    return FALSE;
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction appelée quand une entry perd le focus */
/* si elle ne contient rien, on remet la fonction en gris */
/***********************************************************************************************************/

gboolean entree_ventilation_perd_focus ( GtkWidget *entree, GdkEventFocus *ev,
					 gint *no_origine )
{
    gchar *texte;

    texte = NULL;

    switch ( GPOINTER_TO_INT ( no_origine ))
    {
	/* on sort des catégories */
	case TRANSACTION_BREAKDOWN_FORM_CATEGORY :
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

				if ( !GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] )
				     ||
				     ( recupere_no_compte ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] )
				       !=
				       compte_virement ))
				{
				    /* vérifie quel est le montant entré, affiche les types opposés de l'autre compte */

				    GtkWidget *menu;

				    if ( gtk_widget_get_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] ) == style_entree_formulaire[ENCLAIR] )
					/* il y a un montant dans le crédit */
					menu = creation_menu_types ( 1, compte_virement, 2  );
				    else
					/* il y a un montant dans le débit ou défaut */
					menu = creation_menu_types ( 2, compte_virement, 2  );

				    /* si un menu à été créé, on l'affiche */

				    if ( menu )
				    {
					gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
								   menu );
					gtk_widget_show ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] );
				    }

				    /* on associe le no de compte de virement au formulaire pour le retrouver */
				    /* rapidement s'il y a un chgt débit/crédit */

				    gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
							  "compte_virement",
							  GINT_TO_POINTER ( compte_virement ));
				}
			    }
			    else
				gtk_widget_hide ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] );
			}
			else
			    gtk_widget_hide ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] );
		    }
		    else
			gtk_widget_hide ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] );
		}
		else
		    gtk_widget_hide ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] );

		g_strfreev ( tableau_char );
	    }
	    else
		texte = _("Categories : Sub-categories");
	    break;

	    /* sort des notes */

	case TRANSACTION_BREAKDOWN_FORM_NOTES :
	    if ( !strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Notes");
	    break;

	    /* sort du débit */
	    /*   soit vide, soit change le menu des types s'il ne correspond pas */

	case TRANSACTION_BREAKDOWN_FORM_DEBIT :

	    if ( strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree ))))
		 &&
		 gtk_widget_get_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	    {
		/* on  commence par virer ce qu'il y avait dans les crédits */

		if ( gtk_widget_get_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] ) == style_entree_formulaire[ENCLAIR] )
		{
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] ),
					 "" );
		    gtk_widget_set_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT],
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT]),
					 _("Credit") );
		}

		/* comme il y a eu un changement de signe, on change aussi le type de l'opé associée */
		/* s'il est affiché */

		if ( GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] )
		     &&
		     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ) -> menu ),
							     "signe_menu" ))
		     ==
		     1 )
		{
		    GtkWidget *menu;

		    menu = creation_menu_types ( 2,
						 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
											 "compte_virement" )),
						 2  );

		    if ( menu )
			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
						   menu );
		    else
			gtk_option_menu_remove_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ));
		}
	    }
	    else
		texte = _("Debit");
	    break;

	    /* sort du crédit */

	case TRANSACTION_BREAKDOWN_FORM_CREDIT :
	    if ( strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree ))))
		 &&
		 gtk_widget_get_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] ) == style_entree_formulaire[ENCLAIR])
	    {
		/* on  commence par virer ce qu'il y avait dans les débits */

		if ( gtk_widget_get_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
		{
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] ),
					 "" );
		    gtk_widget_set_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT],
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT]),
					 _("Debit") );
		}

		/* comme il y a eu un changement de signe, on change aussi le type de l'opé associée */
		/* s'il est affiché */

		if ( GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] )
		     &&
		     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ) -> menu ),
							     "signe_menu" ))
		     ==
		     2 )
		{
		    GtkWidget *menu;

		    menu = creation_menu_types ( 1,
						 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
											 "compte_virement" )),
						 2  );

		    if ( menu )
			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
						   menu );
		    else
			gtk_option_menu_remove_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ));
		}
	    }
	    else
		texte = _("Credit");
	    break;

	    /* sort de l'ib */

	case TRANSACTION_BREAKDOWN_FORM_BUDGETARY :
	    if ( !strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Budgetary line");
	    break;

	    /* sort de la pièce comptable */

	case TRANSACTION_BREAKDOWN_FORM_VOUCHER :
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
/* Fonction ventiler_operation */
/* appelée lorsque la catégorie est Ventilation lors de l'enregistrement d'une opé */
/* ou lors d'une modif d'une opé ventilée */
/* Arguments : montant de l'opé */
/*******************************************************************************************/

void ventiler_operation ( gdouble montant )
{
    /*     on affiche la tree_view des ventilations */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    gtk_widget_hide ( SCROLLED_WINDOW_LISTE_OPERATIONS );

    gtk_widget_show ( scrolled_window_liste_ventilations );

    /* on met la taille au formulaire et à la liste */

    ancienne_largeur_ventilation = 0;

    montant_operation_ventilee = montant;

    ligne_selectionnee_ventilation = -1;

    /* remplit la liste */

    remplit_liste_ventilation ();

    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view_liste_ventilations  ),
			      GTK_TREE_MODEL ( list_store_ventils ));

    /* met à jour les labels */

    gtk_label_set_text ( GTK_LABEL ( label_somme_ventilee ),
			 g_strdup_printf ( "%4.2f",
					   somme_ventilee ) );


    /*   s'il n'y a pas de montant total, celui ci = la somme ventilée */

    if ( montant_operation_ventilee )
    {
	gtk_label_set_text ( GTK_LABEL ( label_non_affecte ),
			     g_strdup_printf ( "%4.2f",
					       montant_operation_ventilee - somme_ventilee ));
	gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee ),
			     g_strdup_printf ( "%4.2f",
					       montant_operation_ventilee ));
    }
    else
    {
	gtk_label_set_text ( GTK_LABEL ( label_non_affecte ),
			     "" );
	gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee ),
			     g_strdup_printf ( "%4.2f",
					       somme_ventilee ));
    }


    /* affiche les pages de ventilation */

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			    1 );
    gtk_widget_hide ( formulaire );
    gtk_widget_hide ( frame_droite_bas );
    gtk_widget_show ( frame_droite_bas );
    gtk_widget_hide ( barre_outils );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_formulaire ),
			    1 );

    /* on grise tout ce qu'il faut */

    echap_formulaire_ventilation ();

    gtk_widget_grab_focus ( tree_view_liste_ventilations );
}
/*******************************************************************************************/





/***************************************************************************************************/
/* Fonction traitement_clavier_liste */
/* gère le clavier sur la clist */
/***************************************************************************************************/

gboolean traitement_clavier_liste_ventilation ( GtkWidget *widget_variable,
						GdkEventKey *evenement )
{
    switch ( evenement->keyval )
    {
	/* entrée */
	case GDK_KP_Enter:
	case GDK_Return:

	    edition_operation_ventilation ();
	    break;

	case GDK_Up :		/* touches flèche haut */
	case GDK_KP_Up :

	    if ( ligne_selectionnee_ventilation )
		selectionne_ligne_ventilation ( ligne_selectionnee_ventilation - 1 );
	    break;


	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :

	    if ( ligne_selectionnee_ventilation
		 !=
		 (GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_ventilations )))->length - 1 ))
		selectionne_ligne_ventilation ( ligne_selectionnee_ventilation + 1 );
	    break;


	    /*  del  */
	case GDK_Delete:

	    supprime_operation_ventilation ();
	    break;


	default : 
	    return TRUE;
    }

    return TRUE;
}
/***************************************************************************************************/






/***************************************************************************************************/
/* Fonction selectionne_ligne_souris */
/* place la sélection sur l'opé clickée */
/***************************************************************************************************/

gboolean selectionne_ligne_souris_ventilation ( GtkWidget *tree_view,
						GdkEventButton *evenement )
{
    gint x, y;
    gint ligne;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_colonne;

/*     on n'accède pas à cette fonction par un signal, mais par selectionne_ligne_souris */
/* 	qui provient de la liste d'opé */

    /* Récupération des coordonnées de la souris */

    gdk_window_get_pointer ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
			     &x,
			     &y,
			     FALSE );

    /*     on récupère le path aux coordonnées */
    /* 	si ce n'est pas une ligne de la liste, on se barre */

    if ( !gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
					  x,
					  y,
					  &path,
					  &tree_colonne,
					  NULL,
					  NULL ))
	return (TRUE);

    /* Récupération de la ligne de l'opération cliquée */

    ligne = my_atoi ( gtk_tree_path_to_string ( path ));

    selectionne_ligne_ventilation( ligne );

    /*  si on a double-cliqué sur une opération, c'est ici */

    if ( evenement -> type == GDK_2BUTTON_PRESS )
	edition_operation_ventilation ();
    else
	gtk_widget_grab_focus ( tree_view );

    return ( TRUE );
}
/***************************************************************************************************/



/***********************************************************************************************************/
/* Fonction appui_touche_ventilation  */
/* gére l'action du clavier sur les entrées du formulaire de ventilation */
/***********************************************************************************************************/
gboolean appui_touche_ventilation ( GtkWidget *entree,
				    GdkEventKey *evenement,
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

	    gtk_widget_grab_focus ( entree );
	    return TRUE;
	    break;


	case GDK_Tab:

	    /* on efface la sélection en cours si c'est une entrée ou un combofix */

	    if ( GTK_IS_ENTRY ( entree ))
		gtk_entry_select_region ( GTK_ENTRY ( entree ), 0, 0);
	    else
		if ( GTK_IS_COMBOFIX ( entree ))
		    gtk_entry_select_region ( GTK_ENTRY(GTK_COMBOFIX(entree) -> entry),
					      0, 0);

	    /* on donne le focus au widget suivant */

	    origine = (origine + 1 ) % TRANSACTION_BREAKDOWN_FORM_TOTAL_WIDGET;

	    while ( !(GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation[origine] )
		      &&
		      GTK_WIDGET_SENSITIVE ( widget_formulaire_ventilation[origine] )
		      &&
		      ( GTK_IS_COMBOFIX (widget_formulaire_ventilation[origine] )
			||
			GTK_IS_ENTRY ( widget_formulaire_ventilation[origine] )
			||
			GTK_IS_BUTTON ( widget_formulaire_ventilation[origine] ) )))
		origine = (origine + 1 ) % TRANSACTION_BREAKDOWN_FORM_TOTAL_WIDGET;

	    /*       si on se retrouve sur les catég et que etat.entree = 0, on enregistre l'opérations */

	    if ( !origine && !etat.entree )
	    {
		fin_edition_ventilation();
		return TRUE;
	    }

	    /* si on se retrouve sur le crédit et qu'il y a qque chose dans le débit, on passe au suivant */
	    /*       à ce niveau, il n'y a pas eu encore de focus out donc on peut tester par strlen */

	    if ( origine == TRANSACTION_BREAKDOWN_FORM_CREDIT
		 &&
		 strlen ( (char *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] ))))
		origine = (origine + 1 ) % TRANSACTION_BREAKDOWN_FORM_TOTAL_WIDGET;

	    /* on sélectionne le contenu de la nouvelle entrée */

	    if ( GTK_IS_COMBOFIX ( widget_formulaire_ventilation[origine] ) )
	    {
		gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation[origine] ) -> entry );  
		gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( widget_formulaire_ventilation[origine] ) -> entry ),
					  0,
					  -1 );
	    }
	    else
	    {
		if ( GTK_IS_ENTRY ( widget_formulaire_ventilation[origine] ) )
		    gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_ventilation[origine] ),
					      0,
					      -1 );

		gtk_widget_grab_focus ( widget_formulaire_ventilation[origine]  );
	    }
	    return TRUE;
	    break;


	case GDK_KP_Enter:
	case GDK_Return:

	    fin_edition_ventilation ();
	    return TRUE;
	    break;


	case GDK_Escape :

	    echap_formulaire_ventilation ();
	    break;

	default:
	    return FALSE;
    }
    return FALSE;
}
/***********************************************************************************************************/





/***********************************************************************************************************/
void echap_formulaire_ventilation ( void )
{

    /* on met les styles des entrées au gris */

    gtk_widget_set_style ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] )->entry,
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT],
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] )->entry,
			   style_entree_formulaire[ENGRIS] );
    gtk_widget_set_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER],
			   style_entree_formulaire[ENGRIS] );


    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
			    _("Categories : Sub-categories") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES]),
			 _("Notes") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT]),
			 _("Debit") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT]),
			 _("Credit") );
    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] ),
			    _("Budgetary line") );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER]),
			 _("Voucher") );

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
				  0 );

    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
			       FALSE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE] ),
			       FALSE );
    gtk_widget_set_sensitive ( GTK_WIDGET ( hbox_valider_annuler_ventil ),
			       FALSE );
    gtk_widget_set_sensitive ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY],
			       TRUE );
    gtk_widget_set_sensitive ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT],
			       TRUE );
    gtk_widget_set_sensitive ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT],
			       TRUE );

    gtk_widget_hide ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] );

    /*   met l'adr de l'opé dans le formulaire à -1 */

    gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
			  "adr_struct_ope",
			  GINT_TO_POINTER ( -1 ) );
    
    gtk_widget_grab_focus ( tree_view_liste_ventilations );

}
/***********************************************************************************************************/





/***********************************************************************************************************/
void fin_edition_ventilation ( void )
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

    gtk_widget_grab_focus ( tree_view_liste_ventilations );

    /* perte ligne sélectionnée sera à 1 s'il y a une magouille avec les virements et */
    /* qu'on recrée une opé au lieu de la modifier. dans ce cas on remettra la ligne */
    /* sélectionné sur la nouvelle opé */

    perte_ligne_selectionnee = 0;

    /*   dans cette fonction, on récupère les infos du formulaire qu'on met dans une structure */
    /* de ventilation, et on ajoute cette structure à celle en cours (ou modifie si elle existait */
    /* déjà */

    /* on vérifie si c'est un virement que le compte est valide et que ce n'est pas un virement sur lui-même */


    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	/*       on split déjÃ  les catég, sans libérer la variable, pour la récupérer ensuite pour les categ */

	tableau_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ))),
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
			{
			    if ( COMPTE_CLOTURE )
				compte_vire = -2;
			    else
				compte_vire = i;
			}
		    }

		    if ( compte_vire == -1 )
		    {
			dialogue_error ( _("The associated account for this transfer is invalid") );
			return;
		    }

		    if ( compte_vire == -2 )
		    {
			dialogue_error ( _("The associated account for this transfer is closed") );
			return;
		    }

		    if ( compte_vire == compte_courant )
		    {
			dialogue_error ( _("It's impossible to transfer an account to itself") );
			return;
		    }
		}
		else
		{
		    dialogue_error ( _("No account associated with the transfer") );
		    return;
		}
	    }
	}
    }


    /*   on récupère l'adresse de l'opération, soit c'est une modif, soit c'est une nouvelle (-1) */

    operation = gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
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
    /* soit virement vers le mÃªme compte */
    /* soit virement vers un autre compte */
    /* soit ce n'est plus un virement */
    /*     pour la 1ère, c'est une modif normale d'opé */
    /*     pour les 2nde et 3ème, on supprime cette opé et en recrée une nouvelle */

    /* il faut donc mettre la récup des catég en premier car il peut y avoir un changement au niveau des */
    /* modif avec suppression de l'ancienne et création d'une nouvelle ope */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	struct struct_categ *categ;

	if ( strlen ( tableau_char[0] ) )
	{
	    /* on vérifie ici si c'est un virement */

	    if ( strcmp ( tableau_char[0],
			  _("Transfer") ) )
	    {
		/* ce n'est pas un virement, recherche les catég */

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

		categ = categ_par_nom ( tableau_char[0],
					1,
					operation -> montant < 0,
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
	/* il n'y a aucune catég, si c'est une modif d'opé et que cette opé était un virement, */
	/* on marque cette opé comme supprimée et on en recrée une nouvelle */

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
    }

    /* récupération du type d'opé associée s'il est affiché */

    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ))
	operation -> no_type_associe = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ) -> menu_item ),
									       "no_type" ));

    /* récupération des notes */

    if ( gtk_widget_get_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES] ) == style_entree_formulaire[ENCLAIR] )
	operation -> notes = g_strdup ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES] ))));
    else
	operation -> notes = NULL;


    /* récupération du montant */

    if ( gtk_widget_get_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] ) == style_entree_formulaire[ENCLAIR] )
	/* c'est un débit */
	operation -> montant = -my_strtod ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] ))),
					    NULL );
    else
	operation -> montant = my_strtod ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] ))),
					   NULL );



    /* récupération de l'imputation budgétaire */

    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	struct struct_imputation *imputation;
	gchar **tableau_char;

	tableau_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] )),
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
	    else
		operation -> sous_imputation = 0;
	}
	else
	    operation -> imputation = 0;

	g_strfreev ( tableau_char );
    }

    /* récupération de l'exercice */
    /* si l'exo est à -1, c'est que c'est sur non affiché */
    /* soit c'est une modif d'opé et on touche pas à l'exo */
    /* soit c'est une nouvelle opé et on met l'exo à 0 */

    if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE] ) -> menu_item ),
						 "no_exercice" )) == -1 )
    {
	if ( !operation -> no_operation )
	    operation -> no_exercice = 0;
    }
    else
	operation -> no_exercice = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE] ) -> menu_item ),
									   "no_exercice" ));


    /* récupération du no de pièce comptable */

    if ( gtk_widget_get_style ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER] ) == style_entree_formulaire[ENCLAIR] )
	operation -> no_piece_comptable = g_strdup ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER] ))));
    else
	operation -> no_piece_comptable = NULL;



    /* on a rempli l'opération, on l'ajoute à la liste */
    /* si c'est une modif */

    if ( !modification )
    {
	GSList *liste_struct_ventilations;

	/* récupération de la liste de ventilations */

	liste_struct_ventilations = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
							  "liste_adr_ventilation" );

	/*   si cette liste est à -1 (ce qui veut dire qu'elle est nulle en réalité mais */
	/* qu'elle a déjà été éditée ), on la met à 0 */

	if ( liste_struct_ventilations == GINT_TO_POINTER ( -1 ))
	    liste_struct_ventilations = NULL;

	/* on ajoute l'opé */

	liste_struct_ventilations = g_slist_append ( liste_struct_ventilations,
						     operation );

	gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			      "liste_adr_ventilation",
			      liste_struct_ventilations );
    }


    /* on met à jour la liste des ventilations */

    remplit_liste_ventilation ();

    /*   si perte_ligne_selectionnee = 1, c'est qu'au lieu de modifier une opé (virement), on l'a */
    /* effacé puis recréÃ© une nouvelle. comme ça se fait que lors d'une modif d'opé, on remet */
    /* la selection sur cette nouvelle opé */

    if ( perte_ligne_selectionnee == 1 )
	selectionne_ligne_ventilation ( cherche_ligne_from_operation_ventilee ( operation ));


    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ ();
    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();

    /* efface le formulaire et prépare l'opé suivante */

    echap_formulaire_ventilation ();

    if ( modification )
	gtk_widget_grab_focus ( tree_view_liste_ventilations );
    else
    {
	clique_champ_formulaire_ventilation ();
	gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ) -> entry );
    }
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* recherche la ligne d'une ventil donnée en argument */
/* renvoie le no ou -1 si pas trouvée */
/***********************************************************************************************************/
gint cherche_ligne_from_operation_ventilee ( struct struct_ope_ventil *operation )
{
    gint no_ligne;
    GtkTreeIter iter;

    gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( list_store_ventils ),
				    &iter );
    no_ligne = -1;

    do
    {
	gpointer adresse;

	gtk_tree_model_get ( GTK_TREE_MODEL ( list_store_ventils ),
			     &iter,
			     3, &adresse,
			     -1 );

	if ( adresse == operation )
	    no_ligne = my_atoi ( gtk_tree_model_get_string_from_iter ( GTK_TREE_MODEL ( list_store_ventils ),
								    &iter ));
    }
    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( list_store_ventils ),
				       &iter)
	    &&
	    no_ligne == -1 );

    return no_ligne;
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* recherche l'opé de ventil dont le no de ligne est donné en argument */
/* renvoie le no ou NULL si pas trouvée */
/***********************************************************************************************************/
struct struct_ope_ventil *cherche_operation_ventilee_from_ligne ( gint no_ligne )
{
    GtkTreeIter iter;
    struct struct_ope_ventil *operation;

    if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( list_store_ventils ),
					       &iter,
					       itoa ( no_ligne )))
    {
	gtk_tree_model_get ( GTK_TREE_MODEL ( list_store_ventils ),
			     &iter,
			     3, &operation,
			     -1 );
    }
    else
	operation = NULL;

    return operation;
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction edition_operation_ventilation */
/* appelé lors d'un double click ou entrée sur une opé de ventilation */
/***********************************************************************************************************/

void edition_operation_ventilation ( void )
{
    struct struct_ope_ventil *operation;
    gchar *char_tmp;

    /* on récupère la struc de l'opé de ventil, ou -1 si c'est une nouvelle */

    operation = cherche_operation_ventilee_from_ligne (ligne_selectionnee_ventilation);

    echap_formulaire_ventilation ();

    /* dégrise ce qui est nécessaire */

    clique_champ_formulaire_ventilation ();

    gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
			  "adr_struct_ope",
			  operation );

    /* si l'opé est -1, c'est que c'est une nouvelle opé */

    if ( operation == GINT_TO_POINTER ( -1 ) )
    {
	gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ) -> entry );
	ligne_selectionnee_ventilation = -1;
	return;
    }


    /*   l'opé n'est pas -1, c'est une modif, on remplit les champs */

    gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
			  "adr_struct_ope",
			  operation );


    /* mise en forme du montant */


    if ( operation -> montant < 0 )
    {
	entree_prend_focus (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] ),
			     g_strdup_printf ( "%4.2f", -operation -> montant ));
    }
    else
    {
	entree_prend_focus (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] ),
			     g_strdup_printf ( "%4.2f", operation -> montant ));
    }

    /* si l'opération est relevée, empêche la modif du montant */

    if ( operation -> pointe == 3 )
    {
	gtk_widget_set_sensitive ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT],
				   FALSE );
	gtk_widget_set_sensitive ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT],
				   FALSE );
    }

    /* mise en forme des catégories */

    if ( operation -> relation_no_operation )
    {
	/* c'est un virement */

	GtkWidget *menu;

	entree_prend_focus (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] );

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
				g_strconcat ( COLON(_("Transfer")),
					      NOM_DU_COMPTE,
					      NULL ));

	/*       si la contre opération est relevée, on désensitive les montants et les categ */
	/* seulement valable si ce virement existe déjà */

	if ( operation -> no_operation )
	{
	    struct structure_operation *contre_operation;

	    contre_operation = operation_par_no ( operation -> relation_no_operation,
						  operation -> relation_no_compte );

	    if ( contre_operation
		 &&
		 contre_operation -> pointe == 3 )
	    {
		gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
					   FALSE );
		gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] ),
					   FALSE );
		gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] ),
					   FALSE );
	    }
	}
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
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
				       menu );

	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
					  cherche_no_menu_type_associe ( operation -> no_type_associe,
									 1 ));
	    gtk_widget_show ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] );

	}
    }
    else
    {
	gchar *char_tmp;

	char_tmp = nom_categ_par_no ( operation -> categorie,
				      operation -> sous_categorie );

	if ( char_tmp )
	{
	    entree_prend_focus (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] );
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
				    char_tmp);
	}
    }


    /* mise en forme des notes */

    if ( operation -> notes )
    {
	entree_prend_focus (widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES] ),
			     operation -> notes );
    }


    /* met en place l'imputation budgétaire */

    char_tmp = nom_imputation_par_no ( operation -> imputation,
				       operation -> sous_imputation );
    if ( char_tmp )
    {
	entree_prend_focus ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] );
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] ),
				char_tmp );
    }


    /* met en place l'exercice */

    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE] ),
				   cherche_no_menu_exercice ( operation -> no_exercice,
							      widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE] ));

    /* mise en place de la pièce comptable */

    if ( operation -> no_piece_comptable )
    {
	entree_prend_focus ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER] ),
			     operation -> no_piece_comptable );
    }


    /*   on a fini de remplir le formulaire, on donne le focus à la date */

    if ( GTK_WIDGET_SENSITIVE ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ))
    {
	gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ) -> entry ),
				  0,
				  -1);

	gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ) -> entry );
    }
    else
    {
	gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES] ),
				  0,
				  -1);
	gtk_widget_grab_focus ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES] );
    }

    enregistre_ope_au_retour = 1 ;
}
/***********************************************************************************************************/





/***********************************************************************************************************/
void supprime_operation_ventilation ( void )
{
    struct struct_ope_ventil *operation;
    GtkTreeIter iter;

/*     supprime l'opé de ventil pointÃ©e par ligne_selectionnee_ventilation */

    operation = cherche_operation_ventilee_from_ligne ( ligne_selectionnee_ventilation );

    if ( operation == GINT_TO_POINTER ( -1 )
	 ||
	 !operation )
	return;

    /* si l'opération est relevée ou si c'est un virement et que la contre opération est */
    /*   relevée, on ne peut la supprimer */

    if ( operation -> pointe == 3 )
    {
	dialogue_error ( _("This transaction has a reconciled breakdown line, deletion canceled.") );
	return;
    }
    else
    {
	if ( operation -> relation_no_operation
	     &&
	     operation -> relation_no_operation != -1 )
	{
	    /* on va chercher la contre opération */

	    struct structure_operation *operation_associee;

	    operation_associee = operation_par_no ( operation -> relation_no_operation,
						    operation -> relation_no_compte );

	    if ( operation_associee
		 &&
		 operation_associee -> pointe == 3 )
	    {
		dialogue_error ( _("This transfer has a reconciled contra-transaction, deletion canceled.") );
		return;
	    }
	}
    }


    /* on marque cette opération comme supprimée si ce n'est pas une nouvelle */
    /* sinon on la supprime tout simplement de la liste */

    if ( operation -> no_operation )
	operation -> supprime = 1;
    else
    {
	GSList *liste_struct_ventilations;

	liste_struct_ventilations = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
							  "liste_adr_ventilation" );

	liste_struct_ventilations = g_slist_remove ( liste_struct_ventilations,
						     operation );
	gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			      "liste_adr_ventilation",
			      liste_struct_ventilations );
    }

    selectionne_ligne_ventilation (ligne_selectionnee_ventilation + 1);

    /* supprime l'opération de la liste */

    if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( list_store_ventils ),
					       &iter,
					       itoa (ligne_selectionnee_ventilation - 1)))
	gtk_list_store_remove ( GTK_LIST_STORE( list_store_ventils ),
				&iter );

    calcule_montant_ventilation();
    mise_a_jour_couleurs_liste_ventilation();
    enregistre_ope_au_retour = 1 ;
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction remplit_liste_ventilation */
/* récupère la liste des struct d'opé de ventil sur le formulaire et affiche ces opés */
/***********************************************************************************************************/

void remplit_liste_ventilation ( void )
{
    GSList *liste_tmp;
    GtkTreeIter iter;
    gint i;

    somme_ventilee = 0;

    if ( list_store_ventils )
	gtk_list_store_clear ( GTK_LIST_STORE( list_store_ventils ));
    else
    {
	/*     on crée le list_store qui va contenir les ventils */
	/* 	col 0 à 2 -> les données */
	/* 	col 3 -> l'adr de l'opé */
	/* 	    col 4 -> couleur du fond */
	/* 	    clo 5 -> sauvegarde couleur background quand ligne sélectionnée */
	/* 	    col 6 -> la fonte */

	list_store_ventils = gtk_list_store_new ( 7,
						  G_TYPE_STRING,
						  G_TYPE_STRING,
						  G_TYPE_STRING,
						  G_TYPE_POINTER,
						  GDK_TYPE_COLOR,
						  GDK_TYPE_COLOR,
						  PANGO_TYPE_FONT_DESCRIPTION );
    }


    
    /* récupère la liste des struct_ope_ventil */

    liste_tmp = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				      "liste_adr_ventilation" );

    while ( liste_tmp && GPOINTER_TO_INT ( liste_tmp ) != -1 )
    {
	ajoute_ope_sur_liste_ventilation ( liste_tmp -> data );

	liste_tmp = liste_tmp -> next;
    }


    /* ajoute la ligne blanche associee à -1 */

    gtk_list_store_append ( list_store_ventils,
			    &iter );

    for ( i=0 ; i<3 ; i++ )
	gtk_list_store_set ( list_store_ventils,
			     &iter,
			     i, NULL,
			     -1 );

    /*     si elle est sélectionnée, c'est ici */

    if ( ligne_selectionnee_ventilation == -1 )
	{
	    gtk_list_store_set ( list_store_ventils,
				 &iter,
				 4, &couleur_selection,
				 -1 );

	    ligne_selectionnee_ventilation = my_atoi ( gtk_tree_model_get_string_from_iter ( GTK_TREE_MODEL ( list_store_ventils ),
									      &iter ));
	}

    /* on met le no d'opération de cette ligne à -1 */

    gtk_list_store_set ( list_store_ventils,
			 &iter,
			 3, GINT_TO_POINTER (-1),
			 -1 );



    /* on met la couleur */

    mise_a_jour_couleurs_liste_ventilation ();


    /* on met à jour les labels d'état */

    calcule_montant_ventilation ();
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* prend en argument une opé de ventil dont l'adr de la struct est donnée en argument */
/***********************************************************************************************************/

void ajoute_ope_sur_liste_ventilation ( struct struct_ope_ventil *operation )
{
    gchar *ligne[3];
    GtkTreeIter iter;
    gint i;

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
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;
    }
    else
	/* c'est des categ : sous categ */
	ligne[0] = nom_categ_par_no ( operation -> categorie,
				      operation -> sous_categorie );

    /* mise en forme des notes */

    ligne[1] = operation -> notes;

    /* mise en forme du montant */

    ligne[2] = g_strdup_printf ( "%4.2f",
				 operation -> montant );


    /* ajoute la ligne  */

    gtk_list_store_append ( list_store_ventils,
			    &iter );

    for ( i=0 ; i<3 ; i++ )
	gtk_list_store_set ( list_store_ventils,
			     &iter,
			     i, ligne[i],
			     -1 );

    /*     si elle est sélectionnée, c'est ici */

    if ( ligne_selectionnee_ventilation == my_atoi ( gtk_tree_model_get_string_from_iter ( GTK_TREE_MODEL ( list_store_ventils ),
											&iter )))
	gtk_list_store_set ( list_store_ventils,
			     &iter,
			     4, &couleur_selection,
			     -1 );

    /* 		    si on utilise une fonte perso, c'est ici */

    if ( etat.utilise_fonte_listes )
	gtk_list_store_set ( list_store_ventils,
			     &iter,
			     6, pango_desc_fonte_liste,
			     -1 );

    /* on met le no d'opération  */

    gtk_list_store_set ( list_store_ventils,
			 &iter,
			 3, operation,
			 -1 );
}
/***********************************************************************************************************/


/***********************************************************************************************************/
void calcule_montant_ventilation ( void )
{
    GSList *liste_tmp;

    /* fait le tour de la liste pour retrouver les ventil affichée pour calculer le montant */

    somme_ventilee = 0;
    liste_tmp = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				      "liste_adr_ventilation" );

    while ( liste_tmp && GPOINTER_TO_INT ( liste_tmp ) != -1 )
    {
	struct struct_ope_ventil *operation;

	operation = liste_tmp -> data;

	if ( !operation -> supprime )
	    somme_ventilee = somme_ventilee + operation -> montant;

	liste_tmp = liste_tmp -> next;
    }

    mise_a_jour_labels_ventilation ();
}
/***********************************************************************************************************/




/***********************************************************************************************************/
void mise_a_jour_labels_ventilation ( void )
{
    gtk_label_set_text ( GTK_LABEL ( label_somme_ventilee ),
			 g_strdup_printf ( "%4.2f",
					   somme_ventilee ));

    if ( montant_operation_ventilee )
    {
	gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee ),
			     g_strdup_printf ( "%4.2f",
					       montant_operation_ventilee ));

	gtk_label_set_text ( GTK_LABEL ( label_non_affecte ),
			     g_strdup_printf ( "%4.2f",
					       montant_operation_ventilee - somme_ventilee ));
    }
    else
    {
	gtk_label_set_text ( GTK_LABEL ( label_non_affecte ),
			     "" );
	gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee ),
			     g_strdup_printf ( "%4.2f",
					       somme_ventilee ));
    }
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fait le tour le la liste de ventilation et met bien les couleurs */
/***********************************************************************************************************/

void mise_a_jour_couleurs_liste_ventilation ( void )
{
    gint couleur_en_cours;
    GtkTreeIter iter;

/*     met l'alternance de couleurs de la liste */
/* 	ne sélectionne pas car déjà fait avant */

/*     pas besoin de test car il y a au moins la ligne blanche... */

    gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( list_store_ventils ),
				    &iter );
    couleur_en_cours = 0;
    do
    {
/* 	si la ligne est sélectionnée, on le place en sauvegarde de background */

	if ( ligne_selectionnee_ventilation == my_atoi ( gtk_tree_model_get_string_from_iter ( GTK_TREE_MODEL ( list_store_ventils ),
											    &iter )))
	    gtk_list_store_set ( list_store_ventils,
				 &iter,
				 5, &couleur_fond[couleur_en_cours],
				 -1 );
	else
	    gtk_list_store_set ( list_store_ventils,
				 &iter,
				 4, &couleur_fond[couleur_en_cours],
				 -1 );
	couleur_en_cours = 1 - couleur_en_cours;

    }
    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( list_store_ventils ),
				       &iter ));
}
/***********************************************************************************************************/


/***********************************************************************************************************/
void selectionne_ligne_ventilation ( gint nouvelle_ligne )
{
    GtkTreeIter iter;
    GdkColor *couleur;

    if ( DEBUG )
	printf ( "selectionne_ligne ventilation\n" );

    /*     si on est déjà dessus, on se barre */

    if ( nouvelle_ligne == ligne_selectionnee_ventilation )
	return;

    /*   vire l'ancienne sélection : consiste à remettre la couleur d'origine du background */

    if ( ligne_selectionnee_ventilation != -1
	 &&
	 gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_ventilations ))),
					       &iter,
					       itoa ( ligne_selectionnee_ventilation )))
    {
	/* 	iter est maintenant positionné sur la 1ère ligne de l'opé à désélectionner */

	gtk_tree_model_get ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_ventilations ))),
			     &iter,
			     5, &couleur,
			     -1 );
	gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_ventilations ))),
			     &iter,
			     4,couleur,
			     -1 );
	gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_ventilations ))),
			     &iter,
			     5, NULL,
			     -1 );

    }

    ligne_selectionnee_ventilation = nouvelle_ligne;

    if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_ventilations ))),
					       &iter,
					       itoa ( ligne_selectionnee_ventilation )))
    {

	/* 	iter est maintenant positionné sur la 1ère ligne de l'opé à sélectionner */

	gtk_tree_model_get ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_ventilations ))),
			     &iter,
			     4, &couleur,
			     -1 );
	gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_ventilations ))),
			     &iter,
			     4, &couleur_selection,
			     -1 );
	gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_ventilations ))),
			     &iter,
			     5, couleur,
			     -1 );
    }

    /*     on déplace le scrolling de la liste si nécessaire pour afficher la sélection */

    ajuste_scrolling_liste_ventilations_a_selection ();
}
/***********************************************************************************************************/


/* ************************************************************************** */
/* Fonction valider_ventilation                                               */
/* appelée par appui du bouton valider                                        */
/* ************************************************************************** */
void valider_ventilation ( void )
{
    /* Cette fonction est toute simple car la liste des structures des
       ventilations a été mise à jour au fur et à mesure et toujours associée
       au formulaire des opérations. Donc, il faut juste réafficher ce qu'il faut
       et return. C'est la validation réelle de l'opération qui créera/supprimera
       toutes les opérations */

    /* Si par contre cette liste est null, on met -1 sur le formulaire pour
       montrer qu'on est passé par là et qu'on veut une liste nulle */

    /* On associe l'adresse de la nouvelle liste des ventilation au formulaire,
       met -1 si la liste est vide */

    if ( !gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				"liste_adr_ventilation" ))
	gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			      "liste_adr_ventilation",
			      GINT_TO_POINTER ( -1 ) );
    /*
       if ( gtk_object_get_data ( GTK_OBJECT ( formulaire ), "liste_adr_ventilation" ) == GINT_TO_POINTER ( -1 ) )
       dialogue("Liste nulle");
       */
    if ( fabs ( montant_operation_ventilee - somme_ventilee ) >= 0.000001 )
    {
	if ( ! question_yes_no_hint ( _("Incomplete breakdown"),
				      _("Transaction amount isn't fully broken down.\nProceed anyway?") ))
	    return;
    }

    quitter_ventilation ();

    if ( enregistre_ope_au_retour )
	fin_edition();
}
/* ************************************************************************** */


/* ************************************************************************** */
/* Fonction annuler_ventilation                                               */
/* appelée par appui du bouton annuler                                        */
/* ************************************************************************** */
void annuler_ventilation ( void )
{
    /* Cette fonction remet la liste des structures de ventilation par défaut
       en recherchant les opérations de ventilation dans la liste des opérations
       puis appelle valider ventilation */
    /*     si des opés de ventils avaient été mises par complétion du tiers, en faisant */
    /* 	annuler elles vont disparaitre car elles ne sont pas encore enregistrées */
    /* 	dans ce cas on les ajoutes à la nouvelle liste créé */

    GSList *nouvelle_liste;
    GSList *ancienne_liste;
    GSList *liste_tmp;


    ancienne_liste = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
					   "liste_adr_ventilation" );
    nouvelle_liste = creation_liste_ope_de_ventil ( gtk_object_get_data ( GTK_OBJECT ( formulaire ),
									       "adr_struct_ope" ));

    if ( ancienne_liste != GINT_TO_POINTER (-1))
    {
	liste_tmp = ancienne_liste;

	while ( liste_tmp )
	{
	    struct struct_ope_ventil *ventil;

	    ventil = liste_tmp -> data;

	    if ( ventil -> par_completion )
		nouvelle_liste = g_slist_append ( nouvelle_liste,
						  ventil );
	    liste_tmp = liste_tmp -> next;
	}

	g_slist_free ( ancienne_liste );
    }
    else
	nouvelle_liste = GINT_TO_POINTER (-1);


    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "liste_adr_ventilation",
			  nouvelle_liste );

    quitter_ventilation ();
}
/* ************************************************************************** */

/* ************************************************************************** */
/* Fonction quitter_ventilation                                               */
/* appelée valider_ventilation et quitter_ventilation                         */
/* ************************************************************************** */
void quitter_ventilation ( void )
{
    /* Cette fonction remet la liste des structures de ventilation par défaut
       en recherchant les opérations de ventilation dans la liste des opérations
       puis appelle valider ventilation */

    /*     on réaffiche la tree_view de la liste d'opés en cours */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    gtk_widget_hide ( scrolled_window_liste_ventilations );
    gtk_widget_show ( SCROLLED_WINDOW_LISTE_OPERATIONS );

    gtk_widget_show ( barre_outils );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ), 0 );
    gtk_widget_show ( formulaire );

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_formulaire ), 0 );

    gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY) ) -> entry );

    if ( !montant_operation_ventilee )
    {
	if ( somme_ventilee < 0 )
	{
	    entree_prend_focus ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT) );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT) ),
				 g_strdup_printf ( "%4.2f", fabs ( somme_ventilee ) ));
	}
	else
	{
	    entree_prend_focus ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT) );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT) ),
				 g_strdup_printf ( "%4.2f", somme_ventilee ));
	}
    }
}
/* ************************************************************************** */

/***********************************************************************************************************/
/* Cette fonction prend une opé ventilée en argument et crée la liste des opés de ventil */
/* qui correspondent avec des struct struct_ope_ventil */
/* renvoie cette liste */
/***********************************************************************************************************/

GSList *creation_liste_ope_de_ventil ( struct structure_operation *operation )
{
    GSList *liste_ventil;
    GSList *liste_operations;
    GSList *liste_tmp;

    liste_ventil = NULL;
    liste_operations = NULL;

    /* si c'est une nouvelle opé, il n'y a aucun opé de ventil associée */

    if ( !operation )
	return ( NULL );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

    liste_tmp = LISTE_OPERATIONS;

    while ( liste_tmp )
    {
	struct structure_operation *operation_2;

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

	    if ( ope_ventil -> no_piece_comptable )
		ope_ventil -> no_piece_comptable = g_strdup ( operation_2 -> no_piece_comptable );

	    ope_ventil -> relation_no_operation = operation_2 -> relation_no_operation;
	    ope_ventil -> relation_no_compte = operation_2 -> relation_no_compte;
	    ope_ventil -> pointe = operation_2 -> pointe;

	    /* si c'est un virement, on va rechercher le type de l'autre opération */

	    if ( ope_ventil -> relation_no_operation )
	    {
		struct structure_operation *operation_associee;

		operation_associee = operation_par_no ( ope_ventil -> relation_no_operation,
							ope_ventil -> relation_no_compte );

		if ( operation_associee )
		    ope_ventil -> no_type_associe = operation_associee -> type_ope;
	    }

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

void validation_ope_de_ventilation ( struct structure_operation *operation )
{
    GSList *liste_struct_ventilations;

    /* récupération de la liste de ventilations */

    liste_struct_ventilations = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
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
	/* et si c'était un virement, l'autre va être automatiquement supprimée */

	if ( ope_ventil -> supprime )
	{
	    /* petite protection quand même, normalement le texte ne devrait jamais apparaitre */

	    if ( !ope_ventil -> no_operation )
		dialogue_warning ( _("A breakdown line is to be deleted though it is not yet registered."));
	    else
		supprime_operation  ( operation_par_no ( ope_ventil -> no_operation,
							 compte_courant ));
	}
	else
	{
	    /* l'opération ne doit pas être supprimée, c'est qu'elle doit être créée ou modifiée */
	    /* 	  on n'a pas à s'embêter avec des changements de virements ou autres trucs bizarres, dans */
	    /* ce cas il y aura eu une suppression puis une nouvelle opération */

	    if ( ope_ventil -> no_operation )
	    {
		/* c'est une modif d'opération */

		struct structure_operation *ope_modifiee;

		ope_modifiee = operation_par_no ( ope_ventil -> no_operation,
						  compte_courant );

		if ( ope_modifiee )
		{
		    /* on récupère d'abord les modifs de l'opé de ventil */

		    ope_modifiee -> montant = ope_ventil -> montant;
		    ope_modifiee -> categorie = ope_ventil -> categorie;
		    ope_modifiee -> sous_categorie = ope_ventil -> sous_categorie;

		    if ( ope_ventil -> notes )
			ope_modifiee -> notes = g_strdup ( ope_ventil -> notes );

		    ope_modifiee -> no_exercice = ope_ventil -> no_exercice;

		    ope_modifiee -> imputation = ope_ventil -> imputation;
		    ope_modifiee -> sous_imputation = ope_ventil -> sous_imputation;

		    if ( ope_ventil -> no_piece_comptable )
			ope_modifiee -> no_piece_comptable = g_strdup ( ope_ventil -> no_piece_comptable );

		    /* on récupère ensuite les modifs de la ventilation */

		    ope_modifiee -> jour = operation -> jour;
		    ope_modifiee -> mois = operation -> mois;
		    ope_modifiee -> annee = operation -> annee;

		    ope_modifiee -> date = g_date_new_dmy ( operation -> jour,
							    operation -> mois,
							    operation -> annee );

		    ope_modifiee -> jour_bancaire = operation -> jour_bancaire;
		    ope_modifiee -> mois_bancaire = operation -> mois_bancaire;
		    ope_modifiee -> annee_bancaire = operation -> annee_bancaire;

		    if ( operation -> jour_bancaire )
			ope_modifiee -> date_bancaire = g_date_new_dmy ( operation -> jour_bancaire,
									 operation -> mois_bancaire,
									 operation -> annee_bancaire );

		    ope_modifiee -> no_compte = operation -> no_compte;
		    ope_modifiee -> devise = operation -> devise;
		    ope_modifiee -> une_devise_compte_egale_x_devise_ope = operation -> une_devise_compte_egale_x_devise_ope;
		    ope_modifiee -> taux_change = operation -> taux_change;
		    ope_modifiee -> frais_change = operation -> frais_change;
		    ope_modifiee -> tiers = operation -> tiers;
		    ope_modifiee -> type_ope = operation -> type_ope;
		    ope_modifiee -> pointe = operation -> pointe;
		    ope_modifiee -> auto_man = operation -> auto_man;
		    ope_modifiee -> no_rapprochement = operation -> no_rapprochement;

		    /* théoriquement, cette ligne n'est pas nécessaire vu que c'est une modif d'opé de ventil */

		    ope_modifiee -> no_operation_ventilee_associee = operation -> no_operation;

		    /* si cette opé de ventil est un virement, on met à jour la contre opération */

		    if ( ope_ventil -> relation_no_operation )
		    {
			/*  soit c'était un virement, et on modifie l'opé associée */
			/* soit c'est un nouveau virement, et on crée l'opé associée */

			p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ope_ventil -> relation_no_compte;

			MISE_A_JOUR = 1;

			if ( ope_ventil -> relation_no_operation != -1 )
			{
			    /* c'était déjà un virement vers ce compte */

			    struct structure_operation *ope_modifiee_2;

			    ope_modifiee_2 = operation_par_no ( ope_ventil -> relation_no_operation,
								ope_ventil -> relation_no_compte );

			    if ( ope_modifiee_2 )
			    {
				/* on commence par mettre le type choisi */

				ope_modifiee_2 -> type_ope = ope_ventil -> no_type_associe;

				ope_modifiee_2 -> montant = -ope_modifiee -> montant;
				ope_modifiee_2 -> categorie = ope_modifiee -> categorie;
				ope_modifiee_2 -> sous_categorie = ope_modifiee -> sous_categorie;

				if ( ope_modifiee -> notes )
				    ope_modifiee_2 -> notes = g_strdup ( ope_modifiee -> notes );

				ope_modifiee_2 -> imputation = ope_modifiee -> imputation;
				ope_modifiee_2 -> sous_imputation = ope_modifiee -> sous_imputation;

				if ( ope_modifiee -> no_piece_comptable )
				    ope_modifiee_2 -> no_piece_comptable = g_strdup ( ope_modifiee -> no_piece_comptable );

				ope_modifiee_2 -> jour = ope_modifiee -> jour;
				ope_modifiee_2 -> mois = ope_modifiee -> mois;
				ope_modifiee_2 -> annee = ope_modifiee -> annee;

				ope_modifiee_2 -> date = g_date_new_dmy ( operation -> jour,
									  operation -> mois,
									  operation -> annee );

				ope_modifiee_2 -> jour_bancaire = ope_modifiee -> jour_bancaire;
				ope_modifiee_2 -> mois_bancaire = ope_modifiee -> mois_bancaire;
				ope_modifiee_2 -> annee_bancaire = ope_modifiee -> annee_bancaire;

				if ( operation -> jour_bancaire )
				    ope_modifiee_2 -> date_bancaire = g_date_new_dmy ( operation -> jour_bancaire,
										       operation -> mois_bancaire,
										       operation -> annee_bancaire );

				if ( ope_modifiee_2 -> devise != ope_modifiee -> devise)
				{
				    struct struct_devise *devise_compte_1;
				    struct struct_devise *devise_compte_2;

				    ope_modifiee_2 -> devise = ope_modifiee -> devise;

				    ope_modifiee_2 -> une_devise_compte_egale_x_devise_ope = ope_modifiee -> une_devise_compte_egale_x_devise_ope;

				    devise_compte_1 = devise_par_no ( ope_modifiee -> devise );
				    devise_compte_2 = devise_par_no ( DEVISE );

				    if ( !( ope_modifiee -> devise == DEVISE
					    ||
					    ( devise_compte_2 -> passage_euro && !strcmp ( devise_compte_1 -> nom_devise, _("Euro") ))
					    ||
					    ( !strcmp ( devise_compte_2 -> nom_devise, _("Euro") ) && devise_compte_1 -> passage_euro )))
				    {
					/* c'est une devise étrangère, on demande le taux de change et les frais de change */

					demande_taux_de_change ( devise_compte_2,
								 devise_compte_1,
								 1,
								 (gdouble ) 0,
								 (gdouble ) 0,
								 FALSE );

					ope_modifiee_2 -> taux_change = taux_de_change[0];
					ope_modifiee_2 -> frais_change = taux_de_change[1];

					if ( ope_modifiee_2 -> taux_change < 0 )
					{
					    ope_modifiee_2 -> taux_change = -ope_modifiee_2 -> taux_change;
					    ope_modifiee_2 -> une_devise_compte_egale_x_devise_ope = 1;
					}
				    }
				}
				/* 			      ope_modifiee_2 -> une_devise_compte_egale_x_devise_ope = ope_modifiee -> une_devise_compte_egale_x_devise_ope; */
				/* 			      ope_modifiee_2 -> taux_change = ope_modifiee -> taux_change; */
				/* 			      ope_modifiee_2 -> frais_change = ope_modifiee -> frais_change; */
				ope_modifiee_2 -> tiers = ope_modifiee -> tiers;
				ope_modifiee_2 -> auto_man = ope_modifiee -> auto_man;
				ope_modifiee_2 -> no_exercice = ope_modifiee -> no_exercice;
			    }
			}
			else
			{
			    /* c'est un nouveau virement, on doit créer l'opé associée */

			    struct structure_operation *nouvelle_ope_2;

			    nouvelle_ope_2 = calloc ( 1,
						      sizeof ( struct structure_operation ));

			    /* on commence par mettre le type choisi */

			    nouvelle_ope_2 -> type_ope = ope_ventil -> no_type_associe;
			    nouvelle_ope_2 -> no_compte = ope_ventil -> relation_no_compte;

			    nouvelle_ope_2 -> montant = -ope_modifiee ->  montant;
			    nouvelle_ope_2 -> categorie = ope_modifiee ->  categorie;
			    nouvelle_ope_2 -> sous_categorie = ope_modifiee ->  sous_categorie;

			    if ( ope_modifiee -> notes )
				nouvelle_ope_2 -> notes = g_strdup ( ope_modifiee ->  notes );

			    nouvelle_ope_2 -> imputation = ope_modifiee ->  imputation;
			    nouvelle_ope_2 -> sous_imputation = ope_modifiee ->  sous_imputation;

			    if ( ope_modifiee -> no_piece_comptable )
				nouvelle_ope_2 -> no_piece_comptable = g_strdup ( ope_modifiee ->  no_piece_comptable );

			    nouvelle_ope_2 -> jour = ope_modifiee ->  jour;
			    nouvelle_ope_2 -> mois = ope_modifiee ->  mois;
			    nouvelle_ope_2 -> annee = ope_modifiee ->  annee;

			    nouvelle_ope_2 -> date = g_date_new_dmy ( operation -> jour,
								      operation -> mois,
								      operation -> annee );

			    nouvelle_ope_2 -> jour_bancaire = ope_modifiee ->  jour_bancaire;
			    nouvelle_ope_2 -> mois_bancaire = ope_modifiee ->  mois_bancaire;
			    nouvelle_ope_2 -> annee_bancaire = ope_modifiee ->  annee_bancaire;

			    if ( operation -> jour_bancaire )
				nouvelle_ope_2 -> date_bancaire = g_date_new_dmy ( operation -> jour_bancaire,
										   operation -> mois_bancaire,
										   operation -> annee_bancaire );

			    nouvelle_ope_2 -> devise = ope_modifiee ->  devise;
			    nouvelle_ope_2 -> une_devise_compte_egale_x_devise_ope = ope_modifiee ->  une_devise_compte_egale_x_devise_ope;
			    nouvelle_ope_2 -> taux_change = ope_modifiee ->  taux_change;
			    nouvelle_ope_2 -> frais_change = ope_modifiee ->  frais_change;
			    nouvelle_ope_2 -> tiers = ope_modifiee ->  tiers;
			    nouvelle_ope_2 -> auto_man = ope_modifiee ->  auto_man;
			    nouvelle_ope_2 -> no_exercice = ope_modifiee ->  no_exercice;

			    ajout_operation ( nouvelle_ope_2 );

			    /* on met les relations */

			    ope_modifiee -> relation_no_operation = nouvelle_ope_2 -> no_operation;
			    ope_modifiee -> relation_no_compte = nouvelle_ope_2 -> no_compte;
			    nouvelle_ope_2 -> relation_no_operation = ope_modifiee ->  no_operation;
			    nouvelle_ope_2 -> relation_no_compte = ope_modifiee ->  no_compte;

			    /* 		    on réaffiche la contre opération pour faire apparaitre la catég */

			    remplit_ligne_operation ( nouvelle_ope_2,
						      NULL );
			}
		    }
		}
	    }
	    else
	    {
		/* c'est une nouvelle opération */
		/*  on la crée, l'ajoute et si c'est un virement on crée la contre opération */

		struct structure_operation *nouvelle_ope;

		nouvelle_ope = calloc ( 1,
					sizeof ( struct structure_operation ));

		/* on récupère d'abord les modifs de l'opé de ventil */

		nouvelle_ope -> montant = ope_ventil -> montant;
		nouvelle_ope -> categorie = ope_ventil -> categorie;
		nouvelle_ope -> sous_categorie = ope_ventil -> sous_categorie;

		if ( ope_ventil -> notes )
		    nouvelle_ope -> notes = g_strdup ( ope_ventil -> notes );

		nouvelle_ope -> imputation = ope_ventil -> imputation;
		nouvelle_ope -> sous_imputation = ope_ventil -> sous_imputation;

		if ( ope_ventil -> no_piece_comptable )
		    nouvelle_ope -> no_piece_comptable = g_strdup ( ope_ventil -> no_piece_comptable );

		nouvelle_ope -> no_exercice = ope_ventil -> no_exercice;

		/* on récupère ensuite les modifs de la ventilation */

		nouvelle_ope -> jour = operation -> jour;
		nouvelle_ope -> mois = operation -> mois;
		nouvelle_ope -> annee = operation -> annee;

		nouvelle_ope -> date = g_date_new_dmy ( operation -> jour,
							operation -> mois,
							operation -> annee );

		nouvelle_ope -> jour_bancaire = operation -> jour_bancaire;
		nouvelle_ope -> mois_bancaire = operation -> mois_bancaire;
		nouvelle_ope -> annee_bancaire = operation -> annee_bancaire;

		if ( operation -> jour_bancaire )
		    nouvelle_ope -> date_bancaire = g_date_new_dmy ( operation -> jour_bancaire,
								     operation -> mois_bancaire,
								     operation -> annee_bancaire );

		nouvelle_ope -> no_compte = operation -> no_compte;
		nouvelle_ope -> devise = operation -> devise;
		nouvelle_ope -> une_devise_compte_egale_x_devise_ope = operation -> une_devise_compte_egale_x_devise_ope;
		nouvelle_ope -> taux_change = operation -> taux_change;
		nouvelle_ope -> frais_change = operation -> frais_change;
		nouvelle_ope -> tiers = operation -> tiers;
		nouvelle_ope -> type_ope = operation -> type_ope;
		nouvelle_ope -> pointe = operation -> pointe;
		nouvelle_ope -> auto_man = operation -> auto_man;
		nouvelle_ope -> no_rapprochement = operation -> no_rapprochement;

		nouvelle_ope -> no_operation_ventilee_associee = operation -> no_operation;

		/* on ajoute cette opé à la liste */

		ajout_operation ( nouvelle_ope );

		/* si cette opé de ventil est un virement, on crée la contre opération */

		if ( ope_ventil -> relation_no_operation )
		{
		    struct structure_operation *nouvelle_ope_2;
		    struct struct_devise *devise_compte_1;
		    struct struct_devise *devise_compte_2;

		    nouvelle_ope_2 = calloc ( 1,
					      sizeof ( struct structure_operation ));

		    /* on commence par mettre le type choisi */

		    nouvelle_ope_2 -> type_ope = ope_ventil -> no_type_associe;
		    nouvelle_ope_2 -> no_compte = ope_ventil -> relation_no_compte;

		    nouvelle_ope_2 -> montant = -nouvelle_ope -> montant;
		    nouvelle_ope_2 -> categorie = nouvelle_ope -> categorie;
		    nouvelle_ope_2 -> sous_categorie = nouvelle_ope -> sous_categorie;

		    if ( nouvelle_ope -> notes )
			nouvelle_ope_2 -> notes = g_strdup ( nouvelle_ope -> notes );

		    nouvelle_ope_2 -> imputation = nouvelle_ope -> imputation;
		    nouvelle_ope_2 -> sous_imputation = nouvelle_ope -> sous_imputation;

		    if ( nouvelle_ope -> no_piece_comptable )
			nouvelle_ope_2 -> no_piece_comptable = g_strdup ( nouvelle_ope -> no_piece_comptable );

		    nouvelle_ope_2 -> jour = nouvelle_ope -> jour;
		    nouvelle_ope_2 -> mois = nouvelle_ope -> mois;
		    nouvelle_ope_2 -> annee = nouvelle_ope -> annee;

		    nouvelle_ope_2 -> date = g_date_new_dmy ( operation -> jour,
							      operation -> mois,
							      operation -> annee );

		    nouvelle_ope_2 -> jour_bancaire = nouvelle_ope -> jour_bancaire;
		    nouvelle_ope_2 -> mois_bancaire = nouvelle_ope -> mois_bancaire;
		    nouvelle_ope_2 -> annee_bancaire = nouvelle_ope -> annee_bancaire;
		    nouvelle_ope_2 -> devise = nouvelle_ope -> devise;

		    if ( operation -> jour_bancaire )
			nouvelle_ope_2 -> date_bancaire = g_date_new_dmy ( operation -> jour_bancaire,
									   operation -> mois_bancaire,
									   operation -> annee_bancaire );

		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + nouvelle_ope_2 -> no_compte;

		    nouvelle_ope_2 -> une_devise_compte_egale_x_devise_ope = nouvelle_ope -> une_devise_compte_egale_x_devise_ope;

		    devise_compte_1 = devise_par_no ( nouvelle_ope -> devise );
		    devise_compte_2 = devise_par_no ( DEVISE );

		    if ( !( nouvelle_ope -> devise == DEVISE
			    ||
			    ( devise_compte_2 -> passage_euro && !strcmp ( devise_compte_1 -> nom_devise, _("Euro") ))
			    ||
			    ( !strcmp ( devise_compte_2 -> nom_devise, _("Euro") ) && devise_compte_1 -> passage_euro )))
		    {
			/* c'est une devise étrangère, on demande le taux de change et les frais de change */

			demande_taux_de_change ( devise_compte_2, devise_compte_1, 1,
						 (gdouble ) 0, (gdouble ) 0, FALSE );

			nouvelle_ope_2 -> taux_change = taux_de_change[0];
			nouvelle_ope_2 -> frais_change = taux_de_change[1];

			if ( nouvelle_ope_2 -> taux_change < 0 )
			{
			    nouvelle_ope_2 -> taux_change = -nouvelle_ope_2 -> taux_change;
			    nouvelle_ope_2 -> une_devise_compte_egale_x_devise_ope = 1;
			}
		    }

		    /* 		  nouvelle_ope_2 -> taux_change = nouvelle_ope -> taux_change; */
		    /* 		  nouvelle_ope_2 -> frais_change = nouvelle_ope -> frais_change; */

		    nouvelle_ope_2 -> tiers = nouvelle_ope -> tiers;
		    nouvelle_ope_2 -> auto_man = nouvelle_ope -> auto_man;
		    nouvelle_ope_2 -> no_exercice = nouvelle_ope -> no_exercice;

		    ajout_operation ( nouvelle_ope_2 );

		    /* on met les relations */

		    nouvelle_ope -> relation_no_operation = nouvelle_ope_2 -> no_operation;
		    nouvelle_ope -> relation_no_compte = nouvelle_ope_2 -> no_compte;
		    nouvelle_ope_2 -> relation_no_operation = nouvelle_ope -> no_operation;
		    nouvelle_ope_2 -> relation_no_compte = nouvelle_ope -> no_compte;

		    /* 		    on réaffiche la contre opération pour faire apparaitre la catég */

		    remplit_ligne_operation ( nouvelle_ope_2,
					      NULL );
		}
	    }
	}
	liste_struct_ventilations = liste_struct_ventilations -> next;
    }
}
/***********************************************************************************************************/





/******************************************************************************/
/* Fonction changement_taille_liste_ope					      */
/* appelée dès que la taille de la clist a changé			      */
/* pour mettre la taille des différentes colonnes			      */
/******************************************************************************/
gboolean changement_taille_liste_ope_ventilation ( GtkWidget *tree_view,
						   GtkAllocation *allocation )
{
    gint col0, col1, col2;
    gint categorie;
    gint description;
    gint montant;


    /*     pour éviter que le système ne s'emballe... */

    if ( allocation -> width
	 ==
	 ancienne_largeur_ventilation )
	return FALSE;

    ancienne_largeur_ventilation = allocation -> width;

    categorie = ( 40 * ancienne_largeur_ventilation) / 100;
    description = ( 35 * ancienne_largeur_ventilation) / 100;
    montant = ( 10 * ancienne_largeur_ventilation) / 100;

    gtk_tree_view_column_set_fixed_width ( colonnes_liste_ventils[0],
					   categorie );
    gtk_tree_view_column_set_fixed_width ( colonnes_liste_ventils[1],
					   description );
    gtk_tree_view_column_set_fixed_width ( colonnes_liste_ventils[2],
					   montant );


    /* met les entrées du formulaire à la même taille */

    col0 = ancienne_largeur_ventilation * 32  / 100;
    col1 = ancienne_largeur_ventilation * 32  / 100;
    col2 = ancienne_largeur_ventilation * 15  / 100;

    /* 1ère ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CATEGORY] ),
			   col0,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_NOTES] ),
			   col1,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_DEBIT] ),
			   col2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CREDIT] ),
			   col2,
			   FALSE );

    /* 2ème ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] ),
			   col0,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA] ),
			   col1 / 2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE] ),
			   col1/2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER] ),
			   col2,
			   FALSE );

    update_ecran ();

    return FALSE;
}
/******************************************************************************/






/******************************************************************************/
/* cette fonction affiche les traits verticaux et horizontaux sur la liste des opés */
/******************************************************************************/
void affichage_traits_liste_ventilation ( void )
{
    GdkWindow *fenetre;
    gint i;
    gint largeur, hauteur;
    gint x, y;
    GtkAdjustment *adjustment;
    gint derniere_ligne;

    /*  FIXME   sachant qu'on appelle ça à chaque expose-event, cad très souvent ( dès que la souris passe dessus ), */
    /*     ça peut ralentir bcp... à vérifier  */

    fenetre = gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view_liste_ventilations ));
    gdk_drawable_get_size ( GDK_DRAWABLE ( fenetre ),
			    &largeur,
			    &hauteur );

    if ( !gc_separateur_operation )
	gc_separateur_operation = gdk_gc_new ( GDK_DRAWABLE ( fenetre ));

    /*     si la hauteur des lignes n'est pas encore calculée, on le fait ici */

    if ( !hauteur_ligne_liste_opes )
	hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( tree_view_liste_ventilations );

    /*     on commence par calculer la dernière ligne en pixel correspondant à la dernière opé de la liste */
    /* 	pour éviter de dessiner les traits en dessous */

    derniere_ligne = hauteur_ligne_liste_opes * GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_liste_ventilations ))) -> length;
    hauteur = MIN ( derniere_ligne,
		    hauteur );

    /*     le plus facile en premier... les lignes verticales */
    /*     dépend de si on est en train de ventiler ou non */
    /*     on en profite pour ajuster nb_ligne_ope_tree_view */

    x=0;

    for ( i=0 ; i<3 ; i++ )
    {
	x = x + gtk_tree_view_column_get_width ( GTK_TREE_VIEW_COLUMN ( colonnes_liste_ventils[i] ));
	gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			gc_separateur_operation,
			x, 0,
			x, hauteur );
    }

    /*     les lignes horizontales : il faut calculer la position y de chaque changement d'opé à l'écran */
    /*     on calcule la position y de la 1ère ligne à afficher */

    adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( tree_view_liste_ventilations ));

    y = hauteur_ligne_liste_opes * ( ceil ( adjustment->value / hauteur_ligne_liste_opes)) - adjustment -> value;

    do
    {
	gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			gc_separateur_operation,
			0, y, 
			largeur, y );
	y = y + hauteur_ligne_liste_opes;
    }
    while ( y < ( adjustment -> value + adjustment -> page_size )
	    &&
	    y <= derniere_ligne );
}
/******************************************************************************/




/******************************************************************************/
void ajuste_scrolling_liste_ventilations_a_selection ( void )
{
    GtkAdjustment *v_adjustment;
    gint y_ligne;

    /*     si on n'a pas encore récupéré la hauteur des lignes, on va le faire ici */

    if ( !hauteur_ligne_liste_opes )
	hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( tree_view_liste_ventilations );

    v_adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( tree_view_liste_ventilations ));

    y_ligne = ligne_selectionnee_ventilation * hauteur_ligne_liste_opes;

    /*     si l'opé est trop haute, on la rentre et la met en haut */

    if ( y_ligne < v_adjustment -> value )
	gtk_adjustment_set_value ( GTK_ADJUSTMENT ( v_adjustment ),
				   y_ligne );
    else
	if ( (y_ligne + hauteur_ligne_liste_opes ) > ( v_adjustment -> value + v_adjustment -> page_size ))
	    gtk_adjustment_set_value ( GTK_ADJUSTMENT ( v_adjustment ),
				       y_ligne + hauteur_ligne_liste_opes - v_adjustment -> page_size );

}
/******************************************************************************/



