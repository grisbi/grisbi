/* ************************************************************************** */
/* Ce fichier s'occupe de la gestion du formulaire de saisie des opérations   */
/* 			formulaire.c                                          */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "operations_formulaire_constants.h"


/*START_INCLUDE*/
#include "operations_formulaire.h"
#include "accueil.h"
#include "exercice.h"
#include "operations_liste.h"
#include "utils_devises.h"
#include "utils_montants.h"
#include "utils_editables.h"
#include "utils_categories.h"
#include "type_operations.h"
#include "devises.h"
#include "dialog.h"
#include "equilibrage.h"
#include "calendar.h"
#include "utils_dates.h"
#include "gtk_combofix.h"
#include "utils_ib.h"
#include "utils_str.h"
#include "menu.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "operations_comptes.h"
#include "traitement_variables.h"
#include "utils_operations.h"
#include "affichage_formulaire.h"
#include "utils_comptes.h"
#include "etats_calculs.h"
#include "utils_tiers.h"
#include "utils_types.h"
#include "utils.h"
#include "ventilation.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void click_sur_bouton_voir_change ( void );
static gboolean completion_operation_par_tiers ( GtkWidget *entree );
static GtkWidget *creation_boutons_formulaire ( void );
static GtkWidget *cree_element_formulaire_par_no ( gint no_element );
static gboolean element_focusable ( gint no_element );
static void modifie_operation ( struct structure_operation *operation );
static struct structure_operation *recherche_derniere_occurence_tiers ( gint no_tiers );
static gint recherche_element_suivant_formulaire ( gint element_courant,
					    gint sens_deplacement );
static void recuperation_categorie_formulaire ( struct structure_operation *operation,
					 gint modification );
static void recuperation_donnees_generales_formulaire ( struct structure_operation *operation );
static void recuperation_ope_filles_completion_tiers ( struct structure_operation *operation );
static gboolean touches_champ_formulaire ( GtkWidget *widget,
				    GdkEventKey *ev,
				    gint *no_origine );
static gint verification_validation_operation ( struct structure_operation *operation );
static void verifie_champs_dates ( gint origine );
static void widget_grab_focus_formulaire ( gint no_element );
/*END_STATIC*/





/* vbox contenant le formulaire du compte courant */

GtkWidget *vbox_elements_formulaire;

/* vbox contenant le séparateur et les boutons valider/annuler */
/* affichée ou non suivant la conf */

GtkWidget *vbox_boutons_formulaire;

/* les 2 styles pour les éléments du formulaire : */
/* normal ou grisé */

GtkStyle *style_entree_formulaire[2];


/* contient les adresses des widgets dans le formulaire en fonction */
/* de leur place */

GtkWidget *tab_widget_formulaire[4][6];



/*START_EXTERN*/
extern gboolean block_menu_cb ;
extern GtkWidget *bouton_affiche_cache_formulaire;
extern gint compte_courant;
extern GdkColor couleur_grise;
extern gchar *derniere_date;
extern struct struct_devise *devise_compte;
extern gint enregistre_ope_au_retour;
extern GtkWidget *fleche_bas;
extern GtkWidget *fleche_haut;
extern GtkWidget *formulaire;
extern GtkWidget *frame_droite_bas;
extern gint hauteur_ligne_liste_opes;
extern GtkItemFactory *item_factory_menu_general;
extern GSList *liste_categories_combofix;
extern GSList *liste_imputations_combofix;
extern GSList *liste_struct_devises;
extern GSList *liste_struct_etats;
extern GSList *liste_tiers_combofix;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern gint nb_colonnes;
extern gint nb_comptes;
extern gint no_derniere_operation;
extern FILE * out;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern gdouble taux_de_change[2];
extern GtkTooltips *tooltips_general_grisbi;
/*END_EXTERN*/





/******************************************************************************/
/*  Routine qui crée le formulaire et le renvoie                              */
/******************************************************************************/
GtkWidget *creation_formulaire ( void )
{
    GdkColor couleur_normale;
    GdkColor couleur_grise;

    /* On crée tout de suite les styles qui seront appliqués aux entrées du formulaire : */
    /*     style_entree_formulaire[ENCLAIR] sera la couleur noire, normale */
    /*     style_entree_formulaire[ENGRIS] sera une couleur atténuée quand le formulaire est vide */

    couleur_normale.red = COULEUR_NOIRE_RED;
    couleur_normale.green = COULEUR_NOIRE_GREEN;
    couleur_normale.blue = COULEUR_NOIRE_BLUE;

    couleur_grise.red = COULEUR_GRISE_RED;
    couleur_grise.green = COULEUR_GRISE_GREEN;
    couleur_grise.blue = COULEUR_GRISE_BLUE;

    style_entree_formulaire[ENCLAIR] = gtk_style_new();
    style_entree_formulaire[ENCLAIR] -> text[GTK_STATE_NORMAL] = couleur_normale;

    style_entree_formulaire[ENGRIS] = gtk_style_new();
    style_entree_formulaire[ENGRIS] -> text[GTK_STATE_NORMAL] = couleur_grise;

    g_object_ref ( style_entree_formulaire[ENCLAIR] );
    g_object_ref ( style_entree_formulaire[ENGRIS] );

    /* le formulaire est une vbox avec en haut un tableau de 6 colonnes, et */
    /* en bas si demandé les boutons valider et annuler */

    formulaire = gtk_vbox_new ( FALSE, 5 );


    /*     mise en place de la vbox qui contient les éléments du formulaire */

    vbox_elements_formulaire = gtk_vbox_new ( FALSE,
					      0 );
    gtk_box_pack_start ( GTK_BOX ( formulaire ),
			 vbox_elements_formulaire,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox_elements_formulaire );


    /*     mise en place des boutons valider/annuler */

    vbox_boutons_formulaire = creation_boutons_formulaire ();
    gtk_box_pack_start ( GTK_BOX ( formulaire ),
			 vbox_boutons_formulaire,
			 FALSE,
			 FALSE,
			 0 );


    return ( formulaire );
}
/******************************************************************************/



/******************************************************************************/
/* cette fonction remplit le formulaire en fonction de l'organisation du fomulaire */
/* du compte donné en argument */
/******************************************************************************/
void remplissage_formulaire ( gint no_compte )
{
    struct organisation_formulaire *organisation_formulaire;
    gint i, j;
    GtkWidget *table;


    /*     s'il y avait déjà un formulaire, on l'efface */

    if ( GTK_BOX ( vbox_elements_formulaire ) -> children )
    {
	GtkBoxChild *child;

	child = GTK_BOX ( vbox_elements_formulaire ) -> children -> data;

	gtk_widget_destroy ( child -> widget );
    }
	 
    /*     récupère l'organisation du formulaire courant */

    organisation_formulaire = renvoie_organisation_formulaire ();

	 
    /* le formulaire est une table */

    table = gtk_table_new ( organisation_formulaire -> nb_lignes,
			    organisation_formulaire -> nb_colonnes,
			    FALSE );

    gtk_table_set_col_spacings ( GTK_TABLE ( table ),
				 10 );
    gtk_box_pack_start ( GTK_BOX ( vbox_elements_formulaire ),
			 table,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( table );


    for ( i=0 ; i < organisation_formulaire -> nb_lignes ; i++ )
	for ( j=0 ; j < organisation_formulaire -> nb_colonnes ; j++ )
	{
	    GtkWidget *widget;

	    widget = cree_element_formulaire_par_no ( organisation_formulaire -> tab_remplissage_formulaire[i][j] );

	    tab_widget_formulaire[i][j] = widget;

	    if ( !widget )
		continue;

	    gtk_table_attach ( GTK_TABLE ( table ),
			       widget,
			       j, j+1,
			       i, i+1,
			       GTK_SHRINK | GTK_FILL,
			       GTK_SHRINK | GTK_FILL,
			       0, 0);
	}

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    /*     on met le type courant */

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_TYPE )
	 &&
	 GTK_WIDGET_VISIBLE ( widget_formulaire_par_element( TRANSACTION_FORM_TYPE )))
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_par_element( TRANSACTION_FORM_TYPE )),
				      cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );

    formulaire_a_zero ();

    mise_a_jour_taille_formulaire ( formulaire -> allocation.width );

}
/******************************************************************************/



/******************************************************************************/
/* cette fonction retourne le widget correspondant à l'élément donné en argument */
/* ce widget est initialisé, shown, et les signaux sont activés */
/******************************************************************************/
GtkWidget *cree_element_formulaire_par_no ( gint no_element )
{
    GtkWidget *widget;
    GtkWidget *menu;

    if ( !no_element )
	return NULL;

    widget = NULL;

    switch ( no_element )
    {
	case TRANSACTION_FORM_DATE:
	case TRANSACTION_FORM_DEBIT:
	case TRANSACTION_FORM_CREDIT:
	case TRANSACTION_FORM_VALUE_DATE:
	case TRANSACTION_FORM_NOTES:
	case TRANSACTION_FORM_CHEQUE:
	case TRANSACTION_FORM_VOUCHER:
	case TRANSACTION_FORM_BANK:

	    widget = gtk_entry_new();
	    gtk_widget_show ( widget );
	    break;


	case TRANSACTION_FORM_EXERCICE:
	    /* met l'option menu de l'exercice */

	    widget = gtk_option_menu_new ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose the financial year"),
				   _("Choose the financial year") );
	    menu = gtk_menu_new ();
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
				       creation_menu_exercices (0) );
	    gtk_widget_show ( widget );
	    break;

	case TRANSACTION_FORM_PARTY:
	    /*  entrée du tiers : c'est une combofix */

	    widget = gtk_combofix_new_complex ( liste_tiers_combofix,
						FALSE,
						TRUE,
						TRUE,
						50 );
	    gtk_widget_show ( widget );
	    break;

	case TRANSACTION_FORM_CATEGORY:
	    /*  Affiche les catégories / sous-catégories */

	    widget = gtk_combofix_new_complex ( liste_categories_combofix,
						FALSE,
						TRUE,
						TRUE,
						0 );
	    gtk_widget_show ( widget );
	    break;

	case TRANSACTION_FORM_BREAKDOWN:
	    /* mise en forme du bouton ventilation */

	    widget = gtk_button_new_with_label ( _("Breakdown") );
	    gtk_button_set_relief ( GTK_BUTTON ( widget ),
				    GTK_RELIEF_NONE );
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "clicked",
				 GTK_SIGNAL_FUNC ( basculer_vers_ventilation ),
				 NULL );
	    gtk_widget_show ( widget );
	    break;

	case TRANSACTION_FORM_BUDGET:
	    /* Affiche l'imputation budgétaire */

	    widget = gtk_combofix_new_complex ( liste_imputations_combofix,
						FALSE,
						TRUE,
						TRUE,
						0 );

	    gtk_widget_show ( widget );
	    break;


	case TRANSACTION_FORM_TYPE:
	    /* Affiche l'option menu des types */

	    widget = gtk_option_menu_new ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose the method of payment"),
				   _("Choose the method of payment") );

	    /* le menu par défaut est celui des débits */
	    /* 	    le widget ne s'afffiche que s'il y a des types */

	    if ( ( menu = creation_menu_types ( 1, compte_courant, 0 ) ) )
	    {
		gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
					   menu );
		gtk_widget_show ( widget );
	    }
	    break;

	case TRANSACTION_FORM_DEVISE:
	    /* met l'option menu des devises */

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

	    widget = gtk_option_menu_new ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose currency"),
				   _("Choose currency") );
	    menu = creation_option_menu_devises ( -1,
						  liste_struct_devises );
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
				       menu );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
					  g_slist_index ( liste_struct_devises,
							  devise_par_no ( DEVISE )));
	    gtk_widget_show ( widget );
	    break;

	case TRANSACTION_FORM_CHANGE:
	    /* mise en forme du bouton change */

	    widget = gtk_button_new_with_label ( _("Change") );
	    gtk_button_set_relief ( GTK_BUTTON ( widget ),
				    GTK_RELIEF_NONE );
	    gtk_signal_connect ( GTK_OBJECT (  widget ),
				 "clicked",
				 GTK_SIGNAL_FUNC ( click_sur_bouton_voir_change ),
				 NULL );
	    gtk_widget_show ( widget );
	    break;


	case TRANSACTION_FORM_CONTRA:
	    /* mise en place du type associé lors d'un virement */
	    /* non affiché au départ et pas de menu au départ */

	    widget = gtk_option_menu_new ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Contra-transaction method of payment"),
				   _("Contra-transaction method of payment") );
	    break;

	case TRANSACTION_FORM_OP_NB:
	case TRANSACTION_FORM_MODE:
	    widget = gtk_label_new ( "" );
	    gtk_widget_show ( widget );
	    break;
    }

    if ( widget )
    {
	if ( GTK_IS_ENTRY ( widget ))
	{
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "focus-in-event",
				 GTK_SIGNAL_FUNC ( entree_prend_focus ),
				 NULL );
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "focus-out-event",
				 GTK_SIGNAL_FUNC ( entree_perd_focus ),
				 GINT_TO_POINTER ( no_element ));
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "button-press-event",
				 GTK_SIGNAL_FUNC ( clique_champ_formulaire ),
				 GINT_TO_POINTER ( no_element ));
	    gtk_signal_connect ( GTK_OBJECT ( widget ),
				 "key-press-event",
				 GTK_SIGNAL_FUNC ( touches_champ_formulaire ),
				 GINT_TO_POINTER ( no_element ));
	}
	else
	{
	    if ( GTK_IS_COMBOFIX ( widget ))
	    {
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget ) -> entry ),
				     "focus-in-event",
				     GTK_SIGNAL_FUNC ( entree_prend_focus ),
				     NULL );
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				     "focus-out-event",
				     GTK_SIGNAL_FUNC ( entree_perd_focus ),
				     GINT_TO_POINTER ( no_element ));
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				     "button-press-event",
				     GTK_SIGNAL_FUNC ( clique_champ_formulaire ),
				     GINT_TO_POINTER ( no_element ));
		gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				     "key-press-event",
				     GTK_SIGNAL_FUNC ( touches_champ_formulaire ),
				     GINT_TO_POINTER ( no_element ));
	    }
	    else
		/* ce n'est ni un combofix, ni une entrée */
		gtk_signal_connect ( GTK_OBJECT ( widget ),
				     "key-press-event",
				     GTK_SIGNAL_FUNC ( touches_champ_formulaire ),
				     GINT_TO_POINTER ( no_element ));
	}
    }

    return widget;
}
/******************************************************************************/


/******************************************************************************/
/* crée la partie basse du formulaire : les boutons valider et annuler */
/* et renvoie la vbox */
/******************************************************************************/
GtkWidget *creation_boutons_formulaire ( void )
{
    GtkWidget *vbox;
    GtkWidget *separateur;
    GtkWidget *hbox;
    GtkWidget *bouton;

 
    vbox = gtk_vbox_new ( FALSE,
			  0);
    
    /* séparation d'avec les boutons */

    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );

    /* mise en place des boutons */

    hbox = gtk_hbox_new ( FALSE,
					      5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton = gtk_button_new_from_stock ( GTK_STOCK_OK );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( fin_edition ),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_from_stock ( GTK_STOCK_CANCEL );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( echap_formulaire ),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( bouton );

    if ( etat.affiche_boutons_valider_annuler )
	gtk_widget_show ( vbox );

    return vbox;
}
/******************************************************************************/





/******************************************************************************/
void echap_formulaire ( void )
{
    GSList *liste_tmp;

    /*     on libère la mémoire des ventilations */
    /* 	dans tous les cas, toutes les modifs apportées ne l'étaient que */
    /* 	dans cette liste */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    liste_tmp = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				      "liste_adr_ventilation" );

    if ( liste_tmp != GINT_TO_POINTER(-1))
    {
	while ( liste_tmp )
	{
	    free ( liste_tmp -> data );
	    liste_tmp = liste_tmp -> next;
	}

	g_slist_free ( gtk_object_get_data ( GTK_OBJECT ( formulaire ),
					     "liste_adr_ventilation" ));
    }

    formulaire_a_zero();

    if ( !etat.formulaire_toujours_affiche )
	gtk_widget_hide ( frame_droite_bas );

    gtk_widget_grab_focus ( TREE_VIEW_LISTE_OPERATIONS );

}
/******************************************************************************/




/******************************************************************************/
/* Fonction appelée quand une entry prend le focus */
/* si elle contient encore des éléments grisés, on les enlève */
/******************************************************************************/
gboolean entree_prend_focus ( GtkWidget *entree )
{

    /*     l'entrée qui arrive peut être un combofix */
    /* si le style est le gris, on efface le contenu de l'entrée, sinon on fait rien */

    if ( GTK_IS_ENTRY ( entree ))
    {
	if ( gtk_widget_get_style ( entree ) == style_entree_formulaire[ENGRIS] )
	{
	    gtk_entry_set_text ( GTK_ENTRY ( entree ), "" );
	    gtk_widget_set_style ( entree, style_entree_formulaire[ENCLAIR] );
	}
    }
    else
    {
	if ( gtk_widget_get_style ( GTK_COMBOFIX (entree) -> entry ) == style_entree_formulaire[ENGRIS] )
	{
	    gtk_combofix_set_text ( GTK_COMBOFIX ( entree ), "" );
	    gtk_widget_set_style ( GTK_COMBOFIX (entree) -> entry, style_entree_formulaire[ENCLAIR] );
	}
    }


    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
/* Fonction appelée quand une entry perd le focus */
/* si elle ne contient rien, on remet la fonction en gris */
/******************************************************************************/
gboolean entree_perd_focus ( GtkWidget *entree,
			     GdkEventFocus *ev,
			     gint *no_origine )
{
    gchar *texte;

    /*     on retire la sélection */

    gtk_editable_select_region ( GTK_EDITABLE ( entree ),
				 0,
				 0 );

    /*     on efface la popup du combofix si elle est affichée(arrive parfois pas trouvé pourquoi) */

    if ( GPOINTER_TO_INT ( no_origine ) == TRANSACTION_FORM_PARTY
	 ||
	 GPOINTER_TO_INT ( no_origine ) == TRANSACTION_FORM_CATEGORY
	 ||
	 GPOINTER_TO_INT ( no_origine ) == TRANSACTION_FORM_BUDGET )
    {
	GtkWidget *combofix;

	combofix = widget_formulaire_par_element ( GPOINTER_TO_INT ( no_origine ));

	gtk_grab_remove ( GTK_COMBOFIX ( combofix ) -> popup );
	gdk_pointer_ungrab ( GDK_CURRENT_TIME );
	gtk_widget_hide ( GTK_COMBOFIX ( combofix ) ->popup );
    }

    texte = NULL;

    switch ( GPOINTER_TO_INT ( no_origine ) )
    {
	/* on sort de la date, soit c'est vide, soit on la vérifie,
	   la complète si nécessaire et met à jour l'exercice */
	case TRANSACTION_FORM_DATE :

	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		modifie_date ( entree );

		/* si c'est une modif d'opé, on ne change pas l'exercice */
		/* 		si on n'utilise pas l'exo, la fonction ne fera rien */

		if ( !gtk_object_get_data ( GTK_OBJECT ( formulaire ),
					    "adr_struct_ope" ))
		    affiche_exercice_par_date( widget_formulaire_par_element (TRANSACTION_FORM_DATE),
					       widget_formulaire_par_element (TRANSACTION_FORM_EXERCICE) );
	    }
	    else
		texte = _("Date");
	    break;

	    /* on sort du tiers : soit vide soit complète le reste de l'opé */

	case TRANSACTION_FORM_PARTY :

	    if ( !completion_operation_par_tiers ( entree ))
		texte = _("Third party");
	    break;

	    /* on sort du débit : soit vide, soit change le menu des types s'il ne correspond pas */

	case TRANSACTION_FORM_DEBIT :

	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		/* on commence par virer ce qu'il y avait dans les crédits */

		if ( gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT) ) == style_entree_formulaire[ENCLAIR] )
		{
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT) ),
					 "" );
		    entree_perd_focus ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT),
					NULL,
					GINT_TO_POINTER ( TRANSACTION_FORM_CREDIT ) );
		}

		/* si c'est un menu de crédit, on y met le menu de débit */

		if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_TYPE ))
		{
		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

		    if ( ( GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) )
			   &&
			   GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ) -> menu ),
								   "signe_menu" ))
			   ==
			   2 )
			 ||
			 !GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ))
		    {
			/* on crée le nouveau menu et on met le défaut */

			GtkWidget *menu;

			if ( ( menu = creation_menu_types ( 1, compte_courant, 0  )))
			{
			    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ),
						       menu );
			    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ),
							  cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );
			    gtk_widget_show ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) );
			}
			else
			{
			    gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) );
			    gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );
			}

			/* comme il y a eu un changement de signe, on change aussi le type de l'opé associée */
			/* s'il est affiché */

			if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CONTRA )
			     &&
			     GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) )
			     &&
			     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ) -> menu ),
								     "signe_menu" ))
			     ==
			     1 )
			{
			    GtkWidget *menu;

			    menu = creation_menu_types ( 2,
							 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ),
												 "compte_virement" )),
							 2  );

			    if ( menu )
				gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ),
							   menu );
			    else
				gtk_option_menu_remove_menu ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ));
			}
		    }	
		    else
		    {
			/* on n'a pas recréé de menu, donc soit c'est déjà un menu de débit, soit tous les types */
			/* sont affichés, soit le widget n'est pas visible */
			/* on met donc le défaut, sauf si il y a qque chose dans les categ ou que le widget n'est pas visible */

			if ( GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) )
			     &&
			     ( !verifie_element_formulaire_existe ( TRANSACTION_FORM_CATEGORY )
			       ||
			       gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY) ) == style_entree_formulaire[ENGRIS] ))
			    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ),
							  cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );
		    }
		}
	    }
	    else
		texte = _("Debit");
	    break;

	    /* on sort du crédit : soit vide, soit change le menu des types
	       s'il n'y a aucun tiers ( <=> nouveau tiers ) */

	case TRANSACTION_FORM_CREDIT :

	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		/* on commence par virer ce qu'il y avait dans les débits */

		if ( gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT) ) == style_entree_formulaire[ENCLAIR] )
		{
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT) ),
					 "" );
		    entree_perd_focus ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT),
					NULL,
					GINT_TO_POINTER ( TRANSACTION_FORM_DEBIT ));
		}

		/* si c'est un menu de crédit, on y met le menu de débit,
		   sauf si tous les types sont affichés */

		if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_TYPE ))
		{
		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

		    if ( ( GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) )
			   &&
			   GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ) -> menu ),
								   "signe_menu" )) == 1 ) ||
			 !GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ))
		    {
			/* on crée le nouveau menu et on met le défaut */

			GtkWidget *menu;

			if ( ( menu = creation_menu_types ( 2, compte_courant, 0  ) ) )
			{
			    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ),
						       menu );
			    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ),
							  cherche_no_menu_type ( TYPE_DEFAUT_CREDIT ) );
			    gtk_widget_show ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) );
			}
			else
			{
			    gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) );
			    gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );
			}

			/* comme il y a eu un changement de signe, on change aussi le type de l'opé associée */
			/* s'il est affiché */

			if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CONTRA )
			     &&
			     GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) )
			     &&
			     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ) -> menu ),
								     "signe_menu" )) == 2 )
			{
			    GtkWidget *menu;

			    menu = creation_menu_types ( 1,
							 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ),
												 "compte_virement" )),
							 2  );

			    if ( menu )
				gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ),
							   menu );
			    else
				gtk_option_menu_remove_menu ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ));
			}
		    }
		    else
		    {
			/* on n'a pas recréé de menu, donc soit c'est déjà un menu de débit, soit tous les types */
			/* sont affichés, soit le widget n'est pas visible */
			/* on met donc le défaut, sauf si il y a qque chose dans les categ ou que le widget n'est pas visible */


			if ( GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) )
			     &&
			     ( !verifie_element_formulaire_existe ( TRANSACTION_FORM_CATEGORY )
			       ||
			       gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY) ) == style_entree_formulaire[ENGRIS] ))
			    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ),
							  cherche_no_menu_type ( TYPE_DEFAUT_CREDIT ) );
		    }
		}
	    }
	    else
		texte = _("Credit");
	    break;

	case TRANSACTION_FORM_VALUE_DATE :
	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		modifie_date ( entree );
	    else
		texte = _("Value date");
	    break;

	    /* sort des catégories : si c'est une opé ventilée, affiche le bouton de ventilation */
	    /* si c'est un virement affiche le bouton des types de l'autre compte */

	case TRANSACTION_FORM_CATEGORY :

	    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	    {
		if ( strcmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY) ))),
			      _("Breakdown of transaction") ))
		{
		    gchar **tableau_char;

		    gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_BREAKDOWN) );
		    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_EXERCICE ))
			gtk_widget_set_sensitive ( widget_formulaire_par_element (TRANSACTION_FORM_EXERCICE),
						   TRUE );
		    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_BUDGET ))
			gtk_widget_set_sensitive ( widget_formulaire_par_element (TRANSACTION_FORM_BUDGET),
						   TRUE );

		    /* vérification que ce n'est pas un virement */

		    tableau_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY) )),
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
			     && strlen ( tableau_char[1] ) )
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
				     compte_virement != compte_courant
				     &&
				     verifie_element_formulaire_existe ( TRANSACTION_FORM_CONTRA ))
				{
				    /* si le menu affiché est déjà celui du compte de virement, on n'y touche pas */

				    if ( !GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) )
					 ||
					 ( recupere_no_compte ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) )
					   !=
					   compte_virement ))
				    {
					/* vérifie quel est le montant entré, affiche les types opposés de l'autre compte */

					GtkWidget *menu;

					if ( gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT) ) == style_entree_formulaire[ENCLAIR] )
					    /* il y a un montant dans le crédit */
					    menu = creation_menu_types ( 1, compte_virement, 2  );
					else
					    /* il y a un montant dans le débit ou défaut */
					    menu = creation_menu_types ( 2, compte_virement, 2  );

					/* si un menu à été créé, on l'affiche */

					if ( menu )
					{
					    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ),
								       menu );
					    gtk_widget_show ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) );
					}

					/* on associe le no de compte de virement au formulaire pour le retrouver */
					/* rapidement s'il y a un chgt débit/crédit */

					gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ),
							      "compte_virement",
							      GINT_TO_POINTER ( compte_virement ));
				    }
				}
				else
				    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CONTRA ))
					gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) );
			    }
			    else
				if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CONTRA ))
				    gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) );
			}
			else
			    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CONTRA ))
				gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) );
		    }
		    else
			if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CONTRA ))
			    gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) );

		    g_strfreev ( tableau_char );
		}
		else
		{
		    gtk_widget_show ( widget_formulaire_par_element (TRANSACTION_FORM_BREAKDOWN) );
		    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_EXERCICE ))
			gtk_widget_set_sensitive ( widget_formulaire_par_element (TRANSACTION_FORM_EXERCICE),
						   FALSE );
		    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_BUDGET ))
			gtk_widget_set_sensitive ( widget_formulaire_par_element (TRANSACTION_FORM_BUDGET),
						   FALSE );
		}
	    }
	    else
		texte = _("Categories : Sub-categories");

	    break;

	case TRANSACTION_FORM_CHEQUE :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Cheque/Transfer number");
	    break;

	case TRANSACTION_FORM_BUDGET :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Budgetary line");
	    break;

	case TRANSACTION_FORM_VOUCHER :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Voucher");

	    break;

	case TRANSACTION_FORM_NOTES :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Notes");
	    break;

	case TRANSACTION_FORM_BANK :

	    if ( !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )))))
		texte = _("Bank references");
	    break;

	default :
	    break;
    }

    /* si l'entrée était vide, on remet le défaut */
    /* si l'origine était un combofix, il faut remettre le texte */
    /* avec le gtk_combofix (sinon risque de complétion), donc utiliser l'origine */

    if ( texte )
    {
	switch ( GPOINTER_TO_INT ( no_origine ))
	{
	    case TRANSACTION_FORM_PARTY :
	    case TRANSACTION_FORM_CATEGORY :
	    case TRANSACTION_FORM_BUDGET :
		gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_par_element (GPOINTER_TO_INT ( no_origine )) ),
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
gboolean clique_champ_formulaire ( GtkWidget *entree,
				   GdkEventButton *ev,
				   gint *no_origine )
{
    GtkWidget *popup_cal;
    struct struct_type_ope *type;

    /* on rend sensitif tout ce qui ne l'était pas sur le formulaire */

    degrise_formulaire_operations ();

    /* si l'entrée de la date est grise, on met la date courante seulement
       si la date réelle est grise aussi. Dans le cas contraire,
       c'est elle qui prend le focus */

    if ( gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_DATE) ) == style_entree_formulaire[ENGRIS] )
    {
	if ( !verifie_element_formulaire_existe ( TRANSACTION_FORM_VALUE_DATE )
	     ||
	     gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_VALUE_DATE) ) == style_entree_formulaire[ENGRIS] )
	    {
		if ( !derniere_date )
		    derniere_date = gsb_today();
		gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_DATE) ),
				     derniere_date );
		gtk_widget_set_style ( widget_formulaire_par_element (TRANSACTION_FORM_DATE),
				       style_entree_formulaire[ENCLAIR] );
	    }
    }


    /*   si le type par défaut est un chèque, on met le nouveau numéro */

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_TYPE )
	 &&
	 GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ))
    {
	type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ) -> menu_item ),
				     "adr_type" );

	if ( type -> numerotation_auto )
	{
	    entree_prend_focus ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );

	    if ( !strlen ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) ))))
		gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) ),
				     itoa ( type -> no_en_cours  + 1));
	}
    }

    /* si ev est null ( cad que ça ne vient pas d'un click mais appelé par ex
       à la fin de fin_edition ), on se barre */

    if ( !ev )
	return FALSE;

    /* énumération suivant l'entrée où on clique */

    switch ( GPOINTER_TO_INT ( no_origine ) )
    {
	case TRANSACTION_FORM_DATE :
	case TRANSACTION_FORM_VALUE_DATE :

	    /* si double click, on popup le calendrier */
	    if ( ev -> type == GDK_2BUTTON_PRESS )
	    {
		popup_cal = gsb_calendar_new ( entree );
		return TRUE;
	    }
	    break;

	default :

	    break;
    }
    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
gboolean touches_champ_formulaire ( GtkWidget *widget,
				    GdkEventKey *ev,
				    gint *no_origine )
{
    gint origine;
    gint element_suivant;
    GtkWidget *popup_cal;

    origine = GPOINTER_TO_INT ( no_origine );

    /* si etat.entree = 1, la touche entrée finit l'opération ( fonction par défaut ) */
    /* sinon elle fait comme tab */

    if ( !etat.entree && ( ev -> keyval == GDK_Return || ev -> keyval == GDK_KP_Enter ))
	ev->keyval = GDK_Tab ;

    switch ( ev -> keyval )
    {
	case GDK_Escape :		/* échap */

	    echap_formulaire();
	    break;


	    /* 	    touche haut */

	case GDK_Up:

	    element_suivant = recherche_element_suivant_formulaire ( origine,
								     2 );
	    widget_grab_focus_formulaire ( element_suivant );
	    return TRUE;
	    break;

	    /* 	    touche bas */

	case GDK_Down:

	    element_suivant = recherche_element_suivant_formulaire ( origine,
								     3 );
	    widget_grab_focus_formulaire ( element_suivant );
	    return TRUE;
	    break;


	    /* 	    tabulation inversée (+SHIFT) */

	case GDK_ISO_Left_Tab:

	    /* 	    on passe au widget précédent */

	    element_suivant = recherche_element_suivant_formulaire ( origine,
								     0 );
	    widget_grab_focus_formulaire ( element_suivant );

	    return TRUE;
	    break;

	    /* tabulation */

	case GDK_Tab :

	    /* une tabulation passe au widget affiché suivant */
	    /* et retourne à la date ou enregistre l'opé s'il est à la fin */

	    /* 	    si on est sur le débit et qu'on a entré qque chose dedans, on efface le crédit */

	    if ( origine == TRANSACTION_FORM_DEBIT
		 &&
		 strlen ( gtk_entry_get_text ( GTK_ENTRY ( widget ))))
	    {
		gtk_widget_set_style ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT),
				       style_entree_formulaire[ENGRIS] );
		gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT) ),
				     _("Credit") );
	    }
	    
	    /* 	    si on est sur le crédit et qu'on a entré qque chose dedans, on efface le débit */

	    if ( origine == TRANSACTION_FORM_CREDIT
		 &&
		 strlen ( gtk_entry_get_text ( GTK_ENTRY ( widget ))))
	    {
		gtk_widget_set_style ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT),
				       style_entree_formulaire[ENGRIS] );
		gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT) ),
				     _("Debit") );
	    }


	    element_suivant = recherche_element_suivant_formulaire ( origine,
								     1 );

	    if ( element_suivant == -2 )
		fin_edition();
	    else
		widget_grab_focus_formulaire ( element_suivant );

	    return TRUE;
	    break;

	    /* touches entrée */

	case GDK_KP_Enter :
	case GDK_Return :

	    /* si la touche CTRL est elle aussi active et si on est sur un champ
	     * de date, c'est que l'on souhaite ouvrir un calendrier */

	    if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK
		 &&
		 ( origine == TRANSACTION_FORM_DATE
		   ||
		   origine == TRANSACTION_FORM_VALUE_DATE ))
	    {
		popup_cal = gsb_calendar_new ( GTK_WIDGET ( GTK_ENTRY ( widget_formulaire_par_element (origine) ) ) );
		gtk_widget_grab_focus ( GTK_WIDGET ( popup_cal ) );
		return TRUE;
	    }

	    fin_edition();

	    return TRUE;
	    break;

	    /* touches + */

	case GDK_KP_Add:
	case GDK_plus:

	    /* si on est dans une entree de date, on augmente d'un jour
	       ou d'une semaine(si ctrl) la date */

	    switch ( origine )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    verifie_champs_dates ( origine );

		    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK ||
			 ev -> keyval != GDK_KP_Add )
			inc_dec_date ( widget, ONE_DAY );
		    else
			inc_dec_date ( widget, ONE_WEEK );

		    return TRUE;
		    break;

		case TRANSACTION_FORM_CHEQUE:

		    increment_decrement_champ ( widget,
						1 );
		    return TRUE;
		    break;
	    }
	    break;


	    /* touches - */

	case GDK_KP_Subtract:
	case GDK_minus:

	    /* si on est dans une entree de date, on diminue d'un jour
	       ou d'une semaine(si ctrl) la date */

	    switch ( origine )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    verifie_champs_dates ( origine );

		    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK ||
			 ev -> keyval != GDK_KP_Subtract  )
			inc_dec_date ( widget, -ONE_DAY );
		    else
			inc_dec_date ( widget, -ONE_WEEK );

		    return TRUE;
		    break;

		case TRANSACTION_FORM_CHEQUE:
		    
		    increment_decrement_champ ( widget,
						-1 );
		    return TRUE;
		    break;
	    }
	    break;



	    /* touche PgUp */

	case GDK_Page_Up :
	case GDK_KP_Page_Up :

	    /* si on est dans une entree de date, on augmente d'un mois 
	       ou d'un an (si ctrl) la date */

	    switch ( origine )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    verifie_champs_dates ( origine );

		    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK )
			inc_dec_date ( widget,
				       ONE_MONTH );
		    else
			inc_dec_date ( widget,
				       ONE_YEAR );

		    return TRUE;
		    break;
	    }
	    break;


	    /* touche PgDown */

	case GDK_Page_Down :
	case GDK_KP_Page_Down :

	    /* si on est dans une entree de date, on diminue d'un mois 
	       ou d'un an (si ctrl) la date */

	    switch ( origine )
	    {
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DATE:

		    verifie_champs_dates ( origine );

		    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK )
			inc_dec_date ( widget,
				       -ONE_MONTH );
		    else
			inc_dec_date ( widget,
				       -ONE_YEAR );

		    return TRUE;
		    break;
	    }
	    break;
    
    }
    return FALSE;
}
/******************************************************************************/



/******************************************************************************/
/* cette fonction vérifie s'il y a une date entrée dans le widget date */
/* dont le no est donné en argument */
/* si l'entrée est vide,  */
/*     soit c'est une entree de date de valeur */
/*     	on met la date de la date d'opération */
/* 	ou la date du jour si l'opé n'a pas de date */
/*    soit c'est la date d'opé, on met la date du jour */
/* \param origine le no du champ de date : */
/* 	TRANSACTION_FORM_DATE */
/* 	TRANSACTION_FORM_VALUE_DATE */
/******************************************************************************/
void verifie_champs_dates ( gint origine )
{
    GtkWidget *widget;

    switch ( origine )
    {
	case TRANSACTION_FORM_VALUE_DATE:

	    /* si la date de valeur est vide, alors on
	       récupère la date d'opération comme date de valeur */

	    widget = widget_formulaire_par_element ( origine );

	    if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( widget ))))
	    {
		/* si la date d'opération est vide elle aussi */
		/* 		    on met la date du jour dans les 2 champs */

		GtkWidget *entree_date;

		entree_date = widget_formulaire_par_element (TRANSACTION_FORM_DATE);

		if ( gtk_widget_get_style (entree_date) == style_entree_formulaire[ENCLAIR]
		     &&
		     strlen ( gtk_entry_get_text ( GTK_ENTRY (entree_date))))
		{
		    /* il y a qque chose dans la date, on le recopie */

		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 gtk_entry_get_text ( GTK_ENTRY (entree_date)));
		}
		else
		{
		    /* il n'y a rien dans la date, on y met la date du jour ainsi que dans la date de valeur */

		    entree_prend_focus ( entree_date );
		    gtk_entry_set_text ( GTK_ENTRY ( entree_date ),
					 gsb_today() );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 gsb_today() );
		}
	    }
	    break;


	case TRANSACTION_FORM_DATE:

	    /* si le champ est vide, alors on
	       récupère la date du jour comme date d'opération */

	    widget = widget_formulaire_par_element ( origine );

	    if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( widget ))))
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     gsb_today() );
	    break;
    }
}
/******************************************************************************/


/******************************************************************************/
/* renvoie le no d'élément dans le formulaire correspondant à l'élément suivant */
/* dans le sens donné en argument */
/* l'élément retourné peut recevoir le focus et il est affiché */
/* \param element_courant le no d'élément d'origine */
/* \param sens_deplacement le sens de déplacement pour retrouver l'élément suivant 
 * 				(0=gauche, 1=droite, 2=haut, 3=bas) */
/* \return no de l'élément qui recevra le focus,
 * 		-1 en cas de pb ou pas de changement
 * 		-2 en cas de fin d'opération (tab sur bout du formulaire)*/
/******************************************************************************/
gint recherche_element_suivant_formulaire ( gint element_courant,
					    gint sens_deplacement )
{
    struct organisation_formulaire *organisation_formulaire;
    gint ligne;
    gint colonne;
    gint no_element_retour;

    organisation_formulaire = renvoie_organisation_formulaire ();

    if ( !recherche_place_element_formulaire ( organisation_formulaire,
					       element_courant,
					       &ligne,
					       &colonne ))
	return -1;

    no_element_retour = 0;

    while ( !element_focusable ( no_element_retour )) 
    {
	switch ( sens_deplacement )
	{
	    /* 	    gauche */
	    case 0:

		/* 		on recherche l'élément précédent, si on est en haut à gauche */
		/* 		    on passe en bas à droite */

		if ( !colonne && !ligne )
		{
		    colonne = organisation_formulaire -> nb_colonnes;
		    ligne = organisation_formulaire -> nb_lignes -1; 
		}

		if ( --colonne == -1 )
		{
		    colonne = organisation_formulaire -> nb_colonnes - 1;
		    ligne--;
		}
		no_element_retour = organisation_formulaire -> tab_remplissage_formulaire[ligne][colonne];
		break;

		/* 		droite */
	    case 1:

		/* 		on recherche l'élément suivant */
		/* 		    si on est en bas à droite, on passe en haut à gauche */
		/* 		ou on enregistre l'opé en fonction de la conf */

		if ( colonne == organisation_formulaire -> nb_colonnes - 1
		     &&
		     ligne == organisation_formulaire -> nb_lignes - 1 )
		{
		    if ( !etat.entree )
		    {
			no_element_retour = -2;
			continue;
		    }

		    colonne = -1;
		    ligne = 0; 
		}

		if ( ++colonne == organisation_formulaire -> nb_colonnes )
		{
		    colonne = 0;
		    ligne++;
		}
		no_element_retour = organisation_formulaire -> tab_remplissage_formulaire[ligne][colonne];
		break;

		/* 		haut */
	    case 2:

		if ( !ligne )
		{
		    no_element_retour = -1;
		    continue;
		}

		ligne--;
		no_element_retour = organisation_formulaire -> tab_remplissage_formulaire[ligne][colonne];
		break;

		/* 		bas */
	    case 3:

		if ( ligne == organisation_formulaire -> nb_lignes - 1)
		{
		    no_element_retour = -1;
		    continue;
		}

		ligne++;
		no_element_retour = organisation_formulaire -> tab_remplissage_formulaire[ligne][colonne];
		break;

	    default:
		no_element_retour = -1;
	}
    }
    return no_element_retour;
}
/******************************************************************************/



/******************************************************************************/
/* vérifie si l'élément du formulaire donné en argument peut recevoir le focus */
/* \param no_element l'élément à tester */
/* \return TRUE s'il peut recevoir le focus */
/******************************************************************************/
gboolean element_focusable ( gint no_element )
{
    GtkWidget *widget;

    /*     si le no_element est -1 ou -2, on renvoie TRUE */
    
    if ( no_element == -1
	 ||
	 no_element == -2 )
	return TRUE;

    widget = widget_formulaire_par_element ( no_element );

    if ( !widget )
	return FALSE;
    
    if ( !GTK_WIDGET_VISIBLE (widget))
	return FALSE;

    if ( !GTK_WIDGET_SENSITIVE ( widget ))
	return FALSE;

    if ( !(GTK_IS_COMBOFIX ( widget )
	   ||
	   GTK_IS_ENTRY ( widget )
	   ||
	   GTK_IS_BUTTON ( widget )))
	return FALSE;

    return TRUE;
}
/******************************************************************************/



/******************************************************************************/
/* met le focus sur le widget du formulaire dont le no est donné en argument */
/******************************************************************************/
void widget_grab_focus_formulaire ( gint no_element )
{
    GtkWidget *widget;

    widget = widget_formulaire_par_element ( no_element );

    if ( !widget )
	return;
    
    if ( GTK_IS_COMBOFIX ( widget ))
	gtk_widget_grab_focus ( GTK_COMBOFIX ( widget ) -> entry );
    else
	gtk_widget_grab_focus ( widget );

    return;
}
/******************************************************************************/


/******************************************************************************/
/* Fonction completion_operation_par_tiers                                    */
/* appelée lorsque le tiers perd le focus                                     */
/* récupère le tiers, et recherche la dernière opé associée à ce tiers        */
/* remplit le reste de l'opération avec les dernières données                 */
/* \return FALSE pas de tiers, ou pb */
/******************************************************************************/
gboolean completion_operation_par_tiers ( GtkWidget *entree )
{
    struct struct_tiers *tiers;
    struct structure_operation *operation;
    gchar *char_tmp;
    gint i,j;
    struct organisation_formulaire *organisation_formulaire;
    gint pas_de_completion;
    gchar *nom_tiers;
    GtkWidget *widget;
    GtkWidget *menu;


    /*     s'il n'y a rien dans le tiers, on retourne FALSE */

    nom_tiers = g_strstrip ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entree ))));

    if ( !strlen ( nom_tiers ))
	return FALSE;

    /*     pour la complétion, seul la date et le tiers doivent être remplis, sinon on ne fait rien */

    organisation_formulaire = renvoie_organisation_formulaire ();
    pas_de_completion = 0;

    for ( i=0 ; i < organisation_formulaire -> nb_lignes ; i++ )
	for ( j=0 ; j < organisation_formulaire -> nb_colonnes ; j++ )
	{
	    if ( organisation_formulaire -> tab_remplissage_formulaire[i][j]
		 &&
		 organisation_formulaire -> tab_remplissage_formulaire[i][j] != TRANSACTION_FORM_DATE
		 &&
		 organisation_formulaire -> tab_remplissage_formulaire[i][j] != TRANSACTION_FORM_PARTY )
	    {
		GtkWidget *widget;

		widget = widget_formulaire_par_element ( organisation_formulaire -> tab_remplissage_formulaire[i][j] );

		if ( GTK_IS_ENTRY ( widget ))
		{
		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			pas_de_completion = 1;
		}
		if ( GTK_IS_COMBOFIX ( widget ))
		{
		    if ( gtk_widget_get_style ( GTK_COMBOFIX (widget) -> entry ) == style_entree_formulaire[ENCLAIR] )
			pas_de_completion = 1;
		}
	    }
	}

    if ( pas_de_completion )
	return TRUE;
    

    /* recherche le tiers demandé */

    tiers = tiers_par_nom ( nom_tiers,
			    0 );

    /*   si nouveau tiers,  on s'en va simplement */

    if ( !tiers )
	return TRUE;

    /*     on essaie de retrouver la dernière opé entrée avec ce tiers */

    operation = recherche_derniere_occurence_tiers ( tiers -> no_tiers );

    /* si on n'a trouvé aucune opération, on se tire */

    if ( !operation )
	return TRUE;

    /*     on fait le tour du formulaire en ne remplissant que ce qui est nécessaire */

    for ( i=0 ; i < organisation_formulaire -> nb_lignes ; i++ )
	for ( j=0 ; j <  organisation_formulaire -> nb_colonnes ; j++ )
	{
	    widget =  widget_formulaire_par_element ( organisation_formulaire -> tab_remplissage_formulaire[i][j] );

	    switch ( organisation_formulaire -> tab_remplissage_formulaire[i][j] )
	    {
		case TRANSACTION_FORM_DEBIT:

		    if ( operation -> montant < 0 )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     g_strdup_printf ( "%4.2f",
							       -operation -> montant ));
		    }
		    break;

		case TRANSACTION_FORM_CREDIT:

		    if ( operation -> montant >= 0 )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     g_strdup_printf ( "%4.2f",
							       operation -> montant ));
		    }
		    break;

		case TRANSACTION_FORM_CATEGORY:

		    if ( operation -> operation_ventilee )
		    {
			/* 			c'est une opé ventilée, on met la catég, affiche le bouton, et propose de */
			/* 			    récupérer les opé filles */

			entree_prend_focus ( widget );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Breakdown of transaction") );
			gtk_widget_show ( widget_formulaire_par_element (TRANSACTION_FORM_BREAKDOWN) );

			recuperation_ope_filles_completion_tiers ( operation );
		    }
		    else
		    {
			if ( operation -> relation_no_operation )
			{
			    /* c'est un virement */

			    entree_prend_focus ( widget );

			    if ( operation -> relation_no_operation != -1 )
			    {
				p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;
				gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
							g_strconcat ( COLON(_("Transfer")),
								      NOM_DU_COMPTE,
								      NULL ));
			    }
			    else
				gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
							_("Transfer") );
			}
			else
			{
			    /* c'est des catégories normales */

			    char_tmp = nom_categ_par_no ( operation -> categorie,
							  operation -> sous_categorie );
			    if ( char_tmp )
			    {
				entree_prend_focus ( widget );
				gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
							char_tmp );
			    }
			}
		    }
		    break;

		case TRANSACTION_FORM_BUDGET:

		    char_tmp = nom_imputation_par_no ( operation -> imputation,
						       operation -> sous_imputation );
		    if ( char_tmp )
		    {
			entree_prend_focus ( widget );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						char_tmp );
		    }
		    break;

		case TRANSACTION_FORM_NOTES:

		    if ( operation -> notes )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     operation -> notes );
		    }
		    break;

		case TRANSACTION_FORM_TYPE:

		    if ( operation -> montant < 0 )
			menu = creation_menu_types ( 1, compte_courant, 0  );
		    else
			menu = creation_menu_types ( 2, compte_courant, 0  );

		    if ( menu )
		    {
			/* on met en place les types et se place sur celui correspondant à l'opé */

			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
						   menu );
			gtk_widget_show ( widget );

			place_type_formulaire ( operation -> type_ope,
						TRANSACTION_FORM_TYPE,
						NULL );
		    }
		    else
		    {
			gtk_widget_hide ( widget );
			gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );
		    }

		    break;

		case TRANSACTION_FORM_DEVISE:

		    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
						  g_slist_index ( liste_struct_devises,
								  devise_par_no ( operation -> devise )));
		    verification_bouton_change_devise ();
		    break;

		case TRANSACTION_FORM_BANK:

		    if ( operation -> info_banque_guichet )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     operation -> info_banque_guichet );
		    }
		    break;

		case TRANSACTION_FORM_VOUCHER:

		    if ( operation -> no_piece_comptable )
		    {
			entree_prend_focus ( widget );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     operation -> no_piece_comptable );
		    }
		    break;

		case TRANSACTION_FORM_CONTRA:

		    if ( operation -> relation_no_operation )
		    {
			if ( operation -> montant < 0 )
			    menu = creation_menu_types ( 2, operation -> relation_no_compte, 0  );
			else
			    menu = creation_menu_types ( 1, operation -> relation_no_compte, 0  );

			if ( menu )
			{
			    struct structure_operation *contre_operation;

			    /* on met en place les types et se place sur celui correspondant à l'opé */

			    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
						       menu );
			    gtk_widget_show ( widget );

			    contre_operation = operation_par_no ( operation -> relation_no_operation,
								  operation -> relation_no_compte );
			    if ( contre_operation )
				place_type_formulaire ( contre_operation -> type_ope,
							TRANSACTION_FORM_CONTRA,
							NULL );
			}
			else
			    gtk_widget_hide ( widget );
		    }
		    break;
	    }
	}
    return TRUE;
}
/******************************************************************************/




/******************************************************************************/
/* cette fonction est appelée lorsque l'option menu des devises du formulaire a changé */
/* elle vérifie si la devise choisie à un taux de change fixe avec la devise du compte */
/* sinon elle affiche le bouton de change */
/******************************************************************************/
void verification_bouton_change_devise ( void )
{
    struct struct_devise *devise_compte;
    struct struct_devise *devise;

    /*   si la devise n'est pas celle du compte ni l'euro si le compte va y passer, affiche le bouton change */

    devise_compte = devise_par_no ( DEVISE );
    devise = g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_DEVISE)) -> menu_item ),
				 "adr_devise" );

    if ( !( devise -> no_devise == DEVISE
	    ||
	    ( devise_compte -> passage_euro && !strcmp ( devise -> nom_devise, _("Euro") ))
	    ||
	    ( !strcmp ( devise_compte -> nom_devise, _("Euro") ) && devise -> passage_euro )))
	gtk_widget_show ( widget_formulaire_par_element (TRANSACTION_FORM_CHANGE) );
    else
	gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CHANGE) );
}
/******************************************************************************/


/******************************************************************************/
/* cette fonction recherche la dernière occurence du tiers donné en argument */
/* commence la recherche dans le comptes courant */
/* puis fait le tour de tous les comptes si elle n'a pas trouvé */
/******************************************************************************/
struct structure_operation *recherche_derniere_occurence_tiers ( gint no_tiers )
{
    struct structure_operation *operation;
    GSList *pointeur_ope;
    struct structure_operation *ope_test;
    gint i;

    /* on fait d'abord le tour du compte courant pour recherche une opé avec ce tiers */
    /* s'il n'y a aucune opé correspondante, on fait le tour de tous les comptes */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    operation = NULL;
    pointeur_ope = LISTE_OPERATIONS;

    while ( pointeur_ope )
    {
	ope_test = pointeur_ope -> data;

	if ( ope_test -> tiers == no_tiers
	     &&
	     !ope_test -> no_operation_ventilee_associee )
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
	/* on recherche dans les autres comptes, la première trouvée fera l'affaire */
	/* FIXME : tester si c'est beaucoup plus long sur un gros fichier de récupérer la dernière opé entrée... */

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    if ( i != compte_courant )
	    {
		pointeur_ope = LISTE_OPERATIONS;

		while ( pointeur_ope )
		{
		    ope_test = pointeur_ope -> data;

		    if ( ope_test -> tiers == no_tiers )
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

    return operation;
}
/******************************************************************************/



/******************************************************************************/
/* cette fonction est appelée lors de la complétion par tiers si l'opé de complétion */
/* est une ventilation. propose de récupérer les opés filles */
/******************************************************************************/
void recuperation_ope_filles_completion_tiers ( struct structure_operation *operation )
{
    gint result;
    GSList *liste_des_opes_de_ventilation;
    GSList *pointeur_ope;

    /* affiche la question de récupération */

    result = question_yes_no_hint ( _("Recover breakdown?"),
				    _("This is a breakdown of transaction, associated transactions can be recovered as in last transaction with this third party.") );

    if ( !result )
	return;

    /* récupération des anciennes opés filles de la ventilation */

    liste_des_opes_de_ventilation = NULL;
    pointeur_ope = LISTE_OPERATIONS;

    while ( pointeur_ope )
    {
	struct structure_operation *ope_test;

	ope_test = pointeur_ope -> data;

	if ( ope_test -> no_operation_ventilee_associee == operation -> no_operation )
	{
	    struct struct_ope_ventil *nouvelle_operation;

	    nouvelle_operation = calloc ( 1,
					  sizeof ( struct struct_ope_ventil ));

	    nouvelle_operation -> montant = ope_test -> montant;
	    nouvelle_operation -> categorie = ope_test -> categorie;
	    nouvelle_operation -> sous_categorie = ope_test -> sous_categorie;

	    if ( ope_test -> notes )
		nouvelle_operation -> notes = g_strdup ( ope_test -> notes );

	    nouvelle_operation -> no_exercice = ope_test -> no_exercice;
	    nouvelle_operation -> imputation = ope_test -> imputation;
	    nouvelle_operation -> sous_imputation = ope_test -> sous_imputation;

	    if ( ope_test -> no_piece_comptable )
		nouvelle_operation -> no_piece_comptable = g_strdup ( ope_test -> no_piece_comptable );


	    /* 		    on met par_completion à 1 pour éviter de les effacer si on va dans l'échéance */
	    /* 			et qu'on annule */

	    nouvelle_operation -> par_completion = 1;

	    /* si c'est un virement, on met le compte de virement et le type choisi */

	    if ( ope_test -> relation_no_operation )
	    {
		struct structure_operation *contre_operation;

		nouvelle_operation -> relation_no_operation = -1;
		nouvelle_operation -> relation_no_compte = ope_test -> relation_no_compte;

		/*  on va chercher le type de l'opé associée */

		contre_operation = operation_par_no ( ope_test -> relation_no_operation,
						      ope_test -> relation_no_compte );

		if ( contre_operation )
		    nouvelle_operation -> no_type_associe = contre_operation -> type_ope;
	    }

	    liste_des_opes_de_ventilation = g_slist_append ( liste_des_opes_de_ventilation,
							     nouvelle_operation );
	}
	pointeur_ope = pointeur_ope -> next;
    }

    if ( liste_des_opes_de_ventilation )
	gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			      "liste_adr_ventilation",
			      liste_des_opes_de_ventilation );
    else
	gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			      "liste_adr_ventilation",
			      GINT_TO_POINTER ( -1 ) );
}
/******************************************************************************/


/******************************************************************************/
/* place l'option_menu des types de paiement sur le no de type donné en argument */
/* s'il ne le trouve pas met le type par défaut du compte */
/* \param no_type le no de type voulu */
/* \param no_option_menu TRANSACTION_FORM_TYPE ou TRANSACTION_FORM_CONTRA */
/* \param contenu le contenu du type à afficher si nécessaire */
/******************************************************************************/
void place_type_formulaire ( gint no_type,
			     gint no_option_menu,
			     gchar *contenu )
{
    gint place_type;
    struct struct_type_ope *type;
    GtkWidget *option_menu;
    GtkWidget *entree_cheque;

    option_menu = widget_formulaire_par_element (no_option_menu);

    /* recherche le type de l'opé */

    place_type = cherche_no_menu_type ( no_type );

    /* si aucun type n'a été trouvé, on cherche le défaut */

    if ( place_type == -1 )
    {
	gint no_compte;
	gint signe;

	no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu ),
							    "no_compte"));
	signe = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu ),
							    "signe_menu"));

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

	if ( signe == 1 )
	    place_type = cherche_no_menu_type ( TYPE_DEFAUT_DEBIT );
	else
	    place_type = cherche_no_menu_type ( TYPE_DEFAUT_CREDIT );

	/* si le type par défaut n'est pas trouvé, on met 0 */

	if ( place_type == -1 )
	    place_type = 0;
    }

    /*       à ce niveau, place type est mis */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu ),
				  place_type );

    /*     si on est sur le contre type, on vire ici car on ne donne pas la possibilité de rentrer un commentaire */

    if ( no_option_menu == TRANSACTION_FORM_CONTRA )
	return;

    /* récupère l'adr du type pour mettre un n° auto si nécessaire */

    type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
				 "adr_type" );

    if ( type -> affiche_entree )
    {
	entree_cheque = widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE);

	if ( contenu )
	{
	    entree_prend_focus ( entree_cheque );
	    gtk_entry_set_text ( GTK_ENTRY ( entree_cheque ),
				 contenu );
	}
	else
	    if ( type -> numerotation_auto )
	    {
		entree_prend_focus ( entree_cheque );
		gtk_entry_set_text ( GTK_ENTRY ( entree_cheque ),
				     itoa ( type -> no_en_cours + 1 ));
	    }

	gtk_widget_show ( entree_cheque );
    }
}
/******************************************************************************/



/******************************************************************************/
/* procédure appelée lors de la fin de l'édition d'opération */
/* crée une nouvelle opération à partir du formulaire */
/* puis appelle ajout_operation pour la sauver */
/******************************************************************************/
gboolean fin_edition ( void )
{
    struct structure_operation *operation;
    gint modification;
    GSList *liste_no_tiers;
    GSList *liste_tmp;

    /* récupération de l'opération : soit l'adr de la struct, soit NULL si nouvelle */

    operation = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				      "adr_struct_ope" );

    if ( operation )
	modification = 1;
    else
	modification = 0;

    /*     on donne le focus à la liste, pour que le widget en cours perde le focus */
    /* 	et applique les modifs nécessaires */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    gtk_widget_grab_focus ( TREE_VIEW_LISTE_OPERATIONS );
    
   /* on commence par vérifier que les données entrées sont correctes */
    /* si la fonction renvoie false, c'est qu'on doit arrêter là */

    if ( !verification_validation_operation ( operation ))
	return FALSE;

    /* si le tiers est un état, on va faire autant d'opérations qu'il y a de tiers
       dans l'état concerné. On va créer une liste avec les nos de tiers
       ( ou -1, le tiers n'est pas un état), puis on fera une boucle sur cette liste
       pour ajouter autant d'opérations que de tiers */

    liste_no_tiers = NULL;

    /*     on vérifie que le champ tiers existe... */

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_PARTY ))
    {
	if ( strncmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_PARTY) ))),
		       COLON(_("Report")),
		       7 ))
	    /*     ce n'est pas un état, on met -1 comme no de tiers */
	    liste_no_tiers = g_slist_append (liste_no_tiers,
					     GINT_TO_POINTER ( -1 ));
	else
	{
	    /* c'est bien un état */
	    /* on commence par retrouver le nom de l'état */
	    /* toutes les vérifications ont été faites précédemment */

	    gchar **tableau_char;
	    struct struct_etat *etat;
	    GSList *liste_opes_selectionnees;

	    tableau_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_PARTY) ))),
					":",
					2 );

	    if ( tableau_char[1] )
	    {
		tableau_char[1] = g_strstrip ( tableau_char[1] );
		liste_tmp = liste_struct_etats;
		etat = NULL;

		while ( liste_tmp )
		{
		    etat = liste_tmp -> data;

		    if ( !strcmp ( etat -> nom_etat,
				   tableau_char[1] ))
			liste_tmp = NULL;
		    else
			liste_tmp = liste_tmp -> next;
		}

		g_strfreev ( tableau_char );

		/* à ce niveau, etat contient l'adr le la struct de l'état choisi */

		liste_opes_selectionnees = recupere_opes_etat ( etat );

		liste_tmp = liste_opes_selectionnees;

		while ( liste_tmp )
		{
		    struct structure_operation *operation;

		    operation = liste_tmp -> data;

		    if ( !g_slist_find ( liste_no_tiers,
					 GINT_TO_POINTER ( operation -> tiers )))
			liste_no_tiers = g_slist_append ( liste_no_tiers,
							  GINT_TO_POINTER ( operation -> tiers ));

		    liste_tmp = liste_tmp -> next;
		}

		g_slist_free ( liste_opes_selectionnees );
	    }
	}
    }
    else
	/* 	il n'y a pas de tiers, donc ce n'est pas un état */
	liste_no_tiers = g_slist_append (liste_no_tiers,
					 GINT_TO_POINTER ( -1 ));


    /* à ce niveau, liste_no_tiers contient la liste des no de tiers pour chacun desquels on */
    /* fera une opé, ou -1 si on utilise que le tiers dans l'entrée du formulaire */
    /* on fait donc le tour de cette liste en ajoutant l'opé à chaque fois */

    liste_tmp = liste_no_tiers;

    while ( liste_tmp )
    {
	/* soit on va chercher le tiers dans la liste des no de tiers et on le met dans le formulaire, */
	/* soit on laisse tel quel et on met liste_tmp à NULL */

	if ( liste_tmp -> data == GINT_TO_POINTER ( -1 ) )
	    liste_tmp = NULL;
	else
	{
	    if ( !liste_tmp -> data )
	    {
		dialogue_error ( _("No third party selected for this report."));
		return FALSE;
	    }
	    else
	    {
		gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_PARTY) ),
					tiers_name_by_no ( GPOINTER_TO_INT (liste_tmp -> data), TRUE ));

		/* si le moyen de paiement est à incrémentation automatique, à partir de la 2ème opé, */
		/* on incrémente le contenu (no de chèque en général) */

		/* comme c'est une liste de tiers, c'est forcemment une nouvelle opé */
		/* donc si operation n'est pas nul, c'est qu'on n'est pas sur la 1ère */
		/* donc on peut incrémenter si nécessaire le contenu */
		/* et on peut récupérer le no_type de l'ancienne opé */

		if ( operation )
		{
		    struct struct_type_ope *type;

		    type = type_ope_par_no ( operation -> type_ope,
					     compte_courant );

		    if ( type
			 &&
			 type -> affiche_entree
			 &&
			 type -> numerotation_auto
			 &&
			 verifie_element_formulaire_existe ( TRANSACTION_FORM_CHEQUE ))
			gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) ),
					     itoa ( type -> no_en_cours + 1 ));
		}
		liste_tmp = liste_tmp -> next;
	    }
	}

	/* si c'est une nouvelle opé, on la crée en mettant tout à 0 sauf le no de compte et la devise */

	if ( !modification )
	{
	    operation = calloc ( 1,
				 sizeof ( struct structure_operation ) );
	    if ( !operation )
	    {
		dialogue_error ( _("Cannot allocate memory, bad things will happen soon") );
		return FALSE;
	    }
	    operation -> no_compte = compte_courant;

	    /* 	    la devise par défaut est celle du compte, elle sera modifiée si nécessaire plus tard */

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

	    operation -> devise = DEVISE;
	}

	/* on récupère les données du formulaire sauf la categ qui est traitée plus tard */

	recuperation_donnees_generales_formulaire ( operation );

	/* il faut ajouter l'opération à la liste à ce niveau pour lui attribuer un numéro */
	/* celui ci sera utilisé si c'est un virement ou si c'est une ventil qui contient des */
	/* virements */

	if ( modification )
	    modifie_operation ( operation );
	else
	    ajout_operation ( operation );

	/*   récupération des catégories / sous-catég, s'ils n'existent pas, on les crée */
	/* à mettre en dernier car si c'est une opé ventilée, chaque opé de ventil va récupérer les données du dessus */

	recuperation_categorie_formulaire ( operation,
					    modification );
    }

    /* on libère la liste des no tiers */

    g_slist_free ( liste_no_tiers );

    /* si on est en train d'équilibrer => recalcule le total pointé */

    if ( etat.equilibrage )
	calcule_total_pointe_compte ( compte_courant );


    /* si c'était une nouvelle opération, on efface le formulaire,
       on remet la date pour la suivante,
       si c'était une modification, on redonne le focus à la liste */

    formulaire_a_zero ();

    if ( modification )
    {
	if ( !etat.formulaire_toujours_affiche )
	    gtk_widget_hide ( frame_droite_bas );
    }
    else
    {
	/* 	c'est une nouvelle opé, on sauvegarde la dernière date */

	GtkWidget *entree_date;

	entree_date = widget_formulaire_par_element (TRANSACTION_FORM_DATE);

	derniere_date = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entree_date )));

	clique_champ_formulaire ( entree_date,
				  NULL,
				  GINT_TO_POINTER ( TRANSACTION_FORM_DATE ) );
	gtk_entry_select_region ( GTK_ENTRY ( entree_date ), 0, -1);
	gtk_widget_grab_focus ( GTK_WIDGET ( entree_date ) );
    }


    /* met à jour les listes ( nouvelle opération associée au tiers et à la catégorie ) */

    if ( mise_a_jour_combofix_tiers_necessaire )
	mise_a_jour_combofix_tiers ();
    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ ();
    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();

    /*     on affiche un avertissement si nécessaire */

    affiche_dialogue_soldes_minimaux ();

    modification_fichier ( TRUE );
    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
/* vérifie que les données dans le formulaire sont valides pour enregistrer   */
/* l'opération appelée lors de la fin_edition d'une opé                       */
/******************************************************************************/
gint verification_validation_operation ( struct structure_operation *operation )
{
    gchar **tableau_char;
    GSList *liste_tmp;


    /* on vérifie qu'il y a bien une date */

    if ( gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_DATE) ) != style_entree_formulaire[ENCLAIR] )
    {
	dialogue_error ( _("You must enter a date.") );
	return (FALSE);
    }

    /* vérifie que la date est correcte */

    if ( !modifie_date ( widget_formulaire_par_element (TRANSACTION_FORM_DATE) ))
    {
	dialogue_error ( _("Invalid date") );
	gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_DATE) ),
				  0,
				  -1);
	gtk_widget_grab_focus ( widget_formulaire_par_element (TRANSACTION_FORM_DATE) );
	return (FALSE);
    }


    /* vérifie que la date de valeur est correcte */

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_VALUE_DATE )
	 &&
	 gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_VALUE_DATE) ) == style_entree_formulaire[ENCLAIR]
	 &&
	 !modifie_date ( widget_formulaire_par_element (TRANSACTION_FORM_VALUE_DATE) ) )
    {
	dialogue_error ( _("Invalid value date.") );
	gtk_entry_select_region ( GTK_ENTRY (  widget_formulaire_par_element (TRANSACTION_FORM_VALUE_DATE) ),
				  0,
				  -1);
	gtk_widget_grab_focus ( widget_formulaire_par_element (TRANSACTION_FORM_VALUE_DATE) );
	return (FALSE);
    }

    /* vérification que ce n'est pas un virement sur lui-même */
    /* et que le compte de virement existe */

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CATEGORY )
	 &&
	 gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY) ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	tableau_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY) )),
				    ":",
				    2 );

	tableau_char[0] = g_strstrip ( tableau_char[0] );

	if ( tableau_char[1] )
	    tableau_char[1] = g_strstrip ( tableau_char[1] );


	/* Si c'est un virement, on fait les vérifications */
	
	if ( !strcmp ( tableau_char[0], _("Transfer") ) )
	{
	    /* S'il n'y a rien après "Transfer", alors : */
	    
	    if ( !tableau_char[1] ||
		 !strlen ( tableau_char[1] ) )
	    {
		dialogue_error ( _("There is no associated account for this transfer.") );
		return (FALSE);
	    }
	    
	    /* si c'est un virement vers un compte supprimé, laisse passer */

	    if ( strcmp ( tableau_char[1],
			  _("Deleted account") ) )
	    {
		/* recherche le no de compte du virement */

		gint compte_virement;
		gint i;

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
		    return (FALSE);
		}

		if ( compte_virement == compte_courant )
		{
		    dialogue_error ( _("Can't issue a transfer its own account.") );
		    return (FALSE);
		}

		/* 		    vérifie si le compte n'est pas clos */

		p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_virement;

		if ( COMPTE_CLOTURE )
		{
		    dialogue_error ( _("Can't issue a transfer on a closed account." ));
		    return ( FALSE );
		}
	    }
	}
	g_strfreev ( tableau_char );
    }

    /* pour les types qui sont à incrémentation automatique ( surtout les chèques ) */
    /* on fait le tour des operations pour voir si le no n'a pas déjà été utilisé */
    /* si operation n'est pas nul, c'est une modif donc on ne fait pas ce test */

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CHEQUE )
	 &&
	 GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) ))
    {
	struct struct_type_ope *type;

	type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ) -> menu_item ),
				     "adr_type" );

	if ( type -> numerotation_auto )
	{
	    struct structure_operation *operation_tmp;

	    /* vérifie s'il y a quelque chose */

	    if ( gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) ) == style_entree_formulaire[ENGRIS] )
	    {
		if ( question ( _("Selected method of payment has an automatic incremental number\nbut doesn't contain any number.\nContinue anyway?") ) )
		    goto sort_test_cheques;
		else
		    return (FALSE);
	    }

	    /* vérifie si le no de chèque n'est pas déjà utilisé */

	    operation_tmp = operation_par_cheque ( my_atoi ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) )))),
						   compte_courant );

	    /* si on a trouvé le même no de chèque, si c'est une nouvelle opé, c'est pas normal, */
	    /* si c'est une modif d'opé, c'est normal que si c'est cette opé qu'on a trouvé */

	    if ( operation_tmp
		 &&
		 (!operation
		  ||
		  operation_tmp -> no_operation != operation -> no_operation ))
	    {
		if ( question ( _("Warning: this cheque number is already used.\nContinue anyway?") ))
		    goto sort_test_cheques;
		else
		    return (FALSE);
	    }
	}
    }

sort_test_cheques :

    /* vérifie  si c'est une opération ventilée, */
    /* si c'est le cas, si la liste des ventilation existe (soit adr de liste, soit -1), on va l'enregistrer plus tard */
    /* sinon on va ventiler tout de suite */

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CATEGORY )
	 &&
	 GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_BREAKDOWN))
	 &&
	 !gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				"liste_adr_ventilation" ))
    {
	enregistre_ope_au_retour = 1;

	if ( gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT) ) == style_entree_formulaire[ENCLAIR] )
	    ventiler_operation ( -my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT))),
					      NULL ));
	else
	    ventiler_operation ( my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT))),
					     NULL ));

	return (FALSE);
    }


    /* on vérifie si le tiers est un état, que c'est une nouvelle opération */

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_PARTY )
	 &&
	 !strncmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_PARTY) ))),
		    COLON(_("Report")),
		    7 ))
    {
	gint trouve;

	/* on vérifie d'abord si c'est une modif d'opé */

	if ( operation )
	{
	    dialogue_error ( _("A transaction with a multiple third party must be a new one.") );
	    return (FALSE);
	}

	/* on vérifie maintenant si l'état existe */

	tableau_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_PARTY) ))),
				    ":",
				    2 );

	if ( tableau_char[1] )
	{
	    tableau_char[1] = g_strstrip ( tableau_char[1] );
	    liste_tmp = liste_struct_etats;
	    trouve = 0;

	    while ( liste_tmp )
	    {
		struct struct_etat *etat;

		etat = liste_tmp -> data;

		if ( !strcmp ( etat -> nom_etat,
			       tableau_char[1] ))
		{
		    trouve = 1;
		    liste_tmp = NULL;
		}
		else
		    liste_tmp = liste_tmp -> next;
	    }

	    g_strfreev ( tableau_char );

	    if ( !trouve )
	    {
		dialogue_error ( _("Invalid multiple third party.") );
		return (FALSE);
	    }
	}
	else
	{
	    dialogue_error  ( _("The word \"Report\" is reserved. Please use another one."));
		return FALSE;
	}
    }

    /* Check if there is no budgetary line (see #208) */
    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_BUDGET )
	 &&
	 gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_BUDGET) ) -> entry ) == style_entree_formulaire[ENGRIS] )
      {
	dialogue_conditional_hint ( _("No budgetary line was entered"),
				    _("This transaction has no budgetary line entered.  You should use them to easily produce budgets and make reports on them."),
				    &(etat.display_message_no_budgetary_line) );
	return FALSE;
     }

    return ( TRUE );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction récupère les données du formulaire et les met dans          */
/* l'opération en argument sauf la catég                                      */
/******************************************************************************/
void recuperation_donnees_generales_formulaire ( struct structure_operation *operation )
{
    gchar **tableau_char;
    struct struct_devise *devise;
    gint i, j;
    struct organisation_formulaire *organisation_formulaire;


    /*     on fait le tour du formulaire en ne récupérant que ce qui est nécessaire */

    organisation_formulaire = renvoie_organisation_formulaire ();

    for ( i=0 ; i < organisation_formulaire -> nb_lignes ; i++ )
	for ( j=0 ; j <  organisation_formulaire -> nb_colonnes ; j++ )
	{
	    GtkWidget *widget;

	    widget =  widget_formulaire_par_element ( organisation_formulaire -> tab_remplissage_formulaire[i][j] );

	    switch ( organisation_formulaire -> tab_remplissage_formulaire[i][j] )
	    {
		case TRANSACTION_FORM_DATE:

		    sscanf ( gtk_entry_get_text ( GTK_ENTRY ( widget )),
			     "%d/%d/%d",
			     &operation -> jour,
			     &operation -> mois,
			     &operation -> annee );

		    operation -> date = g_date_new_dmy ( operation -> jour,
							 operation -> mois,
							 operation -> annee );

		    break;

		case TRANSACTION_FORM_VALUE_DATE:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
		    {
			sscanf ( gtk_entry_get_text ( GTK_ENTRY ( widget )),
				 "%d/%d/%d",
				 &operation -> jour_bancaire,
				 &operation -> mois_bancaire,
				 &operation -> annee_bancaire );

			operation -> date_bancaire = g_date_new_dmy ( operation -> jour_bancaire,
								      operation -> mois_bancaire,
								      operation -> annee_bancaire );
		    }
		    else
		    {
			operation -> jour_bancaire = 0;
			operation -> mois_bancaire = 0;
			operation -> annee_bancaire = 0;

			operation->date_bancaire = NULL;
		    }
		    break;

		case TRANSACTION_FORM_EXERCICE:

		    /* si l'exo est à -1, c'est que c'est sur non affiché */
		    /* soit c'est une modif d'opé et on touche pas à l'exo */
		    /* soit c'est une nouvelle opé et on met l'exo à 0 */

		    if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget ) -> menu_item ),
								 "no_exercice" )) == -1 )
		    {
			if ( !operation -> no_operation )
			    operation -> no_exercice = 0;
		    }
		    else
			operation -> no_exercice = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget ) -> menu_item ),
											   "no_exercice" ));

		    break;

		case TRANSACTION_FORM_PARTY:

		    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget ) -> entry ) == style_entree_formulaire[ENCLAIR] )
		    {
			operation -> tiers = tiers_par_nom ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget )),
							     1 ) -> no_tiers;
		    }
		    else
			operation -> tiers = 0;

		    break;

		case TRANSACTION_FORM_DEBIT:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			operation -> montant = -calcule_total_entree ( widget );
		    break;

		case TRANSACTION_FORM_CREDIT:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			operation -> montant = calcule_total_entree ( widget );
		    break;

		case TRANSACTION_FORM_BUDGET:

		    /* si c'est une opé ventilée, on ne récupère pas l'ib */

		    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget ) -> entry ) == style_entree_formulaire[ENCLAIR]
			 &&
			 (!verifie_element_formulaire_existe ( TRANSACTION_FORM_BREAKDOWN )
			  ||
			  !GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_BREAKDOWN) )))
		    {
			struct struct_imputation *imputation;

			tableau_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget )),
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
		    else
		    {
			operation -> imputation = 0;
			operation -> sous_imputation = 0;
		    }

		    break;

		case TRANSACTION_FORM_NOTES:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			operation -> notes = g_strstrip ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget ))));
		    else
			operation -> notes = NULL;
		    break;

		case TRANSACTION_FORM_TYPE:

		    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ))
		    {
			operation -> type_ope = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget ) -> menu_item ),
											"no_type" ));

			if ( GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) )
			     &&
			     gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) ) == style_entree_formulaire[ENCLAIR] )
			{
			    struct struct_type_ope *type;

			    type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget ) -> menu_item ),
							 "adr_type" );

			    operation -> contenu_type = g_strstrip ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) ))));

			    if ( type -> numerotation_auto )
				type -> no_en_cours = ( my_atoi ( operation -> contenu_type ));
			}
			else
			    operation -> contenu_type = NULL;
		    }
		    else
		    {
			operation -> type_ope = 0;
			operation -> contenu_type = NULL;
		    }
		    break;

		case TRANSACTION_FORM_DEVISE:

		    /* récupération de la devise */

		    devise = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget ) -> menu_item ),
						   "adr_devise" );

		    /* si c'est la devise du compte ou */
		    /* si c'est un compte qui doit passer à l'euro ( la transfo se fait au niveau de l'affichage de la liste ) */
		    /* ou si c'est un compte en euro et l'opé est dans une devise qui doit passer à l'euro -> pas de change à demander */

		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

		    if ( !devise_compte
			 ||
			 devise_compte -> no_devise != DEVISE )
			devise_compte = devise_par_no ( DEVISE );

		    operation -> devise = devise -> no_devise;

		    if ( !( operation -> no_operation
			    ||
			    devise -> no_devise == DEVISE
			    ||
			    ( devise_compte -> passage_euro
			      &&
			      !strcmp ( devise -> nom_devise, _("Euro") ))
			    ||
			    ( !strcmp ( devise_compte -> nom_devise, _("Euro") )
			      &&
			      devise -> passage_euro )))
		    {
			/* c'est une devise étrangère, on demande le taux de change et les frais de change */

			demande_taux_de_change ( devise_compte, devise, 1,
						 ( gdouble ) 0, ( gdouble ) 0, FALSE );

			operation -> taux_change = taux_de_change[0];
			operation -> frais_change = taux_de_change[1];

			if ( operation -> taux_change < 0 )
			{
			    operation -> taux_change = -operation -> taux_change;
			    operation -> une_devise_compte_egale_x_devise_ope = 1;
			}
		    }

		    break;

		case TRANSACTION_FORM_BANK:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			operation -> info_banque_guichet = g_strstrip ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget ))));
		    else
			operation -> info_banque_guichet = NULL;
		    break;

		case TRANSACTION_FORM_VOUCHER:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			operation -> no_piece_comptable = g_strstrip ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget ))));
		    else
			operation -> no_piece_comptable = NULL;

		    break;
	    }
	}
}
/******************************************************************************/

/******************************************************************************/
/* récupération des categ du formulaire, crée la contre opération si          */
/* nécessaire ou remplit les opés de ventil                                   */
/******************************************************************************/
void recuperation_categorie_formulaire ( struct structure_operation *operation,
					 gint modification )
{
    gchar *pointeur_char;
    gchar **tableau_char;
    struct structure_operation *contre_operation;
    GtkWidget *entree_categ;

    if ( !verifie_element_formulaire_existe ( TRANSACTION_FORM_CATEGORY ))
	return;

    entree_categ = GTK_COMBOFIX ( widget_formulaire_par_element (TRANSACTION_FORM_CATEGORY) ) -> entry;

    if ( gtk_widget_get_style ( entree_categ ) == style_entree_formulaire[ENCLAIR] )
    {
	struct struct_categ *categ;

	pointeur_char = g_strstrip ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entree_categ ))));

	/* récupération de la ventilation si nécessaire */

	if ( !strcmp ( pointeur_char,
		       _("Breakdown of transaction") ))
	{
	    /* c'est une opé ventilée, on va appeler la fonction validation_ope_de_ventilation */
	    /* qui va créer les nouvelles opé, les contre-opérations et faire toutes les */
	    /* suppressions nécessaires */

	    /*  auparavant, si c'est une modif d'opé et que l'ancienne opé était un virement, on  */
	    /* vire l'ancienne opé associée */

	    if ( modification
		 &&
		 operation -> relation_no_operation )
	    {
		/* c'était un virement, et ce ne l'est plus, donc on efface l'opé en relation */

		contre_operation = operation_par_no ( operation -> relation_no_operation,
						      operation -> relation_no_compte );

		if ( contre_operation )
		{
		    contre_operation -> relation_no_operation = 0;
		    supprime_operation ( contre_operation );
		}

		operation -> relation_no_operation = 0;
		operation -> relation_no_compte = 0;
	    }

	    validation_ope_de_ventilation ( operation );
	    operation -> operation_ventilee = 1;
	    operation -> categorie = 0;
	    operation -> sous_categorie = 0;
	}
	else
	{
	    /* ce n'est pas une opé ventilée, si c'est une modif d'opé et que */
	    /* c'en était une, on supprime les opés de ventil asssociées */

	    if ( modification
		 &&
		 operation -> operation_ventilee )
	    {
		GSList *liste_tmp;

		p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

		liste_tmp = LISTE_OPERATIONS;

		while ( liste_tmp )
		{
		    struct structure_operation *ope_tmp;

		    ope_tmp = liste_tmp -> data;

		    if ( ope_tmp -> no_operation_ventilee_associee == operation -> no_operation )
		    {
			liste_tmp = liste_tmp -> next;
			supprime_operation ( ope_tmp );
		    }
		    else
			liste_tmp = liste_tmp -> next;
		}
		operation -> operation_ventilee = 0;
	    }

	    /* on va maintenant séparer entre virement et catég normale */

	    tableau_char = g_strsplit ( pointeur_char,
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
		     && strlen ( tableau_char[1] ) )
		{
		    /* c'est un virement, il n'y a donc aucune catégorie */

		    operation -> categorie = 0;
		    operation -> sous_categorie = 0;

		    /* sépare entre virement vers un compte et virement vers un compte supprimé */

		    if ( strcmp ( tableau_char[1],
				  _("Deleted account") ) )
		    {
			/* c'est un virement normal, on appelle la fonction qui va traiter ça */

			validation_virement_operation ( operation,
							modification,
							tableau_char[1] );
		    }
		    else
		    {
			/* c'est un virement vers un compte supprimé */

			operation -> relation_no_compte = -1;
			operation -> relation_no_operation = 1;
		    }
		}
		else
		{
		    /* c'est une catég normale, si c'est une modif d'opé, vérifier si ce n'était pas un virement */

		    if ( modification
			 &&
			 operation -> relation_no_operation )
		    {
			/* c'était un virement, et ce ne l'est plus, donc on efface l'opé en relation */

			contre_operation = operation_par_no ( operation -> relation_no_operation,
							      operation -> relation_no_compte );

			if ( contre_operation )
			{
			    contre_operation -> relation_no_operation = 0;
			    supprime_operation ( contre_operation );
			}

			operation -> relation_no_operation = 0;
			operation -> relation_no_compte = 0;
		    }

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
	    }
	    g_strfreev ( tableau_char );
	}
    }
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée la contre-opération, l'enregistre et met en place      */
/* les liens si c'est une modif de virement, elle supprime aussi l'ancienne   */
/* contre opération                                                           */
/******************************************************************************/
void validation_virement_operation ( struct structure_operation *operation,
				     gint modification,
				     gchar *nom_compte_vire )
{
    struct struct_devise *devise, *devise_compte_2;
    struct structure_operation *contre_operation;
    gpointer **save_ptab;
    gint compte_virement, i;

    save_ptab = p_tab_nom_de_compte_variable;

    /* on n'a plus besoin de faire de tests, ceux ci ont été fait dans verification_validation_operation */

    /* récupère le no du compte de la contre opération */

    compte_virement = -1;

    for ( i = 0 ; i < nb_comptes ; i++ )
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	if ( !g_strcasecmp ( NOM_DU_COMPTE,
			     nom_compte_vire ))
	{
	    compte_virement = i;
	    i = nb_comptes;
	}
    }

    /*   2 possibilités, soit c'est un nouveau virement ou une modif qui n'était pas un virement, */
    /* soit c'est une modif de virement */
    /*     dans le 2nd cas, si on a modifié le compte de la contre opération */
    /*     on supprime l'ancienne contre opération et c'est un nouveau virement */

    if ( modification )
    {
	/*       c'est une modif d'opé */
	/* 	soit c'était un virement vers un autre compte et dans ce cas on vire la contre-opération pour la recréer plus tard */
	/* soit c'est un virement vers le même compte et dans ce cas on fait rien, la contre opé sera modifiée automatiquement */
	/* soit ce n'était pas un virement et dans ce cas on considère l'opé comme une nouvelle opé */

	if ( operation -> relation_no_operation )
	{
	    /* c'était déjà un virement, on ne vire la contre opé que si le compte cible a changé */

	    if ( operation -> relation_no_compte != compte_virement )
	    {
		/* il faut retirer l'ancienne contre opération */

		contre_operation = operation_par_no ( operation -> relation_no_operation,
						      operation -> relation_no_compte );

		if ( contre_operation )
		{
		    contre_operation -> relation_no_operation = 0;

		    supprime_operation ( contre_operation );
		    modification = 0;
		}
	    }
	}
	else
	{
	    /* ce n'était pas un virement, on considère que c'est une nouvelle opé pour créer la contre opération */

	    modification = 0;
	}
    }

    /*   on en est maintenant à soit nouveau virement, soit modif de virement sans chgt de compte */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_virement;

    if ( modification
	 &&
	 operation -> relation_no_operation )
	contre_operation = operation_par_no ( operation -> relation_no_operation,
					      compte_virement );
    else
    {
	contre_operation = calloc ( 1,
				    sizeof ( struct structure_operation ) );
	contre_operation -> no_compte = compte_virement;
	contre_operation -> taux_change = 0;
	contre_operation -> frais_change = 0;
    }

    /* remplit la nouvelle opé */

    contre_operation -> jour = operation -> jour;
    contre_operation -> mois = operation -> mois;
    contre_operation -> annee = operation -> annee;
    contre_operation -> date = g_date_new_dmy ( contre_operation->jour,
						contre_operation->mois,
						contre_operation->annee);
    contre_operation -> montant = -operation -> montant;

    /* si c'est la devise du compte ou si c'est un compte qui doit passer à l'euro ( la transfo se fait au niveau */
    /* de l'affichage de la liste ) ou si c'est un compte en euro et l'opé est dans une devise qui doit passer à l'euro -> ok */

    devise_compte_2 = devise_par_no ( DEVISE );

    devise = devise_par_no ( operation -> devise );

    contre_operation -> devise = operation -> devise;

    if ( !( contre_operation-> no_operation
	    ||
	    devise -> no_devise == DEVISE
	    ||
	    ( devise_compte_2 -> passage_euro && is_euro(devise))
	    ||
	    ( is_euro(devise_compte_2) && devise -> passage_euro )))
    {
	/* c'est une devise étrangère, on demande le taux de change et les frais de change */

	demande_taux_de_change ( devise_compte_2, devise, 1,
				 (gdouble ) 0, (gdouble ) 0, FALSE );

	contre_operation -> taux_change = taux_de_change[0];
	contre_operation -> frais_change = taux_de_change[1];

	if ( contre_operation -> taux_change < 0 )
	{
	    contre_operation -> taux_change = -contre_operation -> taux_change;
	    contre_operation -> une_devise_compte_egale_x_devise_ope = 1;
	}
    }

    contre_operation -> tiers = operation -> tiers;
    contre_operation -> categorie = operation -> categorie;
    contre_operation -> sous_categorie = operation -> sous_categorie;

    if ( operation -> notes )
	contre_operation -> notes = g_strdup ( operation -> notes);

    contre_operation -> auto_man = operation -> auto_man;

    /* récupération du type de l'autre opé */

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CONTRA )
	 &&
	 GTK_WIDGET_VISIBLE ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ))
	contre_operation -> type_ope = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_CONTRA) ) -> menu_item ),
									       "no_type" ));
    else
    {
	contre_operation -> type_ope = 0;
	operation -> contenu_type = NULL;
    }

    if ( operation -> contenu_type )
	contre_operation -> contenu_type = g_strdup ( operation -> contenu_type );

    contre_operation -> no_exercice = operation -> no_exercice;
    contre_operation -> imputation = operation -> imputation;
    contre_operation -> sous_imputation = operation -> sous_imputation;

    if ( operation -> no_piece_comptable )
	contre_operation -> no_piece_comptable = g_strdup ( operation -> no_piece_comptable );

    if ( operation -> info_banque_guichet )
	contre_operation -> info_banque_guichet = g_strdup ( operation -> info_banque_guichet );

    contre_operation -> pointe = operation -> pointe;
    contre_operation -> no_rapprochement = operation -> no_rapprochement;

    /*   on a fini de remplir l'opé, on peut l'ajouter Ã  la liste */

    if ( contre_operation -> no_operation )
	modifie_operation ( contre_operation );
    else
	ajout_operation ( contre_operation );

    /* on met maintenant les relations entre les diffÃ©rentes opé */

    operation -> relation_no_operation = contre_operation -> no_operation;
    operation -> relation_no_compte = contre_operation -> no_compte;
    contre_operation -> relation_no_operation = operation -> no_operation;
    contre_operation -> relation_no_compte = operation -> no_compte;

    /*     on réaffiche juste les 2 opés, sans rien modifier d'autre */
    /* 	juste pour afficher la categ (virement) */

    remplit_ligne_operation ( operation,
			      NULL );
    remplit_ligne_operation ( contre_operation,
			      NULL );
	
    p_tab_nom_de_compte_variable = save_ptab;
}
/******************************************************************************/





/******************************************************************************/
/* Fonction ajout_operation                                                   */
/* ajoute l'opération donnée en argument à la liste des opés, trie la liste   */
/* et réaffiche la clist                                                      */
/* remplit le no d'opération     	                                    */
/******************************************************************************/
void ajout_operation ( struct structure_operation *operation )
{
    gpointer **save_ptab;
    GtkTreeIter *iter;
    struct structure_operation *operation_suivante;
    GSList *liste_tmp;

    if ( DEBUG )
	printf ( "ajout_operation\n" );


    save_ptab = p_tab_nom_de_compte_variable;

    /* on met l'opé dant la liste en la classant */
    /*     cette fonction place en même temps p_tab_nom_de_compte_variable */

    insere_operation_dans_liste ( operation );

    /*     on ne met à jour le store que si l'opé est affichée, cad si c'est une opé de ventil */
    /* 	ou R et que les R ne sont pas affichés, on ne fait rien */

    if ( !(operation -> no_operation_ventilee_associee
	   ||
	   ( operation -> pointe == 3
	     &&
	     !AFFICHAGE_R )))
    {
	/*     si la liste n'est pas finie, on la finie avant */

	verification_list_store_termine ( operation -> no_compte );

	/* on recherche l'iter de l'opé suivante */
	/*     on est sûr que l'opé va être trouvée vu qu'on vient de l'ajouter */

	liste_tmp = g_slist_find ( LISTE_OPERATIONS,
				   operation ) -> next;

	if ( liste_tmp )
	{
	    /* 	    si l'opé suivant n'est pas affichée, il faut rechercher la 1ère affichée */

	    operation_suivante = liste_tmp -> data;

	    while ( liste_tmp
		    &&
		    ( operation_suivante -> no_operation_ventilee_associee
		      ||
		      ( operation_suivante -> pointe == 3
			&&
			!AFFICHAGE_R )))
	    {
		liste_tmp = liste_tmp -> next;
		operation_suivante = liste_tmp -> data;
	    }

	    if ( !liste_tmp )
		operation_suivante = GINT_TO_POINTER (-1);
	}
	else
	    operation_suivante = GINT_TO_POINTER (-1);

	/*     on recherche l'iter de l'opé suivant pour insérer l'opé juste avant */

	iter = cherche_iter_operation ( operation_suivante );

	/*     on insère l'iter de l'opération qu'on ajoute */

	remplit_ligne_operation ( operation,
				  iter );

	/*     on recherche l'iter de l'opération qu'on vient d'ajouter */
	/* 	pour mettre à jour les couleurs et les soldes */
	/*     il est nécessaire de faire une copie d'iter, car chaque fonction va */
	/* 	le modifier */

	iter = cherche_iter_operation ( operation );

	update_couleurs_background ( operation -> no_compte,
				     gtk_tree_iter_copy (iter));
	update_soldes_list_store ( operation -> no_compte,
				   gtk_tree_iter_copy (iter));
	selectionne_ligne ( OPERATION_SELECTIONNEE );
	ajuste_scrolling_liste_operations_a_selection ( operation -> no_compte );
    }

    /*     calcul du solde courant */

    SOLDE_COURANT = SOLDE_COURANT + calcule_montant_devise_renvoi ( operation -> montant,
								    DEVISE,
								    operation -> devise,
								    operation -> une_devise_compte_egale_x_devise_ope,
								    operation -> taux_change,
								    operation -> frais_change );

    /* on met à jour les labels des soldes */

    mise_a_jour_labels_soldes ();

    /* on réaffichera l'accueil */

    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    p_tab_nom_de_compte_variable = save_ptab;
}
/******************************************************************************/


/******************************************************************************/
/* cette fonction met juste l'opé dans la sliste, et ajuste le nb d'opérations */
/* elle ne fait aucune autre mise à jour */
/* si l'opé a déjà un no, elle ne fait rien */
/******************************************************************************/

void insere_operation_dans_liste ( struct structure_operation *operation )
{
    if ( !operation -> no_operation )
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	operation -> no_operation = ++no_derniere_operation;

	LISTE_OPERATIONS = g_slist_insert_sorted ( LISTE_OPERATIONS,
						   operation,
						   (GCompareFunc) CLASSEMENT_COURANT );
	
	selectionne_ligne ( operation );
    }
}
/******************************************************************************/



/******************************************************************************/
/* Fonction modifie_operation                                                 */
/* modifie l'opération donnée en argument sur la liste des opés, trie la liste*/
/******************************************************************************/
void modifie_operation ( struct structure_operation *operation )
{
    gpointer **save_ptab;

    if ( DEBUG )
	printf ( "modifie_operation\n" );

    save_ptab = p_tab_nom_de_compte_variable;

    /*     si la liste n'est pas finie, on la finie avant */
    /*     théoriquement elle est finie à ce niveau car soit on modifie une opé */
    /* 	qu'on voit, soit ajout_operation a été appelé avant dans le cas */
    /* 	de virements ; mais bon, ça coute rien de tester... */
	
     verification_list_store_termine ( operation -> no_compte );

    /*     réaffiche l'opÃ©ration */

    remplit_ligne_operation ( operation,
			      NULL );

    /*     on recherche l'iter de l'opération qu'on vient d'ajouter */
    /* 	pour mettre à jour les soldes */

    update_soldes_list_store ( operation -> no_compte,
			       cherche_iter_operation ( operation ));

    /*     on recalcule les soldes pointés ou totaux */
    /* 	ya plus rapide mais j'ai la flemme */
    /* 	voir si ralentit vraiment... */

    SOLDE_COURANT = calcule_solde_compte ( operation -> no_compte );
    SOLDE_POINTE = calcule_solde_pointe_compte ( operation -> no_compte );

    /* on met à jour les labels des soldes */

    mise_a_jour_labels_soldes ();

    /* on réaffichera l'accueil */

    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;


    p_tab_nom_de_compte_variable = save_ptab;
}
/******************************************************************************/


/******************************************************************************/
/* efface le contenu du formulaire                                            */
/******************************************************************************/
void formulaire_a_zero (void)
{
    gint i, j;
    struct organisation_formulaire *organisation_formulaire;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    /*     on fait le tour du formulaire en ne récupérant que ce qui est nécessaire */

    organisation_formulaire = renvoie_organisation_formulaire ();

    for ( i=0 ; i < organisation_formulaire -> nb_lignes ; i++ )
	for ( j=0 ; j <  organisation_formulaire -> nb_colonnes ; j++ )
	{
	    GtkWidget *widget;

	    widget =  widget_formulaire_par_element ( organisation_formulaire -> tab_remplissage_formulaire[i][j] );

	    switch ( organisation_formulaire -> tab_remplissage_formulaire[i][j] )
	    {
		case TRANSACTION_FORM_DATE:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Date") );
		    break;

		case TRANSACTION_FORM_VALUE_DATE:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Value date") );
		    break;

		case TRANSACTION_FORM_EXERCICE:

		    gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
					       FALSE );
		    break;

		case TRANSACTION_FORM_PARTY:

		    gtk_widget_set_style ( GTK_COMBOFIX ( widget ) -> entry,
					   style_entree_formulaire[ENGRIS] );
		    gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
					    _("Third party") );
		    break;

		case TRANSACTION_FORM_DEBIT:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Debit") );
		    gtk_widget_set_sensitive ( widget,
					       TRUE );
		    break;

		case TRANSACTION_FORM_CREDIT:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Credit") );
		    gtk_widget_set_sensitive ( widget,
					       TRUE );
		    break;

		case TRANSACTION_FORM_CATEGORY:

		    gtk_widget_set_style ( GTK_COMBOFIX ( widget ) -> entry,
					   style_entree_formulaire[ENGRIS] );
		    gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
					    _("Categories : Sub-categories") );
		    gtk_widget_set_sensitive ( widget,
					       TRUE );
		    break;

		case TRANSACTION_FORM_BREAKDOWN:

		    gtk_widget_hide ( widget );
		    break;

		case TRANSACTION_FORM_BUDGET:

		    gtk_widget_set_style ( GTK_COMBOFIX ( widget ) -> entry,
					   style_entree_formulaire[ENGRIS] );
		    gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
					    _("Budgetary line") );
		    break;

		case TRANSACTION_FORM_NOTES:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Notes") );
		    break;

		case TRANSACTION_FORM_TYPE:

		    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
						  cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );
		    gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
					       FALSE );
		    break;

		case TRANSACTION_FORM_CONTRA:

		    gtk_widget_hide ( widget );

		    break;

		case TRANSACTION_FORM_CHEQUE:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Cheque/Transfer number") );
		    break;

		case TRANSACTION_FORM_DEVISE:

		    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
						  g_slist_index ( liste_struct_devises,
								  devise_par_no ( DEVISE )));
		    gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
					       FALSE );
		    break;

		case TRANSACTION_FORM_CHANGE:

		    gtk_widget_hide ( widget );

		    break;

		case TRANSACTION_FORM_BANK:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Bank references") );
		    break;

		case TRANSACTION_FORM_VOUCHER:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Voucher") );
		    break;

		case TRANSACTION_FORM_OP_NB:

		    gtk_label_set_text ( GTK_LABEL ( widget ),
					 "" );
		    break;

		case TRANSACTION_FORM_MODE:

		    gtk_label_set_text ( GTK_LABEL ( widget ),
					 "" );
		    break;

	    }
	}

    gtk_widget_set_sensitive ( GTK_WIDGET ( vbox_boutons_formulaire ),
			       FALSE );

    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "adr_struct_ope",
			  NULL );
    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "liste_adr_ventilation",
			  NULL );

    gtk_widget_set_sensitive ( bouton_affiche_cache_formulaire,
			       TRUE );
}
/******************************************************************************/

/******************************************************************************/
/* Fonction affiche_cache_le_formulaire                                       */
/* si le formulaire était affichÃ©, le cache et vice-versa                     */
/******************************************************************************/

void affiche_cache_le_formulaire ( void )
{
    gpointer **save_ptab;
    GtkWidget * widget;

    save_ptab = p_tab_nom_de_compte_variable;
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    if ( etat.formulaire_toujours_affiche )
    {
	gtk_widget_hide ( fleche_bas );
	gtk_widget_show ( fleche_haut );

	gtk_widget_hide ( frame_droite_bas );
	etat.formulaire_toujours_affiche = 0;
    }
    else
    {
	GtkAdjustment *ajustement;
	gint position_ligne_selectionnee;

	gtk_widget_hide ( fleche_haut );
	gtk_widget_show ( fleche_bas );

	gtk_widget_show ( frame_droite_bas );
	etat.formulaire_toujours_affiche = 1;

	/* 	si après avoir remonté le formulaire la ligne sélectionnée est cachée, */
	/* 	on la remonte pour la mettre juste au dessus du formulaire */

	update_ecran ();
	ajustement = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( TREE_VIEW_LISTE_OPERATIONS ));
	
	position_ligne_selectionnee = ( cherche_ligne_operation ( OPERATION_SELECTIONNEE )
					+ NB_LIGNES_OPE ) * hauteur_ligne_liste_opes;

	if ( position_ligne_selectionnee  > (ajustement->value + ajustement->page_size))
	    gtk_adjustment_set_value ( ajustement,
				       position_ligne_selectionnee - ajustement->page_size );
    }

    block_menu_cb = TRUE;
    widget = gtk_item_factory_get_item ( item_factory_menu_general,
					 menu_name(_("View"), _("Show transaction form"), NULL) );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (widget), etat.formulaire_toujours_affiche );
    block_menu_cb = FALSE;

    p_tab_nom_de_compte_variable = save_ptab;
}
/******************************************************************************/



/******************************************************************************/
/* Fonction basculer_vers_ventilation                                         */
/* appelée par l'appui du bouton Ventilation...                               */
/* permet de voir les opés ventilées d'une ventilation                        */
/******************************************************************************/
void basculer_vers_ventilation ( GtkWidget *bouton,
				 gpointer null )
{
    enregistre_ope_au_retour = 0;

    if ( gtk_widget_get_style ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT) ) == style_entree_formulaire[ENCLAIR] )
	ventiler_operation ( -my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_DEBIT))),
					  NULL ));
    else
	ventiler_operation ( my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CREDIT))),
					 NULL ));
}
/******************************************************************************/

/******************************************************************************/
/* Fonction click_sur_bouton_voir_change  */
/* permet de modifier un change établi pour une opération */
/******************************************************************************/
void click_sur_bouton_voir_change ( void )
{
    struct structure_operation *operation;
    struct struct_devise *devise;

    gtk_widget_grab_focus ( widget_formulaire_par_element (TRANSACTION_FORM_DATE) );

    operation = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				      "adr_struct_ope" );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    if ( !devise_compte ||
	 devise_compte -> no_devise != DEVISE )
	devise_compte = devise_par_no ( DEVISE );

    devise = devise_par_no ( operation -> devise );

    demande_taux_de_change ( devise_compte, devise,
			     operation -> une_devise_compte_egale_x_devise_ope,
			     operation -> taux_change, operation -> frais_change, 
			     TRUE );

    if ( taux_de_change[0] ||  taux_de_change[1] )
    {
	operation -> taux_change = taux_de_change[0];
	operation -> frais_change = taux_de_change[1];

	if ( operation -> taux_change < 0 )
	{
	    operation -> taux_change = -operation -> taux_change;
	    operation -> une_devise_compte_egale_x_devise_ope = 1;
	}
	else
	    operation -> une_devise_compte_egale_x_devise_ope = 0;
    }
}
/******************************************************************************/

/******************************************************************************/
void degrise_formulaire_operations ( void )
{

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_TYPE ))
	gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ),
				   TRUE );

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_DEVISE ))
	gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_par_element (TRANSACTION_FORM_DEVISE) ),
				   TRUE );

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_EXERCICE ))
	gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_par_element (TRANSACTION_FORM_EXERCICE) ),
				   TRUE );

    gtk_widget_set_sensitive ( GTK_WIDGET ( vbox_boutons_formulaire ),
			       TRUE );
}
/******************************************************************************/



/******************************************************************************/
/* renvoie l'organisation du formulaire courant en fonction des paramètres */
/******************************************************************************/
struct organisation_formulaire *renvoie_organisation_formulaire ( void )
{
    struct organisation_formulaire *retour;
    gpointer **save_ptab;

    save_ptab = p_tab_nom_de_compte_variable;

    if ( etat.formulaire_distinct_par_compte )
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;
    else
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    retour = ORGANISATION_FORMULAIRE;

    p_tab_nom_de_compte_variable = save_ptab;

    return retour;
}
/******************************************************************************/


/******************************************************************************/
/* renvoie l'adr du widget du formulaire dont l'élément est donné en argument */
/******************************************************************************/
GtkWidget *widget_formulaire_par_element ( gint no_element )
{
    struct organisation_formulaire *organisation_formulaire;
    gint ligne;
    gint colonne;

    if ( !no_element )
	return NULL;

    organisation_formulaire = renvoie_organisation_formulaire();

    if ( recherche_place_element_formulaire ( organisation_formulaire,
					      no_element,
					      &ligne,
					      &colonne ))
	return tab_widget_formulaire[ligne][colonne];
    
    return NULL;
}
/******************************************************************************/


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
