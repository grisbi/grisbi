/*  Fichier qui s'occupe de former les différentes fenêtres de l'onglet des opés */
/*      onglet_operations.c */

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
#include "comptes_onglet.h"
#include "gtkcombofix.h"
#include "ventilation.h"
#include "operations_liste.h"
#include "comptes_onglet.h"
#include "equilibrage.h"
#include "operations_formulaire.h"
#include "echeancier_formulaire.h"
#include "operations_comptes.h"


/*****************************************************************************************************/
GtkWidget *creation_onglet_operations ( void )
{
  GtkWidget *onglet;
  GtkWidget *frame_gauche;
  GtkWidget *vbox;
  GtkWidget *frame_droite_haut;
  GtkWidget *fenetre_operations;

  onglet = gtk_hpaned_new ( );
  gtk_paned_set_position ( GTK_PANED(onglet), 200 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ), 10 );
  gtk_widget_show ( onglet );

  /*   création de la fenetre des comptes / ventilation / équilibrage
       à gauche */
  frame_gauche = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_gauche ), GTK_SHADOW_IN );
  gtk_paned_pack1 ( GTK_PANED(onglet), frame_gauche, TRUE, TRUE );
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
  gtk_paned_pack2 ( GTK_PANED(onglet), vbox, TRUE, TRUE );
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
  gtk_signal_connect ( GTK_OBJECT ( frame_droite_bas ), "show",
		       GTK_SIGNAL_FUNC ( allocation_taille_formulaire ), NULL );
  gtk_signal_connect ( GTK_OBJECT ( frame_droite_bas ), "hide",
		       GTK_SIGNAL_FUNC ( efface_formulaire ), NULL );
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

  return ( onglet );
}
/*****************************************************************************************************/
