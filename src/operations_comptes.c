/*  Fichier qui gère la liste des comptes, la partie gauche de l'onglet opérations */
/*      operations_comptes.c */

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
#include "operations_comptes.h"
#include "ventilation.h"
#include "equilibrage.h"
#include "data_account.h"
#include "gtk_list_button.h"
#include "menu.h"
#include "barre_outils.h"
#include "operations_liste.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "comptes_onglet.h"
#include "operations_formulaire.h"
#include "utils.h"
#include "utils_comptes.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void changement_compte_par_menu ( gpointer null,
				  gint compte_plus_un );
static GtkWidget *comptes_appel ( gint no_de_compte );;
static void verifie_compte_clos ( gint no_nouveau_compte );
/*END_STATIC*/


GtkWidget *comptes_appel ( gint no_de_compte );
static void verifie_compte_clos ( gint no_nouveau_compte );
GtkWidget *label_compte_courant;

/*  n° de compte en cours de visualisation */

gint compte_courant;

/* adresse de la vbox contenant les icones de comptes */

GtkWidget *vbox_liste_comptes;

/* adr du label du dernier relevé */

GtkWidget *label_releve;

GSList *ordre_comptes;


/*START_EXTERN*/
extern gchar *derniere_date;
extern GtkWidget *formulaire;
extern GtkItemFactory *item_factory_menu_general;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint nb_colonnes;
extern gint nb_comptes;
extern GtkWidget *notebook_comptes_equilibrage;
extern GtkWidget *notebook_general;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern GtkWidget *tree_view;
/*END_EXTERN*/




/* ********************************************************************************************************** */
/*** Création de la fenêtre de comptes ***/
/* **************************************************************************************************** */

GtkWidget *creation_liste_comptes (void)
{
    GtkWidget *onglet;
    GtkWidget *frame_label_compte_courant;
    GtkWidget *bouton;
    GtkWidget *frame_equilibrage;
    GtkWidget *vbox_frame_equilibrage;
    GtkWidget *scrolled_window;

    /*  Création d'une fenêtre générale*/

    onglet = gtk_vbox_new ( FALSE,
			    10);
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				     10 );
    gtk_signal_connect ( GTK_OBJECT ( onglet ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( traitement_clavier_liste ),
			 NULL );
    gtk_widget_show ( onglet );

    /*  Création du label Comptes en haut */


    /*   on place le label dans une frame */

    frame_label_compte_courant = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_label_compte_courant ),
				GTK_SHADOW_ETCHED_OUT );
    gtk_box_pack_start ( GTK_BOX (onglet),
			 frame_label_compte_courant,
			 FALSE,
			 TRUE,
			 0);
    gtk_widget_show (frame_label_compte_courant);


    /*   on ne met rien dans le label, il sera rempli ensuite */

    label_compte_courant = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC (label_compte_courant  ),
			     0.5,
			     0.5);
    gtk_container_add ( GTK_CONTAINER ( frame_label_compte_courant ),
			label_compte_courant );
    gtk_widget_show (label_compte_courant);


    /*  Création de la fenêtre des comptes */
    /*  qui peut contenir des barres de défilement si */
    /*  nécessaire */

    scrolled_window = gtk_scrolled_window_new ( NULL,
						NULL);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0);
    gtk_widget_show ( scrolled_window );


    /*  création d'une vbox contenant la liste des comptes */

    vbox_liste_comptes = gtk_vbox_new ( FALSE,
					0);
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (scrolled_window ),
					    vbox_liste_comptes);
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window )  -> child ),
				   GTK_SHADOW_NONE );
    gtk_widget_show (vbox_liste_comptes);


    /*  Création d'une icone et du nom par compte, et placement dans la
	liste selon l'ordre désiré  */
    if ( nb_comptes )
    {
	reaffiche_liste_comptes ();
    }

    /* ajoute le bouton et le label pour l'équilibrage de compte */
    /* les 2 seront intégrés dans une frame */

    frame_equilibrage = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_equilibrage ),
				GTK_SHADOW_ETCHED_IN );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 frame_equilibrage,
			 FALSE,
			 TRUE,
			 0);
    gtk_widget_show ( frame_equilibrage );


    vbox_frame_equilibrage = gtk_vbox_new ( FALSE,
					    5 );
    gtk_container_add ( GTK_CONTAINER  ( frame_equilibrage ),
			vbox_frame_equilibrage );
    gtk_widget_show ( vbox_frame_equilibrage );


    /* mise en place du label */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    if ( nb_comptes )
	label_releve = gtk_label_new ( g_strconcat ( COLON(_("Last statement")),
						     gsb_account_get_current_reconcile_date (compte_courant),
						     NULL ) );
    else
	label_releve = gtk_label_new ( COLON(_("Last statement")) );

    gtk_box_pack_start ( GTK_BOX ( vbox_frame_equilibrage ),
			 label_releve,
			 FALSE,
			 TRUE,
			 0);
    gtk_widget_show ( label_releve );


    /* mise en place du bouton équilibrage */

    bouton = gtk_button_new_with_label ( _("Reconcile") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE);
    gtk_box_pack_start ( GTK_BOX ( vbox_frame_equilibrage ),
			 bouton,
			 FALSE,
			 TRUE,
			 0);
    gtk_signal_connect ( GTK_OBJECT (bouton),
			 "clicked",
			 GTK_SIGNAL_FUNC ( equilibrage ),
			 NULL );
    gtk_widget_show ( bouton );


    return ( onglet );

}
/* ********************************************************************************************************** */




/* ********************************************************************************************************** */
/** Fonction qui renvoie un widget contenant un bouton **/
/** de compte associé à son nom **/
/* ********************************************************************************************************** */

GtkWidget *comptes_appel ( gint no_de_compte )
{
    GtkWidget *bouton;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_de_compte;

    bouton = gtk_list_button_new ( gsb_account_get_name (no_de_compte), 2, TRUE, GINT_TO_POINTER (no_de_compte));
    BOUTON_COMPTE = bouton;
    gtk_signal_connect_object ( GTK_OBJECT (bouton),
				"clicked",
				GTK_SIGNAL_FUNC ( changement_compte ),
				GINT_TO_POINTER ( no_de_compte ) );
    g_signal_connect ( G_OBJECT ( bouton ),
		       "reordered",
		       G_CALLBACK ( changement_ordre_liste_comptes ),
		       NULL );
    gtk_widget_show ( bouton );

    return ( bouton );
}
/* ********************************************************************************************************** */


/* ********************************************************************************************************** */
void changement_compte_par_menu ( gpointer null,
				  gint compte_plus_un )
{
    changement_compte ( GINT_TO_POINTER ( compte_plus_un - 1) );
}
/* ********************************************************************************************************** */




/* ********************************************************************************************************** */
/*  Routine appelée lors de changement de compte */
/* ********************************************************************************************************** */

gboolean changement_compte ( gint *compte)
{
    /*   si on n'est pas sur l'onglet comptes du notebook, on y passe */

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general ) ) != 1 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
				1 );

    /* si on était dans une ventilation d'opération, alors on annule la ventilation */
    /*     utile si on cherche Ã  accéder à un compte clos par ex */

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ) ) == 1 )
	annuler_ventilation();

    if ( GPOINTER_TO_INT ( compte ) == compte_courant )
	return FALSE;

    /*     si compte = -1, c'est que c'est la 1ère fois qu'on va sur l'onglet */

    if ( GPOINTER_TO_INT ( compte ) != -1 )
    {
	/* 	on va sur un compte, on vérifie que ce n'est pas un compte clos */
	/* 	    si c'est le cas, on ferme l'icone */

	verifie_compte_clos ( GPOINTER_TO_INT ( compte ));

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;
	gsb_account_set_adjustment_value ( compte_courant,
					   gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( gsb_account_get_tree_view (compte_courant) )) -> value );

	/*     on cache le tree_view */

	gtk_widget_hide ( gsb_account_get_scrolled_window (compte_courant) );

    /*     on se place sur les données du nouveau compte */

	compte_courant = GPOINTER_TO_INT ( compte );
    }

    /*     si compte=-1, compte_courant était déjà réglé */
    /* 	sinon on vient juste de le régler */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    /* change le nom du compte courant */

    gtk_label_set_text ( GTK_LABEL ( label_compte_courant), gsb_account_get_name (compte_courant));


    /*     affiche le nouveau formulaire  */
    /*     il met aussi à jour la devise courante et les types */

    remplissage_formulaire ( compte_courant );


    /*     mise en place de la date du dernier relevé */

    if ( gsb_account_get_current_reconcile_date (compte_courant) )
	gtk_label_set_text ( GTK_LABEL ( label_releve ),
			     g_strdup_printf ( _("Last statement: %02d/%02d/%d"), 
					       g_date_day ( gsb_account_get_current_reconcile_date (compte_courant) ),
					       g_date_month ( gsb_account_get_current_reconcile_date (compte_courant) ),
					       g_date_year ( gsb_account_get_current_reconcile_date (compte_courant) ) ));

    else
	gtk_label_set_text ( GTK_LABEL ( label_releve ),
			     _("Last statement: none") );


    /* affiche le solde final en bas */

    mise_a_jour_labels_soldes ();

    /* met les boutons R et nb lignes par opé comme il faut */

    mise_a_jour_boutons_caract_liste ( compte_courant );

     /*      on termine la liste d'opés si nécessaire */

     verification_list_store_termine ( compte_courant );


     /*     on restore ou initialise la value du tree_view */
     /* 	si VALUE = -1, c'est que c'est la première ouverture, on se met tout en bas */
     /* 	sinon on restore l'ancienne value */


    if ( gsb_account_get_adjustment_value (compte_courant) == -1 )
    {
	GtkAdjustment *ajustment;

	update_ecran ();

	ajustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( gsb_account_get_tree_view (compte_courant) ));
	gtk_adjustment_set_value ( GTK_ADJUSTMENT ( ajustment ),
				   ajustment -> upper - ajustment -> page_size );

    }
    else
	gtk_adjustment_set_value ( GTK_ADJUSTMENT ( gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( gsb_account_get_tree_view (compte_courant) ))),
				   gsb_account_get_adjustment_value (compte_courant) );

    /*     on affiche le tree_view */

    gtk_widget_show ( gsb_account_get_scrolled_window (compte_courant) );

    /* ALAIN-FIXME : nécessaire pour actualiser le compte « grisé »
       (donc compte_courant) dans le liste des comptes pour le menu :
       Édition : Déplacer l'opération vers un autre compte */
    reaffiche_liste_comptes ();
    
    /*     on réinitialise la dernière date entrée */

    derniere_date = NULL;

    return FALSE;
}
/* ********************************************************************************************************** */


/* ********************************************************************************************************** */
/* cette fonction est appelée lors d'un changement de compte */
/* cherche si le nouveau compte est clos, si c'est le cas, ferme l'icone du compte courant */
/* ********************************************************************************************************** */
void verifie_compte_clos ( gint no_nouveau_compte )
{
    gpointer **save_ptab;

    /*     si le compte courant est déjà cloturé, on fait rien */

    if ( COMPTE_CLOTURE )
	return;

    save_ptab = p_tab_nom_de_compte_variable;
    
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_nouveau_compte;

    if ( COMPTE_CLOTURE )
    {
	p_tab_nom_de_compte_variable = save_ptab;

	gtk_list_button_close ( GTK_BUTTON ( BOUTON_COMPTE ));
    }
    else
	p_tab_nom_de_compte_variable = save_ptab;
}
/* ********************************************************************************************************** */




/* *********************************************************************************************************** */
/*   on réaffiche la liste des comptes s'il y a eu un changement */
/* *********************************************************************************************************** */

void reaffiche_liste_comptes ( void )
{
    GSList *ordre_comptes_variable;
    GtkWidget *bouton;
    gint i;

    /* commence par effacer tous les comptes */

    while ( GTK_BOX ( vbox_liste_comptes ) -> children )
	gtk_container_remove ( GTK_CONTAINER ( vbox_liste_comptes ),
			       (( GtkBoxChild *) ( GTK_BOX ( vbox_liste_comptes ) -> children -> data )) -> widget );

    /* on efface les menus des comptes cloturés */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	gchar *tmp;

	tmp = my_strdelimit ( gsb_account_get_name (i),
			      "/",
			      "\\/" );

	gtk_item_factory_delete_item ( item_factory_menu_general,
				       menu_name(_("Accounts"), _("Closed accounts"), tmp ));
	p_tab_nom_de_compte_variable++;
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Accounts"), _("Closed accounts"), NULL)),
			       FALSE );

    /* on efface dans le menu Édition la liste des comptes vers lesquels on peut
       déplacer les opérations */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	gchar *tmp;

	tmp = my_strdelimit ( gsb_account_get_name (i),
			      "/",
			      "\\/" );

	gtk_item_factory_delete_item ( item_factory_menu_general,
				       menu_name(_("Edit"), _("Move transaction to another account"), tmp ));
	p_tab_nom_de_compte_variable++;
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Edit"), _("Move transaction to another account"), NULL)),
			       FALSE );


    /*  Création d'une icone et du nom par compte, et placement dans la liste selon l'ordre désiré  */

    ordre_comptes_variable = ordre_comptes;
    do
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable->data );

	if ( ! COMPTE_CLOTURE )
	{
	    bouton = comptes_appel( GPOINTER_TO_INT ( ordre_comptes_variable->data ));
	    gtk_box_pack_start (GTK_BOX (vbox_liste_comptes), bouton, FALSE, FALSE, 0);
	    gtk_widget_show (bouton);

	    /* 	    si c'est le compte courant, on ouvre le livre */

	    if ( p_tab_nom_de_compte_variable == p_tab_nom_de_compte + compte_courant )
		gtk_list_button_clicked ( GTK_BUTTON ( bouton ));
	}
	else
	{
	    GtkItemFactoryEntry *item_factory_entry;
	    gchar *tmp;


	    item_factory_entry = calloc ( 1,
					  sizeof ( GtkItemFactoryEntry ));

	    tmp = my_strdelimit ( gsb_account_get_name (GPOINTER_TO_INT ( ordre_comptes_variable->data )),
				  "/",
				  "\\/" );
	    tmp = my_strdelimit ( tmp,
				  "_",
				  "__" );

	    item_factory_entry -> path = menu_name(_("Accounts"),  _("Closed accounts"), tmp );
	    item_factory_entry -> callback = G_CALLBACK ( changement_compte_par_menu );

	    /* 	    on rajoute 1 car sinon pour le compte 0 ça passerait pas... */

	    item_factory_entry -> callback_action = GPOINTER_TO_INT ( ordre_comptes_variable->data ) + 1;

	    gtk_item_factory_create_item ( item_factory_menu_general,
					   item_factory_entry,
					   NULL,
					   1 );
	    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								   menu_name(_("Accounts"), _("Closed accounts"), NULL)),
				       TRUE );

	}

    }
    while ( (  ordre_comptes_variable = ordre_comptes_variable->next ) );


    /* Création dans le menu Édition de la liste des comptes vers lesquels on
       peut déplacer les opérations */

    ordre_comptes_variable = ordre_comptes;
    do
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable->data );

	if ( ! COMPTE_CLOTURE )
	{
	    GtkItemFactoryEntry *item_factory_entry;
	    gchar *tmp;


	    item_factory_entry = calloc ( 1, sizeof ( GtkItemFactoryEntry ));

	    tmp = my_strdelimit ( gsb_account_get_name (GPOINTER_TO_INT ( ordre_comptes_variable->data )), "/", "\\/" );
	    ;

	    item_factory_entry -> path = menu_name(_("Edit"), 
						   _("Move transaction to another account"), 
						   my_strdelimit ( tmp, "_", "__" ) ); 

	    item_factory_entry -> callback = G_CALLBACK ( move_selected_operation_to_account_nb );

	    /* 	    on rajoute 1 car sinon pour le compte 0 ça passerait pas... */

	    item_factory_entry -> callback_action = GPOINTER_TO_INT ( ordre_comptes_variable->data ) + 1;

	    gtk_item_factory_create_item ( item_factory_menu_general,
					   item_factory_entry,
					   ordre_comptes_variable -> data,
					   1 );

	    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								   menu_name(_("Edit"), _("Move transaction to another account"), NULL)),
				       TRUE );

	    /* si c'est le compte courant, on grise l'entrée menu */

	    if ( p_tab_nom_de_compte_variable == p_tab_nom_de_compte + compte_courant )
	    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								   menu_name(_("Edit"), _("Move transaction to another account"), tmp)),
				       FALSE );
	}
    }
    while ( (  ordre_comptes_variable = ordre_comptes_variable->next ) );
}
/* *********************************************************************************************************** */





/* *********************************************************************************************************** */
/* cette fonction est appelée lorsque l'ordre des comptes a été changé, soit */
/* par l'onglet de compte, soit par l'onglet de la liste des opérations */
/* *********************************************************************************************************** */
gboolean changement_ordre_liste_comptes ( GtkWidget *bouton )
{
    GSList *nouvelle_liste_comptes;
    GList *liste_tmp;
    GSList *sliste_tmp;


    liste_tmp = GTK_BOX ( bouton-> parent ) -> children;
    nouvelle_liste_comptes = NULL;

    while ( liste_tmp )
    {
	GtkBoxChild *box_child;

	box_child = liste_tmp -> data;

	nouvelle_liste_comptes = g_slist_append ( nouvelle_liste_comptes,
						  gtk_list_button_get_data ( GTK_LIST_BUTTON ( box_child -> widget )));
	liste_tmp = liste_tmp -> next;
    }

    /*     on va vérifier que tous les comptes de l'ancienne liste sont présents dans la nouvelle */
    /* 	car si l'on part de l'onglet des opérations, les compltes cloturés ne sont pas */
    /* 	affichés */

    sliste_tmp = ordre_comptes;

    while ( sliste_tmp )
    {
	if ( g_slist_index ( nouvelle_liste_comptes,
			     sliste_tmp -> data ) == -1 )
	    nouvelle_liste_comptes = g_slist_append ( nouvelle_liste_comptes,
						      sliste_tmp -> data );
	sliste_tmp = sliste_tmp -> next;
    }

    g_slist_free ( ordre_comptes );
    ordre_comptes = nouvelle_liste_comptes;

    /*     on réaffiche la liste des comptes de l'autre fenetre */

    if ( bouton -> parent == vbox_liste_comptes )
	/* 	on est sur la liste des comptes de l'onglet opérations, donc on réaffiche l'onglet comptes */
	reaffiche_liste_comptes_onglet ();
    else
	reaffiche_liste_comptes ();

    mise_a_jour_liste_comptes_accueil = 1;

    update_options_menus_comptes ();
    modification_fichier (TRUE);

    return ( FALSE );
}
/* *********************************************************************************************************** */




/******************************************************************************/
/* règle la taille des widgets dans le formulaire des opés en fonction */
/* des paramètres */
/******************************************************************************/
void mise_a_jour_taille_formulaire ( gint largeur_formulaire )
{

    gint i, j;
    struct organisation_formulaire *organisation_formulaire;

    if ( !largeur_formulaire )
	return;

    organisation_formulaire = renvoie_organisation_formulaire ();

    for ( i=0 ; i < organisation_formulaire -> nb_lignes ; i++ )
	for ( j=0 ; j < organisation_formulaire -> nb_colonnes ; j++ )
	{
	    GtkWidget *widget;

	    widget = widget_formulaire_par_element ( organisation_formulaire -> tab_remplissage_formulaire[i][j] );

	    if ( widget )
		gtk_widget_set_usize ( widget,
				       organisation_formulaire -> taille_colonne_pourcent[j] * largeur_formulaire / 100,
				       FALSE );
	}
}
/******************************************************************************/




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
