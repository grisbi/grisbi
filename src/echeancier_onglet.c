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
#include "echeancier_onglet.h"
#include "echeancier_ventilation.h"
#include "echeancier_formulaire.h"
#include "echeancier_liste.h"
#include "echeancier_infos.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/



GtkWidget *notebook_calendrier_ventilations;
GtkWidget *formulaire_echeancier;
GtkWidget *notebook_formulaire_echeances;
GtkWidget *notebook_liste_ventil_echeances;

/*START_EXTERN*/
extern GtkWidget *formulaire;
extern GtkWidget *frame_formulaire_echeancier;
/*END_EXTERN*/

/*****************************************************************************************************/
GtkWidget *creation_onglet_echeancier ( void )
{
    GtkWidget *frame;
    GtkWidget *vbox;

    /* création de la partie droite : les listes échéances/ventil
     * en haut, formulaires en bas */

    vbox = gtk_vbox_new ( FALSE, 10 );
    gtk_widget_show ( vbox );

    /*  Création de la liste des opérations */

    frame = gtk_frame_new ( NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 frame, 
			 TRUE,
			 TRUE,
			 0 );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
				GTK_SHADOW_IN );
    gtk_widget_show (frame);

    notebook_liste_ventil_echeances = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK( notebook_liste_ventil_echeances ),
				 FALSE );
    gtk_container_add ( GTK_CONTAINER ( frame ),
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


    /* création des formulaires */

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

    return ( vbox );
}
/*****************************************************************************************************/



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
