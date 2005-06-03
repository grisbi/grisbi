/*  Fichier qui gère la liste des no_accounts, la partie gauche de l'onglet opérations */
/*      operations_no_accounts.c */

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
#include "operations_comptes.h"
#include "gsb_account.h"
#include "menu.h"
#include "operations_liste.h"
#include "gtk_list_button.h"
#include "barre_outils.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "comptes_traitements.h"
#include "operations_formulaire.h"
#include "utils_comptes.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_account_list_gui_change_order ( GtkWidget *button );
static GtkWidget *gsb_account_list_gui_create_account_button ( gint no_account,
							gint group,
							gpointer callback );
static void verifie_no_account_clos ( gint no_nouveau_no_account );
/*END_STATIC*/


/* adresse de la vbox contenant les icones de no_accounts */

GtkWidget *vbox_liste_comptes;

/* adr du label du dernier relevé */

GtkWidget *label_last_statement;


/*START_EXTERN*/
extern GtkWidget *formulaire;
extern GtkItemFactory *item_factory_menu_general;
extern gchar *last_date;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint nb_colonnes;
/*END_EXTERN*/




/** return a button with an icon and the name of the account
 * \param no_account
 * \param group 1 for the transactions list, 2 for the account page
 * \param callback function to call when clicked
 * \return a GtkButton
 * */
GtkWidget *gsb_account_list_gui_create_account_button ( gint no_account,
							gint group,
							gpointer callback )
{
    GtkWidget *button;

    button = gtk_list_button_new ( gsb_account_get_name (no_account), group, TRUE, GINT_TO_POINTER (no_account));

    if ( group == 1 )
	gsb_account_set_account_button( no_account,
					button );

    g_signal_connect_swapped ( G_OBJECT (button),
			       "clicked",
			       G_CALLBACK ( callback ),
			       GINT_TO_POINTER ( no_account ) );
    g_signal_connect ( G_OBJECT ( button ),
		       "reordered",
		       G_CALLBACK ( gsb_account_list_gui_change_order ),
		       NULL );
    gtk_widget_show ( button );

    return ( button );
}


void changement_no_compte_par_menu ( gpointer null,
				     gint no_account_plus_un )
{
    gsb_account_list_gui_change_current_account ( GINT_TO_POINTER ( no_account_plus_un - 1) );
}




/** 
 * change the list of transactions, according to the new account
 * \param no_account a pointer on the number of the account we want to see
 * \return FALSE
 * */
gboolean gsb_account_list_gui_change_current_account ( gint *no_account )
{
    gint new_account;
    gint current_account;

    new_account = GPOINTER_TO_INT ( no_account );
    current_account = gsb_account_get_current_account ();

    if ( DEBUG )
	printf ( "gsb_account_list_gui_change_current_account : %d\n", new_account );

    /* sensitive the last account in the menu */

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Edit"),
								     _("Move transaction to another account"),
								     my_strdelimit ( gsb_account_get_name (current_account),
										     "/",
										     "\\/" ))),
			       TRUE );

    /* save the adjustment of the last account */

    if ( gsb_account_get_vertical_adjustment_value (current_account) != -1 )
    {
	GtkAdjustment *adjustment;

	adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()));
	gsb_account_set_vertical_adjustment_value ( current_account,
						    gtk_adjustment_get_value ( adjustment ));
    }
    
    /*     on se place sur les données du nouveau no_account */

    gsb_account_set_current_account (new_account);

    gsb_transactions_list_set_visibles_rows_on_account (new_account);
    gsb_transactions_list_set_background_color (new_account);
    gsb_transactions_list_set_transactions_balances (new_account);
    /*     affiche le nouveau formulaire  */
    /*     il met aussi à jour la devise courante et les types */

    remplissage_formulaire ( new_account );

    /*     mise en place de la date du dernier relevé */

    if ( gsb_account_get_current_reconcile_date (new_account) )
	gtk_label_set_text ( GTK_LABEL ( label_last_statement ),
			     g_strdup_printf ( _("Last statement: %02d/%02d/%d"), 
					       g_date_day ( gsb_account_get_current_reconcile_date (new_account)),
					       g_date_month ( gsb_account_get_current_reconcile_date (new_account)),
					       g_date_year ( gsb_account_get_current_reconcile_date (new_account))));

    else
	gtk_label_set_text ( GTK_LABEL ( label_last_statement ),
			     _("Last statement: none") );


    /* affiche le solde final en bas */

    mise_a_jour_labels_soldes ();

    /* met les boutons R et nb lignes par opé comme il faut */

    mise_a_jour_boutons_caract_liste ( new_account );

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Edit"),
								     _("Move transaction to another account"),
								     my_strdelimit ( gsb_account_get_name (new_account),
										     "/",
										     "\\/" ))),
			       FALSE );

    gsb_transactions_list_set_adjustment_value (new_account);

    /* unset the last date written */

    last_date = NULL;

    return FALSE;
}
/* ********************************************************************************************************** */


/* ********************************************************************************************************** */
/* cette fonction est appelée lors d'un changement de no_account */
/* cherche si le nouveau no_account est clos, si c'est le cas, ferme l'icone du no_account courant */
/* ********************************************************************************************************** */
void verifie_no_account_clos ( gint no_nouveau_no_account )
{
    /*     si le no_account courant est déjà cloturé, on fait rien */

    if ( gsb_account_get_closed_account (gsb_account_get_current_account ()) )
	return;

    if ( gsb_account_get_closed_account (no_nouveau_no_account) )
    {
	gtk_list_button_close ( GTK_BUTTON ( gsb_account_get_account_button (gsb_account_get_current_account ()) ));
    }
}
/* ********************************************************************************************************** */





/** 
 * Called when the order of accounts changed
 *
 * \param button
 * \return FALSE
 * */
gboolean gsb_account_list_gui_change_order ( GtkWidget *button )
{
    GSList *new_list_order;
    GList *list_buttons_accounts;

    list_buttons_accounts = GTK_BOX ( button-> parent ) -> children;
    new_list_order = NULL;

    while ( list_buttons_accounts )
    {
	GtkBoxChild *box_child;

	box_child = list_buttons_accounts -> data;

	new_list_order = g_slist_append ( new_list_order,
					  gtk_list_button_get_data ( GTK_LIST_BUTTON ( box_child -> widget )));
	list_buttons_accounts = list_buttons_accounts -> next;
    }

    gsb_account_reorder ( new_list_order );
    g_slist_free (new_list_order);

    /* we remake the other accounts list */

    gsb_menu_update_accounts_in_menus ();

    mise_a_jour_liste_comptes_accueil = 1;

    update_options_menus_comptes ();
    modification_fichier (TRUE);

    return ( FALSE );
}
/* *********************************************************************************************************** */




/******************************************************************************/
/* règle la taille des widgets dans le formulaire des opés en fonction */
/* des paramètres */
/******************************************************************************/
void mise_a_jour_taille_formulaire ( gint largeur_formulaire )
{

    gint i, j;
    struct organisation_formulaire *organisation_formulaire;

    if ( !largeur_formulaire )
	return;

    organisation_formulaire = renvoie_organisation_formulaire ();

    for ( i=0 ; i < organisation_formulaire -> nb_lignes ; i++ )
	for ( j=0 ; j < organisation_formulaire -> nb_colonnes ; j++ )
	{
	    GtkWidget *widget;

	    widget = widget_formulaire_par_element ( organisation_formulaire -> tab_remplissage_formulaire[i][j] );

	    if ( widget )
		gtk_widget_set_usize ( widget,
				       organisation_formulaire -> taille_colonne_pourcent[j] * largeur_formulaire / 100,
				       FALSE );
	}
}
/******************************************************************************/




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
