/* ************************************************************************** */
/* fichier qui s'occupe de l'onglet de gestion des imputations                */
/* 			imputation_budgetaire.c                               */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)		      */
/*			2004 Alain Portal (dionysos@grisbi.org) 	      */
/*			http://www.grisbi.org   			      */
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
#include "echeancier_formulaire_constants.h"
#include "ventilation_constants.h"
#include "operations_formulaire_constants.h"



/*START_INCLUDE*/
#include "imputation_budgetaire.h"
#include "utils_devises.h"
#include "operations_comptes.h"
#include "tiers_onglet.h"
#include "fichiers_io.h"
#include "barre_outils.h"
#include "operations_liste.h"
#include "dialog.h"
#include "gtk_combofix.h"
#include "utils_ib.h"
#include "utils_str.h"
#include "traitement_variables.h"
#include "utils_operations.h"
#include "search_glist.h"
#include "etats_config.h"
#include "affichage_formulaire.h"
#include "operations_formulaire.h"
#include "utils_file_selection.h"
#include "utils_files.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void appui_sur_ajout_imputation ( void );
static void appui_sur_ajout_sous_imputation ( void );
static void calcule_total_montant_imputation ( void );
static gchar *calcule_total_montant_imputation_par_compte ( gint imputation, gint sous_imputation, 
						     gint no_compte );
static void clique_sur_annuler_imputation ( void );
static void clique_sur_modifier_imputation ( void );
static void enleve_selection_ligne_imputation ( void );
static gboolean expand_selected_ib (  );
static void exporter_ib ( void );
static void fusion_categories_imputation ( void );
static void importer_ib ( void );
static gboolean keypress_ib ( GtkWidget *widget, GdkEventKey *ev, gint *no_origine );
static void modification_du_texte_imputation ( void );
static void ouverture_node_imputation ( GtkWidget *arbre,
				 GtkCTreeNode *node,
				 gpointer null );
static void selection_ligne_imputation ( GtkCTree *arbre,
				  GtkCTreeNode *noeud,
				  gint colonne,
				  gpointer null );
static void supprimer_imputation ( void );
static void supprimer_sous_imputation ( void );
static gboolean verifie_double_click_imputation ( GtkWidget *liste, GdkEventButton *ev,
					   gpointer null );
/*END_STATIC*/



GtkWidget *arbre_imputation;
GtkWidget *entree_nom_imputation;
GtkWidget *bouton_imputation_debit;
GtkWidget *bouton_imputation_credit;
GtkWidget *bouton_modif_imputation_modifier;
GtkWidget *bouton_modif_imputation_annuler;
GtkWidget *bouton_supprimer_imputation;
GtkWidget *bouton_ajouter_imputation;
GtkWidget *bouton_ajouter_sous_imputation;

GSList *liste_struct_imputation;    /* liste des structures de catég */
GSList *liste_imputations_combofix;        /*  liste des noms des imputation et sous imputation pour le combofix */
gint nb_enregistrements_imputations;        /* nombre de catégories */
gint no_derniere_imputation;
gfloat *tab_montant_imputation;             /* buffer */
gfloat **tab_montant_sous_imputation;            /* buffer */
gint *nb_ecritures_par_imputation;           /* buffer */
gint **nb_ecritures_par_sous_imputation;           /* buffer */
gint mise_a_jour_combofix_imputation_necessaire;



/*START_EXTERN*/
extern gint compte_courant;
extern gchar *dernier_chemin_de_travail;
extern struct struct_devise *devise_compte;
extern struct struct_devise *devise_operation;
extern struct struct_etat *etat_courant;
extern GtkWidget *formulaire;
extern GSList *liste_struct_categories;
extern GSList *liste_struct_echeances;
extern GdkBitmap *masque_ferme;
extern GdkBitmap *masque_ouvre;
extern gint modif_imputation;
extern gint nb_comptes;
extern gint nb_ecritures_par_comptes;
extern gint no_derniere_operation;
extern gint no_devise_totaux_tiers;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern GdkPixmap *pixmap_ferme;
extern GdkPixmap *pixmap_ouvre;
extern GtkTreeSelection * selection;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_TOTAL_WIDGET];
extern GtkWidget *window;
/*END_EXTERN*/





/* **************************************************************************************************** */
/* Fonction onglet_imputations : */
/* crée et renvoie le widget contenu dans l'onglet */
/* **************************************************************************************************** */

GtkWidget *onglet_imputations ( void )
{
    GtkWidget *onglet;
    GtkWidget *scroll_window;
    gchar *titres[] =
    {
	_("Budgetary lines list"),
	_("Amount per budget line"),
	_("Amount per sub-budget line"),
	_("Amount per account")
    };
    GtkWidget *vbox;
    GtkWidget *frame;
    GtkWidget *vbox_frame;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *separateur;
    GtkWidget *bouton;


    /* création de la fenêtre qui sera renvoyée */

    onglet = gtk_hbox_new ( FALSE,
			    5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				     10 );
    gtk_widget_show ( onglet );



    /* mise en place de la partie de gauche */

    /*   création de la frame de gauche */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
				GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show (frame );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     15 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );


    frame = gtk_frame_new ( SPACIFY(COLON(_("Information"))) );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    vbox_frame = gtk_vbox_new ( FALSE,
				5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_frame ),
				     5 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox_frame );
    gtk_widget_show ( vbox_frame );

    entree_nom_imputation = gtk_entry_new ();
    gtk_widget_set_sensitive ( entree_nom_imputation,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( entree_nom_imputation ),
			 "changed",
			 GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 entree_nom_imputation,
			 FALSE,
			 FALSE,
			 10 );
    gtk_widget_show ( entree_nom_imputation );


    /* création des radio bouton débit/crédit */

    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );


    label = gtk_label_new ( COLON(_("Sorting")) );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 label,
			 FALSE,
			 FALSE,
			 10 );
    gtk_widget_show ( label );

    bouton_imputation_debit = gtk_radio_button_new_with_label ( NULL,
								_("Debit") );
    gtk_widget_set_sensitive ( bouton_imputation_debit,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_imputation_debit ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 bouton_imputation_debit,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_imputation_debit );


    bouton_imputation_credit = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( bouton_imputation_debit ),
									     _("Credit") );
    gtk_widget_set_sensitive ( bouton_imputation_credit,
			       FALSE );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 bouton_imputation_credit,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_imputation_credit );


    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 separateur,
			 FALSE,
			 FALSE,
			 10 );
    gtk_widget_show ( separateur );




    /*   création des boutons modifier et annuler */

    hbox = gtk_hbox_new ( TRUE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    /* FIXME */
    bouton_modif_imputation_modifier = gtk_button_new_from_stock (GTK_STOCK_APPLY);
    /*   bouton_modif_imputation_modifier = GTK_STOCK ( GTK_STOCK_APPLY ); */
    gtk_button_set_relief ( GTK_BUTTON ( bouton_modif_imputation_modifier ),
			    GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_modifier,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_modif_imputation_modifier ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( clique_sur_modifier_imputation ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_modif_imputation_modifier,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_modif_imputation_modifier );

    /* FIXME */
    bouton_modif_imputation_annuler = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    /*   bouton_modif_imputation_annuler = GTK_STOCK ( GTK_STOCK_CANCEL ); */
    gtk_button_set_relief ( GTK_BUTTON ( bouton_modif_imputation_annuler ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_modif_imputation_annuler ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( clique_sur_annuler_imputation ),
			 NULL );
    gtk_widget_set_sensitive ( bouton_modif_imputation_annuler,
			       FALSE );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_modif_imputation_annuler,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_modif_imputation_annuler);

    /* FIXME */
    bouton_supprimer_imputation = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    /*   bouton_supprimer_imputation = GTK_STOCK ( GNOME_STOCK_PIXMAP_REMOVE ); */
    gtk_button_set_relief ( GTK_BUTTON ( bouton_supprimer_imputation ),
			    GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_supprimer_imputation,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_imputation ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( supprimer_imputation ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			 bouton_supprimer_imputation,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_supprimer_imputation );


    /* mise en place des boutons ajout d'1 imput / sous-imput */

    bouton_ajouter_imputation = gtk_button_new_with_label ( _("Add a budget line") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_imputation ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_imputation ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( appui_sur_ajout_imputation ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_ajouter_imputation,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_ajouter_imputation );

    bouton_ajouter_sous_imputation = gtk_button_new_with_label ( _("Add a sub-budget line") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_sous_imputation ),
			    GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_ajouter_sous_imputation,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_sous_imputation ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( appui_sur_ajout_sous_imputation ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_ajouter_sous_imputation,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_ajouter_sous_imputation );

    /* séparation */

    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 5 );
    gtk_widget_show ( separateur );

    /* mise en place du bouton fusionner avec les catégories */

    bouton = gtk_button_new_with_label ( _("Merge categories") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( fusion_categories_imputation ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    /* séparation */

    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 5 );
    gtk_widget_show ( separateur );

    /* mise en place du bouton exporter */

    bouton = gtk_button_new_with_label ( _("Export") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( exporter_ib ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    /* mise en place du bouton importer */

    bouton = gtk_button_new_with_label ( _("Import") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( importer_ib ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    /*   création de la frame de droite */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
				GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 frame,
			 TRUE,
			 TRUE,
			 5 );
    gtk_widget_show (frame );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     10 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );

    /* on y ajoute la barre d'outils */

    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 creation_barre_outils_imputation(),
			 FALSE,
			 FALSE,
			 0 );

    /* création de l'arbre principal */

    scroll_window = gtk_scrolled_window_new ( NULL,
					      NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scroll_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 scroll_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scroll_window );


    arbre_imputation = gtk_ctree_new_with_titles ( 4,
						   0,
						   titres );
    gtk_ctree_set_line_style ( GTK_CTREE ( arbre_imputation ),
			       GTK_CTREE_LINES_DOTTED );
    gtk_ctree_set_expander_style ( GTK_CTREE ( arbre_imputation ),
				   GTK_CTREE_EXPANDER_CIRCULAR );
    gtk_clist_column_titles_passive ( GTK_CLIST ( arbre_imputation ));

    gtk_clist_set_column_justification ( GTK_CLIST ( arbre_imputation ),
					 0,
					 GTK_JUSTIFY_LEFT);
    gtk_clist_set_column_justification ( GTK_CLIST ( arbre_imputation ),
					 1,
					 GTK_JUSTIFY_CENTER);
    gtk_clist_set_column_justification ( GTK_CLIST ( arbre_imputation ),
					 2,
					 GTK_JUSTIFY_CENTER);
    gtk_clist_set_column_justification ( GTK_CLIST ( arbre_imputation ),
					 3,
					 GTK_JUSTIFY_CENTER);

    gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_imputation ),
				      0,
				      FALSE );
    gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_imputation ),
				      1,
				      FALSE );
    gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_imputation ),
				      2,
				      FALSE );
    gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_imputation ),
				      3,
				      FALSE );

    /* on met la fontion de tri alphabétique en prenant en compte les accents */

    gtk_clist_set_compare_func ( GTK_CLIST ( arbre_imputation ),
				 (GtkCListCompareFunc) classement_alphabetique_tree );

    gtk_signal_connect ( GTK_OBJECT ( arbre_imputation ),
			 "tree-select-row",
			 GTK_SIGNAL_FUNC ( selection_ligne_imputation ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( arbre_imputation ),
			 "tree-unselect-row",
			 GTK_SIGNAL_FUNC ( enleve_selection_ligne_imputation ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( arbre_imputation ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( verifie_double_click_imputation ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( arbre_imputation ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( keypress_ib ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( arbre_imputation ),
			 "size-allocate",
			 GTK_SIGNAL_FUNC ( changement_taille_liste_tiers ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( arbre_imputation ),
			 "tree-expand",
			 GTK_SIGNAL_FUNC ( ouverture_node_imputation ),
			 NULL );
    gtk_container_add ( GTK_CONTAINER (  scroll_window ),
			arbre_imputation );
    gtk_widget_show ( arbre_imputation );


    gtk_clist_set_compare_func ( GTK_CLIST ( arbre_imputation ),
				 (GtkCListCompareFunc) classement_alphabetique_tree );

    /* la 1ère fois qu'on affichera les catég, il faudra remplir la liste */

    modif_imputation = 1;

    return ( onglet );
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* Fonction remplit_arbre_imputation */
/* le vide et le remplit */
/* **************************************************************************************************** */

void remplit_arbre_imputation ( void )
{
    gchar *text[4];
    GSList *liste_imputation_tmp;
    gint place_imputation;
    gint i;

    /* freeze le ctree */

    gtk_clist_freeze ( GTK_CLIST ( arbre_imputation ));

    /* retire la sélection */

    gtk_clist_unselect_all ( GTK_CLIST ( arbre_imputation ));

    /*   efface l'ancien arbre */

    gtk_clist_clear ( GTK_CLIST ( arbre_imputation ));


    /* récupération de la devise des paramètres */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    if ( !devise_compte
	 ||
	 devise_compte -> no_devise != no_devise_totaux_tiers )
	devise_compte = devise_par_no ( no_devise_totaux_tiers );

    /* calcule les montants des imputations et sous imputations */

    calcule_total_montant_imputation ();

    /* remplit l'arbre */

    liste_imputation_tmp = liste_struct_imputation;
    place_imputation = 0;

    while ( liste_imputation_tmp )
    {
	struct struct_imputation *imputation;
	GtkCTreeNode *ligne;
	GSList *liste_sous_imputation_tmp;
	GtkCTreeNode *ligne_sous_imputation;
	gint place_sous_imputation;

	imputation = liste_imputation_tmp -> data;

	if ( etat.affiche_nb_ecritures_listes
	     &&
	     nb_ecritures_par_imputation[place_imputation+1] )
	    text[0] = g_strconcat ( imputation -> nom_imputation,
				    " (",
				    itoa ( nb_ecritures_par_imputation[place_imputation+1] ),
				    ")",
				    NULL );
	else
	    text[0] = imputation -> nom_imputation ;

	if ( tab_montant_imputation[place_imputation+1] )
	    text[1] = g_strdup_printf ( _("%4.2f %s"),
					tab_montant_imputation[place_imputation+1],
					devise_code ( devise_compte ) );
	else
	    text[1] = NULL;

	text[2] = NULL;
	text[3] = NULL;

	ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
					NULL,
					NULL,
					text,
					10,
					pixmap_ferme,
					masque_ferme,
					pixmap_ouvre,
					masque_ouvre,
					FALSE,
					FALSE );

	/* on associe à ce imputation à l'adr de sa struct */

	gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
				      ligne,
				      imputation );


	/*       pour chaque imputation, on met ses sous-imputation */


	liste_sous_imputation_tmp = imputation -> liste_sous_imputation;
	place_sous_imputation = 0;

	while ( liste_sous_imputation_tmp )
	{
	    struct struct_sous_imputation *sous_imputation;

	    sous_imputation = liste_sous_imputation_tmp -> data;

	    if ( tab_montant_sous_imputation[place_imputation]
		 &&
		 tab_montant_sous_imputation[place_imputation][place_sous_imputation+1]
		 &&
		 etat.affiche_nb_ecritures_listes
		 &&
		 nb_ecritures_par_sous_imputation[place_imputation][place_sous_imputation+1] )
		text[0] = g_strconcat ( sous_imputation -> nom_sous_imputation,
					" (",
					itoa ( nb_ecritures_par_sous_imputation[place_imputation][place_sous_imputation+1] ),
					")",
					NULL );
	    else
		text[0] = sous_imputation -> nom_sous_imputation ;

	    text[1] = NULL;

	    if ( tab_montant_sous_imputation[place_imputation]
		 &&
		 tab_montant_sous_imputation[place_imputation][place_sous_imputation+1] )
		text[2] = g_strdup_printf ( _("%4.2f %s"),
					    tab_montant_sous_imputation[place_imputation][place_sous_imputation+1],
					    devise_code ( devise_compte ) );
	    else
		text[2] = NULL;

	    text[3] = NULL;

	    ligne_sous_imputation = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
							    ligne,
							    NULL,
							    text,
							    10,
							    NULL,
							    NULL,
							    NULL,
							    NULL,
							    FALSE,
							    FALSE );

	    /* on associe cette sous_imputation à l'adr de sa struct */

	    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
					  ligne_sous_imputation,
					  sous_imputation );


	    /* pour chacun des sous imputation, on met un fils bidon pour pouvoir l'ouvrir */

	    ligne_sous_imputation = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
							    ligne_sous_imputation,
							    NULL,
							    text,
							    5,
							    NULL,
							    NULL,
							    NULL,
							    NULL,
							    FALSE,
							    FALSE );

	    /* on associe le fils bidon à -1 */

	    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
					  ligne_sous_imputation,
					  GINT_TO_POINTER (-1));

	    place_sous_imputation++;
	    liste_sous_imputation_tmp = liste_sous_imputation_tmp -> next;
	}

	/*       on a fini de saisir les sous catégories, s'il y avait des opés sans sous catég, on les mets ici */

	if ( tab_montant_sous_imputation[place_imputation]
	     &&
	     nb_ecritures_par_sous_imputation[place_imputation][0] )
	{
	    if ( etat.affiche_nb_ecritures_listes )
		text[0] = g_strdup_printf ( _("No sub-budget line (%d)"),
					    nb_ecritures_par_sous_imputation[place_imputation][0] );
	    else
		text[0] = _("No sub-budget line");

	    text[1] = NULL;

	    if ( tab_montant_sous_imputation[place_imputation][0] )
		text[2] = g_strdup_printf ( _("%4.2f %s"),
					    tab_montant_sous_imputation[place_imputation][0],
					    devise_code ( devise_compte ) );
	    else
		text[2] = NULL;

	    text[3] = NULL;

	    ligne_sous_imputation = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
							    ligne,
							    NULL,
							    text,
							    10,
							    NULL,
							    NULL,
							    NULL,
							    NULL,
							    FALSE,
							    FALSE );

	    /* pour chacun des sous imputation, on met un fils bidon pour pouvoir l'ouvrir */

	    ligne_sous_imputation = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
							    ligne_sous_imputation,
							    NULL,
							    text,
							    5,
							    NULL,
							    NULL,
							    NULL,
							    NULL,
							    FALSE,
							    FALSE );

	    /* on associe le fils bidon à -1 */

	    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
					  ligne_sous_imputation,
					  GINT_TO_POINTER (-1));

	}
	place_imputation++;
	liste_imputation_tmp = liste_imputation_tmp -> next;
    }

    /*   on a fini de mettre les catégories, on met ici les opés sans catég */

    if ( tab_montant_imputation[0] )
    {
	GtkCTreeNode *ligne;

	if ( etat.affiche_nb_ecritures_listes
	     &&
	     nb_ecritures_par_imputation[0] )
	    text[0] = g_strdup_printf ( _("No budget line (%d)"),
					nb_ecritures_par_imputation[0] );
	else
	    text[0] = _("No budget line");

	text[1] = g_strdup_printf ( _("%4.2f %s"),
				    tab_montant_imputation[0],
				    devise_code ( devise_compte ) );
	text[2] = NULL;
	text[3] = NULL;

	ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
					NULL,
					NULL,
					text,
					10,
					pixmap_ferme,
					masque_ferme,
					pixmap_ouvre,
					masque_ouvre,
					FALSE,
					FALSE );

	/* on met aucune sous imput */

	if ( etat.affiche_nb_ecritures_listes
	     &&
	     nb_ecritures_par_imputation[0] )
	    text[0] = g_strdup_printf ( _("No sub-budget line (%d)"),
					nb_ecritures_par_imputation[0] );
	else
	    text[0] = _("No sub-budget line");

	text[1] = NULL;
	text[2] = g_strdup_printf ( _("%4.2f %s"),
				    tab_montant_imputation[0],
				    devise_code ( devise_compte ) );
	text[3] = NULL;

	ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
					ligne,
					NULL,
					text,
					10,
					NULL,
					NULL,
					NULL,
					NULL,
					FALSE,
					FALSE );

	/* on met un fils bidon pour pouvoir l'ouvrir */

	ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
					ligne,
					NULL,
					text,
					5,
					NULL,
					NULL,
					NULL,
					NULL,
					FALSE,
					FALSE );

	/* on associe le fils bidon à -1 */

	gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
				      ligne,
				      GINT_TO_POINTER (-1));
    }

    /*   on efface les variables */

    free ( tab_montant_imputation );
    free ( nb_ecritures_par_imputation );

    for ( i=0 ; i<nb_enregistrements_imputations ; i++ )
	free ( tab_montant_sous_imputation[i] );
    free ( tab_montant_sous_imputation );

    for ( i=0 ; i<nb_enregistrements_imputations ; i++ )
	free ( nb_ecritures_par_sous_imputation[i] );
    free ( nb_ecritures_par_sous_imputation );



    gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_imputation ),
			       NULL );

    /* defreeze le ctree */

    gtk_clist_thaw ( GTK_CLIST ( arbre_imputation ));

    enleve_selection_ligne_imputation ();
    modif_imputation = 0;
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* Fonction ouverture_node_imputation */
/* appeléé lorsqu'on ouvre une imputation, sous imputation ou le compte d'une imputation */
/* remplit ce qui doit être affiché */
/* **************************************************************************************************** */

void ouverture_node_imputation ( GtkWidget *arbre,
				 GtkCTreeNode *node,
				 gpointer null )
{			    
    GtkCTreeRow *row;
    gchar *text[4];
    GtkCTreeNode *node_insertion;

    row = GTK_CTREE_ROW ( node );

    /*   si on ouvre une imputation, on fait rien */

    if ( row->level == 1 )
	return;

    /*   si le fiston = -1, c'est qu'il n'a pas encore été créé */
    /* dans le cas contraire, on vire */

    if ( GPOINTER_TO_INT ( gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
							 row -> children )) != -1 )
	return;


    /* freeze le ctree */

    gtk_clist_freeze ( GTK_CLIST ( arbre_imputation ));


    /* on commence par virer la ligne bidon qui était attachée à ce noeud */

    gtk_ctree_remove_node ( GTK_CTREE ( arbre_imputation ),
			    row -> children );

    /* séparation entre ouverture de sous-imputation ( 2 ) et ouverture de compte ( 3 ) */

    if ( row -> level == 2 )
    {
	/* c'est une ouverture de sous imputation, on récupère sa structure  */

	struct struct_imputation *imputation;
	struct struct_sous_imputation *sous_imputation;
	gint no_imputation;
	gint no_sous_imputation;
	gint i;

	if ( ( imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
							  GTK_CTREE_ROW ( node ) -> parent )) &&
	     imputation != GINT_TO_POINTER(-1) )
	{
	    no_imputation = imputation -> no_imputation;
	    if (( sous_imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
								  node)))
		no_sous_imputation = sous_imputation -> no_sous_imputation;
	    else
		no_sous_imputation = 0;
	}
	else
	{
	    no_imputation = 0;
	    no_sous_imputation = 0;
	}

	/* on va scanner tous les comptes, dès qu'une imputation correspondant à l'imput sélectionnée est trouvé */
	/* on affiche le nom du compte */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    GSList *pointeur_ope;

	    pointeur_ope = LISTE_OPERATIONS;

	    while ( pointeur_ope )
	    {
		struct structure_operation *operation;

		operation = pointeur_ope -> data;

		if ( operation -> imputation == no_imputation
		     &&
		     operation -> sous_imputation == no_sous_imputation
		     &&
		     !operation -> operation_ventilee )
		{
		    /* affiche le compte courant */

		    text[3] = calcule_total_montant_imputation_par_compte ( operation -> imputation,
									    operation -> sous_imputation,
									    operation -> no_compte );

		    if ( etat.affiche_nb_ecritures_listes
			 &&
			 nb_ecritures_par_comptes )
			text[0] = g_strconcat ( NOM_DU_COMPTE,
						" (",
						itoa ( nb_ecritures_par_comptes ),
						")",
						NULL );
		    else
			text[0] = NOM_DU_COMPTE;

		    text[1] = NULL;
		    text[2] = NULL;

		    node_insertion = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
							     node,
							     NULL,
							     text,
							     5,
							     NULL,
							     NULL,
							     NULL,
							     NULL,
							     FALSE,
							     FALSE );

		    /* associe le no de compte à la ligne du compte */

		    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
						  node_insertion,
						  GINT_TO_POINTER ( i ));

		    /* on met une ligne bidon pour pouvoir l'ouvrir */

		    node_insertion = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
							     node_insertion,
							     NULL,
							     text,
							     5,
							     NULL,
							     NULL,
							     NULL,
							     NULL,
							     FALSE,
							     FALSE );

		    /* on associe le fils bidon à -1 */

		    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
						  node_insertion,
						  GINT_TO_POINTER (-1));


		    pointeur_ope = NULL;
		}
		else
		    pointeur_ope = pointeur_ope -> next;
	    }

	    p_tab_nom_de_compte_variable++;
	}
    }
    else
    {
	/* c'est une ouverture d'un compte */
	/*       cette fois, on fait le tour de toutes les opés du compte pour afficher celles qui correspondent à la imputation */

	struct struct_imputation *imputation;
	struct struct_sous_imputation *sous_imputation;
	GSList *pointeur_ope;
	gint no_imputation;
	gint no_sous_imputation;

	if (( imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
							 GTK_CTREE_ROW ( GTK_CTREE_ROW ( node ) -> parent ) -> parent )) &&
	    imputation != GINT_TO_POINTER(-1) )
	{
	    no_imputation = imputation -> no_imputation;

	    if (( sous_imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
								  GTK_CTREE_ROW ( node ) -> parent ))
		&& sous_imputation != GINT_TO_POINTER(-1) )
		no_sous_imputation = sous_imputation -> no_sous_imputation;
	    else
		no_sous_imputation = 0;
	}
	else
	{
	    no_imputation = 0;
	    no_sous_imputation = 0;
	}

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
													     node ));

	pointeur_ope = LISTE_OPERATIONS;

	while ( pointeur_ope )
	{
	    struct struct_devise *devise_operation;
	    struct structure_operation *operation;

	    operation = pointeur_ope -> data;

	    devise_operation = devise_par_no ( operation -> devise );

	    if ( operation -> imputation == no_imputation
		 &&
		 operation -> sous_imputation == no_sous_imputation
		 &&
		 !operation -> operation_ventilee  )
	    {
		if ( operation -> notes )
		{
		    if ( operation -> no_operation_ventilee_associee )
			text[0] = g_strdup_printf ( _("%d/%d/%d : %4.2f %s (breakdown) [ %s ]"),
						    operation -> jour,
						    operation -> mois,
						    operation -> annee,
						    operation -> montant,
						    devise_code ( devise_operation ),
						    operation -> notes );
		    else
			text[0] = g_strdup_printf ( _("%d/%d/%d : %4.2f %s [ %s ]"),
						    operation -> jour,
						    operation -> mois,
						    operation -> annee,
						    operation -> montant,
						    devise_code ( devise_operation ),
						    operation -> notes );
		}
		else
		{
		    if ( operation -> no_operation_ventilee_associee )
			text[0] = g_strdup_printf ( _("%d/%d/%d : %4.2f %s (breakdown)"),
						    operation -> jour,
						    operation -> mois,
						    operation -> annee,
						    operation -> montant,
						    devise_code ( devise_operation ) );
		    else
			text[0] = g_strdup_printf ( "%d/%d/%d : %4.2f %s",
						    operation -> jour,
						    operation -> mois,
						    operation -> annee,
						    operation -> montant,
						    devise_code ( devise_operation ) );
		}

		text[1] = NULL;
		text[2] = NULL;
		text[3] = NULL;

		node_insertion = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
							 node,
							 NULL,
							 text,
							 5,
							 NULL,
							 NULL,
							 NULL,
							 NULL,
							 FALSE,
							 FALSE );

		/* on associe à cette opé l'adr de sa struct */

		gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
					      node_insertion,
					      operation );
	    }
	    pointeur_ope = pointeur_ope -> next;
	}
    }

    /* defreeze le ctree */

    gtk_clist_thaw ( GTK_CLIST ( arbre_imputation ));

}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
void selection_ligne_imputation ( GtkCTree *arbre,
				  GtkCTreeNode *noeud,
				  gint colonne,
				  gpointer null )
{
    GtkCTreeNode *node_tmp;

    /* on allume le bouton ajouter une sous imputation si ce n'est pas dans aucune imputation */

    node_tmp = noeud;

    while ( GTK_CTREE_ROW ( node_tmp ) -> level  != 1 )
	node_tmp = GTK_CTREE_ROW ( node_tmp ) -> parent;

    if ( gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
				       node_tmp ))
	gtk_widget_set_sensitive ( bouton_ajouter_sous_imputation,
				   TRUE );

    if ( GTK_CTREE_ROW ( noeud ) -> level  == 1
	 &&
	 gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
				       noeud ))
    {
	struct struct_imputation *imputation;

	imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
						   noeud );

	gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_imputation ),
					   GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
					   NULL );

	gtk_entry_set_text ( GTK_ENTRY ( entree_nom_imputation ),
			     imputation -> nom_imputation );

	gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_imputation ),
					     GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
					     NULL );

	gtk_object_set_data ( GTK_OBJECT ( entree_nom_imputation ),
			      "adr_struct_imputation",
			      imputation );
	gtk_object_set_data ( GTK_OBJECT ( entree_nom_imputation ),
			      "adr_struct_sous_imputation",
			      NULL );

	gtk_widget_set_sensitive ( bouton_supprimer_imputation,
				   TRUE );
	gtk_widget_set_sensitive ( entree_nom_imputation,
				   TRUE );
	gtk_widget_set_sensitive ( bouton_imputation_debit,
				   TRUE );
	gtk_widget_set_sensitive ( bouton_imputation_credit,
				   TRUE );

	gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_imputation_debit ),
					   GTK_SIGNAL_FUNC ( modification_du_texte_imputation ),
					   NULL );
	if ( imputation -> type_imputation )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_imputation_debit ),
					   TRUE );
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_imputation_credit ),
					   TRUE );

	gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_imputation_debit ),
					     GTK_SIGNAL_FUNC ( modification_du_texte_imputation ),
					     NULL );


    }
    else
	if ( GTK_CTREE_ROW ( noeud ) -> level  == 2
	     &&
	     gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
					   noeud )
	     &&
	     gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
					   GTK_CTREE_ROW ( noeud ) -> parent ))
	{
	    struct struct_sous_imputation *sous_imputation;

	    sous_imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
							    noeud );
	    gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_imputation ),
					       GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
					       NULL );

	    gtk_entry_set_text ( GTK_ENTRY ( entree_nom_imputation ),
				 sous_imputation -> nom_sous_imputation );

	    gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_imputation ),
						 GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
						 NULL );

	    gtk_object_set_data ( GTK_OBJECT ( entree_nom_imputation ),
				  "adr_struct_imputation",
				  NULL );
	    gtk_object_set_data ( GTK_OBJECT ( entree_nom_imputation ),
				  "adr_struct_sous_imputation",
				  sous_imputation );

	    gtk_widget_set_sensitive ( bouton_supprimer_imputation,
				       TRUE );
	    gtk_widget_set_sensitive ( entree_nom_imputation,
				       TRUE );
	}
}
/* **************************************************************************************************** */


gboolean verifie_double_click_imputation ( GtkWidget *liste, GdkEventButton *ev,
					   gpointer null )
{
    if ( ev -> type == GDK_2BUTTON_PRESS )
    {
	expand_selected_ib();
    }

    return FALSE;
}



gboolean keypress_ib ( GtkWidget *widget, GdkEventKey *ev, gint *no_origine )
{
    GtkCTreeNode *node;

    if ( ev->keyval == GDK_Return || 
	 ev->keyval == GDK_KP_Enter )
    { 
	node = gtk_ctree_node_nth ( GTK_CTREE(arbre_imputation), 
				    GTK_CLIST(arbre_imputation) -> focus_row );
	gtk_ctree_select ( GTK_CTREE(arbre_imputation), node );
	gtk_ctree_expand ( GTK_CTREE(arbre_imputation), node );

	expand_selected_ib ();
    }

    return FALSE;
}



gboolean expand_selected_ib (  )
{
    struct structure_operation *operation;

    if ( GTK_CLIST ( arbre_imputation ) -> selection
	 &&
	 ( GTK_CTREE_ROW ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data ) -> level == 4
	   ||
	   ( GTK_CTREE_ROW ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data ) -> level == 3
	     &&
	     !gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
					    GTK_CTREE_ROW ( GTK_CTREE_ROW (( GTK_CLIST ( arbre_imputation ) -> selection ) -> data ) -> parent ) -> parent ))))
    {
	/* passage sur le compte concerné */

	operation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
						  GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data ) );

	/* si c'est une opé de ventilation, on se place sur l'opé ventilée correspondante */

	if ( operation -> no_operation_ventilee_associee )
	    operation = operation_par_no ( operation -> no_operation_ventilee_associee,
					   operation -> no_compte );

	if ( operation )
	{
	    changement_compte ( GINT_TO_POINTER ( operation -> no_compte ));

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

	    if ( operation -> pointe == 3 && !AFFICHAGE_R )
	    {
		AFFICHAGE_R = 1;
		remplissage_liste_operations ( compte_courant );
	    }

	    selectionne_ligne ( OPERATION_SELECTIONNEE );
	}
    }

    return FALSE;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void enleve_selection_ligne_imputation ( void )
{
    gtk_widget_set_sensitive ( bouton_supprimer_imputation,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_modifier,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_annuler,
			       FALSE );
    gtk_widget_set_sensitive ( entree_nom_imputation,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_imputation_debit,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_imputation_credit,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_ajouter_sous_imputation,
			       FALSE );

    gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_imputation ),
				       GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
				       NULL );

    gtk_editable_delete_text ( GTK_EDITABLE ( entree_nom_imputation ),
			       0,
			       -1 );

    gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_imputation ),
					 GTK_SIGNAL_FUNC ( modification_du_texte_imputation),
					 NULL );

}
/* **************************************************************************************************** */






/* **************************************************************************************************** */
void modification_du_texte_imputation ( void )
{
    gtk_widget_set_sensitive ( bouton_modif_imputation_modifier,
			       TRUE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_annuler,
			       TRUE );
}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
void clique_sur_modifier_imputation ( void )
{
    GtkCTreeNode *node;


    /* si c'est une imputation et si c'est une modif du nom, on doit réafficher la liste des imputation et les listes des opés, sinon, on change juste la imputation et */
    /* met à jour les listes de imputations */
    /* si c'est une sous imputation, c'est obligmy_atoirement un changement de nom */

    if ( gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
			       "adr_struct_imputation" ) )
    {
	/* c'est une imputation */

	struct struct_imputation *imputation;


	imputation =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
					    "adr_struct_imputation" );


	if ( strcmp ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_imputation ))),
		      imputation -> nom_imputation ))
	{
	    free ( imputation -> nom_imputation );

	    imputation -> nom_imputation = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_imputation ))) );


	    node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data );

	    if ( GTK_CTREE_ROW ( node ) -> expanded )
		gtk_ctree_node_set_pixtext ( GTK_CTREE ( arbre_imputation ),
					     node,
					     0,
					     imputation -> nom_imputation,
					     10,
					     pixmap_ouvre,
					     masque_ouvre );
	    else
		gtk_ctree_node_set_pixtext ( GTK_CTREE ( arbre_imputation ),
					     node,
					     0,
					     imputation -> nom_imputation,
					     10,
					     pixmap_ferme,
					     masque_ferme );

	    demande_mise_a_jour_tous_comptes ();
	}
	else
	    imputation -> type_imputation = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_imputation_debit ));
    }
    else
    {
	/* c'est une sous imputation */

	struct struct_sous_imputation *sous_imputation;

	sous_imputation =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
						 "adr_struct_sous_imputation" );

	free ( sous_imputation -> nom_sous_imputation );

	sous_imputation -> nom_sous_imputation = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_imputation ))) );


	node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data );

	gtk_ctree_node_set_text ( GTK_CTREE ( arbre_imputation ),
				  node,
				  0,
				  sous_imputation -> nom_sous_imputation );

	demande_mise_a_jour_tous_comptes ();
    }

    gtk_clist_unselect_all ( GTK_CLIST ( arbre_imputation ));

    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();

    gtk_widget_set_sensitive ( bouton_modif_imputation_modifier,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_annuler,
			       FALSE );

    modification_fichier(TRUE);
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void clique_sur_annuler_imputation ( void )
{
    if ( gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
			       "adr_struct_imputation" ))
    {
	/* c'est une imputation */

	struct struct_imputation *imputation;

	imputation =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
					    "adr_struct_imputation" );

	gtk_entry_set_text ( GTK_ENTRY ( entree_nom_imputation ),
			     imputation -> nom_imputation );

	if ( imputation -> type_imputation )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_imputation_debit ),
					   TRUE );
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_imputation_credit ),
					   TRUE );
    }
    else
    {
	/* c'est une sous imputation */

	struct struct_sous_imputation *sous_imputation;

	sous_imputation =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
						 "adr_struct_sous_imputation" );

	gtk_entry_set_text ( GTK_ENTRY ( entree_nom_imputation ),
			     sous_imputation -> nom_sous_imputation );

    }

    gtk_widget_set_sensitive ( bouton_modif_imputation_modifier,
			       FALSE );
    gtk_widget_set_sensitive ( bouton_modif_imputation_annuler,
			       FALSE );

}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
void supprimer_imputation ( void )
{
    struct struct_imputation *imputation;
    GtkCTreeNode *node;
    gint i;
    gint ope_trouvee;


    if ( !gtk_object_get_data ( GTK_OBJECT (  entree_nom_imputation ),
				"adr_struct_imputation" ))
    {
	supprimer_sous_imputation();
	return;
    }

    node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data );

    imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
					       node );

    /* fait le tour des opés pour en trouver une qui a cette catégorie */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
    ope_trouvee = 0;

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	GSList *liste_tmp;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *operation;

	    operation = liste_tmp -> data;

	    if ( operation -> imputation == imputation -> no_imputation )
	    {
		ope_trouvee = 1;
		liste_tmp = NULL;
		i = nb_comptes;
	    }
	    else
		liste_tmp = liste_tmp -> next;
	}
	p_tab_nom_de_compte_variable++;
    }


    if ( ope_trouvee )
    {
	GtkWidget *dialog;
	GtkWidget *label;
	gint resultat;
	GtkWidget *separation;
	GtkWidget *hbox;
	GtkWidget *bouton_imputation_generique;
	GtkWidget *combofix;
	GSList *liste_combofix;
	GSList *pointeur;
	GSList *liste_tmp;
	GtkWidget *bouton_transfert;
	gint i;
	struct struct_imputation *nouvelle_imputation;
	struct struct_sous_imputation *nouvelle_sous_imputation;
	GSList *liste_imputation_credit;
	GSList *liste_imputation_debit;
	gchar **split_imputation;
	gint nouveau_no_imputation = 0;
	gint nouveau_no_sous_imputation = 0;

	dialog = gtk_dialog_new_with_buttons ( _("Delete a budgetary line"),
					       GTK_WINDOW (window),
					       GTK_DIALOG_MODAL,
					       GTK_STOCK_OK,0,
					       GTK_STOCK_CANCEL,1,
					       NULL);

	label = gtk_label_new ( COLON(_("Some transactions are still assigned to this budgetary line.\n\nYou can")) );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     label,
			     FALSE,
			     FALSE,
			     0 );

	separation = gtk_hseparator_new ( );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     separation,
			     FALSE,
			     FALSE,
			     0 );

	/*       mise en place du choix tranfert vers un autre imputation */

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );

	bouton_transfert = gtk_radio_button_new_with_label ( NULL,
							     COLON(_("Move the transactions to"))  );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton_transfert,
			     FALSE,
			     FALSE,
			     0 );


	pointeur = liste_struct_imputation;
	liste_combofix = NULL;
	liste_imputation_credit = NULL;
	liste_imputation_debit = NULL;

	while ( pointeur )
	{
	    struct struct_imputation *imputation_tmp;
	    GSList *sous_pointeur;

	    imputation_tmp = pointeur -> data;

	    if ( imputation_tmp -> no_imputation != imputation -> no_imputation )
	    {
		if ( imputation_tmp -> type_imputation )
		    liste_imputation_debit = g_slist_append ( liste_imputation_debit,
							      g_strdup ( imputation_tmp -> nom_imputation ) );
		else
		    liste_imputation_credit = g_slist_append ( liste_imputation_credit,
							       g_strdup ( imputation_tmp -> nom_imputation ) );


		sous_pointeur = imputation_tmp -> liste_sous_imputation;

		while ( sous_pointeur )
		{
		    struct struct_sous_imputation *sous_imputation;

		    sous_imputation = sous_pointeur -> data;

		    if ( imputation_tmp -> type_imputation )
			liste_imputation_debit = g_slist_append ( liste_imputation_debit,
								  g_strconcat ( "\t",
										sous_imputation -> nom_sous_imputation,
										NULL ) );
		    else
			liste_imputation_credit = g_slist_append ( liste_imputation_credit,
								   g_strconcat ( "\t",
										 sous_imputation -> nom_sous_imputation,
										 NULL ) );

		    sous_pointeur = sous_pointeur -> next;
		}
	    }
	    pointeur = pointeur -> next;
	}

	/*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */

	liste_combofix = g_slist_append ( liste_combofix,
					  liste_imputation_debit );
	liste_combofix = g_slist_append ( liste_combofix,
					  liste_imputation_credit );


	combofix = gtk_combofix_new_complex ( liste_combofix,
					      TRUE,
					      TRUE,
					      TRUE,
					      0 );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     combofix,
			     FALSE,
			     FALSE,
			     0 );

	/*       mise en place du choix supprimer le imputation */

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );

	bouton_imputation_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )),
									_("Just delete the budgetary line.") );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton_imputation_generique,
			     FALSE,
			     FALSE,
			     0 );

	gtk_widget_show_all ( dialog );


retour_dialogue:
	resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

	if ( resultat )
	{
	    if ( resultat == 1 )
		gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
	    return;
	}


	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_transfert )) )
	{

	    if ( !strlen (gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ))))
	    {
		dialogue_error ( _("Please enter a budgetary line!") );
		goto retour_dialogue;
	    }

	    /* transfert des imputation au niveau des listes */

	    split_imputation = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
					    " : ",
					    2 );

	    nouvelle_imputation = imputation_par_nom ( split_imputation[0],
						       0,
						       0,
						       0 );

	    if ( nouvelle_imputation )
	    {
		nouveau_no_imputation = nouvelle_imputation -> no_imputation;

		nouvelle_sous_imputation = sous_imputation_par_nom ( nouvelle_imputation,
								     split_imputation[1],
								     0 );

		if ( nouvelle_sous_imputation )
		    nouveau_no_sous_imputation = nouvelle_sous_imputation -> no_sous_imputation;
		else
		    nouveau_no_sous_imputation = 0;
	    }
	    g_strfreev ( split_imputation );
	}
	else
	{
	    nouveau_no_imputation = 0;
	    nouveau_no_sous_imputation = 0;
	}


	/* on fait le tour des opés pour mettre le nouveau numéro de imputation et sous_imputation */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    liste_tmp = LISTE_OPERATIONS;

	    while ( liste_tmp )
	    {
		struct structure_operation *operation;

		operation = liste_tmp -> data;

		if ( operation -> imputation == imputation -> no_imputation )
		{
		    operation -> imputation = nouveau_no_imputation;
		    operation -> sous_imputation = nouveau_no_sous_imputation;
		}

		liste_tmp = liste_tmp -> next;
	    }

	    p_tab_nom_de_compte_variable++;
	}

	/* fait le tour des échéances pour mettre le nouveau numéro de imputation et sous_imputation  */

	liste_tmp = liste_struct_echeances;

	while ( liste_tmp )
	{
	    struct operation_echeance *echeance;

	    echeance = liste_tmp -> data;

	    if ( echeance -> imputation == imputation -> no_imputation )
	    {
		echeance -> imputation = nouveau_no_imputation;
		echeance -> sous_imputation = nouveau_no_sous_imputation;

		if ( !echeance -> imputation )
		    echeance -> compte_virement = -1;
	    }

	    liste_tmp = liste_tmp -> next;
	}


	demande_mise_a_jour_tous_comptes ();

	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    }


    /* supprime dans la liste des imputation  */

    liste_struct_imputation = g_slist_remove ( liste_struct_imputation,
					       imputation );
    nb_enregistrements_imputations--;

    enleve_selection_ligne_imputation();

    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation  ();
    remplit_arbre_imputation ();

    modification_fichier(TRUE);
}
/* **************************************************************************************************** */








/* **************************************************************************************************** */
void supprimer_sous_imputation ( void )
{
    struct struct_imputation *imputation;
    struct struct_sous_imputation *sous_imputation;
    GtkCTreeNode *node;
    gint i;
    gint ope_trouvee;



    node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data );

    sous_imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
						    node );

    node = GTK_CTREE_ROW ( ( GTK_CLIST ( arbre_imputation ) -> selection ) -> data ) -> parent;

    imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
					       node );

    /* fait le tour des opés pour en trouver une qui a cette sous-catégorie */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
    ope_trouvee = 0;

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	GSList *liste_tmp;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *operation;

	    operation = liste_tmp -> data;

	    if ( operation -> imputation == imputation -> no_imputation
		 &&
		 operation -> sous_imputation == sous_imputation -> no_sous_imputation )
	    {
		ope_trouvee = 1;
		liste_tmp = NULL;
		i = nb_comptes;
	    }
	    else
		liste_tmp = liste_tmp -> next;
	}
	p_tab_nom_de_compte_variable++;
    }


    if ( ope_trouvee )
    {
	GtkWidget *dialog;
	GtkWidget *label;
	gint resultat;
	GtkWidget *separation;
	GtkWidget *hbox;
	GtkWidget *bouton_imputation_generique;
	GtkWidget *combofix;
	GSList *liste_combofix;
	GSList *pointeur;
	GSList *liste_tmp;
	GtkWidget *bouton_transfert;
	gint i;
	struct struct_imputation *nouvelle_imputation;
	struct struct_sous_imputation *nouvelle_sous_imputation;
	GSList *liste_imputation_credit;
	GSList *liste_imputation_debit;
	gchar **split_imputation;
	gint nouveau_no_imputation = 0;
	gint nouveau_no_sous_imputation = 0;

	dialog = gtk_dialog_new_with_buttons ( _("Deleting a budgetaty line"),
					       GTK_WINDOW (window),
					       GTK_DIALOG_MODAL,
					       GTK_STOCK_OK,0,
					       GTK_STOCK_CANCEL,1,
					       NULL);

	label = gtk_label_new ( COLON(_("Some transactions are still assigned to this sub-budgetary line.\n\nYou can")) );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     label,
			     FALSE,
			     FALSE,
			     0 );

	separation = gtk_hseparator_new ( );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     separation,
			     FALSE,
			     FALSE,
			     0 );

	/*       mise en place du choix tranfert vers un autre sous imputation */

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );

	bouton_transfert = gtk_radio_button_new_with_label ( NULL,
							     COLON(_("Move the transactions to"))  );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton_transfert,
			     FALSE,
			     FALSE,
			     0 );


	pointeur = liste_struct_imputation;
	liste_combofix = NULL;
	liste_imputation_credit = NULL;
	liste_imputation_debit = NULL;

	while ( pointeur )
	{
	    struct struct_imputation *imputation;
	    GSList *sous_pointeur;

	    imputation = pointeur -> data;

	    if ( imputation -> type_imputation )
		liste_imputation_debit = g_slist_append ( liste_imputation_debit,
							  g_strdup ( imputation -> nom_imputation ) );
	    else
		liste_imputation_credit = g_slist_append ( liste_imputation_credit,
							   g_strdup ( imputation -> nom_imputation ) );


	    sous_pointeur = imputation -> liste_sous_imputation;

	    while ( sous_pointeur )
	    {
		struct struct_sous_imputation *sous_imputation_tmp;

		sous_imputation_tmp = sous_pointeur -> data;

		if ( sous_imputation_tmp -> no_sous_imputation != sous_imputation -> no_sous_imputation )
		{
		    if ( imputation -> type_imputation )
			liste_imputation_debit = g_slist_append ( liste_imputation_debit,
								  g_strconcat ( "\t",
										sous_imputation_tmp -> nom_sous_imputation,
										NULL ) );
		    else
			liste_imputation_credit = g_slist_append ( liste_imputation_credit,
								   g_strconcat ( "\t",
										 sous_imputation_tmp -> nom_sous_imputation,
										 NULL ) );
		}
		sous_pointeur = sous_pointeur -> next;
	    }
	    pointeur = pointeur -> next;
	}

	/*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */

	liste_combofix = g_slist_append ( liste_combofix,
					  liste_imputation_debit );
	liste_combofix = g_slist_append ( liste_combofix,
					  liste_imputation_credit );


	combofix = gtk_combofix_new_complex ( liste_combofix,
					      TRUE,
					      TRUE,
					      TRUE,
					      0 );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     combofix,
			     FALSE,
			     FALSE,
			     0 );

	/*       mise en place du choix supprimer le imputation */

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );

	bouton_imputation_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )),
									PRESPACIFY(_("Just delete the sub-budgetary line.")) );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton_imputation_generique,
			     FALSE,
			     FALSE,
			     0 );

	gtk_widget_show_all ( dialog );


retour_dialogue:
	resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

	if ( resultat )
	{
	    if ( resultat == 1 )
		gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
	    return;
	}


	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_transfert )) )
	{

	    if ( !strlen (gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ))))
	    {
		dialogue_error ( _("Please enter a budgetary line!") );
		goto retour_dialogue;
	    }

	    /* récupère les no de imputation et sous imputation */

	    split_imputation = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
					    " : ",
					    2 );

	    nouvelle_imputation = imputation_par_nom ( split_imputation[0],
						       0,
						       0,
						       0 );

	    if ( nouvelle_imputation )
	    {
		nouveau_no_imputation = nouvelle_imputation -> no_imputation;
		
		nouvelle_sous_imputation = sous_imputation_par_nom ( nouvelle_imputation,
								     split_imputation[1],
								     0 );

		if ( nouvelle_sous_imputation )
		nouveau_no_sous_imputation = nouvelle_sous_imputation -> no_sous_imputation;
		else
		    nouveau_no_sous_imputation = 0;
	    }
	    else
		nouveau_no_imputation = 0;

	    g_strfreev ( split_imputation );
	}
	else
	{
	    nouveau_no_imputation = 0;
	    nouveau_no_sous_imputation = 0;
	}


	/* on fait le tour des opés pour mettre le nouveau numéro de imputation et sous_imputation */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    liste_tmp = LISTE_OPERATIONS;

	    while ( liste_tmp )
	    {
		struct structure_operation *operation;

		operation = liste_tmp -> data;

		if ( operation -> imputation == imputation -> no_imputation
		     &&
		     operation -> sous_imputation == sous_imputation -> no_sous_imputation )
		{
		    operation -> imputation = nouveau_no_imputation;
		    operation -> sous_imputation = nouveau_no_sous_imputation;
		}

		liste_tmp = liste_tmp -> next;
	    }

	    p_tab_nom_de_compte_variable++;
	}

	/* fait le tour des échéances pour mettre le nouveau numéro de imputation et sous_imputation  */

	liste_tmp = liste_struct_echeances;

	while ( liste_tmp )
	{
	    struct operation_echeance *echeance;

	    echeance = liste_tmp -> data;

	    if ( echeance -> imputation == imputation -> no_imputation
		 &&
		 echeance -> sous_imputation == sous_imputation -> no_sous_imputation )
	    {
		echeance -> imputation = nouveau_no_imputation;
		echeance -> sous_imputation = nouveau_no_sous_imputation;

		if ( !echeance -> imputation )
		    echeance -> compte_virement = -1;
	    }

	    liste_tmp = liste_tmp -> next;
	}


	demande_mise_a_jour_tous_comptes ();

	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    }


    /* supprime dans la liste des imputation  */

    imputation -> liste_sous_imputation = g_slist_remove ( imputation -> liste_sous_imputation,
							   sous_imputation );


    enleve_selection_ligne_imputation();

    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation  ();
    remplit_arbre_imputation ();

    modification_fichier(TRUE);
}
/* **************************************************************************************************** */






/***********************************************************************************************************/
/*  Routine qui crée la liste des catégories pour les combofix du formulaire et de la ventilation */
/* c'est à dire 3 listes dans 1 liste : */
/* la première contient les catégories de débit */
/* la seconde contient les catégories de crédit */
/* la troisième contient les catégories spéciales ( virement, retrait, ventilation ) */
/* la ventilation n'apparait pas dans les échéances ( et dans la ventilation ) */
/***********************************************************************************************************/

void creation_liste_imputation_combofix ( void )
{
    GSList *pointeur;
    GSList *liste_imputation_credit;
    GSList *liste_imputation_debit;

    if ( DEBUG )
	printf ( "creation_liste_imputation_combofix\n" );

    liste_imputations_combofix = NULL;
    liste_imputation_credit = NULL;
    liste_imputation_debit = NULL;


    pointeur = liste_struct_imputation;

    while ( pointeur )
    {
	struct struct_imputation *imputation;
	GSList *sous_pointeur;

	imputation = pointeur -> data;

	if ( imputation -> type_imputation )
	    liste_imputation_debit = g_slist_append ( liste_imputation_debit,
						      g_strdup ( imputation -> nom_imputation ) );
	else
	    liste_imputation_credit = g_slist_append ( liste_imputation_credit,
						       g_strdup ( imputation -> nom_imputation ) );

	sous_pointeur = imputation -> liste_sous_imputation;

	while ( sous_pointeur )
	{
	    struct struct_sous_imputation *sous_imputation;

	    sous_imputation = sous_pointeur -> data;

	    if ( imputation -> type_imputation )
		liste_imputation_debit = g_slist_append ( liste_imputation_debit,
							  g_strconcat ( "\t",
									sous_imputation -> nom_sous_imputation,
									NULL ) );
	    else
		liste_imputation_credit = g_slist_append ( liste_imputation_credit,
							   g_strconcat ( "\t",
									 sous_imputation -> nom_sous_imputation,
									 NULL ) );
	    sous_pointeur = sous_pointeur -> next;
	}
	pointeur = pointeur -> next;
    }


    /*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */

    liste_imputations_combofix = g_slist_append ( liste_imputations_combofix,
						  liste_imputation_debit );
    liste_imputations_combofix = g_slist_append ( liste_imputations_combofix,
						  liste_imputation_credit );
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction mise_a_jour_combofix_imputation */
/* recrée les listes de catégories des combofix */
/* et remet les combofix à jour */
/***********************************************************************************************************/

void mise_a_jour_combofix_imputation ( void )
{
    if ( DEBUG )
	printf ( "mise_a_jour_combofix_imputation\n" );

    creation_liste_imputation_combofix ();

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_BUDGET ))
	gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_BUDGET) ),
				liste_imputations_combofix,
				TRUE,
				TRUE );

    gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY] ),
			    liste_imputations_combofix,
			    TRUE,
			    TRUE );

    gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
			    liste_imputations_combofix,
			    TRUE,
			    TRUE );

    /* FIXME : ça ne devrait pas se trouver dans cette fonction */

    if ( etat_courant )
    {
	remplissage_liste_ib_etats ();
	selectionne_devise_ib_etat_courant ();
    }

    mise_a_jour_combofix_imputation_necessaire = 0;
    modif_imputation = 1;
}
/***********************************************************************************************************/




/***************************************************************************************************/
void fusion_categories_imputation ( void )
{
    /* on fait le tour des catégories et on ajoute aux imputations celles qui n'existent pas */

    GSList *liste_tmp;

    if ( !question ( _("Warning: this will add all the categories and subcategories to the budgetary lines!\nBesides you can't cancel this afterwards.\nWe advise you not to use this unless you know exactly what you are doing.\nDo you want to continue anyway?") ))
	return;

    liste_tmp = liste_struct_categories;

    while ( liste_tmp )
    {
	struct struct_categ *categorie;
	struct struct_imputation *imputation;
	GSList *liste_sous_tmp;

	categorie = liste_tmp -> data;

	/* vérifie si une imputation du nom de la catégorie existe */

	imputation = imputation_par_nom ( categorie -> nom_categ,
					  1,
					  categorie -> type_categ,
					  0 );
	
	if ( imputation )
	{
	    /* on fait maintenant la comparaison avec les sous catég et les sous imputations */

	    liste_sous_tmp = categorie -> liste_sous_categ;

	    while ( liste_sous_tmp )
	    {
		struct struct_sous_categ *sous_categ;
		struct struct_sous_imputation *sous_ib;

		sous_categ = liste_sous_tmp -> data;

		sous_ib = sous_imputation_par_nom ( imputation,
						    sous_categ -> nom_sous_categ,
						    1 );

		liste_sous_tmp = liste_sous_tmp -> next;
	    }
	}
	liste_tmp = liste_tmp -> next;
    }

    /* on met à jour les listes */

    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();
    remplit_arbre_imputation ();

    modification_fichier(TRUE);
}
/***************************************************************************************************/




/* **************************************************************************************************** */
/* crée un tableau de imputation et de sous imputation aussi gds que le nb de tiers */
/* et les renvoie dans un tab de 2 pointeurs */
/* **************************************************************************************************** */

void calcule_total_montant_imputation ( void )
{
    gint i;

    /* on crée les tableaux de montant */

    /* le +1 pour reserver le [0] pour aucune catégorie */

    tab_montant_imputation = calloc ( nb_enregistrements_imputations + 1,
				      sizeof ( gfloat ));
    nb_ecritures_par_imputation = calloc ( nb_enregistrements_imputations + 1,
					   sizeof ( gint ));

    tab_montant_sous_imputation = calloc ( nb_enregistrements_imputations,
					   sizeof ( gpointer ));
    nb_ecritures_par_sous_imputation = calloc ( nb_enregistrements_imputations,
						sizeof ( gpointer ));


    for ( i=0 ; i<nb_comptes ; i++ )
    {
	GSList *liste_tmp;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *operation;
	    gdouble montant;
	    gint place_imputation;


	    operation = liste_tmp -> data;

	    /* on commence par calculer le montant dans la devise demandée */

	    montant = calcule_montant_devise_renvoi ( operation -> montant,
						      no_devise_totaux_tiers,
						      operation -> devise,
						      operation -> une_devise_compte_egale_x_devise_ope,
						      operation -> taux_change,
						      operation -> frais_change );

	    /* on traite ensuite l'opération */

	    if ( operation -> imputation )
	    {
		struct struct_imputation *imputation;

		/* il y a une ib */

		imputation = imputation_par_no ( operation -> imputation );

		/* recherche la place du tiers dans la liste */

		place_imputation = g_slist_index ( liste_struct_imputation,
						   imputation );

		/* crée la place pour les sous catég de cette imputation si ce n'est déjà fait */

		if ( !tab_montant_sous_imputation[place_imputation] )
		{
		    gint nb_sous_imputation;

		    nb_sous_imputation = g_slist_length ( imputation -> liste_sous_imputation );

		    /* on réserve nb_sous_imputation + 1 pour aucune sous imputation qui sera en [0] */

		    tab_montant_sous_imputation[place_imputation] = calloc ( nb_sous_imputation + 1,
									     sizeof ( float ));
		    nb_ecritures_par_sous_imputation[place_imputation] = calloc ( nb_sous_imputation + 1,
										  sizeof ( gint ));
		}

		tab_montant_imputation[place_imputation+1] = tab_montant_imputation[place_imputation+1] + montant;
		nb_ecritures_par_imputation[place_imputation+1]++;

		/* on ajoute maintenant le montant à la sous imputation si elle existe */

		if ( operation -> sous_imputation )
		{
		    gint place_sous_imputation;

		    place_sous_imputation = g_slist_position ( imputation -> liste_sous_imputation,
							       g_slist_find_custom ( imputation -> liste_sous_imputation,
										     GINT_TO_POINTER ( operation -> sous_imputation ),
										     (GCompareFunc) recherche_sous_imputation_par_no ));
		    tab_montant_sous_imputation[place_imputation][place_sous_imputation+1] = tab_montant_sous_imputation[place_imputation][place_sous_imputation+1] + montant;
		    nb_ecritures_par_sous_imputation[place_imputation][place_sous_imputation+1]++;
		}
		else
		{
		    if ( tab_montant_sous_imputation[place_imputation] )
		    {
			tab_montant_sous_imputation[place_imputation][0] = tab_montant_sous_imputation[place_imputation][0] + montant;
			nb_ecritures_par_sous_imputation[place_imputation][0]++;
		    }
		}
	    }
	    else
		/* il n'y a pas d'imputation */
		/* on met le montant dans tab_montant_imputation[0} si ce n'est une ventil */
		if ( !operation -> operation_ventilee )
		{
		    tab_montant_imputation[0] = tab_montant_imputation[0] + montant;
		    nb_ecritures_par_imputation[0]++;
		}
	    liste_tmp = liste_tmp -> next;
    }
}
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
gchar *calcule_total_montant_imputation_par_compte ( gint imputation, gint sous_imputation, 
						     gint no_compte )
{
    gdouble retour_int;
    GSList *liste_tmp;

    retour_int = 0;
    nb_ecritures_par_comptes = 0;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

    liste_tmp = LISTE_OPERATIONS;


    while ( liste_tmp )
    {
	struct structure_operation *operation;

	operation = liste_tmp -> data;

	if ( operation -> imputation == imputation
	     &&
	     operation -> sous_imputation == sous_imputation
	     &&
	     !operation -> operation_ventilee   )
	{
	    gdouble montant;

	    montant = calcule_montant_devise_renvoi ( operation -> montant,
						      no_devise_totaux_tiers,
						      operation -> devise,
						      operation -> une_devise_compte_egale_x_devise_ope,
						      operation -> taux_change,
						      operation -> frais_change );

	    retour_int = retour_int + montant;
	    nb_ecritures_par_comptes++;
	}
	liste_tmp = liste_tmp -> next;
    }

    if ( retour_int )
	return ( g_strdup_printf ( _("%4.2f %s"),
				   retour_int,
				   devise_code_by_no ( no_devise_totaux_tiers ) ));
    else
	return ( NULL );
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void appui_sur_ajout_imputation ( void )
{
    gchar *nom_imputation;
    struct struct_imputation *nouvelle_imputation;
    gchar *text[4];
    GtkCTreeNode *ligne;

    if ( !( nom_imputation = demande_texte ( _("New budgetary line"),
					     COLON(_("Enter the name of the new budgetary line")) )))
	return;

    /* on l'ajoute à la liste des opés */

    nouvelle_imputation = imputation_par_nom ( nom_imputation,
					       1,
					       0,
					       0 );

    /* on l'ajoute directement au ctree et on fait le tri pour éviter de toute la réafficher */

    text[0] = nouvelle_imputation -> nom_imputation;
    text[1] = NULL;
    text[2] = NULL;
    text[3] = NULL;

    ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
				    NULL,
				    NULL,
				    text,
				    10,
				    pixmap_ferme,
				    masque_ferme,
				    pixmap_ouvre,
				    masque_ouvre,
				    FALSE,
				    FALSE );

    /* on associe à ce imputation à l'adr de sa struct */

    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
				  ligne,
				  nouvelle_imputation );

    gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_imputation ),
			       NULL );

    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation();
    modif_imputation = 0;
    modification_fichier(TRUE);
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void appui_sur_ajout_sous_imputation ( void )
{
    gchar *nom_sous_imputation;
    struct struct_sous_imputation *nouvelle_sous_imputation;
    struct struct_imputation *imputation;
    gchar *text[4];
    GtkCTreeNode *ligne;
    GtkCTreeNode *node_parent;

    if ( !( nom_sous_imputation = demande_texte ( _("New sub-budgetary line"),
						  COLON(_("Enter the name of the new sub-budgetary line")) )))
	return;

    /* récupère le node parent */

    node_parent = GTK_CLIST ( arbre_imputation ) -> selection -> data;

    while ( GTK_CTREE_ROW ( node_parent ) -> level != 1 )
	node_parent = GTK_CTREE_ROW ( node_parent ) -> parent;

    /* on récupère l'imputation parente */

    imputation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_imputation ),
					       node_parent );

    /* on l'ajoute à la liste des opés */

    nouvelle_sous_imputation = sous_imputation_par_nom ( imputation,
							 nom_sous_imputation,
							 1 );


    /* on l'ajoute directement au ctree et on fait le tri pour éviter de toute la réafficher */

    text[0] = nouvelle_sous_imputation -> nom_sous_imputation;
    text[1] = NULL;
    text[2] = NULL;
    text[3] = NULL;

    ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_imputation ),
				    node_parent,
				    NULL,
				    text,
				    10,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    FALSE,
				    FALSE );

    /* on associe à ce imputation à l'adr de sa struct */

    gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_imputation ),
				  ligne,
				  nouvelle_sous_imputation );

    gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_imputation ),
			       NULL );


    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation();
    modif_imputation = 0;
    modification_fichier(TRUE);
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void exporter_ib ( void )
{
    GtkWidget *fenetre_nom;
    gint resultat;
    struct stat test_fichier;
    gchar *nom_ib;

    fenetre_nom = gtk_file_selection_new (  _("Export the budgetary lines"));
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
				      dernier_chemin_de_travail );
    file_selection_set_entry ( GTK_FILE_SELECTION ( fenetre_nom ), ".igsb" );
    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nom_ib =file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));

	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

	    /* vérification que c'est possible */

	    if ( !strlen ( nom_ib ))
		return;

	    if ( utf8_stat ( nom_ib,
			&test_fichier ) != -1 )
	    {
		if ( S_ISREG ( test_fichier.st_mode ) )
		{
		    if ( !question_yes_no_hint ( _("Save file"),
						 _("File exists. Do you want to overwrite it?")))
			return;
		}
		else
		{
		    dialogue_error ( g_strdup_printf ( _("Invalid filename \"%s\"!"), nom_ib ));
		    return;
		}
	    }

	    if ( !enregistre_ib ( nom_ib ))
	    {
		dialogue_error ( _("Cannot save file.") );
		return;
	    }

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
	    return;
    }
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void importer_ib ( void )
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *fenetre_nom;
    gint resultat;
    gchar *nom_ib;
    GtkWidget *bouton_merge_remplace;
    GtkWidget *menu;
    GtkWidget *menu_item;


    fenetre_nom = gtk_file_selection_new (_("Import the budgetary lines" ));
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
				      dernier_chemin_de_travail );
    file_selection_set_entry ( GTK_FILE_SELECTION ( fenetre_nom ), ".igsb" );

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( fenetre_nom );
	return;
    }

    nom_ib = file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));

    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

    /* vérification que c'est possible */

    if ( !strlen ( nom_ib ))
	return;

    /* on permet de remplacer/fusionner la liste */

    dialog = gtk_dialog_new_with_buttons ( _("Import the budgetary lines"),
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_OK,0,
					   GTK_STOCK_CANCEL,1,
					   NULL );
    gtk_signal_connect ( GTK_OBJECT ( dialog ),
			 "destroy",
			 GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ),
			 "destroy" );

    /* pour éviter un warning lors de la compil */

    bouton_merge_remplace = NULL;

    if ( no_derniere_operation )
    {
	/*       il y a déjà des opérations dans le fichier, on ne peut que fusionner */

	label = gtk_label_new ( _("File already contains transactions, the two budgetary lines lists will be merged." ));
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     label,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( label );
    }
    else
    {
	label = gtk_label_new ( COLON(_("Do you want")));
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     label,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( label );

	bouton_merge_remplace = gtk_option_menu_new ();
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     bouton_merge_remplace,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( bouton_merge_remplace );

	menu = gtk_menu_new ();

	menu_item = gtk_menu_item_new_with_label ( _("Merge the two lists of budgetary lines" ));
	gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			      "choix",
			      NULL );
	gtk_menu_append ( GTK_MENU ( menu ),
			  menu_item );
	gtk_widget_show ( menu_item );

	menu_item = gtk_menu_item_new_with_label ( _("Replace the old list of budgetary lines" ));
	gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			      "choix",
			      GINT_TO_POINTER ( 1 ));
	gtk_menu_append ( GTK_MENU ( menu ),
			  menu_item );
	gtk_widget_show ( menu_item );

	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_merge_remplace ),
				   menu );
	gtk_widget_show ( menu );
    }


    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch ( resultat )
    {
	case 0 :
	    /* si on a choisi de remplacer l'ancienne liste, on la vire ici */

	    if ( !no_derniere_operation
		 &&
		 gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_merge_remplace ) -> menu_item ),
				       "choix" ))
	    {
		g_slist_free ( liste_struct_imputation );
		liste_struct_imputation = NULL;
		no_derniere_imputation = 0;
		nb_enregistrements_imputations = 0;
	    }

	    if ( !charge_ib ( nom_ib ))
	    {
		dialogue_error ( _("Cannot import file.") );
		return;
	    }
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ));

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
	    return;
    }
}
/* **************************************************************************************************** */





/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
