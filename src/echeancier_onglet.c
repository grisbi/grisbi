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
#include "structures.h"
#include "variables-extern.c"
#include "echeancier_onglet.h"

#include "echeancier_formulaire.h"
#include "echeancier_liste.h"
#include "echeancier_ventilation.h"
#include "search_glist.h"


GtkWidget *notebook_calendrier_ventilations;
GtkWidget *formulaire_echeancier;
GtkWidget *fenetre_ventilations;
GtkWidget *notebook_formulaire_echeances;
GtkWidget *notebook_liste_ventil_echeances;
GtkWidget *paned_onglet_echeancier;


extern GtkWidget *frame_formulaire_echeancier;
extern GSList *liste_struct_echeances;



/*****************************************************************************************************/
GtkWidget *creation_onglet_echeancier ( void )
{
    GtkWidget *frame_gauche;
    GtkWidget *vbox;
    GtkWidget *frame_droite_haut;

    paned_onglet_echeancier = gtk_hpaned_new ( );

    if ( !etat.largeur_colonne_echeancier )
	etat.largeur_colonne_echeancier = 200;

    gtk_paned_set_position ( GTK_PANED(paned_onglet_echeancier),
			     etat.largeur_colonne_echeancier );
    gtk_container_set_border_width ( GTK_CONTAINER ( paned_onglet_echeancier ),
				     10 );
    gtk_widget_show ( paned_onglet_echeancier );

    /*   création de la fenetre des calendrier / ventilation à gauche */

    frame_gauche = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_gauche ),
				GTK_SHADOW_IN );
    gtk_paned_pack1 ( GTK_PANED(paned_onglet_echeancier), 
		      frame_gauche,
		      TRUE,
		      TRUE );
    gtk_widget_show (frame_gauche);

    notebook_calendrier_ventilations = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK( notebook_calendrier_ventilations ),
				 FALSE );
    gtk_container_add ( GTK_CONTAINER ( frame_gauche ),
			notebook_calendrier_ventilations );
    gtk_widget_show ( notebook_calendrier_ventilations );

    /*  Création de la fenêtre du calendrier */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_calendrier_ventilations ),
			       creation_partie_gauche_echeancier(),
			       gtk_label_new ( _("Calendar") ) );


   /* création de la fenetre de ventilation */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_calendrier_ventilations ),
			       creation_verification_ventilation_echeances (),
			       gtk_label_new ( _("Breakdown") ) );



    /* création de la partie droite */
    vbox = gtk_vbox_new ( FALSE,
			  10 );
    gtk_paned_pack2 ( GTK_PANED(paned_onglet_echeancier),
		      vbox,
		      TRUE,
		      TRUE );
    gtk_widget_show ( vbox );

    /*  Création de la liste des opérations */
    frame_droite_haut = gtk_frame_new ( NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 frame_droite_haut, 
			 TRUE,
			 TRUE,
			 0 );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_droite_haut ),
				GTK_SHADOW_IN );
    gtk_widget_show (frame_droite_haut);

    notebook_liste_ventil_echeances = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK( notebook_liste_ventil_echeances ),
				 FALSE );
    gtk_container_add ( GTK_CONTAINER ( frame_droite_haut ),
			notebook_liste_ventil_echeances );
    gtk_widget_show ( notebook_liste_ventil_echeances );

    /*  Création de la fenêtre de la liste des échéances */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_liste_ventil_echeances ),
			       creation_liste_echeances(),
			       gtk_label_new ( _("Scheduler list") ) );

    /*  Création de la fenêtre de la liste des ventils des échéances */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_liste_ventil_echeances ),
			       creation_fenetre_ventilation_echeances(),
			       gtk_label_new ( _("Scheduler broken down transactions") ) );





    /* création du formulaire */

    frame_formulaire_echeancier = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_formulaire_echeancier ),
				GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 frame_formulaire_echeancier,
			 FALSE,
			 FALSE,
			 0 );


    /* création du notebook du formulaire ( contient le formulaire et le
       formulaire simplifié pour la ventilation ) */
    notebook_formulaire_echeances = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK( notebook_formulaire_echeances ),
				 FALSE );
    gtk_container_add ( GTK_CONTAINER ( frame_formulaire_echeancier ),
			notebook_formulaire_echeances );
    gtk_widget_show ( notebook_formulaire_echeances );

    if ( etat.formulaire_echeancier_toujours_affiche )
	gtk_widget_show (frame_formulaire_echeancier);

    /* création du formulaire */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_formulaire_echeances ),
			       creation_formulaire_echeancier (),
			       gtk_label_new ( _("Form") ) );

    /* création du formulaire de ventilation */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_formulaire_echeances ),
			       creation_formulaire_ventilation_echeances  (),
			       gtk_label_new ( _("Breakdown") ) );


    return ( paned_onglet_echeancier );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* renvoie l'adr de l'échéance demandée par son no */
/* renvoie NULL si pas trouvée */
/*****************************************************************************************************/
struct operation_echeance *echeance_par_no ( gint no_echeance )
{
   if ( no_echeance )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_echeances,
					  GINT_TO_POINTER ( no_echeance ),
					  (GCompareFunc) recherche_echeance_par_no );

	if ( liste_tmp )
	    return ( liste_tmp -> data );
    }
    return NULL;
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* renvoie l'adr de l'échéance demandée par son no de banque */
/* renvoie NULL si pas trouvée */
/*****************************************************************************************************/
struct operation_echeance *echeance_par_no_compte ( gint no_compte )
{
   if ( no_compte )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_echeances,
					  GINT_TO_POINTER ( no_compte ),
					  (GCompareFunc) recherche_echeance_par_no_compte );

	if ( liste_tmp )
	    return ( liste_tmp -> data );
    }
    return NULL;
}
/*****************************************************************************************************/

