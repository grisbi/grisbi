/* ce fichier contient les définitions de tous les menus et barres d'outils */


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
#include "en_tete.h"


/***********************************************
** Définition de la barre des menus**
***********************************************/

void init_menus ( GtkWidget * win )
{
  static GnomeUIInfo tmp_menu_cloture [] = 
    {
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_comptes [] = 
    {
      GNOMEUIINFO_ITEM_STOCK (N_("New account"), 
			      N_("Create a new account"),
			      nouveau_compte,
			      GNOME_STOCK_PIXMAP_BOOK_OPEN),
      GNOMEUIINFO_ITEM_STOCK (N_("Remove an account"), 
			      N_("Remove an account"),
			      supprimer_compte,
			      GNOME_STOCK_MENU_TRASH),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_SUBTREE (N_("Closed accounts"),
			   &tmp_menu_cloture),
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_parametres [] = 
    {
      GNOMEUIINFO_ITEM_DATA  ( N_("Main"),
			       N_("Main"),
			       preferences,
			       NULL,
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("Files"),
			       N_("Files"),
			       preferences,
			       GINT_TO_POINTER ( 1 ),
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("Scheduler"),
			       N_("Scheduler"),
			       preferences,
			       GINT_TO_POINTER ( 2 ),
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("Display"),
			       N_("Display"),
			       preferences,
			       GINT_TO_POINTER ( 3 ),
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("Applet"),
			       N_("Applet"),
			       preferences,
			       GINT_TO_POINTER ( 4 ),
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("Currencies"),
			       N_("Currencies"),
			       preferences,
			       GINT_TO_POINTER ( 5 ),
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("Banks"),
			       N_("Banks"),
			       preferences,
			       GINT_TO_POINTER ( 6 ),
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("Financial years"),
			       N_("Financial years"),
			       preferences,
			       GINT_TO_POINTER ( 7 ),
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("Methods of payment"),
			       N_("Methods of payment"),
			       preferences,
			       GINT_TO_POINTER ( 8 ),
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("Informations"),
			       N_("Informations"),
			       preferences,
			       GINT_TO_POINTER ( 9 ),
			       NULL ),
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_importer [] = 
    {
      GNOMEUIINFO_ITEM_STOCK ( N_("QIF File"),
			       N_("Import QIF files"),
			       importer_fichier_qif,
			       GNOME_STOCK_PIXMAP_CONVERT ),
/*       GNOMEUIINFO_ITEM_STOCK ( N_("Fichier QIF GD"), */
/* 			       N_("Import QIF files"), */
/* 			       iqc_fichier, */
/* 			       GNOME_STOCK_PIXMAP_CONVERT ), */
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_exporter [] = 
    {
      GNOMEUIINFO_ITEM_STOCK ( N_("QIF File"),
			       N_("Export QIF files"),
			       exporter_fichier_qif,
			       GNOME_STOCK_PIXMAP_REMOVE ),
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_derniers_fichiers [] = 
    {
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_help_menu [] =
    {
      GNOMEUIINFO_HELP ( "grisbi" ),
      GNOMEUIINFO_MENU_ABOUT_ITEM (  a_propos,
				     GINT_TO_POINTER (1)  ),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_ITEM_DATA  ( N_("Grisbi website"),
			       N_("Grisbi website"),
			       lien_web,
			       NULL,
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("Report a bug"),
			       N_("Report a bug"),
			       lien_web,
			       GINT_TO_POINTER ( 1 ),
			       NULL ),
      GNOMEUIINFO_ITEM_DATA  ( N_("On line User's guide"),
			       N_("On line User's guide"),
			       lien_web,
			       GINT_TO_POINTER ( 2 ),
			       NULL ),
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_fichier [] = 
    {
      GNOMEUIINFO_ITEM_STOCK (N_("New"), 
			      N_("Create a new accounts file"),
			      nouveau_fichier,
			      GNOME_STOCK_MENU_NEW),
      GNOMEUIINFO_MENU_OPEN_ITEM ( ouvrir_fichier, NULL),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_MENU_SAVE_ITEM ( enregistrement_fichier, NULL),
      GNOMEUIINFO_MENU_SAVE_AS_ITEM ( enregistrer_fichier_sous, NULL),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_SUBTREE ( N_("Recently opened files"),
			    &tmp_menu_derniers_fichiers),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_SUBTREE (N_("Import"),
			   &tmp_menu_importer),
      GNOMEUIINFO_SUBTREE (N_("Export"),
			   &tmp_menu_exporter),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_MENU_CLOSE_ITEM ( fermer_fichier, NULL),
      GNOMEUIINFO_MENU_EXIT_ITEM ( fermeture_grisbi, NULL),
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_principal [15] = 
    {
      GNOMEUIINFO_SUBTREE (N_("File"),
			   &tmp_menu_fichier),
      GNOMEUIINFO_SUBTREE (N_("Accounts"),
			   &tmp_menu_comptes),
      GNOMEUIINFO_SUBTREE (N_("Setup"),
			   &tmp_menu_parametres),
      GNOMEUIINFO_SUBTREE (N_("Help"),
			   &tmp_help_menu),
      GNOMEUIINFO_END
    };

  menu_principal = tmp_menu_principal;
  menu_fichier = tmp_menu_fichier;
  help_menu = tmp_help_menu;
  menu_derniers_fichiers = tmp_menu_derniers_fichiers;
  menu_exporter = tmp_menu_exporter;
  menu_importer = tmp_menu_importer;
  menu_parametres = tmp_menu_parametres;
  menu_comptes = tmp_menu_comptes;
  menu_cloture = tmp_menu_cloture;

  gnome_app_create_menus ( GNOME_APP ( win ), 
			   menu_principal );
}



/* **************************************************************************************************** */
/* fonction efface_derniers_fichiers_ouverts */
/* **************************************************************************************************** */

void efface_derniers_fichiers_ouverts ( void )
{
  /* on met +1 pour la séparation */

  if ( nb_derniers_fichiers_ouverts )
    gnome_app_remove_menus ( GNOME_APP ( window ),
			     _("File/Recently opened files/"),
			     2 * ( nb_derniers_fichiers_ouverts + 1 ));
  gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[DERNIERS_FICHIERS].widget ),
			     FALSE );

}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* fonction affiche_derniers_fichiers_ouverts */
/* **************************************************************************************************** */

void affiche_derniers_fichiers_ouverts ( void )
{
  gint i;

  if ( !nb_derniers_fichiers_ouverts )
    {
      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[DERNIERS_FICHIERS].widget ),
				 FALSE );
      return;
    }

  for ( i=nb_derniers_fichiers_ouverts ; i>0 ; i-- )
    {
      GnomeUIInfo *menu;

      menu = malloc ( 2 * sizeof ( GnomeUIInfo ));

      menu -> type = GNOME_APP_UI_ITEM;
      menu -> label = tab_noms_derniers_fichiers_ouverts[i-1];
      menu -> hint = tab_noms_derniers_fichiers_ouverts[i-1];
      menu -> moreinfo = (gpointer) ouverture_fichier_par_menu;
      menu -> user_data = tab_noms_derniers_fichiers_ouverts[i-1];
      menu -> unused_data = NULL;
      menu -> pixmap_type = 0;
      menu -> pixmap_info = 0;
      menu -> accelerator_key = 0;
      menu -> ac_mods = GDK_BUTTON1_MASK;
      menu -> widget = NULL;

      (menu + 1)->type = GNOME_APP_UI_ENDOFINFO;
    
      gnome_app_insert_menus ( GNOME_APP ( window ),
			       _("File/Recently opened files/"),
			       menu );
    }

  gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[DERNIERS_FICHIERS].widget ),
			     TRUE );
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void lien_web ( GtkWidget *widget,
		gint origine )
{
  switch ( origine )
    {
    case 0 :
      gnome_url_show ( "http://www.grisbi.org" );
      break;

    case 1:
      gnome_url_show ( "http://www.grisbi.org/bugtracking" );
      break;

    case 2:
      gnome_url_show ( "http://www.grisbi.org/manuel.html" );
      break;
    }
}
/* **************************************************************************************************** */
