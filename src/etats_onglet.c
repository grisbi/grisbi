/*  Fichier qui s'occupe de l'onglet états */
/*      etats.c */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2002-2004 Benjamin Drieu (bdrieu@april.org) */
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

#include "erreur.h"
#include "etats_calculs.h"
#include "etats_config.h"
#include "etats_onglet.h"
#include "gtk_list_button.h"
#include "parametres.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"

#include "dialog.h"


void impression_etat ( struct struct_etat *etat );

extern GtkItemFactory *item_factory_menu_general;


/*****************************************************************************************************/
GtkWidget *creation_onglet_etats ( void )
{
    GtkWidget *onglet;
    GtkWidget *frame;
    GtkWidget *vbox;

    /*   au départ, aucun état n'est ouvert */

    bouton_etat_courant = NULL;
    etat_courant = NULL;


    /*   onglet = gtk_hbox_new ( FALSE, 10 ); */
    onglet = gtk_hpaned_new ();
    gtk_paned_set_position ( GTK_PANED(onglet), 200 );
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ), 10 );
    gtk_widget_show ( onglet );

    /*   création de la fenetre des noms des états */
    /* on reprend le principe des comptes dans la fenetre des opés */
    frame_liste_etats = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_liste_etats ),
				GTK_SHADOW_IN );
    gtk_paned_pack1 ( GTK_PANED(onglet), frame_liste_etats, TRUE, TRUE );
    gtk_widget_show (frame_liste_etats);

    /* on y met les rapports et les boutons */
    gtk_container_add ( GTK_CONTAINER ( frame_liste_etats ),
			creation_liste_etats ());

    /* Frame de droite */
    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_IN );
    gtk_paned_add2 ( GTK_PANED(onglet), frame );
    gtk_widget_show (frame);

    /* création du notebook contenant l'état et la config */
    notebook_etats = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( notebook_etats ), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(notebook_etats), FALSE );
    gtk_container_add ( GTK_CONTAINER(frame), notebook_etats);
    gtk_widget_show ( notebook_etats );


    /* création de la partie droite */

    vbox = gtk_vbox_new ( FALSE, 10 );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_etats ), vbox, gtk_label_new ( _("Reports")));
    gtk_widget_show ( vbox );


    /*  Création de la partie contenant l'état */

    frame = gtk_frame_new ( NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), frame, TRUE, TRUE, 0 );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_NONE );
    gtk_widget_show (frame);

    /* on y met une scrolled window qui sera remplit par l'état */

    scrolled_window_etat = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window_etat), GTK_SHADOW_NONE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_etat ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_container_add ( GTK_CONTAINER ( frame ), scrolled_window_etat );
    gtk_widget_show ( scrolled_window_etat );

    /* création de la partie contenant les boutons (personnaliser ...) */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_NONE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), frame,
			 FALSE, FALSE, 0 );
    gtk_widget_show (frame);

    /* on y met les boutons */

    gtk_container_add ( GTK_CONTAINER ( frame ),
			creation_barre_boutons_etats ());


    /*   g_signal_connect ( GTK_PANED(onglet), "move-handle", gtk_container_resize_children, NULL); */

    /* l'onglet de config sera créé que si nécessaire */

    onglet_config_etat = NULL;


    return ( onglet );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
/* vontion creation_liste_etats */
/* renvoie la partie gauche de l'onglet rapports financiers */
/*****************************************************************************************************/

GtkWidget *creation_liste_etats ( void )
{
    GtkWidget *onglet;
    GtkWidget *frame;
    GtkWidget *bouton;
    GtkWidget *vbox;
    GtkWidget *scrolled_window;


    /*  Création d'une fenêtre générale*/

    onglet = gtk_vbox_new ( FALSE,
			    10);
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ), 10 );
    gtk_widget_show ( onglet );


    /*  Création du label contenant le rapport courant en haut */
    /*   on place le label dans une frame */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX (onglet), frame,
			 FALSE, TRUE, 0);
    gtk_widget_show (frame);


    /*   on ne met rien dans le label, il sera rempli ensuite */

    label_etat_courant = gtk_label_new ( "" );
    gtk_label_set_line_wrap ( GTK_LABEL(label_etat_courant), TRUE );
    gtk_misc_set_alignment ( GTK_MISC (label_etat_courant  ),
			     0.5,
			     0.5);
    gtk_container_add ( GTK_CONTAINER ( frame ), label_etat_courant );

    gtk_widget_show (label_etat_courant);
    gtk_label_set_line_wrap ( GTK_LABEL ( label_etat_courant ), TRUE );


    /*  Création de la fenêtre des etats */
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


    /*  création de la vbox qui contient la liste des états */

    vbox_liste_etats = gtk_vbox_new ( FALSE, 10);
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (scrolled_window ),
					    vbox_liste_etats);
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window )  -> child ),
				   GTK_SHADOW_NONE );
    gtk_widget_show (vbox_liste_etats);

    /*  ajout des différents états */

    remplissage_liste_etats ();

    /* ajout des boutons pour supprimer / ajouter un état */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_ETCHED_IN );
    gtk_box_pack_start ( GTK_BOX ( onglet ), frame,
			 FALSE, TRUE, 0);
    gtk_widget_show ( frame );


    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_add ( GTK_CONTAINER  ( frame ),
			vbox );
    gtk_widget_show ( vbox );


    /* mise en place du bouton ajouter */

    bouton = gtk_button_new_with_label ( _("New report") );
    gtk_label_set_line_wrap ( GTK_LABEL(GTK_BIN(bouton)->child), TRUE );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE);
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 FALSE,
			 TRUE,
			 0);
    gtk_signal_connect ( GTK_OBJECT (bouton),
			 "clicked",
			 GTK_SIGNAL_FUNC ( ajout_etat ),
			 NULL );
    gtk_widget_show ( bouton );

    /* on met le bouton personnaliser */

    bouton_personnaliser_etat = gtk_button_new_with_label ( _("Customize report") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_personnaliser_etat ),
			    GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_personnaliser_etat,
			       FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_personnaliser_etat ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( personnalisation_etat ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_personnaliser_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_personnaliser_etat );

    /* on met le bouton dupliquer */

    bouton_dupliquer_etat = gtk_button_new_with_label ( _("Clone report") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_dupliquer_etat ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_dupliquer_etat ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( dupliquer_etat ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_dupliquer_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_dupliquer_etat );

    /* mise en place du bouton effacer état */
    bouton_effacer_etat = gtk_button_new_with_label ( _("Delete report") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_effacer_etat ), GTK_RELIEF_NONE);
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_effacer_etat, FALSE, TRUE, 0);
    gtk_signal_connect ( GTK_OBJECT (bouton_effacer_etat), "clicked",
			 GTK_SIGNAL_FUNC ( efface_etat ), NULL );
    gtk_widget_show ( bouton_effacer_etat );

    if ( !etat_courant )
    {
	gtk_widget_set_sensitive ( bouton_effacer_etat, FALSE );
	gtk_widget_set_sensitive ( bouton_dupliquer_etat, FALSE );
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Clone report")),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Print report")),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Export report")),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Remove report")),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Edit report")),
			       FALSE );

    return ( onglet );

}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* Fonction creation_barre_boutons_etats */
/* renvoie la barre des boutons de la partie droite-bas des rapports financiers */
/*****************************************************************************************************/

GtkWidget *creation_barre_boutons_etats ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *bouton;

    widget_retour = gtk_hbox_new ( FALSE, 5 );
    gtk_widget_show ( widget_retour );

    /* Print report */
    bouton_imprimer_etat = gtk_button_new_with_label ( _("Print") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_imprimer_etat ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_imprimer_etat ),
				"clicked",
				GTK_SIGNAL_FUNC ( impression_etat ),
				NULL );
    gtk_box_pack_start ( GTK_BOX ( widget_retour ), bouton_imprimer_etat,
			 FALSE, FALSE, 0 );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, FALSE );
    gtk_widget_show ( bouton_imprimer_etat );

    /* on met le bouton rafraichir */

    bouton_raffraichir_etat = gtk_button_new_with_label ( _("Refresh") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_raffraichir_etat ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_raffraichir_etat ),
				"clicked",
				GTK_SIGNAL_FUNC ( rafraichissement_etat ),
				NULL );
    gtk_box_pack_start ( GTK_BOX ( widget_retour ), bouton_raffraichir_etat,
			 FALSE, FALSE, 0 );
    gtk_widget_set_sensitive ( bouton_raffraichir_etat, FALSE );
    gtk_widget_show ( bouton_raffraichir_etat );

    /* on met le bouton exporter */

    bouton_exporter_etat = gtk_button_new_with_label ( _("Export") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_exporter_etat ), GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_exporter_etat ), "clicked",
			 GTK_SIGNAL_FUNC ( exporter_etat ), NULL );
    gtk_box_pack_start ( GTK_BOX ( widget_retour ), bouton_exporter_etat,
			 FALSE, FALSE, 0 );
    gtk_widget_show ( bouton_exporter_etat );

    /* on met le bouton importer */

    bouton_importer_etat = gtk_button_new_with_label ( _("Import") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_importer_etat ), GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_importer_etat ), "clicked",
			 GTK_SIGNAL_FUNC ( importer_etat ), NULL );
    gtk_box_pack_start ( GTK_BOX ( widget_retour ), bouton_importer_etat,
			 FALSE, FALSE, 0 );
    gtk_widget_show ( bouton_importer_etat );

    return ( widget_retour );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* Fontion remplissage_liste_etats */
/* vide et remplit la liste des états */
/*****************************************************************************************************/

void remplissage_liste_etats ( void )
{
    GList *pointeur;
    GSList *liste_tmp;
    GtkWidget *label;


    /* on commence par détruire tous les enfants de la vbox */

    pointeur = GTK_BOX ( vbox_liste_etats ) -> children;

    while ( pointeur )
    {
	GtkBoxChild *child;

	child = pointeur -> data;

	pointeur = pointeur -> next;

	gtk_container_remove ( GTK_CONTAINER ( vbox_liste_etats ),
			       child -> widget );
    }

    /* on remplit maintenant avec tous les états */

    liste_tmp = liste_struct_etats;

    while ( liste_tmp )
    {
	struct struct_etat *etat;
	GtkWidget *hbox;
	GtkWidget *bouton;
	GtkWidget *icone;
	GtkWidget *label;

	etat = liste_tmp -> data;

	bouton = gtk_list_button_new ( etat -> nom_etat, 0 );
	gtk_widget_show_all (bouton) ;
	gtk_box_pack_start ( GTK_BOX ( vbox_liste_etats ), bouton,
			     FALSE, FALSE, 0 );

	gtk_signal_connect ( GTK_OBJECT(bouton), "clicked",
			     GTK_SIGNAL_FUNC ( changement_etat ), etat );
	liste_tmp = liste_tmp -> next;
    }

    /*   si on a remplit la liste, c'est qu'il y a eu des modifs ( ajout, dupplication ... ) */
    /* donc on met à jour la liste des tiers */

    mise_a_jour_tiers ();

}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* on propose une liste d'états prémachés et les remplis en fonction du choix */
/* de la personne */
/*****************************************************************************************************/

gboolean ajout_etat ( void )
{
    struct struct_etat *etat;
    struct struct_comparaison_montants_etat *comp_montant;
    GtkWidget *dialog;
    gint resultat;
    GtkWidget *label;
    GtkWidget *frame;
    GtkWidget *option_menu;
    GtkWidget *menu;
    GtkWidget *menu_item;
    GtkWidget *label_description;
    GtkWidget *scrolled_window;


    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION,
				       GTK_BUTTONS_OK_CANCEL,
				       make_hint ( _("Choose template for new report"),
						   _("You are about to create a new report.  For convenience, you can choose between the following templates.  Reports may be customized later." ) ) );

    frame = new_paddingbox_with_title ( GTK_DIALOG(dialog)->vbox, FALSE,
					_("Report type"));

    /* on commence par créer l'option menu */
    option_menu = gtk_option_menu_new ();
    gtk_box_pack_start ( GTK_BOX(frame), option_menu, FALSE, FALSE, 0 );

    /* On met une ligne blanche entre les paddingboxes */
    /*   gtk_box_pack_start ( GTK_BOX(frame), gtk_label_new(""), FALSE, FALSE, 6 ); */

    /* on ajoute maintenant la frame */
    frame = new_paddingbox_with_title ( GTK_DIALOG(dialog)->vbox, FALSE,
					_("Description"));

    /* on met le label dans une scrolled window */
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX(frame), scrolled_window, TRUE, TRUE, 6 );

    /* on ajoute maintenant le label */
    label_description = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_description ), 0, 0 );
    gtk_label_set_line_wrap ( GTK_LABEL ( label_description ), TRUE );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    label_description );

    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( label_description -> parent ),
				   GTK_SHADOW_NONE );

    /* on crée ici le menu qu'on ajoute à l'option menu */
    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("Last month incomes and outgoings"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 0 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    /* on met le texte du 1er choix */
    change_choix_nouvel_etat ( menu_item, label_description );

    menu_item = gtk_menu_item_new_with_label ( _("Current month incomes and outgoings"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item )
	; gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
				GINT_TO_POINTER ( 1 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    menu_item = gtk_menu_item_new_with_label ( _("Annual budget"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 2 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    menu_item = gtk_menu_item_new_with_label ( _("Cheques deposit"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 4 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    menu_item = gtk_menu_item_new_with_label ( _("Monthly outgoings by third party"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 5 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    menu_item = gtk_menu_item_new_with_label ( _("Search"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 6 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    menu_item = gtk_menu_item_new_with_label ( _("Blank report"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 3 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu ), menu );

    gtk_box_set_spacing ( GTK_BOX(GTK_DIALOG(dialog)->vbox), 6 );
    gtk_widget_show_all ( dialog );

    /* on attend le choix de l'utilisateur */

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( dialog );
	return;
    }


    /* on récupère le type d'état voulu */

    resultat = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
						       "no_etat" ));
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));


    /* on crée le nouvel état */

    etat = calloc ( 1, sizeof ( struct struct_etat ));

    etat -> no_etat = ++no_dernier_etat;


    /* on remplit maintenant l'état en fonction de ce qu'on a demandé */

    switch ( resultat )
    {
	case 0:
	    /*  revenus et dépenses du mois précédent  */

	    etat -> nom_etat = g_strdup ( _("Previous month incomes and outgoings") );

	    etat -> separer_revenus_depenses = 1;
	    etat -> no_plage_date = 7;


	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    etat -> type_virement = 2;
	    etat -> utilise_categ = 1;
	    etat -> afficher_sous_categ = 1;
	    etat -> affiche_sous_total_categ = 1;
	    etat -> affiche_sous_total_sous_categ = 1;
	    etat -> afficher_pas_de_sous_categ = 1;
	    etat -> afficher_nom_categ = 1;

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;

	    break;

	case 1:
	    /*  revenus et dépenses du mois courant  */

	    etat -> nom_etat = g_strdup ( _("Current month incomes and outgoings") );

	    etat -> separer_revenus_depenses = 1;
	    etat -> no_plage_date = 3;


	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    etat -> type_virement = 2;
	    etat -> utilise_categ = 1;
	    etat -> afficher_sous_categ = 1;
	    etat -> affiche_sous_total_categ = 1;
	    etat -> affiche_sous_total_sous_categ = 1;
	    etat -> afficher_pas_de_sous_categ = 1;
	    etat -> afficher_nom_categ = 1;

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;

	    break;


	case 2:

	    /* on ajoute le budget annuel */

	    etat -> nom_etat = g_strdup ( _("Annual budget") );

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;

	    etat -> separer_revenus_depenses = 1;
	    etat -> no_plage_date = 4;
	    etat -> utilise_categ = 1;
	    etat -> exclure_ope_sans_categ = 1;
	    etat -> affiche_sous_total_categ = 1;
	    etat -> afficher_sous_categ = 1;
	    etat -> afficher_pas_de_sous_categ = 1;
	    etat -> affiche_sous_total_sous_categ = 1;
	    etat -> afficher_nom_categ = 1;
	    etat -> exclure_montants_nuls = 1;

	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;



	case 3:

	    /* on ajoute un état vierge appelé nouvel état */

	    etat -> nom_etat = g_strdup ( _("New report") );

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;

	    etat -> choix_devise_montant = 1;


	    etat -> afficher_opes = 1;
	    etat -> afficher_date_ope = 1;
	    etat -> afficher_tiers_ope = 1;
	    etat -> afficher_categ_ope = 1;
	    etat -> separer_revenus_depenses = 1;
	    etat -> type_virement = 2;
	    etat -> utilise_categ = 1;
	    etat -> exclure_ope_sans_categ = 1;
	    etat -> affiche_sous_total_categ = 1;
	    etat -> afficher_sous_categ = 1;
	    etat -> affiche_sous_total_sous_categ = 1;
	    etat -> afficher_nom_categ = 1;
	    etat -> exclure_ope_sans_ib = 1;
	    etat -> exclure_montants_nuls = 1;


	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 4:

	    /* remise de chèques */

	    etat -> nom_etat = g_strdup ( _("Cheques deposit") );

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;


	    etat -> afficher_opes = 1;
	    etat -> afficher_nb_opes = 1;
	    etat -> afficher_tiers_ope = 1;
	    etat -> afficher_infobd_ope = 1;
	    etat -> type_virement = 2;
	    etat -> exclure_montants_nuls = 1;
	    etat -> utilise_montant = 1;

	    /* on doit créer une structure de montant qui dit que ça va être positif */

	    comp_montant = calloc ( 1,
				    sizeof ( struct struct_comparaison_montants_etat ));
	    comp_montant -> lien_struct_precedente = -1;
	    comp_montant -> comparateur_1 = 8;
	    comp_montant -> lien_1_2 = 3;

	    etat -> liste_struct_comparaison_montants = g_slist_append ( etat -> liste_struct_comparaison_montants,
									 comp_montant );


	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 5:

	    /* dépenses mensuelles par tiers */

	    etat -> nom_etat = g_strdup ( _("Monthly outgoings by third party") );

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;


	    etat -> afficher_opes = 1;
	    etat -> afficher_nb_opes = 1;
	    etat -> afficher_tiers_ope = 1;
	    etat -> type_classement_ope = 2;
	    etat -> afficher_titre_colonnes = 1;
	    etat -> no_plage_date = 7;
	    etat -> utilise_categ = 1;
	    etat -> affiche_sous_total_categ = 1;
	    etat -> afficher_sous_categ = 1;
	    etat -> afficher_nom_categ = 1;

	    etat -> afficher_date_ope = 1;
	    etat -> afficher_categ_ope = 1;
	    etat -> separer_revenus_depenses = 1;
	    etat -> type_virement = 2;
	    etat -> exclure_ope_sans_categ = 1;
	    etat -> affiche_sous_total_sous_categ = 1;
	    etat -> exclure_ope_sans_ib = 1;
	    etat -> exclure_montants_nuls = 1;


	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 6:

	    /* dépenses mensuelles par tiers */

	    etat -> nom_etat = g_strdup ( _("Search") );

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;


	    etat -> afficher_opes = 1;
	    etat -> afficher_date_ope = 1;
	    etat -> afficher_tiers_ope = 1;
	    etat -> afficher_categ_ope = 1;
	    etat -> afficher_sous_categ_ope = 1;
	    etat -> afficher_type_ope = 1;
	    etat -> afficher_ib_ope = 1;
	    etat -> afficher_sous_ib_ope = 1;
	    etat -> afficher_cheque_ope = 1;
	    etat -> afficher_notes_ope = 1;
	    etat -> afficher_pc_ope = 1;
	    etat -> afficher_rappr_ope = 1;
	    etat -> afficher_infobd_ope = 1;
	    etat -> afficher_exo_ope = 1;

	    etat -> ope_clickables = 1;
	    etat -> no_plage_date = 4;
	    etat -> separation_par_plage = 1;
	    etat -> type_separation_plage = 2;
	    etat -> type_virement = 2;

	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;


	default :
	    dialogue_error ( _("Unknown report type, creation cancelled"));
	    return;
    }

    /* on l'ajoute à la liste */
    liste_struct_etats = g_slist_append ( liste_struct_etats, etat );

    /* on réaffiche la liste des états */
    etat_courant = etat;

    remplissage_liste_etats ();

    gtk_widget_set_sensitive ( bouton_personnaliser_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_raffraichir_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_dupliquer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_effacer_etat, TRUE );

    gtk_label_set_text ( GTK_LABEL ( label_etat_courant ),
			 etat_courant -> nom_etat );

    personnalisation_etat ();
    modification_fichier ( TRUE );

    gtk_widget_destroy ( dialog );

    return FALSE;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void change_choix_nouvel_etat ( GtkWidget *menu_item,
				GtkWidget *label_description )
{
    gchar *description;

    switch ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( menu_item ),
						     "no_etat" )))
    {
	case 0:
	    /* revenus et dépenses du mois dernier  */

	    description = _("This report displays totals for last month's transactions sorted by categories and sub-categories. You just need to select the account(s). By default, all accounts are selected.");
	    break;

	case 1:
	    /* revenus et dépenses du mois en cours  */

	    description = _("This report displays totals of current month's transactions sorted by categories and sub-categories. You just need to select the account(s). By default, all accounts are selected.");
	    break;

	case 2:
	    /* budget annuel  */

	    description = _("This report displays annual budget. You just need to select the account(s). By default all accounts are selected.");
	    break;

	case 3:
	    /* etat vierge  */

	    description = _("This report is an empty one. You need to customise it entirely.");
	    break;

	case 4:
	    /* remise de chèques  */

	    description = _("This report displays the cheques deposit. You just need to select the account(s). By default all accounts are selected.");
	    break;

	case 5:
	    /* dépenses mensuelles par tiers  */

	    description = _("This report displays current month's outgoings sorted by third parties. You just need to select the account(s). By default all accounts areselected.");
	    break;

	case 6:
	    /* recherche  */

	    description = _("This report displays all the information for all transactions of all accounts for the current year. You just have to add the amount, date, third parties etc. criteria thant you want. By default the transactions are clickables.");
	    break;

	default:

	    description = _("???? should not be displayed...");
    }

    gtk_label_set_text ( GTK_LABEL ( label_description ),
			 description );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
void efface_etat ( void )
{
    GtkWidget *dialog;

    if ( !liste_struct_etats || !etat_courant )
	return;

    if ( !question_yes_no_hint ( g_strdup_printf (_("Delete report \"%s\"?"),
						  etat_courant -> nom_etat ),
				 _("This will irreversibly remove this report.  There is no undo for this.") ))
	return;

    /*   on met l'état courant à -1 et */
    /* le bouton à null, et le label de l'état en cours à rien */
    liste_struct_etats = g_slist_remove ( liste_struct_etats, etat_courant );

    etat_courant = NULL;
    bouton_etat_courant = NULL;
    gtk_label_set_text ( GTK_LABEL ( label_etat_courant ), "" );
    gtk_widget_set_sensitive ( bouton_personnaliser_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_raffraichir_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_dupliquer_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_effacer_etat, FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Clone report")),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Print report")),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Export report")),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Remove report")),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Edit report")),
			       FALSE );


    if ( GTK_BIN ( scrolled_window_etat ) -> child )
	gtk_widget_hide ( GTK_BIN ( scrolled_window_etat ) -> child );

    /* on réaffiche la liste des états */

    remplissage_liste_etats ();
    modification_fichier ( TRUE );

}
/*****************************************************************************************************/


/*****************************************************************************************************/
void changement_etat ( GtkWidget *bouton,
		       struct struct_etat *etat )
{
    GtkWidget *icone;

    bouton_etat_courant = bouton;
    etat_courant = etat;
    gtk_widget_set_sensitive ( bouton_personnaliser_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_raffraichir_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_dupliquer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_effacer_etat, TRUE );

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Clone report")),
			       TRUE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Print report")),
			       TRUE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Export report")),
			       TRUE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Remove report")),
			       TRUE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/Reports/Edit report")),
			       TRUE );

    gtk_label_set_text ( GTK_LABEL ( label_etat_courant ), etat -> nom_etat );
    gtk_label_set_line_wrap ( GTK_LABEL ( label_etat_courant ), TRUE );

    /* on affiche l'état */
    rafraichissement_etat ( etat );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint recherche_etat_par_no ( struct struct_etat *etat,
			     gint *no_etat )
{
    return ( etat -> no_etat != GPOINTER_TO_INT (no_etat) );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void exporter_etat ( void )
{
    GtkWidget *fenetre_nom;
    gint resultat;
    struct stat test_fichier;
    gchar *nom_etat;

    fenetre_nom = gtk_file_selection_new (_("Export report") );
    gtk_file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
				      dernier_chemin_de_travail );
    gtk_entry_set_text ( GTK_ENTRY ( GTK_FILE_SELECTION ( fenetre_nom )->selection_entry),
			 g_strconcat ( etat_courant -> nom_etat,
				       ".egsb",
				       NULL ));
    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nom_etat =g_strdup (gtk_file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom )));

	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

	    /* vérification que c'est possible */

	    if ( !strlen ( nom_etat ))
		return;

	    if ( stat ( nom_etat,
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
		    dialogue ( g_strdup_printf ( _("Invalid filename \"%s\"!"),
						 nom_etat ));
		    return;
		}
	    }

	    if ( !enregistre_etat ( nom_etat ))
	    {
		dialogue ( _("Cannot save file.") );
		return;
	    }

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
	    return;
    }
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void importer_etat ( void )
{
    GtkWidget *fenetre_nom;
    gint resultat;
    gchar *nom_etat;

    fenetre_nom = gtk_file_selection_new ( _("Import a report"));
    gtk_file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
				      dernier_chemin_de_travail );
    gtk_entry_set_text ( GTK_ENTRY ( GTK_FILE_SELECTION ( fenetre_nom )->selection_entry),
			 g_strconcat ( dernier_chemin_de_travail,
				       ".egsb",
				       NULL ));

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nom_etat =g_strdup (gtk_file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom )));

	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

	    /* vérification que c'est possible */

	    if ( !strlen ( nom_etat ))
		return;


	    if ( !charge_etat ( nom_etat ))
	    {
		dialogue ( _("Cannot import file.") );
		return;
	    }

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
	    return;
    }
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* cette fonction crée une copie de l'état courant */
/*****************************************************************************************************/

void dupliquer_etat ( void )
{
    struct struct_etat *etat;
    GSList *liste_tmp;

    etat = calloc ( 1,
		    sizeof ( struct struct_etat ));


    /* on recopie les données de l'état courant */

    memcpy ( etat,
	     etat_courant,
	     sizeof ( struct struct_etat ));

    /* il reste juste à faire une copie des listes et des chaines pour terminer */

    etat -> no_etat = ++no_dernier_etat;

    etat -> nom_etat = g_strdup ( etat_courant -> nom_etat );
    etat -> no_exercices = g_slist_copy ( etat_courant -> no_exercices );

    if ( etat_courant -> date_perso_debut )
	etat -> date_perso_debut = g_date_new_dmy ( g_date_day ( etat_courant -> date_perso_debut ),
						    g_date_month ( etat_courant -> date_perso_debut ),
						    g_date_year ( etat_courant -> date_perso_debut ));

    if ( etat_courant -> date_perso_fin )
	etat -> date_perso_fin = g_date_new_dmy ( g_date_day ( etat_courant -> date_perso_fin ),
						  g_date_month ( etat_courant -> date_perso_fin ),
						  g_date_year ( etat_courant -> date_perso_fin ));

    etat -> type_classement = g_list_copy ( etat_courant -> type_classement );
    etat -> no_comptes = g_slist_copy ( etat_courant -> no_comptes );
    etat -> no_comptes_virements = g_slist_copy ( etat_courant -> no_comptes_virements );
    etat -> no_categ = g_slist_copy ( etat_courant -> no_categ );
    etat -> no_ib = g_slist_copy ( etat_courant -> no_ib );
    etat -> no_tiers = g_slist_copy ( etat_courant -> no_tiers );


    /* on fait une copie de la liste des textes */

    if ( etat -> liste_struct_comparaison_textes )
    {
	etat -> liste_struct_comparaison_textes = NULL;

	liste_tmp = etat_courant -> liste_struct_comparaison_textes;

	while ( liste_tmp )
	{
	    struct struct_comparaison_textes_etat *ancien_comp_textes;
	    struct struct_comparaison_textes_etat *comp_textes;

	    ancien_comp_textes = liste_tmp -> data;

	    comp_textes = calloc ( 1,
				   sizeof ( struct struct_comparaison_textes_etat ));

	    memcpy ( comp_textes,
		     ancien_comp_textes,
		     sizeof ( struct struct_comparaison_textes_etat ));

	    comp_textes -> texte = g_strdup ( comp_textes -> texte );

	    etat -> liste_struct_comparaison_textes = g_slist_append ( etat -> liste_struct_comparaison_textes,
								       comp_textes );
	    liste_tmp = liste_tmp -> next;
	}
    }

    /* on fait une copie de la liste des montants */

    if ( etat -> liste_struct_comparaison_montants )
    {
	etat -> liste_struct_comparaison_montants = NULL;

	liste_tmp = etat_courant -> liste_struct_comparaison_montants;

	while ( liste_tmp )
	{
	    struct struct_comparaison_montants_etat *ancien_comp_montants;
	    struct struct_comparaison_montants_etat *comp_montants;

	    ancien_comp_montants = liste_tmp -> data;

	    comp_montants = calloc ( 1,
				     sizeof ( struct struct_comparaison_montants_etat ));

	    memcpy ( comp_montants,
		     ancien_comp_montants,
		     sizeof ( struct struct_comparaison_montants_etat ));

	    etat -> liste_struct_comparaison_montants = g_slist_append ( etat -> liste_struct_comparaison_montants,
									 comp_montants );
	    liste_tmp = liste_tmp -> next;
	}
    }



    /* on l'ajoute à la liste */

    liste_struct_etats = g_slist_append ( liste_struct_etats,
					  etat );

    /* on réaffiche la liste des états */

    etat_courant = etat;

    remplissage_liste_etats ();

    gtk_widget_set_sensitive ( bouton_personnaliser_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_raffraichir_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_dupliquer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_effacer_etat, TRUE );

    gtk_label_set_text ( GTK_LABEL ( label_etat_courant ), etat_courant -> nom_etat );

    gtk_widget_set_sensitive ( bouton_effacer_etat, TRUE );

    personnalisation_etat ();
    modification_fichier ( TRUE );
}
/*****************************************************************************************************/
