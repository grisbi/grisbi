/*  Fichier qui s'occupe de former les différentes fenêtres de l'paned_onglet_operations des opés */
/*      paned_onglet_operations_operations.c */

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



#define START_INCLUDE
#include "operations_onglet.h"
#include "equilibrage.h"
#include "operations_liste.h"
#include "operations_formulaire.h"
#include "ventilation.h"
#include "operations_comptes.h"
#include "search_glist.h"
#define END_INCLUDE

#define START_STATIC
#define END_STATIC




GtkWidget *paned_onglet_operations;



#define START_EXTERN
extern GtkWidget *formulaire;
extern GtkWidget *frame_droite_bas;
extern GtkWidget *notebook_comptes_equilibrage;
extern GtkWidget *notebook_formulaire;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
#define END_EXTERN



/*****************************************************************************************************/
GtkWidget *creation_onglet_operations ( void )
{
    GtkWidget *frame_gauche;
    GtkWidget *vbox;
    GtkWidget *frame_droite_haut;
    GtkWidget *fenetre_operations;

    paned_onglet_operations = gtk_hpaned_new ( );

    if ( !etat.largeur_colonne_comptes_operation )
	etat.largeur_colonne_comptes_operation = 200;

    gtk_paned_set_position ( GTK_PANED(paned_onglet_operations), etat.largeur_colonne_comptes_operation );
    gtk_container_set_border_width ( GTK_CONTAINER ( paned_onglet_operations ), 10 );
    gtk_widget_show ( paned_onglet_operations );

    /*   création de la fenetre des comptes / ventilation / équilibrage
	 à gauche */
    frame_gauche = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_gauche ), GTK_SHADOW_IN );
    gtk_paned_pack1 ( GTK_PANED(paned_onglet_operations), frame_gauche, TRUE, TRUE );
    gtk_widget_show (frame_gauche);

    notebook_comptes_equilibrage = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK( notebook_comptes_equilibrage ),
				 FALSE );
    gtk_container_add ( GTK_CONTAINER ( frame_gauche ),
			notebook_comptes_equilibrage );
    gtk_widget_show ( notebook_comptes_equilibrage );

    /*  Création de la fenêtre des comptes */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			       creation_liste_comptes (),
			       gtk_label_new ( _("Accounts") ) );


   /* création de la fenetre de ventilation */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			       creation_verification_ventilation (),
			       gtk_label_new ( _("Breakdown") ) );


    /* création de la fenetre de l'équilibrage */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			       creation_fenetre_equilibrage (),
			       gtk_label_new ( _("Reconciliation") ) );

    /* création de la partie droite */
    vbox = gtk_vbox_new ( FALSE, 10 );
    gtk_paned_pack2 ( GTK_PANED(paned_onglet_operations), vbox, TRUE, TRUE );
    gtk_widget_show ( vbox );

    /*  Création de la liste des opérations */
    frame_droite_haut = gtk_frame_new ( NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), frame_droite_haut, TRUE, TRUE, 0 );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_droite_haut ), GTK_SHADOW_IN );
    gtk_widget_show (frame_droite_haut);

    fenetre_operations = creation_fenetre_operations();
    gtk_container_set_border_width ( GTK_CONTAINER ( fenetre_operations ), 10);
    gtk_container_add ( GTK_CONTAINER ( frame_droite_haut ), fenetre_operations );
    gtk_widget_show ( fenetre_operations ); 


    /* création du formulaire */

    frame_droite_bas = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_droite_bas ), GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( vbox ), frame_droite_bas, FALSE, FALSE, 0 );


    /* création du notebook du formulaire ( contient le formulaire et le
       formulaire simplifié pour la ventilation ) */
    notebook_formulaire = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK( notebook_formulaire ), FALSE );
    gtk_container_add ( GTK_CONTAINER ( frame_droite_bas ), notebook_formulaire );
    gtk_widget_show ( notebook_formulaire );

    if ( etat.formulaire_toujours_affiche )
	gtk_widget_show (frame_droite_bas);

    /* création du formulaire */
    formulaire = creation_formulaire ();
    gtk_container_set_border_width ( GTK_CONTAINER ( formulaire ), 10);
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_formulaire ), formulaire,
			       gtk_label_new ( _("Form") ) );
    gtk_widget_show (formulaire);

    /* création de la fenetre de ventilation */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_formulaire ),
			       creation_formulaire_ventilation (),
			       gtk_label_new ( _("Breakdown") ) );

    /* on remet la fenetre du formulaire sur le formulaire  */
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_formulaire ), 0 );

    return ( paned_onglet_operations );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* renvoie l'adr de l'opé demandée par son no */
/* ou NULL si pas trouvée */
/*****************************************************************************************************/
struct structure_operation *operation_par_no ( gint no_operation,
					       gint no_compte )
{
    GSList *liste_tmp;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;
    
    liste_tmp = g_slist_find_custom ( LISTE_OPERATIONS,
				      GINT_TO_POINTER ( no_operation ),
				      (GCompareFunc) recherche_operation_par_no );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* renvoie l'adr de l'opé demandée par son no de cheque */
/* ou NULL si pas trouvée */
/*****************************************************************************************************/
struct structure_operation *operation_par_cheque ( gint no_cheque,
						   gint no_compte )
{
    GSList *liste_tmp;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;
    
    liste_tmp = g_slist_find_custom ( LISTE_OPERATIONS,
				      GINT_TO_POINTER ( no_cheque ),
				      (GCompareFunc) recherche_operation_par_cheque );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* renvoie l'adr de l'opé demandée par son no id*/
/* ou NULL si pas trouvée */
/*****************************************************************************************************/
struct structure_operation *operation_par_id ( gchar *no_id,
					       gint no_compte )
{
    GSList *liste_tmp;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;
    
    liste_tmp = g_slist_find_custom ( LISTE_OPERATIONS,
				      g_strstrip ( no_id ),
				      (GCompareFunc) recherche_operation_par_id );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/*****************************************************************************************************/

