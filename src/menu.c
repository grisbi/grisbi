/* ************************************************************************** */
/* Ce fichier contient les définitions de tous les menus et barres d'outils   */
/*                                                                            */
/*                                  menu.c                                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org)	      */
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


/*START_INCLUDE*/
#include "menu.h"
#include "help.h"
#include "operations_formulaire.h"
#include "etats_onglet.h"
#include "barre_outils.h"
#include "operations_comptes.h"
#include "operations_liste.h"
#include "comptes_traitements.h"
#include "fichiers_gestion.h"
#include "qif.h"
#include "erreur.h"
#include "tip.h"
#include "gsb_account.h"
#include "import.h"
#include "etats_calculs.h"
#include "utils.h"
#include "main.h"
#include "utils_str.h"
#include "etats_config.h"
#include "parametres.h"
#include "fichier_configuration.h"
#include "structures.h"
#include "menu.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void affiche_aide_locale ( gpointer null,
			   gint origine );
static void lien_web ( GtkWidget *widget,
		gint origine );
static void view_menu_cb ( gpointer callback_data, guint callback_action, GtkWidget *widget );
/*END_STATIC*/



/*START_EXTERN*/
extern GtkItemFactory *item_factory_menu_general;
extern gint nb_derniers_fichiers_ouverts;
extern gchar **tab_noms_derniers_fichiers_ouverts;
extern GtkWidget *window;
extern GtkTreeModelFilter * navigation_model_filtered;
/*END_EXTERN*/


gboolean block_menu_cb = FALSE;


/***********************************************/
/* définition de la barre des menus, version gtk */
/***********************************************/

GtkWidget *init_menus ( GtkWidget *vbox )
{
    GtkWidget *barre_menu, *widget;
    GtkAccelGroup *accel;
    gint nb_item_menu;

    /* Definition des elements du menu */

    GtkItemFactoryEntry menu_item[] = {
        /* File menu */
	{menu_name(_("File"), NULL, NULL),    NULL,  NULL, 0, "<Branch>", NULL },
	{menu_name(_("File"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("File"), _("New account file"), NULL),  NULL,  G_CALLBACK ( new_file), 0, "<StockItem>", GTK_STOCK_NEW },
	{menu_name(_("File"), _("Open"), NULL),   "<CTRL>O", G_CALLBACK ( ouvrir_fichier ), 0, "<StockItem>", GTK_STOCK_OPEN },
	{menu_name(_("File"), "Sep1", NULL),    NULL,  NULL, 0, "<Separator>", NULL },
	{menu_name(_("File"), _("Save"), NULL),   NULL,  G_CALLBACK ( enregistrement_fichier ) , 1, "<StockItem>", GTK_STOCK_SAVE },
	{menu_name(_("File"), _("Save as"), NULL),   NULL,  G_CALLBACK ( enregistrer_fichier_sous ), 0, "<StockItem>", GTK_STOCK_SAVE_AS },
	{menu_name(_("File"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("File"), _("Recently opened files"), NULL), NULL,NULL , 0, "<Branch>",NULL, },
	{menu_name(_("File"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("File"), _("Import QIF\\/OFX\\/CSV\\/Gnucash file ..."), NULL),   NULL, G_CALLBACK ( importer_fichier), 0, "<StockItem>" ,GTK_STOCK_CONVERT  },
	{menu_name(_("File"), _("Export QIF file ..."), NULL),   NULL,G_CALLBACK ( exporter_fichier_qif ), 0, "<StockItem>", GTK_STOCK_CONVERT  },
	{menu_name(_("File"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("File"), _("Close"), NULL),   NULL,G_CALLBACK ( fermer_fichier ), 0, "<StockItem>", GTK_STOCK_CLOSE },
	{menu_name(_("File"), _("Exit"), NULL),   NULL, G_CALLBACK ( fermeture_grisbi), 0, "<StockItem>", GTK_STOCK_QUIT },

	/* Edit menu */
	{menu_name(_("Edit"), NULL, NULL), NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("Edit"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("Edit"), _("New transaction"), NULL),  "<CTRL>T", G_CALLBACK (new_transaction ), 0, "<StockItem>", GTK_STOCK_NEW },
	{menu_name(_("Edit"), _("Remove transaction"), NULL),   NULL, G_CALLBACK (remove_transaction ), 0, "<StockItem>", GTK_STOCK_DELETE },
	{menu_name(_("Edit"), _("Clone transaction"), NULL), "<SHIFT><CTRL>C", G_CALLBACK ( clone_selected_transaction), 0, "<StockItem>", GTK_STOCK_COPY },
	{menu_name(_("Edit"), _("Edit transaction"), NULL),   NULL, G_CALLBACK ( gsb_transactions_list_edit_current_transaction), 0, "<StockItem>", GTK_STOCK_PROPERTIES },
	{menu_name(_("Edit"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Edit"), _("Convert transaction to scheduled transaction"), NULL),   NULL, NULL, 0, "<StockItem>", GTK_STOCK_CONVERT },
	{menu_name(_("Edit"), _("Move transaction to another account"), NULL),   NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("Edit"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Edit"), _("Preferences"), NULL),   NULL, G_CALLBACK (preferences ), 1, "<StockItem>", GTK_STOCK_PREFERENCES },

	/* View menu */
	{menu_name(_("View"), NULL, NULL), NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("View"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("View"), _("Show transaction form"), NULL),   NULL, G_CALLBACK(view_menu_cb), HIDE_SHOW_TRANSACTION_FORM, "<ToggleItem>", },
	{menu_name(_("View"), _("Show grid"), NULL),   NULL, G_CALLBACK(view_menu_cb), HIDE_SHOW_GRID, "<ToggleItem>", },
	{menu_name(_("View"), _("Show reconciled transactions"), NULL),   NULL, G_CALLBACK(view_menu_cb), HIDE_SHOW_RECONCILED_TRANSACTIONS, "<ToggleItem>", },
	{menu_name(_("View"), _("Show closed accounts"), NULL),   NULL, G_CALLBACK(view_menu_cb), HIDE_SHOW_CLOSED_ACCOUNTS, "<ToggleItem>", },
	{menu_name(_("View"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("View"), _("Show one line per transaction"), NULL),   NULL, G_CALLBACK(view_menu_cb), ONE_LINE_PER_TRANSACTION, "<RadioItem>" },
	{menu_name(_("View"), _("Show two lines per transaction"), NULL),   NULL, G_CALLBACK(view_menu_cb), TWO_LINES_PER_TRANSACTION, menu_name(_("View"), _("Show one line per transaction"), NULL) },
	{menu_name(_("View"), _("Show three lines per transaction"), NULL),   NULL, G_CALLBACK(view_menu_cb), THREE_LINES_PER_TRANSACTION, menu_name(_("View"), _("Show one line per transaction"), NULL)},
	{menu_name(_("View"), _("Show four lines per transaction"), NULL),   NULL, G_CALLBACK(view_menu_cb), FOUR_LINES_PER_TRANSACTION, menu_name(_("View"), _("Show one line per transaction"), NULL)},

	/* Accounts menu */
	{menu_name(_("Accounts"), NULL, NULL), NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("Accounts"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("Accounts"), _("New account"), NULL),   NULL, G_CALLBACK (new_account ), 0, "<StockItem>", GTK_STOCK_NEW },
	{menu_name(_("Accounts"), _("Remove an account"), NULL),   NULL, G_CALLBACK ( delete_account), 0, "<StockItem>", GTK_STOCK_DELETE },

	/* Reports menu */
	{menu_name(_("Reports"), NULL, NULL), NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("Reports"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("Reports"), _("New report"), NULL),   NULL, G_CALLBACK ( ajout_etat), 0, "<StockItem>", GTK_STOCK_NEW },
	{menu_name(_("Reports"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Reports"), _("Clone report"), NULL),   NULL, G_CALLBACK (dupliquer_etat ), 0, "<StockItem>", GTK_STOCK_COPY },
	{menu_name(_("Reports"), _("Print report..."), NULL),   NULL, G_CALLBACK ( impression_etat_courant), 0, "<StockItem>", GTK_STOCK_PRINT },
	{menu_name(_("Reports"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Reports"), _("Import report..."), NULL),   NULL, G_CALLBACK (importer_etat ), 0, "<StockItem>", GTK_STOCK_CONVERT },
	{menu_name(_("Reports"), _("Export report..."), NULL),   NULL, G_CALLBACK ( exporter_etat), 0, "<StockItem>", GTK_STOCK_CONVERT },
	{menu_name(_("Reports"), _("Export report as HTML..."), NULL),   NULL, G_CALLBACK ( export_etat_courant_vers_html), 0, "<StockItem>", GTK_STOCK_CONVERT },
	{menu_name(_("Reports"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Reports"), _("Remove report"), NULL),   NULL, G_CALLBACK ( efface_etat), 0, "<StockItem>", GTK_STOCK_DELETE },
	{menu_name(_("Reports"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Reports"), _("Edit report..."), NULL),   NULL, G_CALLBACK (personnalisation_etat ), 0, "<StockItem>", GTK_STOCK_PROPERTIES },

	/* Help menu */
	{menu_name(_("Help"), NULL, NULL), NULL, NULL, 0, "<Branch>", NULL },
	{menu_name(_("Help"), "Detach", NULL),    NULL,  NULL, 0, "<Tearoff>", NULL },
	{menu_name(_("Help"), _("Manual"), NULL),   NULL, affiche_aide_locale, 1, NULL, NULL },
	{menu_name(_("Help"), _("Quickstart"), NULL),   NULL, affiche_aide_locale, 2, "<StockItem>", GTK_STOCK_INDEX },
	{menu_name(_("Help"), _("Translation"), NULL),   NULL, affiche_aide_locale, 3, NULL, NULL },
	{menu_name(_("Help"), _("About"), NULL),   NULL, G_CALLBACK (a_propos ) , 1, NULL, NULL },
	{menu_name(_("Help"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Help"), _("Grisbi website"), NULL),   NULL, G_CALLBACK (lien_web ), 1, NULL, NULL },
	{menu_name(_("Help"), _("Report a bug"), NULL),   NULL, G_CALLBACK ( lien_web), 2,NULL , NULL },
	{menu_name(_("Help"), _("On line User's guide"), NULL),   NULL, G_CALLBACK ( lien_web), 3, NULL, NULL },
	{menu_name(_("Help"), "Sep1", NULL),    NULL, NULL, 0, "<Separator>", NULL },
	{menu_name(_("Help"), _("Today's tip"), NULL),   NULL, G_CALLBACK (force_display_tip), FALSE, NULL, NULL },
    };

    /* Nombre d elements du menu */
    nb_item_menu = sizeof(menu_item) / sizeof(menu_item[0]);

    /* Creation de la table d acceleration */
    accel = gtk_accel_group_new ();

    /* Creation du menu */
    item_factory_menu_general = gtk_item_factory_new(GTK_TYPE_MENU_BAR,
						     "<main>", accel);

    /* Recuperation des elements du menu */
    gtk_item_factory_create_items(item_factory_menu_general,
				  nb_item_menu,
				  menu_item,
				  NULL );

    /* Recuperation du widget pour l affichage du menu */
    barre_menu = gtk_item_factory_get_widget(item_factory_menu_general,
					     "<main>");

    /* Update View/Show * menu items */
    block_menu_cb = TRUE;
    widget = gtk_item_factory_get_item ( item_factory_menu_general,
					 menu_name(_("View"), _("Show transaction form"), NULL) );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget),
				    etat.formulaire_toujours_affiche );

    widget = gtk_item_factory_get_item ( item_factory_menu_general,
					 menu_name(_("View"), _("Show grid"), NULL) );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), etat.affichage_grille );

    widget = gtk_item_factory_get_item ( item_factory_menu_general,
					 menu_name(_("View"), _("Show closed accounts"), NULL) );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), etat.show_closed_accounts );

    block_menu_cb = FALSE;

    gtk_window_add_accel_group(GTK_WINDOW(window), accel );

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
    gchar *lang = _("_C");

    /* we use lang to translate it, else C will be translated as for checked transactions */

    switch ( origine )
    {
	case 1:
	    lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", lang+1, "/grisbi-manuel.html", NULL ));
	    break;	

	case 2:
	    lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", lang+1, "/quickstart.html", NULL ));
	    break;

	case 3:
	    lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", lang+1, "/translation.html", NULL ));
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


/** 
 * Callback called when an item of the "View" menu is triggered.
 * Responsible for all handler blocking needed by change_aspect_liste.
 *
 * \param callback_data Not used
 * \param callback_action A view_menu_action enum which contains the
 *                        action to perform
 * \param widget The GtkItemFactory that triggered this event.  Not used.
 */
void view_menu_cb ( gpointer callback_data, guint callback_action, GtkWidget *widget )
{
    gboolean widget_state;
    gint current_account;

    /* FIXME benj: ugly but I cannot find a way to block this ... I
       understand why gtkitemfactory is deprecated. */
    if ( block_menu_cb ) return;

    widget_state = gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM(widget) );
    current_account = gsb_account_get_current_account ();

    switch ( callback_action )
    {
	case HIDE_SHOW_TRANSACTION_FORM:
	    affiche_cache_le_formulaire();
	    break;
	case HIDE_SHOW_GRID:
	    change_aspect_liste (0);
	    break;
	case HIDE_SHOW_RECONCILED_TRANSACTIONS:
	    if ( gsb_account_get_r ( current_account ) )
		change_aspect_liste(6);
	    else
		change_aspect_liste(5);
	    gsb_account_set_r ( current_account, widget_state );
	    break;
	case ONE_LINE_PER_TRANSACTION:
	    change_aspect_liste (1);
	    break;
	case TWO_LINES_PER_TRANSACTION:
	    change_aspect_liste (2);
	    break;
	case THREE_LINES_PER_TRANSACTION:
	    change_aspect_liste (3);
	    break;
	case FOUR_LINES_PER_TRANSACTION:
	    change_aspect_liste (4);
	    break;
	case HIDE_SHOW_CLOSED_ACCOUNTS:
	    etat.show_closed_accounts = widget_state;
	    gtk_tree_model_filter_refilter ( navigation_model_filtered );
	    break;
	default:
	    break;
    }

}


/**
 * update the view menu in the menu bar
 * \account_number the account used to update the menu
 * \return FALSE
 * */
gboolean gsb_menu_update_view_menu ( gint account_number )
{
    GtkWidget *check_menu_item;
    gchar * item_name = NULL;

    if ( DEBUG )
	printf ( "gsb_menu_update_view_menu account : %d\n",
		 account_number );

    block_menu_cb = TRUE;

    /* update the showing of reconciled transactions */

    check_menu_item = gtk_item_factory_get_item ( item_factory_menu_general,
						  menu_name(_("View"), _("Show reconciled transactions"), NULL) );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(check_menu_item),
				    gsb_account_get_r (account_number));

    /* update the number of line showed */

    switch ( gsb_account_get_nb_rows (account_number))
    {
	case 1 :
	    item_name = menu_name ( _("View"), _("Show one line per transaction"), NULL );
	    break;
	case 2 :
	    item_name = menu_name ( _("View"), _("Show two lines per transaction"), NULL );
	    break;
	case 3 :
	    item_name = menu_name ( _("View"), _("Show three lines per transaction"), NULL );
	    break;
	case 4 :
	    item_name = menu_name ( _("View"), _("Show four lines per transaction"), NULL );
	    break;
    }

    check_menu_item = gtk_item_factory_get_item ( item_factory_menu_general, item_name );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(check_menu_item),
				    TRUE );
    block_menu_cb = FALSE;

    return FALSE;
}


/** 
 * Update the clickable list of closed accounts and target
 * accounts to move a transaction, in menu.
 *
 * \param
 * \return FALSE
 * */
gboolean gsb_menu_update_accounts_in_menus ( void )
{
    GSList *list_tmp;

    if ( DEBUG )
	printf ( "gsb_menu_update_accounts_in_menus\n" );

    /* erase the closed accounts and accounts in the menu to move a transaction */
    /* FIXME : je pense un bug ici, il va effacer que les comptes dont il connait le nom, peut 
     * être faudrait passer par les menus directement ?? à vérifier (cédric)*/

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	gchar *tmp;

	i = gsb_account_get_no_account ( list_tmp -> data );

	tmp = my_strdelimit ( gsb_account_get_name (i),
			      "/",
			      "\\/" );

	if ( !gsb_account_get_closed_account ( i ))
	    gtk_item_factory_delete_item ( item_factory_menu_general,
					   menu_name(_("Edit"), _("Move transaction to another account"), tmp ));
	
	list_tmp = list_tmp -> next;
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Edit"), _("Move transaction to another account"), NULL)),
			       FALSE );


    /* create the closed accounts and accounts in the menu to move a transaction */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	GtkItemFactoryEntry *item_factory_entry;
	gchar *tmp;

	i = gsb_account_get_no_account ( list_tmp -> data );

	/* 	we put all the accounts in the edition menu */
	item_factory_entry = calloc ( 1, sizeof ( GtkItemFactoryEntry ));

	tmp = my_strdelimit ( gsb_account_get_name (i), "/", "\\/" );

	item_factory_entry -> path = menu_name(_("Edit"), 
					       _("Move transaction to another account"), 
					       my_strdelimit ( tmp, "_", "__" ) ); 

	item_factory_entry -> callback = G_CALLBACK ( move_selected_operation_to_account_nb );

	/* add 1, else for 0 it wouldn't work */
	item_factory_entry -> callback_action = i + 1;

	gtk_item_factory_create_item ( item_factory_menu_general,
				       item_factory_entry,
				       GINT_TO_POINTER (i),
				       1 );

	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("Edit"), _("Move transaction to another account"), NULL)),
				   TRUE );

	if ( i == gsb_account_get_current_account () )
	{
	    /* un-sensitive the name of account in the menu */
	    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								   menu_name(_("Edit"), _("Move transaction to another account"), tmp)),
				       FALSE );
	}

	list_tmp = list_tmp -> next;
    }
    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
