/* ************************************************************************** */
/* Ce fichier contient les définitions de tous les menus et barres d'outils   */
/*                                                                            */
/*                                  menu.c                                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			     2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "menu.h"
#include "export.h"


#include "comptes_traitements.h"
#include "erreur.h"
#include "etats_calculs.h"
#include "etats_config.h"
#include "etats_onglet.h"
#include "fichiers_gestion.h"
#include "help.h"
#include "import.h"
#include "operations_liste.h"
#include "parametres.h"
#include "qif.h"
#include "utils.h"
#include "export.h"


extern GtkItemFactory *item_factory_menu_general;

/***********************************************/
/* définition de la barre des menus, version gtk */
/***********************************************/

GtkWidget *init_menus ( GtkWidget *vbox )
{
    GtkWidget *barre_menu;
    GtkAccelGroup *accel;
    gint nb_item_menu;

    /* Definition des elements du menu */

    GtkItemFactoryEntry menu_item[] = {
        /* File menu */
	{menu_name(_("File"), NULL, NULL),    NULL,  NULL, 0, "<Branch>", NULL },
	{menu_name(_("File"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("File"), _("New account file"), NULL),  NULL,  G_CALLBACK ( nouveau_fichier), 0, "<StockItem>", GTK_STOCK_NEW },
	{menu_name(_("File"), _("_Open"), NULL),   "<CTRL>O", G_CALLBACK ( ouvrir_fichier ), 0, "<StockItem>", GTK_STOCK_OPEN },
	{menu_name(_("File"), "Sep1", NULL),    NULL,  NULL, 0, "<Separator>", NULL },
	{menu_name(_("File"), _("Save"), NULL),   NULL,  G_CALLBACK ( enregistrement_fichier ) , 1, "<StockItem>", GTK_STOCK_SAVE },
	{menu_name(_("File"), _("Save as"), NULL),   NULL,  G_CALLBACK ( enregistrer_fichier_sous ), 0, "<StockItem>", GTK_STOCK_SAVE_AS },
	{menu_name(_("File"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("File"), _("Recently opened files"), NULL), NULL,NULL , 0, "<Branch>",NULL, },
	{menu_name(_("File"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("File"), _("Import"), NULL),   NULL,NULL , 0, "<Branch>",NULL  },
	{menu_name(_("File"), _("Import"), _("QIF\\/OFX\\/Gnucash file ...")),   NULL, G_CALLBACK ( importer_fichier), 0, NULL ,NULL  },
	{menu_name(_("File"), _("Export"), NULL),   NULL, NULL, 0, "<Branch>",NULL  },
	//{menu_name(_("File"), _("Export"), _("QIF file ...")),   NULL,G_CALLBACK ( exporter_fichier_qif ), 0, NULL ,NULL  },
	{menu_name(_("File"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("File"), _("Close"), NULL),   NULL,G_CALLBACK ( fermer_fichier ), 0, "<StockItem>", GTK_STOCK_CLOSE },
	{menu_name(_("File"), _("Exit"), NULL),   NULL, G_CALLBACK ( fermeture_grisbi), 0, "<StockItem>", GTK_STOCK_QUIT },

	/* Edit menu */
	{menu_name(_("Edit"), NULL, NULL), NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("Edit"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("Edit"), _("New transaction"), NULL),  "<CTRL>T", G_CALLBACK (new_transaction ), 0, "<StockItem>", GTK_STOCK_NEW },
	{menu_name(_("Edit"), _("Use selected transaction as a template"), NULL),  NULL, G_CALLBACK ( new_transaction_from_selected ), 0, "<StockItem>", GTK_STOCK_NEW },
	{menu_name(_("Edit"), _("Remove transaction"), NULL),   NULL, G_CALLBACK (remove_transaction ), 0, "<StockItem>", GTK_STOCK_DELETE },
	{menu_name(_("Edit"), _("Clone transaction"), NULL), "<SHIFT><CTRL>C", G_CALLBACK ( clone_selected_transaction), 0, "<StockItem>", GTK_STOCK_COPY },
	{menu_name(_("Edit"), _("Edit transaction"), NULL),   NULL, G_CALLBACK ( edition_operation), 0, "<StockItem>", GTK_STOCK_PROPERTIES },
	{menu_name(_("Edit"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Edit"), _("Convert transaction to scheduled transaction"), NULL),   NULL, NULL, 0, "<StockItem>", GTK_STOCK_CONVERT },
	{menu_name(_("Edit"), _("Move transaction to another account"), NULL),   NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("Edit"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Edit"), _("Preferences"), NULL),   NULL, G_CALLBACK (preferences ), 1, "<StockItem>", GTK_STOCK_PREFERENCES },

	/* Accounts menu */
	{menu_name(_("Accounts"), NULL, NULL), NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("Accounts"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("Accounts"), _("New account"), NULL),   NULL, G_CALLBACK (nouveau_compte ), 0, "<StockItem>", GTK_STOCK_NEW },
	{menu_name(_("Accounts"), _("Remove an account"), NULL),   NULL, G_CALLBACK ( supprimer_compte), 0, "<StockItem>", GTK_STOCK_DELETE },
	{menu_name(_("Accounts"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Accounts"), _("Closed accounts"), NULL),   NULL, NULL , 0, "<Branch>", NULL },

	/* Reports menu */
	{menu_name(_("Reports"), NULL, NULL), NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("Reports"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("Reports"), _("New report"), NULL),   NULL, G_CALLBACK ( ajout_etat), 0, "<StockItem>", GTK_STOCK_NEW },
	{menu_name(_("Reports"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Reports"), _("Clone report"), NULL),   NULL, G_CALLBACK (dupliquer_etat ), 0, "<StockItem>", GTK_STOCK_COPY },
	{menu_name(_("Reports"), _("Print report"), NULL),   NULL, G_CALLBACK ( impression_etat_courant), 0, "<StockItem>", GTK_STOCK_PRINT },
	{menu_name(_("Reports"), _("Export report as HTML..."), NULL),   NULL, G_CALLBACK ( export_etat_courant_vers_html), 0, "<StockItem>", GTK_STOCK_CONVERT },
	{menu_name(_("Reports"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Reports"), _("Import report file"), NULL),   NULL, G_CALLBACK (importer_etat ), 0, "<StockItem>", GTK_STOCK_CONVERT },
	{menu_name(_("Reports"), _("Export report file"), NULL),   NULL, G_CALLBACK ( exporter_etat), 0, "<StockItem>", GTK_STOCK_CONVERT },
	{menu_name(_("Reports"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Reports"), _("Remove report"), NULL),   NULL, G_CALLBACK ( efface_etat), 0, "<StockItem>", GTK_STOCK_DELETE },
	{menu_name(_("Reports"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Reports"), _("Edit report"), NULL),   NULL, G_CALLBACK (personnalisation_etat ), 0, "<StockItem>", GTK_STOCK_PROPERTIES },

	/* Help menu */
	{menu_name(_("Help"), NULL, NULL), NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("Help"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("Help"), _("Grisbi Manual"), NULL),   NULL, affiche_aide_locale, 1, NULL, NULL },
	{menu_name(_("Help"), _("Quickstart"), NULL),   NULL, affiche_aide_locale, 2, "<StockItem>", GTK_STOCK_INDEX },
	{menu_name(_("Help"), _("Translation"), NULL),   NULL, affiche_aide_locale, 3, NULL, NULL },
	{menu_name(_("Help"), _("About"), NULL),   NULL, G_CALLBACK (a_propos ) , 1, NULL, NULL },
	{menu_name(_("Help"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Help"), _("Grisbi website"), NULL),   NULL, G_CALLBACK (lien_web ), 1, NULL, NULL },
	{menu_name(_("Help"), _("Report a bug"), NULL),   NULL, G_CALLBACK ( lien_web), 2,NULL , NULL },
	{menu_name(_("Help"), _("On line User's guide"), NULL),   NULL, G_CALLBACK ( lien_web), 3, NULL, NULL }
    };



    /* Nombre d elements du menu */

    nb_item_menu = sizeof(menu_item) / sizeof(menu_item[0]);


    /* Creation de la table d acceleration */
    accel = gtk_accel_group_new ();

    /* Creation du menu */

    item_factory_menu_general = gtk_item_factory_new(GTK_TYPE_MENU_BAR,
						     "<main>",
						     accel);

    /* Recuperation des elements du menu */

    gtk_item_factory_create_items(item_factory_menu_general,
				  nb_item_menu,
				  menu_item,
				  NULL );

    /* Recuperation du widget pour l affichage du menu */

    barre_menu = gtk_item_factory_get_widget(item_factory_menu_general,
					     "<main>");

    /* Association des raccourcis avec la fenetre */

    gtk_window_add_accel_group(GTK_WINDOW(window),
			       accel );

    
    export_update_supported_format_menu(item_factory_menu_general);

    gtk_widget_show_all ( barre_menu );

    return ( barre_menu );
}
/***********************************************/





/* **************************************************************************************************** */
/* fonction efface_derniers_fichiers_ouverts */
/* **************************************************************************************************** */

void efface_derniers_fichiers_ouverts ( void )
{

    gint i;

    for ( i=0 ; i<nb_derniers_fichiers_ouverts ; i++ )
    {
	gchar *tmp;

	tmp = my_strdelimit ( tab_noms_derniers_fichiers_ouverts[i],
			      C_DIRECTORY_SEPARATOR,
			      "\\" C_DIRECTORY_SEPARATOR );

	gtk_item_factory_delete_item ( item_factory_menu_general,
				       menu_name ( _("File"), _("Recently opened files"), tmp ));
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name ( _("File"), _("Recently opened files"), NULL )),
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
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name ( _("File"), _("Recently opened files"), NULL )),
				   FALSE );

	return;
    }

    for ( i=0 ; i<nb_derniers_fichiers_ouverts ; i++ )
    {
	GtkItemFactoryEntry *item_factory_entry;
	gchar *tmp;

	item_factory_entry = calloc ( 1,
				      sizeof ( GtkItemFactoryEntry ));

	tmp = my_strdelimit ( tab_noms_derniers_fichiers_ouverts[i],
			      C_DIRECTORY_SEPARATOR,
			      "\\" C_DIRECTORY_SEPARATOR);
	tmp = my_strdelimit ( tmp,
			      "_",
			      "__" );

	item_factory_entry -> path = menu_name ( _("File"), _("Recently opened files"), tmp );
	item_factory_entry -> callback = G_CALLBACK ( ouverture_fichier_par_menu );
	item_factory_entry -> callback_action = i;

	gtk_item_factory_create_item ( item_factory_menu_general,
				       item_factory_entry,
				       NULL,
				       1 );
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name ( _("File"), _("Recently opened files"), NULL )),
			       TRUE );

}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void lien_web ( GtkWidget *widget,
		gint origine )
{
    switch ( origine )
    {
	case 1:
	    lance_navigateur_web ( "http://www.grisbi.org");
	    break;	

	case 2:
	    lance_navigateur_web ( "http://www.grisbi.org/bugtracking");
	    break;

	case 3:
	    lance_navigateur_web ( "http://www.grisbi.org/manuel.html");
	    break;
    }
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void affiche_aide_locale ( gpointer null,
			   gint origine )
{
    switch ( origine )
    {
	case 1:
	  lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", _("C"), "/grisbi-manuel.html", NULL ));
	    break;	

	case 2:
	    lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", _("C"), "/quickstart.html", NULL ));
	    break;

	case 3:
	    lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", _("C"), "/translation.html", NULL ));
	    break;
    }
}
/* **************************************************************************************************** */


gchar * menu_name ( gchar * menu, gchar * submenu, gchar * subsubmenu )
{
  if ( subsubmenu )
    return g_strconcat ( "/", menu, "/", submenu, "/", subsubmenu, NULL );
  else if ( submenu )
    return g_strconcat ( "/", menu, "/", submenu, NULL );
  else
    return g_strconcat ( "/", menu, NULL );
}
