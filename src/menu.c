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
#include "menu.h"


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



extern GtkItemFactory *item_factory_menu_general;
extern gchar *nom_navigateur_web;

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
	{_("/File"),    NULL,  NULL, 0, "<Branch>", NULL },
	{_("/File/Detach"),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{_("/File/New account file"),  NULL,  G_CALLBACK ( nouveau_fichier), 0, "<StockItem>", GTK_STOCK_NEW },
	{_("/File/_Open"),   "<CTRL>O", G_CALLBACK ( ouvrir_fichier ), 0, "<StockItem>", GTK_STOCK_OPEN },
	{_("/File/Sep1"),    NULL,  NULL, 0, "<Separator>", NULL },
	{_("/File/Save"),   NULL,  G_CALLBACK ( enregistrement_fichier ) , 1, "<StockItem>", GTK_STOCK_SAVE },
	{_("/File/Save as"),   NULL,  G_CALLBACK ( enregistrer_fichier_sous ), 0, "<StockItem>", GTK_STOCK_SAVE_AS },
	{_("/File/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/File/Recently opened files"), NULL,NULL , 0, "<Branch>",NULL, },
	{_("/File/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/File/Import"),   NULL,NULL , 0, "<Branch>",NULL  },
	{_("/File/Import/QIF\\/OFX file ..."),   NULL, G_CALLBACK ( importer_fichier), 0, NULL ,NULL  },
	{_("/File/Export"),   NULL, NULL, 0, "<Branch>",NULL  },
	{_("/File/Export/QIF file ..."),   NULL,G_CALLBACK ( exporter_fichier_qif ), 0, NULL ,NULL  },
	{_("/File/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/File/Close"),   NULL,G_CALLBACK ( fermer_fichier ), 0, "<StockItem>", GTK_STOCK_CLOSE },
	{_("/File/Exit"),   NULL, G_CALLBACK ( fermeture_grisbi), 0, "<StockItem>", GTK_STOCK_QUIT },

	{ _("/Edit"), NULL, NULL, 0, "<Branch>", NULL },
	{_("/Edit/Detach"),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{_("/Edit/New transaction"),   NULL, G_CALLBACK (new_transaction ), 0, "<StockItem>", GTK_STOCK_NEW },
	{_("/Edit/Remove transaction"),   NULL, G_CALLBACK (remove_transaction ), 0, "<StockItem>", GTK_STOCK_DELETE },
	{_("/Edit/Clone transaction"),   NULL, G_CALLBACK ( clone_selected_transaction), 0, "<StockItem>", GTK_STOCK_COPY },
	{_("/Edit/Edit transaction"),   NULL, G_CALLBACK ( edition_operation), 0, "<StockItem>", GTK_STOCK_PROPERTIES },
	{_("/Edit/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/Edit/Convert transaction to scheduled transaction"),   NULL, NULL, 0, "<StockItem>", GTK_STOCK_CONVERT },
	{_("/Edit/Move transaction to another account"),   NULL, NULL, 0, "<StockItem>", GTK_STOCK_GO_FORWARD },
	{_("/Edit/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/Edit/Preferences"),   NULL, G_CALLBACK (preferences ), 1, "<StockItem>", GTK_STOCK_PREFERENCES },

	{ _("/Accounts"), NULL, NULL, 0, "<Branch>", NULL },
	{_("/Accounts/Detach"),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{_("/Accounts/New account"),   NULL, G_CALLBACK (nouveau_compte ), 0, "<StockItem>", GTK_STOCK_NEW },
	{_("/Accounts/Remove an account"),   NULL, G_CALLBACK ( supprimer_compte), 0, "<StockItem>", GTK_STOCK_DELETE },
	{_("/Accounts/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/Accounts/Closed accounts"),   NULL, NULL , 0, "<Branch>", NULL },

	{ _("/Reports"), NULL, NULL, 0, "<Branch>", NULL },
	{_("/Reports/Detach"),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{_("/Reports/New report"),   NULL, G_CALLBACK ( ajout_etat), 0, "<StockItem>", GTK_STOCK_NEW },
	{_("/Reports/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/Reports/Clone report"),   NULL, G_CALLBACK (dupliquer_etat ), 0, "<StockItem>", GTK_STOCK_COPY },
	{_("/Reports/Print report"),   NULL, G_CALLBACK ( impression_etat_courant), 0, "<StockItem>", GTK_STOCK_PRINT },
	{_("/Reports/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/Reports/Import report"),   NULL, G_CALLBACK (importer_etat ), 0, "<StockItem>", GTK_STOCK_CONVERT },
	{_("/Reports/Export report"),   NULL, G_CALLBACK ( exporter_etat), 0, "<StockItem>", GTK_STOCK_CONVERT },
	{_("/Reports/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/Reports/Remove report"),   NULL, G_CALLBACK ( efface_etat), 0, "<StockItem>", GTK_STOCK_DELETE },
	{_("/Reports/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/Reports/Edit report"),   NULL, G_CALLBACK (personnalisation_etat ), 0, "<StockItem>", GTK_STOCK_PROPERTIES },

	{ _("/Help"), NULL, NULL, 0, "<Branch>", NULL },
	{_("/Help/Detach"),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{_("/Help/Manual"),   NULL, affiche_aide_locale, 1, NULL, NULL },
	{_("/Help/Quickstart"),   NULL, affiche_aide_locale, 2, "<StockItem>", GTK_STOCK_INDEX },
	{_("/Help/Translation"),   NULL, affiche_aide_locale, 3, NULL, NULL },
	{_("/Help/About"),   NULL, G_CALLBACK (a_propos ) , 1, NULL, NULL },
	{_("/Help/Sep1"),    NULL, NULL, 0, "<Separator>", NULL },
	{_("/Help/Grisbi website"),   NULL, G_CALLBACK (lien_web ), 1, NULL, NULL },
	{_("/Help/Report a bug"),   NULL, G_CALLBACK ( lien_web), 2,NULL , NULL },
	{_("/Help/On line User's guide"),   NULL, G_CALLBACK ( lien_web), 3, NULL, NULL }
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

    for ( i=nb_derniers_fichiers_ouverts ; i>0 ; i-- )
    {
	gchar *tmp;

	tmp = my_strdelimit ( tab_noms_derniers_fichiers_ouverts[i-1],
			      "/",
			      "\\/" );

	gtk_item_factory_delete_item ( item_factory_menu_general,
				       g_strconcat ( _("/File/Recently opened files/"),
						     tmp,
						     NULL ));
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/File/Recently opened files")),
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
							       _("/File/Recently opened files")),
				   FALSE );

	return;
    }

    for ( i=nb_derniers_fichiers_ouverts ; i>0 ; i-- )
    {
	GtkItemFactoryEntry *item_factory_entry;
	gchar *tmp;

	item_factory_entry = calloc ( 1,
				      sizeof ( GtkItemFactoryEntry ));

	tmp = my_strdelimit ( tab_noms_derniers_fichiers_ouverts[i-1],
			      "/",
			      "\\/" );
	tmp = my_strdelimit ( tmp,
			      "_",
			      "__" );

	item_factory_entry -> path = g_strconcat ( _("/File/Recently opened files/"),
						   tmp,
						   NULL);
	item_factory_entry -> callback = G_CALLBACK ( ouverture_fichier_par_menu );
	item_factory_entry -> callback_action = i;

	gtk_item_factory_create_item ( item_factory_menu_general,
				       item_factory_entry,
				       NULL,
				       1 );
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   _("/File/Recently opened files")),
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
	    lance_navigateur_web ( "http://www.grisbi.org/modules.php?name=Documentation");
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
	    lance_navigateur_web ( g_strconcat ( HELP,
						 "grisbi-manuel.html",
						 NULL ));
	    break;	

	case 2:
	    lance_navigateur_web ( g_strconcat ( HELP,
						 "quickstart.html",
						 NULL ));
	    break;

	case 3:
	    lance_navigateur_web ( g_strconcat ( HELP,
						 "translation.html",
						 NULL ));
	    break;
    }
}
/* **************************************************************************************************** */


