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
#include "erreur.h"
#include "gsb_data_account.h"
#include "utils_dates.h"
#include "gsb_form.h"
#include "navigation.h"
#include "menu.h"
#include "gsb_transactions_list.h"
#include "comptes_traitements.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/* adresse de la vbox contenant les icones de no_accounts */

GtkWidget *vbox_liste_comptes;


/*START_EXTERN*/
extern gint compte_courant_onglet;
extern GtkWidget *label_last_statement ;
extern GtkTreeSelection * selection;
/*END_EXTERN*/




/** 
 * change the list of transactions, according to the new account
 * 
 * \param no_account a pointer on the number of the account we want to see
 * 
 * \return FALSE
 * */
gboolean gsb_data_account_list_gui_change_current_account ( gint *no_account )
{
    gint new_account;
    gint current_account;
    GtkTreePath *path;

    new_account = GPOINTER_TO_INT ( no_account );

    /* the selection on the navigation bar has already changed, so
     * have to use a buffer variable to get the last account */
    current_account = gsb_gui_navigation_get_last_account ();

    devel_debug ( g_strdup_printf ("gsb_data_account_list_gui_change_current_account : %d", new_account ));

    /* sensitive the last account in the menu */

    gsb_gui_sensitive_menu_item ( "EditMenu", "MoveToAnotherAccount", 
				  gsb_data_account_get_name (current_account),
				  TRUE );

    /* save the adjustment of the last account */
    path = gsb_data_account_get_vertical_adjustment_value (current_account);
    if (path)
    {
	GtkTreePath *new_path;

	if (gtk_tree_view_get_visible_range (GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
					     &new_path, NULL ))
	{
	    gsb_data_account_set_vertical_adjustment_value ( current_account,
							     new_path);
	    gtk_tree_path_free (new_path);
	}
	gtk_tree_path_free (path);
    }
    
    /*     on se place sur les données du nouveau no_account */

    gsb_transactions_list_set_visibles_rows_on_account (new_account);
    gsb_transactions_list_set_background_color (new_account);
    gsb_transactions_list_set_transactions_balances (new_account);

/*     if (etat.formulaire_toujours_affiche) */
/* 	gsb_form_fill_from_account (new_account); */

    /*     mise en place de la date du dernier relevé */

    if ( gsb_data_account_get_current_reconcile_date (new_account) )
	gtk_label_set_text ( GTK_LABEL ( label_last_statement ),
			     g_strdup_printf ( _("Last statement: %s"), 
					       gsb_format_gdate ( gsb_data_account_get_current_reconcile_date (new_account) ) ) );

    else
	gtk_label_set_text ( GTK_LABEL ( label_last_statement ),
			     _("Last statement: none") );


    gsb_gui_sensitive_menu_item ( "EditMenu", "MoveToAnotherAccount", 
				  gsb_data_account_get_name (new_account),
				  FALSE );

    gsb_transactions_list_set_adjustment_value (new_account);

    /* Set current account, then. */
    compte_courant_onglet = new_account;

    /* unset the last date written */
    gsb_date_free_last_date ();

    return FALSE;
}





/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
