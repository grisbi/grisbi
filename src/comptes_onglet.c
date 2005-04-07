/* fichier qui s'occupe de la gestion des comptes */
/*           gestion_comptes.c */

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
#include "comptes_onglet.h"
#include "comptes_gestion.h"
#include "comptes_traitements.h"
#include "gsb_account.h"
#include "operations_comptes.h"
#include "gtk_list_button.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *creation_liste_comptes_onglet ( void );
/*END_STATIC*/



GtkWidget *paned_onglet_comptes;
GtkWidget *bouton_supprimer_compte;
GtkWidget *label_compte_courant_onglet;
gint compte_courant_onglet;
GtkWidget *vbox_liste_comptes_onglet;


/*START_EXTERN*/
/*END_EXTERN*/


/**
 *
 *
 */
GtkWidget *creation_onglet_comptes ( void )
{
    GtkWidget *frame;

    paned_onglet_comptes = gtk_hpaned_new ( );
    if ( !etat.largeur_colonne_comptes_comptes )
	etat.largeur_colonne_comptes_comptes = 200;

    gtk_paned_set_position ( GTK_PANED(paned_onglet_comptes), etat.largeur_colonne_comptes_comptes );
    gtk_container_set_border_width ( GTK_CONTAINER ( paned_onglet_comptes ), 10 );
    gtk_widget_show ( paned_onglet_comptes );


    /*   création de la fenetre des comptes / ventilation / équilibrage à gauche */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_IN );
    gtk_paned_pack1 ( GTK_PANED(paned_onglet_comptes), frame, TRUE, TRUE );
    gtk_widget_show (frame);

    gtk_container_add ( GTK_CONTAINER ( frame ), creation_liste_comptes_onglet ());


    /* création de la partie droite */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_IN );
    gtk_paned_pack2 ( GTK_PANED(paned_onglet_comptes), frame, TRUE, TRUE );
    gtk_widget_show (frame);

    gtk_container_add ( GTK_CONTAINER ( frame ), creation_details_compte ());

    /*  Création d'une icone et du nom par compte, et placement dans la
	liste selon l'ordre désiré et met le livre ouvert pour le compte
	0 */

    compte_courant_onglet = 0;
    gsb_account_page_create_accounts_list ();
    remplissage_details_compte ();

    return ( paned_onglet_comptes );
}



/**
 *
 *
 */
GtkWidget *creation_liste_comptes_onglet ( void )
{
    GtkWidget *onglet;
    GtkWidget *frame;
    GtkWidget *button;
    GtkWidget *vbox_frame;
    GtkWidget *scrolled_window;

    /*  Création d'une fenêtre générale*/

    onglet = gtk_vbox_new ( FALSE, 10);
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ), 10 );
    gtk_widget_show ( onglet );

    /*  Création du label Comptes en haut */
    /*   on place le label dans une frame */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
				GTK_SHADOW_ETCHED_OUT );
    gtk_box_pack_start ( GTK_BOX (onglet),
			 frame,
			 FALSE,
			 TRUE,
			 0);
    gtk_widget_show (frame);


    /*   on ne met rien dans le label, il sera rempli ensuite */

    label_compte_courant_onglet = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC (label_compte_courant_onglet  ), 0.5, 0.5);
    gtk_container_add ( GTK_CONTAINER ( frame ),
			label_compte_courant_onglet );
    gtk_widget_show (label_compte_courant_onglet);


    /*  Création de la fenêtre des comptes */
    /*  qui peut contenir des barres de défilement si */
    /*  nécessaire */

    scrolled_window = gtk_scrolled_window_new ( NULL, NULL);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start ( GTK_BOX ( onglet ), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show ( scrolled_window );


    /*  création d'une vbox contenant la liste des comptes */

    vbox_liste_comptes_onglet = gtk_vbox_new ( FALSE, 0);
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (scrolled_window ),
					    vbox_liste_comptes_onglet );
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window )  -> child ),
				   GTK_SHADOW_NONE );
    gtk_widget_show ( vbox_liste_comptes_onglet );


    /* ajoute les buttons nouveau et supprimer */
    /* les 2 seront intégrés dans une frame */

    frame = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_ETCHED_IN );
    gtk_box_pack_start ( GTK_BOX ( onglet ), frame, FALSE, TRUE, 0);
    gtk_widget_show ( frame );


    vbox_frame = gtk_vbox_new ( FALSE, 5 );
    gtk_container_add ( GTK_CONTAINER  ( frame ), vbox_frame );
    gtk_widget_show ( vbox_frame );

    /* mise en place des buttons du bas */

    button = gtk_button_new_with_label ( _("New account") );
    gtk_button_set_relief ( GTK_BUTTON ( button ), GTK_RELIEF_NONE);
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ), button, FALSE, TRUE, 0);
    gtk_signal_connect ( GTK_OBJECT (button), "clicked",
			 GTK_SIGNAL_FUNC ( new_account ), NULL );
    gtk_widget_show ( button );


    /* mise en place du button "supprimer un compte" */

    bouton_supprimer_compte = gtk_button_new_with_label ( _("Remove an account") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_supprimer_compte ), GTK_RELIEF_NONE);
    gtk_box_pack_start ( GTK_BOX ( vbox_frame ), bouton_supprimer_compte,
			 FALSE, TRUE, 0);
    gtk_signal_connect ( GTK_OBJECT (bouton_supprimer_compte), "clicked",
			 GTK_SIGNAL_FUNC ( delete_account ), NULL );
    gtk_widget_show ( bouton_supprimer_compte );

    if ( !gsb_account_get_accounts_amount () )
	gtk_widget_set_sensitive ( bouton_supprimer_compte, FALSE );

    return ( onglet );
}
/*****************************************************************************************************/




/* ********************************************************************************************************** */
void changement_compte_onglet ( gint *compte )
{
    /* demande si nécessaire si on enregistre */
    sort_du_detail_compte ();

    /* change le nom du compte courant */
    gtk_label_set_text ( GTK_LABEL ( label_compte_courant_onglet),
			 gsb_account_get_name (GPOINTER_TO_INT (compte)));

    compte_courant_onglet = GPOINTER_TO_INT (compte);

    remplissage_details_compte ();
}
/* ********************************************************************************************************** */



/** erase and create the clickable list of accounts, on the left of the accounts page
 * \param none
 * \return FALSE;
 * */

gboolean gsb_account_page_create_accounts_list ( void )
{
    GSList *list_tmp;

    if ( DEBUG )
	printf ( "gsb_account_page_create_accounts_list\n" );

    /*     erase the normal accounts */

    while ( GTK_BOX ( vbox_liste_comptes_onglet ) -> children )
	gtk_container_remove ( GTK_CONTAINER ( vbox_liste_comptes_onglet ),
			       (( GtkBoxChild *) ( GTK_BOX ( vbox_liste_comptes_onglet ) -> children -> data )) -> widget );

    /* create the list : a button and an icon for each account */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	GtkWidget *button;
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	button = gsb_account_list_gui_create_account_button (i,
							     2,
							     G_CALLBACK ( changement_compte_onglet ));
	gtk_box_pack_start (GTK_BOX (vbox_liste_comptes_onglet), button, FALSE, FALSE, 0);
	gtk_widget_show (button);

	/* 	    si c'est le compte courant, on ouvre le livre */

	if ( i == compte_courant_onglet )
	{
	    gtk_list_button_clicked ( GTK_BUTTON ( button ));
	    gtk_label_set_text ( GTK_LABEL ( label_compte_courant_onglet),
				 gsb_account_get_name (i));
	}

	list_tmp = list_tmp -> next;
    }
    return FALSE;
}


    
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
