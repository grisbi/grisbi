/* Fichier traitement_variables.c */
/* Contient toutes les procédures relatives au traitement des variables */

/*     Copyright (C) 2000-2001  Cédric Auger */
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
#include "en_tete.h"



/*****************************************************************************************************/
/* fonction appelée lors de modification ou non de fichier */
/* TRUE indique que le fichier a été modifié */
/* FALSE non */
/* ajuste la sensitive des menus en fonction */
/*****************************************************************************************************/

void modification_fichier ( gboolean modif )
{

  if ( modif )
    {
      etat.modification_fichier = 1;
      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[SAUVER].widget ),
				 TRUE );
    }
  else
    {
      etat.modification_fichier = 0;
      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[SAUVER].widget ),
				 FALSE );

      if ( ( pid_applet = gnome_config_get_int ( "/Grisbi_applet/PID/PID" )))
	kill ( pid_applet, SIGUSR1 );

    }

}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* fonction appelée à chaque ouverture de fichier et qui initialise les variables globales */
/* si ouverture = TRUE, la fonction considère qu'on est en train d'ouvrir un fichier et ajuste les menus */
/* en conséquence */
/* sinon, suppose que fermeture de fichier */
/*****************************************************************************************************/

void init_variables ( gboolean ouverture )
{

  if ( ouverture )
    {
      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[SAUVER_SOUS].widget ),
				 TRUE );
      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[EXPORTER].widget ),
				 TRUE );
      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[FERMER].widget ),
				 TRUE );

      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_comptes[1].widget ),
				 TRUE );
    }
  else
    {
      etat.modification_fichier = 0;

      nom_fichier_comptes = NULL;

      nb_comptes = 0;
      no_derniere_operation = 0;
      p_tab_nom_de_compte = NULL;
      ordre_comptes = NULL;
      compte_courant = 0;
      solde_label = NULL;
      etat.ancienne_date = 0;

      nom_fichier_backup = NULL;

      gsliste_echeances = NULL;
      nb_echeances = 0;
      no_derniere_echeance = 0;
      affichage_echeances = 3;
      affichage_echeances_perso_nb_libre = 0;
      affichage_echeances_perso_j_m_a = 0;

      liste_struct_tiers = NULL;
      nb_enregistrements_tiers = 0;
      no_dernier_tiers = 0;

      liste_struct_categories = NULL;
      nb_enregistrements_categories = 0;
      no_derniere_categorie = 0;

      liste_struct_imputation = NULL;
      nb_enregistrements_imputations = 0;
      no_derniere_imputation = 0;

      liste_struct_devises = NULL;
      nb_devises = 0;
      no_derniere_devise = 0;
      devise_nulle = calloc ( 1,
			      sizeof ( struct struct_devise ));
      no_devise_totaux_tiers = 1;


      liste_struct_banques = NULL;
      nb_banques = 0;
      no_derniere_banque = 0;

      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[SAUVER].widget ),
				 FALSE );
      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[SAUVER_SOUS].widget ),
				 FALSE );
      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[EXPORTER].widget ),
				 FALSE );
      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[FERMER].widget ),
				 FALSE );

      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_comptes[1].widget ),
				 FALSE );
      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_comptes[5].widget ),
				 FALSE );

      liste_no_rapprochements = NULL;

      titre_fichier = NULL;
      adresse_commune = NULL;

      if ( liste_struct_exercices )
	{
	  g_slist_free ( liste_struct_exercices );
	  liste_struct_exercices = NULL;
	}

      liste_struct_etats = NULL;
      no_dernier_etat = 0;
      etat_courant = NULL;
    }
}
/*****************************************************************************************************/

