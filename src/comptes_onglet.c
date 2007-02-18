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
#include "./comptes_gestion.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/



GtkWidget *paned_onglet_comptes;
GtkWidget *bouton_supprimer_compte;
GtkWidget *label_compte_courant_onglet;
gint compte_courant_onglet;
GtkWidget *vbox_liste_comptes_onglet;


/*START_EXTERN*/
/*END_EXTERN*/


/**
 * Create all user interface elements that displays the details of an
 * account and fill them with current account.
 *
 * \return A newly-allocated vbox.
 */
GtkWidget *creation_onglet_comptes ( void )
{
    GtkWidget * box;

    compte_courant_onglet = 0;
    box = creation_details_compte ();

    gtk_widget_show_all (box);

    return ( box );
}




    
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
