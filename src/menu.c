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

#include "comptes_traitements.h"
#include "etats_calculs.h"
#include "etats_config.h"
#include "etats_onglet.h"
#include "erreur.h"
#include "fichiers_gestion.h"
#include "gtkcombofix.h"
#include "help.h"
#include "menu.h"
#include "operations_liste.h"
#include "parametres.h"
#include "import.h"
#include "qif.h"

#include "./xpm/export.xpm"
#include "./xpm/import.xpm"


/***********************************************
** Définition de la barre des menus**
***********************************************/

void init_menus ( GtkWidget * win )
{
  static GnomeUIInfo tmp_menu_cloture [] = 
    {
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_reports [] = 
    {
      GNOMEUIINFO_ITEM_STOCK (N_("New report"), 
			      N_("Create a new report"),
			      ajout_etat,
			      GNOME_STOCK_MENU_NEW),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_ITEM_STOCK (N_("Clone report"), 
			      N_("Clone selected report"),
			      dupliquer_etat,
			      GNOME_STOCK_MENU_COPY),
      GNOMEUIINFO_ITEM_STOCK (N_("Print report"), 
			      N_("Print selected report"),
			      impression_etat_courant,
			      GNOME_STOCK_MENU_PRINT),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_ITEM_STOCK (N_("Import report"),
			      N_("Import report from a XML file"),
			      importer_etat,
			      GNOME_STOCK_MENU_CONVERT),
      GNOMEUIINFO_ITEM_STOCK (N_("Export report"), 
			      N_("Export report to a XML file"),
			      exporter_etat,
			      GNOME_STOCK_MENU_CONVERT),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_ITEM_STOCK (N_("Remove report"), 
			      N_("Remove selected report"),
			      efface_etat,
			      GNOME_STOCK_MENU_TRASH),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_ITEM_STOCK (N_("Edit report"), 
			      N_("Edit selected report"),
			      personnalisation_etat,
			      GNOME_STOCK_MENU_PROP),
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

  static GnomeUIInfo tmp_menu_importer [] = 
    {
      GNOMEUIINFO_ITEM ( N_("File"),
			       N_("Import file"),
			       importer_fichier,
			       import_xpm ),
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_exporter [] = 
    {
      GNOMEUIINFO_ITEM ( N_("QIF File"),
			 N_("Export QIF file"),
			 exporter_fichier_qif,
			 export_xpm ),
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
      GNOMEUIINFO_ITEM_STOCK (N_("New account file"), 
			      N_("Create a new account file"),
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
      GNOMEUIINFO_SUBTREE (N_("Import"), &tmp_menu_importer),
      GNOMEUIINFO_SUBTREE (N_("Export"), &tmp_menu_exporter),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_MENU_CLOSE_ITEM ( fermer_fichier, NULL),
      GNOMEUIINFO_MENU_EXIT_ITEM ( fermeture_grisbi, NULL),
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_editer [10] = 
    {
      GNOMEUIINFO_ITEM_STOCK (N_("New transaction"), 
			      N_("Create a new transaction"),
			      new_transaction,
			      GNOME_STOCK_MENU_NEW),
      GNOMEUIINFO_ITEM_STOCK (N_("Remove transaction"), 
			      N_("Remove selected transaction"),
			      remove_transaction,
			      GNOME_STOCK_MENU_TRASH),
      GNOMEUIINFO_ITEM_STOCK (N_("Clone transaction"), 
			      N_("Clone selected transaction"),
			      clone_selected_transaction,
			      GNOME_STOCK_MENU_COPY),
      GNOMEUIINFO_ITEM_STOCK (N_("Edit transaction"), 
			      N_("Edit selected transaction"),
			      edition_operation,
			      GNOME_STOCK_MENU_PROP),
      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_ITEM_STOCK (N_("Convert transaction to scheduled transaction"), 
			      N_("Convert selected transaction to scheduled transaction"),
			      edition_operation,
			      GNOME_STOCK_MENU_CONVERT),
      GNOMEUIINFO_ITEM_STOCK (N_("Move transaction to another account"), 
			      N_("Move selected transaction to another account"),
			      edition_operation,
			      GNOME_STOCK_MENU_TRASH),
/*       gtk_menu_item_set_submenu ( GTK_MENU_ITEM(menu_item),  */
/* 				  GTK_WIDGET(creation_option_menu_comptes(GTK_SIGNAL_FUNC(move_selected_operation_to_account), FALSE)) ); */
      

      GNOMEUIINFO_SEPARATOR,
      GNOMEUIINFO_MENU_PREFERENCES_ITEM (preferences, NULL),
      GNOMEUIINFO_END
    };

  static GnomeUIInfo tmp_menu_principal [15] = 
    {
      GNOMEUIINFO_SUBTREE (N_("File"), &tmp_menu_fichier),
      GNOMEUIINFO_SUBTREE (N_("Edit"), &tmp_menu_editer),
      GNOMEUIINFO_SUBTREE (N_("Accounts"), &tmp_menu_comptes),
      GNOMEUIINFO_SUBTREE (N_("Reports"), &tmp_menu_reports),
      GNOMEUIINFO_SUBTREE (N_("Help"), &tmp_help_menu),
      GNOMEUIINFO_END
    };

  menu_principal = tmp_menu_principal;
  menu_fichier = tmp_menu_fichier;
  help_menu = tmp_help_menu;
  menu_derniers_fichiers = tmp_menu_derniers_fichiers;
  menu_exporter = tmp_menu_exporter;
  menu_importer = tmp_menu_importer;
  menu_comptes = tmp_menu_comptes;
  menu_reports = tmp_menu_reports;
  menu_cloture = tmp_menu_cloture;

  gnome_app_create_menus ( GNOME_APP ( win ), menu_principal );
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
    GError *error;
    case 0 :
      gnome_url_show ( "http://www.grisbi.org", &error );
      break;

    case 1:
      gnome_url_show ( "http://www.grisbi.org/bugtracking", &error );
      break;

    case 2:
      gnome_url_show ( "http://www.grisbi.org/modules.php?name=Documentation", &error);
      break;
    }
}
/* **************************************************************************************************** */
