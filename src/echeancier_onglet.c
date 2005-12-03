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
#include "echeancier_formulaire.h"
#include "gsb_scheduler_list.h"
#include "include.h"
#include "structures.h"
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
    GtkWidget *vbox;

    /* création de la partie droite : les listes échéances/ventil
     * en haut, formulaires en bas */

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_widget_show ( vbox );

    /*  Création de la liste des opérations */
    notebook_liste_ventil_echeances = gtk_notebook_new ();
    gtk_notebook_set_show_border ( GTK_NOTEBOOK( notebook_liste_ventil_echeances ), FALSE );
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK( notebook_liste_ventil_echeances ), FALSE );
    gtk_box_pack_start ( GTK_BOX(vbox), notebook_liste_ventil_echeances, TRUE, TRUE, 0 );
    gtk_widget_show ( notebook_liste_ventil_echeances );

    /*  Création de la fenêtre de la liste des échéances */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_liste_ventil_echeances ),
			       gsb_scheduler_list_create_list(),
			       gtk_label_new ( _("Scheduler list") ) );

    /* création des formulaires */
    frame_formulaire_echeancier = gtk_expander_new_with_mnemonic ( g_strconcat ( 
							 "<b>",
							 _("Scheduled transaction _form"),
							 "</b>", NULL ) );
    gtk_widget_show ( frame_formulaire_echeancier );
    gtk_expander_set_expanded ( GTK_EXPANDER ( frame_formulaire_echeancier ), 
				etat.formulaire_toujours_affiche );
    gtk_expander_set_use_markup ( GTK_EXPANDER ( frame_formulaire_echeancier ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), frame_formulaire_echeancier, FALSE, FALSE, 0 );

    /* création du notebook du formulaire ( contient le formulaire et le
       formulaire simplifié pour la ventilation ) */
    notebook_formulaire_echeances = gtk_notebook_new ();
    gtk_notebook_set_show_border ( GTK_NOTEBOOK( notebook_formulaire_echeances ), FALSE );
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK( notebook_formulaire_echeances ), FALSE );
    gtk_container_add ( GTK_CONTAINER ( frame_formulaire_echeancier ),
			notebook_formulaire_echeances );
    gtk_widget_show ( notebook_formulaire_echeances );

    /* création du formulaire */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_formulaire_echeances ),
			       creation_formulaire_echeancier (),
			       gtk_label_new ( _("Form") ) );

    return ( vbox );
}
/*****************************************************************************************************/



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
