/* ce fichier contient les définitions de tous les menus et barres d'outils */


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


/***********************************************
** Définition de la barre des menus**
***********************************************/


GnomeUIInfo menu_fichier [] = 
{
  GNOMEUIINFO_ITEM_STOCK (_("Nouveau"), 
			  _("Création d'un nouveau fichier"),
			  nouveau_fichier,
			  GNOME_STOCK_MENU_NEW),
  GNOMEUIINFO_MENU_OPEN_ITEM ( ouvrir_fichier, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_SAVE_ITEM ( enregistrement_fichier, NULL),
  GNOMEUIINFO_MENU_SAVE_AS_ITEM ( enregistrer_fichier_sous, NULL),
  GNOMEUIINFO_MENU_PRINT_ITEM ( impression_fichier, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_SUBTREE ( _("Derniers fichiers"),
			&menu_derniers_fichiers),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_SUBTREE (_("Importer"),
		       &menu_importer),
  GNOMEUIINFO_SUBTREE (_("Exporter"),
		       &menu_exporter),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_CLOSE_ITEM ( fermer_fichier, NULL),
  GNOMEUIINFO_MENU_EXIT_ITEM ( fermeture_grisbi, NULL),
  GNOMEUIINFO_END
};



GnomeUIInfo menu_comptes [] = 
{
  GNOMEUIINFO_ITEM_STOCK (_("Nouveau compte"), 
			  _("Créer un nouveau compte"),
			  nouveau_compte,
			  GNOME_STOCK_PIXMAP_BOOK_OPEN),
  GNOMEUIINFO_ITEM_STOCK (_("Supprimer un compte"), 
			  _("Supprimer un compte"),
			  supprimer_compte,
			  GNOME_STOCK_MENU_TRASH),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_ITEM_STOCK (_("Propriétés du compte"), 
			  _("Propriétés du compte"),
			  affiche_detail_compte,
			  GNOME_STOCK_MENU_ABOUT),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_SUBTREE (_("Comptes cloturés"),
		       &menu_cloture),
  GNOMEUIINFO_END
};


GnomeUIInfo menu_parametres [] = 
{
  GNOMEUIINFO_ITEM_DATA  ( _("Général"),
			   _("Général"),
			   preferences,
			   NULL,
			   NULL ),
  GNOMEUIINFO_ITEM_DATA  ( _("Fichiers"),
			   _("Fichiers"),
			   preferences,
			   GINT_TO_POINTER ( 1 ),
			   NULL ),
  GNOMEUIINFO_ITEM_DATA  ( _("Échéances"),
			   _("Échéances"),
			   preferences,
			   GINT_TO_POINTER ( 2 ),
			   NULL ),
  GNOMEUIINFO_ITEM_DATA  ( _("Affichage"),
			   _("Affichage"),
			   preferences,
			   GINT_TO_POINTER ( 3 ),
			   NULL ),
  GNOMEUIINFO_ITEM_DATA  ( _("Applet"),
			   _("Applet"),
			   preferences,
			   GINT_TO_POINTER ( 4 ),
			   NULL ),
  GNOMEUIINFO_ITEM_DATA  ( _("Devises"),
			   _("Devises"),
			   preferences,
			   GINT_TO_POINTER ( 5 ),
			   NULL ),
  GNOMEUIINFO_ITEM_DATA  ( _("Banques"),
			   _("Banques"),
			   preferences,
			   GINT_TO_POINTER ( 6 ),
			   NULL ),
  GNOMEUIINFO_ITEM_DATA  ( _("Exercices"),
			   _("Exercices"),
			   preferences,
			   GINT_TO_POINTER ( 7 ),
			   NULL ),
  GNOMEUIINFO_ITEM_DATA  ( _("Types d'opérations"),
			   _("Types d'opérations"),
			   preferences,
			   GINT_TO_POINTER ( 8 ),
			   NULL ),
  GNOMEUIINFO_END
};


GnomeUIInfo menu_importer [] = 
{
  GNOMEUIINFO_ITEM_STOCK ( _(" Fichier QIF"),
			   _("Importation de fichiers QIF"),
			   importer_fichier_qif,
			   GNOME_STOCK_PIXMAP_CONVERT ),
  GNOMEUIINFO_END
};



GnomeUIInfo menu_exporter [] = 
{
  GNOMEUIINFO_ITEM_STOCK ( _(" Fichier QIF"),
			   _("Exportation de fichiers QIF"),
			   exporter_fichier_qif,
			   GNOME_STOCK_PIXMAP_REMOVE ),
  GNOMEUIINFO_END
};

GnomeUIInfo menu_derniers_fichiers [] = 
{
  GNOMEUIINFO_END
};



GnomeUIInfo menu_cloture [] = 
{
  GNOMEUIINFO_END
};


/***********************************************
** Définition du menu d'aide                  **
***********************************************/

GnomeUIInfo help_menu [] =
{
  GNOMEUIINFO_HELP ( _("grisbi") ),
  GNOMEUIINFO_MENU_ABOUT_ITEM (  a_propos,
				 GINT_TO_POINTER (1)  ),
  GNOMEUIINFO_END
};


GnomeUIInfo menu_principal [] = 
{
  GNOMEUIINFO_SUBTREE (_("Fichier"),
		       &menu_fichier),
  GNOMEUIINFO_SUBTREE (_("Comptes"),
		       &menu_comptes),
  GNOMEUIINFO_SUBTREE (_("Configuration"),
		       &menu_parametres),
  GNOMEUIINFO_SUBTREE (_("Aide"),
		       &help_menu),
  GNOMEUIINFO_END
};




/* **************************************************************************************************** */
/* fonction efface_derniers_fichiers_ouverts */
/* **************************************************************************************************** */

void efface_derniers_fichiers_ouverts ( void )
{
  /* on met +1 pour la séparation */

  if ( nb_derniers_fichiers_ouverts )
    gnome_app_remove_menus ( GNOME_APP ( window ),
			     _("Fichier/Derniers fichiers/"),
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
			       _("Fichier/Derniers fichiers/"),
			       menu );
    }

  gtk_widget_set_sensitive ( GTK_WIDGET ( menu_fichier[DERNIERS_FICHIERS].widget ),
			     TRUE );
}
/* **************************************************************************************************** */
