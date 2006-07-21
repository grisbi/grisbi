/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2004-2006 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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
#include "metatree.h"
#include "gsb_transactions_list.h"
#include "erreur.h"
#include "dialog.h"
#include "gsb_data_account.h"
#include "operations_comptes.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_currency.h"
#include "utils_dates.h"
#include "fenetre_principale.h"
#include "navigation.h"
#include "gsb_form_transaction.h"
#include "gtk_combofix.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "comptes_gestion.h"
#include "structures.h"
#include "metatree.h"
#include "include.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gboolean division_node_maybe_expand ( GtkTreeModel *model, GtkTreePath *path, 
				      GtkTreeIter *iter, gpointer data );
static void fill_transaction_row ( GtkTreeModel * model, GtkTreeIter * iter, 
			    gint transaction_number );
static gboolean find_associated_transactions ( MetatreeInterface * iface, 
					gint no_division, gint no_sub_division );
static gboolean find_destination_blob ( MetatreeInterface * iface, GtkTreeModel * model, 
				 gpointer division, gpointer sub_division, 
				 gint * no_div, gint * no_sub_div );
static GtkTreeIter * get_iter_from_pointer ( GtkTreeModel * model, gpointer pointer );
static void metatree_fill_division ( GtkTreeModel * model, MetatreeInterface * iface, 
			      int div_id );
static void metatree_fill_new_division ( MetatreeInterface * iface, GtkTreeModel * model, 
				  gint div_id );
static void metatree_fill_new_sub_division ( MetatreeInterface * iface, GtkTreeModel * model, 
				      gint div_id, gint sub_div_id );
static void metatree_fill_sub_division ( GtkTreeModel * model, MetatreeInterface * iface, 
				  int div_id, int sub_div_id );
static gboolean metatree_get ( GtkTreeModel * model, GtkTreePath * path,
			gint column, gpointer * data );
static gboolean metatree_get_row_properties ( GtkTreeModel * tree_model, GtkTreePath * path, 
				       gchar ** text, gint * lvl1, gint * lvl2, 
				       gpointer * data );
static enum meta_tree_row_type metatree_get_row_type ( GtkTreeModel * tree_model, 
						GtkTreePath * path );
static gboolean metatree_model_is_displayed ( GtkTreeModel * model );
static void metatree_new_sub_division ( GtkTreeModel * model, gint div_id );
static void move_transaction_to_sub_division ( gpointer  transaction,
					GtkTreeModel * model,
					GtkTreePath * orig_path, GtkTreePath * dest_path,
					gint no_division, gint no_sub_division );
static gboolean search_for_div_or_subdiv ( GtkTreeModel *model, GtkTreePath *path,
				    GtkTreeIter *iter, gpointer * pointers);
static gboolean search_for_pointer ( GtkTreeModel *model, GtkTreePath *path,
			      GtkTreeIter *iter, gpointer * pointers);
static void supprimer_sub_division ( GtkTreeView * tree_view, GtkTreeModel * model,
			      MetatreeInterface * iface, 
			      gpointer sub_division, gint no_division );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *formulaire;
extern GtkTreeModel * navigation_model;
extern GtkTreeSelection * selection;
extern GtkWidget *tree_view;
/*END_EXTERN*/



/**
 * Determine whether a model is displayed.  That is, in metatree's
 * meaning, it contains at least an iter.
 *
 * \param model		A GtkTreeModel to test.
 *
 * \return		TRUE on success.
 */
gboolean metatree_model_is_displayed ( GtkTreeModel * model )
{
    GtkTreeIter iter;
    
    return gtk_tree_model_get_iter_first ( model, &iter );
}



/**
 * Utility function that returns some values associated to an entry.
 *
 * \param tree_model	Tree model that contains entry.
 * \param path		A GtkTreePath pointing to entry to test.
 * \param text		A pointer to a char array that will be filled
 *			with the text content of entry if non null
 *			(aka META_TREE_TEXT_COLUMN).
 * \param lvl1		A pointer to an int that will be filled
 *			with the division of entry if non null (aka
 *			META_TREE_NO_DIV_COLUMN).
 * \param lvl2		A pointer to an int that will be filled
 *			with the sub division of entry if non null
 *			(aka META_TREE_NO_SUB_DIV_COLUMN).
 * \param data		A pointer to an int that will be filled
 *			with the pointer associated to  entry if non
 *			null (aka META_TREE_NO_SUB_DIV_COLUMN).
 *
 * \return		TRUE on success.
 */
gboolean metatree_get_row_properties ( GtkTreeModel * tree_model, GtkTreePath * path, 
				       gchar ** text, gint * lvl1, gint * lvl2, 
				       gpointer * data )
{
    GtkTreeIter iter;
    gint tmp_lvl1, tmp_lvl2;
    gpointer tmp_data;
    gchar * tmp_text;

    if ( !gtk_tree_model_get_iter ( GTK_TREE_MODEL(tree_model), &iter, path ) )
    {
	/* This can be because drag is not possible, so no croak */
	return FALSE;
    }

    gtk_tree_model_get ( GTK_TREE_MODEL(tree_model), &iter,
			 META_TREE_TEXT_COLUMN, &tmp_text,
			 META_TREE_NO_DIV_COLUMN, &tmp_lvl1,
			 META_TREE_NO_SUB_DIV_COLUMN, &tmp_lvl2,
			 META_TREE_POINTER_COLUMN, &tmp_data,
			 -1);

    if ( text ) *text = tmp_text;
    if ( lvl1 ) *lvl1 = tmp_lvl1;
    if ( lvl2 ) *lvl2 = tmp_lvl2;
    if ( data ) *data = tmp_data;

    return TRUE;
}



/**
 * Determine row type, which can be division, sub division,
 * transaction or invalid.
 * 
 * \param tree_model	GtkTreeModel containing the entry to test.
 * \param path		A GtkTreePath pointing to entry to test.
 *
 * \return		Type of entry.
 */
enum meta_tree_row_type metatree_get_row_type ( GtkTreeModel * tree_model, 
						GtkTreePath * path )
{
    gint lvl1, lvl2;
  
    if ( metatree_get_row_properties ( tree_model, path, NULL, &lvl1, &lvl2, NULL ) )
    {
	if ( lvl1 == -1 )
	{
	    if ( lvl2 == -1 )
		return META_TREE_TRANSACTION;
	    else 
		return META_TREE_INVALID;
	}
	else
	{
	    if ( lvl2 == -1 )
		return META_TREE_DIV;
	    else
		return META_TREE_SUB_DIV;
	}
    }

    return META_TREE_INVALID;
}



/**
 * \todo Document this
 *
 */
gboolean metatree_get ( GtkTreeModel * model, GtkTreePath * path,
			gint column, gpointer * data )
{
    GtkTreeIter iter;

    if ( gtk_tree_model_get_iter ( model, &iter, path ) && data )
    {
	gtk_tree_model_get ( model, &iter, column, data, -1 );
	return TRUE;
    }
    return FALSE;
}



/**
 * Fill a division row with textual representation of a division
 * structure, in the form: "Name (num transactions) Balance".
 *
 * \param model		The GtkTreeModel that contains iter.
 * \param iface		A pointer to the metatree interface to use
 * \param iter		Iter to fill with division data.
 * \param division		Division structure.
 */
void fill_division_row ( GtkTreeModel * model, MetatreeInterface * iface, 
			 GtkTreeIter * iter, gpointer division )
{
    gchar * label = NULL, * balance = NULL;
    const gchar *string_tmp;
    GtkTreeIter dumb_iter;

    if ( ! metatree_model_is_displayed ( model ) )
	return;

    devel_debug ( g_strdup_printf ("fill_division_row %p", division) );

    string_tmp = ( division ? iface -> div_name (division) : _(iface->no_div_label) );
    
    if ( ! division )
	division = iface -> get_without_div_pointer ();

    if ( division &&
	 iface -> div_nb_transactions ( division ) )
	label = g_strconcat ( string_tmp, " (",
			      utils_str_itoa ( iface -> div_nb_transactions (division) ), ")",
			      NULL );

    if ( division && iface -> div_nb_transactions (division) )
	balance = gsb_format_amount ( iface -> div_balance ( division ),
				      iface -> tree_currency () );
    
    if ( iface -> depth == 1 && 
	 ! gtk_tree_model_iter_has_child ( model, iter ) && 
	iface -> div_nb_transactions ( division ) )
    {
	gtk_tree_store_append (GTK_TREE_STORE (model), &dumb_iter, iter );
    }

    gtk_tree_store_set (GTK_TREE_STORE(model), iter,
			META_TREE_TEXT_COLUMN, label,
			META_TREE_POINTER_COLUMN, division,
			META_TREE_BALANCE_COLUMN, balance,
			META_TREE_XALIGN_COLUMN, 1.0,
			META_TREE_NO_DIV_COLUMN, iface -> div_id ( division ),
			META_TREE_NO_SUB_DIV_COLUMN, -1,
			META_TREE_FONT_COLUMN, 800,
			META_TREE_DATE_COLUMN, NULL,
			-1);
    g_free (label);
}



/**
 * Fill a sub-division row with textual representation of a
 * sub-division structure, in the form: "Name (num transactions)
 * Balance".
 *
 * \param model		The GtkTreeModel that contains iter.
 * \param iface		A pointer to the metatree interface to use
 * \param iter		Iter to fill with sub-division data.
 * \param division		Division structure (parent).
 * \param sub_division	Sub-division structure.
 */
void fill_sub_division_row ( GtkTreeModel * model, MetatreeInterface * iface, 
			     GtkTreeIter * iter, gpointer division,
			     gpointer sub_division )
{
    gchar * balance = NULL, *label;
    const gchar *string_tmp;
    GtkTreeIter dumb_iter;
    gint nb_ecritures = 0;

    if ( ! metatree_model_is_displayed ( model ) )
	return;

    devel_debug ( g_strdup_printf ("fill_sub_division_row %p %p", division, sub_division) );

    string_tmp = ( sub_division ? iface -> sub_div_name (sub_division) : _(iface -> no_sub_div_label) );

    if ( ! division )
	division = iface -> get_without_div_pointer ();

    nb_ecritures = iface -> sub_div_nb_transactions ( division, sub_division );
    
    if ( nb_ecritures )
    {
	label = g_strconcat ( string_tmp, " (", utils_str_itoa ( nb_ecritures ), ")", NULL );
	
	if ( ! gtk_tree_model_iter_has_child ( model, iter ) )
	{
	    gtk_tree_store_append (GTK_TREE_STORE (model), &dumb_iter, iter );
	}

	balance = gsb_format_amount ( iface -> sub_div_balance ( division, sub_division ),
				      iface -> tree_currency () );
    }
    
    gtk_tree_store_set ( GTK_TREE_STORE (model), iter,
			 META_TREE_TEXT_COLUMN, label,
			 META_TREE_POINTER_COLUMN, sub_division,
			 META_TREE_BALANCE_COLUMN, balance,
			 META_TREE_XALIGN_COLUMN, 1.0,
			 META_TREE_NO_DIV_COLUMN, iface -> div_id ( division ),
			 META_TREE_NO_SUB_DIV_COLUMN, iface -> sub_div_id ( sub_division ),
			 META_TREE_FONT_COLUMN, 400,
			 META_TREE_DATE_COLUMN, NULL,
			 -1 );
    g_free (label);
}



/**
 * \todo Document this
 *
 * \param model		The GtkTreeModel that contains iter.
 */
void fill_transaction_row ( GtkTreeModel * model, GtkTreeIter * iter, 
			    gint transaction_number )
{
    gchar * account, * montant, * label, * notes = NULL; /* free */

    if ( ! metatree_model_is_displayed ( model ) )
	return;

    if ( gsb_data_transaction_get_notes ( transaction_number))
    {
	if ( strlen ( gsb_data_transaction_get_notes ( transaction_number)) > 30 )
	{
	    gchar * tmp = my_strdup (gsb_data_transaction_get_notes ( transaction_number)) + 30;

	    tmp = strchr ( tmp, ' ' );
	    if ( !tmp )
	    {
		/* We do not risk splitting the string
		   in the middle of a UTF-8 accent
		   ... the end is probably near btw. */
		notes = my_strdup (gsb_data_transaction_get_notes ( transaction_number));
	    }
	    else 
	    {
		gchar * trunc = g_strndup ( gsb_data_transaction_get_notes ( transaction_number), 
					    ( tmp - gsb_data_transaction_get_notes ( transaction_number)) );
		notes = g_strconcat ( trunc, " ...", NULL );
		free ( trunc );
	    }
	    g_free (tmp);
	}
	else 
	{
	    notes = my_strdup (gsb_data_transaction_get_notes ( transaction_number));
	}
    }
    else
    {
	notes = my_strdup (gsb_data_payee_get_name (gsb_data_transaction_get_party_number (transaction_number),
						   TRUE));
    }

    label = gsb_format_gdate ( gsb_data_transaction_get_date (transaction_number));
 
    if ( notes )
    {
	label = g_strconcat ( label, " : ", notes, NULL );
	g_free (notes);
    }

    if ( gsb_data_transaction_get_mother_transaction_number ( transaction_number))
    {
	label = g_strconcat ( label, " (", _("breakdown"), ")", NULL );
    }

    montant = gsb_format_amount ( gsb_data_transaction_get_amount (transaction_number),
				  gsb_data_transaction_get_currency_number (transaction_number) );
    account = gsb_data_account_get_name ( gsb_data_transaction_get_account_number (transaction_number));

    gtk_tree_store_set ( GTK_TREE_STORE(model), iter, 
			 META_TREE_POINTER_COLUMN, gsb_data_transaction_get_pointer_to_transaction (transaction_number),
			 META_TREE_TEXT_COLUMN, label,
			 META_TREE_ACCOUNT_COLUMN, account,
			 META_TREE_BALANCE_COLUMN, montant,
			 META_TREE_NO_DIV_COLUMN, -1,
			 META_TREE_NO_SUB_DIV_COLUMN, -1,
			 META_TREE_XALIGN_COLUMN, 1.0,
			 META_TREE_DATE_COLUMN, gsb_data_transaction_get_date ( transaction_number ),
			 -1);
}



/**
 * Handle request for a new division.  Normally called when user
 * clicked on the "New foo" button.
 *
 * \param model		Model to create a new division for.
 */
void metatree_new_division ( GtkTreeModel * model )
{
    MetatreeInterface * iface;
    gint div_id;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   
    g_return_if_fail ( iface );

    div_id = iface -> add_div ();

    metatree_fill_new_division ( iface, model, div_id );

    modification_fichier ( TRUE );
}



/**
 * \todo Document this
 *
 *
 *
 */
void metatree_fill_new_division ( MetatreeInterface * iface, GtkTreeModel * model, 
				  gint div_id )
{
    GtkTreeIter iter, sub_iter;
    GtkTreeView * tree_view;

    devel_debug ( g_strdup_printf ("metatree_fill_new_division %d", div_id) );

    g_return_if_fail ( iface );
    if ( ! metatree_model_is_displayed ( model ) )
	return;

    gtk_tree_store_append ( GTK_TREE_STORE(model), &iter, NULL );
    fill_division_row ( model, iface, &iter, iface -> get_div_pointer ( div_id ) );

    if ( iface -> depth > 1 )
    {
	gtk_tree_store_append (GTK_TREE_STORE (model), &sub_iter, &iter);
	fill_sub_division_row ( GTK_TREE_MODEL(model), iface, &sub_iter, 
				iface -> get_div_pointer ( div_id ), NULL );
    }

    tree_view = g_object_get_data ( G_OBJECT(model), "tree-view" );
    g_return_if_fail ( tree_view );   

    gtk_tree_selection_select_iter ( gtk_tree_view_get_selection ( tree_view ), &iter );
    gtk_tree_view_scroll_to_cell ( tree_view, 
				   gtk_tree_model_get_path ( model, &iter ),
				   gtk_tree_view_get_column ( tree_view, 0 ),
				   TRUE, 0.5, 0.0 );
}



/**
 * \todo Document this
 *
 *
 *
 */
void metatree_fill_division ( GtkTreeModel * model, MetatreeInterface * iface, 
			      int div_id )
{
    GtkTreeIter * iter;

    g_return_if_fail ( iface );
    if ( ! metatree_model_is_displayed ( model ) )
	return;

    iter = get_iter_from_div ( model, div_id, -1 );
    fill_division_row ( GTK_TREE_MODEL(model), iface, iter, 
			iface -> get_div_pointer ( div_id ) );
}



/**
 * Handle request for a new division.  Normally called when user
 * clicked on the "New foo" button.
 *
 * \param model		Model to create a new division for.
 */
void metatree_new_sub_division ( GtkTreeModel * model, gint div_id )
{
    MetatreeInterface * iface;
    gint sub_div_id;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   
    g_return_if_fail ( iface );

    sub_div_id = iface -> add_sub_div ( div_id );
    if ( sub_div_id == -1 )
	return;

    metatree_fill_new_sub_division ( iface, model, div_id, sub_div_id );

    modification_fichier ( TRUE );
}



/**
 * \todo Document this
 *
 *
 *
 */
void metatree_fill_new_sub_division ( MetatreeInterface * iface, GtkTreeModel * model, 
				      gint div_id, gint sub_div_id )
{
    GtkTreeIter iter, * parent_iter;
    GtkTreeView * tree_view;

    g_return_if_fail ( iface );
    if ( ! metatree_model_is_displayed ( model ) )
	return;

    devel_debug ( g_strdup_printf ("metatree_fill_new_sub_division %d %d", 
				   div_id, sub_div_id) );


    parent_iter = get_iter_from_div ( model, div_id, -1 );

    gtk_tree_store_append ( GTK_TREE_STORE(model), &iter, parent_iter );
    fill_sub_division_row ( model, iface, &iter, 
			    iface -> get_div_pointer ( div_id ),
			    iface -> get_sub_div_pointer ( div_id, sub_div_id ) );

    tree_view = g_object_get_data ( G_OBJECT(model), "tree-view" );
    g_return_if_fail ( tree_view );   

    gtk_tree_selection_select_iter ( gtk_tree_view_get_selection ( tree_view ), &iter );
    gtk_tree_view_scroll_to_cell ( tree_view, 
				   gtk_tree_model_get_path ( model, &iter ),
				   gtk_tree_view_get_column ( tree_view, 0 ),
				   TRUE, 0.5, 0.0 );
}



/**
 * \todo Document this
 *
 *
 *
 */
void metatree_fill_sub_division ( GtkTreeModel * model, MetatreeInterface * iface, 
				  int div_id, int sub_div_id )
{
    GtkTreeIter * iter;

    g_return_if_fail ( iface );
    if ( ! metatree_model_is_displayed ( model ) )
	return;

    iter = get_iter_from_div ( model, div_id, sub_div_id );
    fill_sub_division_row ( GTK_TREE_MODEL(model), iface, iter, 
			    iface -> get_div_pointer ( div_id ), 
			    iface -> get_sub_div_pointer ( div_id, sub_div_id ) );
}



/**
 * \todo Document this
 *
 *
 */
void appui_sur_ajout_sub_division ( GtkTreeModel * model )
{
    MetatreeInterface * iface;
    GtkTreeIter parent_iter;
    GtkTreeView * tree_view;
    GtkTreeSelection * selection;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   
    tree_view = g_object_get_data ( G_OBJECT(model), "tree-view" );
    if ( !iface || !tree_view )
	return;

    selection = gtk_tree_view_get_selection ( tree_view );
    if ( selection && gtk_tree_selection_get_selected ( selection, &model, &parent_iter ) )
    {
	GtkTreePath * path = gtk_tree_model_get_path ( model, &parent_iter );
	gint div_id;

	/* Get parent division id */
	metatree_get_row_properties ( model, path, NULL, &div_id, NULL, NULL ) ;
	if ( div_id == -1 )
	    return;

	metatree_new_sub_division ( model, div_id );
			     
	modification_fichier ( TRUE );

	gtk_tree_path_free ( path );
    }
}



/**
 * \todo Document this
 * 
 *
 */
gboolean supprimer_division ( GtkTreeView * tree_view )
{
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeIter iter, * it;
    GSList * liste_tmp;
    gint no_division = 0, no_sub_division = 0;
    gpointer pointer = NULL;
    MetatreeInterface * iface;

    selection = gtk_tree_view_get_selection ( tree_view );
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_model_get ( model, &iter, 
			     META_TREE_POINTER_COLUMN, &pointer,
			     META_TREE_NO_DIV_COLUMN, &no_division,
			     META_TREE_NO_SUB_DIV_COLUMN, &no_sub_division,
			     -1 );
    }

    if ( pointer == NULL )
    {
	/* FIXME: tell the truth ;-) */
	dialogue_warning_hint ( _("This should not theorically happen."),
				_("Can't remove selected division."));
	return FALSE;
    }

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   

    if ( no_sub_division != -1 )
    {
	supprimer_sub_division ( tree_view, model, iface, pointer, no_division );
	return FALSE;
    }

    if ( find_associated_transactions ( iface, no_division, -1 ) )
    {
	gint nouveau_no_division, nouveau_no_sub_division;
	GSList *list_tmp_transactions;
	
	if ( ! find_destination_blob ( iface, model, pointer, NULL, 
				       &nouveau_no_division, &nouveau_no_sub_division ) )
	    return FALSE;

	/* on fait le tour des opés pour mettre le nouveau numéro de
	 * division et sub_division */

	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( iface -> transaction_div_id (gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp)) == no_division )
	    {
		move_transaction_to_sub_division ( list_tmp_transactions -> data, model,
						   NULL, NULL, nouveau_no_division,
						   nouveau_no_sub_division );
	    }
	    list_tmp_transactions = list_tmp_transactions -> next;
	}


	/* fait le tour des échéances pour mettre le nouveau numéro
	 * de division et sub_division  */

	liste_tmp = gsb_data_scheduled_get_scheduled_list ();

	while ( liste_tmp )
	{
	    gint scheduled_number;

	    scheduled_number = gsb_data_scheduled_get_scheduled_number (liste_tmp -> data);

	    if ( iface -> scheduled_div_id (scheduled_number) == no_division )
	    {
		iface -> scheduled_set_div_id ( scheduled_number, nouveau_no_division );
		iface -> scheduled_set_sub_div_id ( scheduled_number, nouveau_no_sub_division );
	    }

	    liste_tmp = liste_tmp -> next;
	}

	/* Fill sub division */
	it = get_iter_from_div ( model, nouveau_no_division, nouveau_no_sub_division );
	if ( it )
	    fill_sub_division_row ( model, iface, it,
				    iface -> get_div_pointer (nouveau_no_division),
				    iface -> get_sub_div_pointer (nouveau_no_division, 
								  nouveau_no_sub_division ) );

	/* Fill division as well */
	it = get_iter_from_div ( model, nouveau_no_division, -1 );
	if ( it )
	    fill_division_row ( model, iface, it,
				iface -> get_div_pointer (nouveau_no_division) );

	demande_mise_a_jour_tous_comptes ();
    }

    /* supprime dans la liste des division  */
    iface -> remove_div ( no_division ); 

    selection = gtk_tree_view_get_selection ( tree_view );
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_store_remove ( GTK_TREE_STORE(model), &iter );
    }    

    modification_fichier(TRUE);
    
    return FALSE;
}



/**
 * \todo Document this
 * 
 *
 */
void supprimer_sub_division ( GtkTreeView * tree_view, GtkTreeModel * model,
			      MetatreeInterface * iface, 
			      gpointer sub_division, gint no_division )
{
    gpointer division;
    GtkTreeSelection * selection;
    GtkTreeIter iter, * it;

    division = iface -> get_div_pointer ( no_division );

    if ( find_associated_transactions ( iface, no_division, 
					iface -> sub_div_id ( sub_division ) ) )
    {
	gint nouveau_no_division, nouveau_no_sub_division;
	GSList *liste_tmp;
	GSList *list_tmp_transactions;

	if ( ! find_destination_blob ( iface, model, division, sub_division, 
				       &nouveau_no_division, &nouveau_no_sub_division ) )
	    return;

	/* on fait le tour des opés pour mettre le nouveau numéro de
	 * division et sub_division */

	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( ( iface -> transaction_div_id (gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp)) == 
		   iface -> div_id (division) ) &&
		 ( iface -> transaction_sub_div_id (gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp)) == 
		   iface -> sub_div_id (sub_division) ) )
	    {
		iface -> add_transaction_to_sub_div ( gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp), nouveau_no_division,
						      nouveau_no_sub_division );
		iface -> transaction_set_div_id (gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp), nouveau_no_division);
		iface -> transaction_set_sub_div_id (gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp), nouveau_no_sub_division);
	    }
	    list_tmp_transactions = list_tmp_transactions -> next;
	}

	/* fait le tour des échéances pour mettre le nouveau numéro
	 * de division et sub_division  */

	liste_tmp = gsb_data_scheduled_get_scheduled_list ();

	while ( liste_tmp )
	{
	    gint scheduled_number;

	    scheduled_number = gsb_data_scheduled_get_scheduled_number (liste_tmp -> data);

	    if ( iface -> scheduled_div_id (scheduled_number) == no_division &&
		 ( iface -> scheduled_sub_div_id (scheduled_number) == 
		   iface -> sub_div_id ( sub_division ) ) )
	    {
		iface -> scheduled_set_div_id ( scheduled_number, nouveau_no_division );
		iface -> scheduled_set_sub_div_id ( scheduled_number, nouveau_no_sub_division );
	    }

	    liste_tmp = liste_tmp -> next;
	}

	/* Fill sub division */
	it = get_iter_from_div ( model, nouveau_no_division, nouveau_no_sub_division );
	if ( it )
	    fill_sub_division_row ( model, iface, it,
				    iface -> get_div_pointer (nouveau_no_division),
				    iface -> get_sub_div_pointer (nouveau_no_division, 
								  nouveau_no_sub_division ) );

	/* Fill division as well */
	it = get_iter_from_div ( model, nouveau_no_division, -1 );
	if ( it )
	    fill_division_row ( model, iface, it,
				iface -> get_div_pointer (nouveau_no_division) );
	
	modification_fichier(TRUE);

	demande_mise_a_jour_tous_comptes ();

    }

    /* supprime dans la liste des division  */
    iface -> remove_sub_div ( no_division, iface -> sub_div_id ( sub_division ) );

    selection = gtk_tree_view_get_selection ( tree_view );
    if ( selection && gtk_tree_selection_get_selected ( selection, &model, &iter ) )
    {
	gtk_tree_store_remove ( GTK_TREE_STORE(model), &iter );
    }    

    modification_fichier(TRUE);
}



/**
 * \todo Document this
 * 
 *
 */
gboolean division_column_expanded  ( GtkTreeView * treeview, GtkTreeIter * iter, 
				     GtkTreePath * tree_path, gpointer user_data ) 
{
    GtkTreeModel * model;
    GtkTreeIter child_iter;
    gchar *name;
    gint no_division, no_sub_division;
    MetatreeInterface * iface;

    /* Get model and metatree interface */
    model = gtk_tree_view_get_model(treeview);

    gtk_tree_model_iter_children( model, &child_iter, iter );
    gtk_tree_model_get ( model, &child_iter, META_TREE_TEXT_COLUMN, &name, -1 );

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   

    /* If there is already an entry there, don't populate it. */
    if ( !name )
    {
	gboolean first = TRUE;
	GSList *list_tmp_transactions;

	gtk_tree_model_get ( model, iter,
			     META_TREE_NO_DIV_COLUMN, &no_division,
			     META_TREE_NO_SUB_DIV_COLUMN, &no_sub_division,
			     -1 );

	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( transaction_number_tmp &&
		 iface -> transaction_div_id ( gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp)) == no_division &&
		 iface -> transaction_sub_div_id ( gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp)) == no_sub_division )
	    {
		if ( !first )
		{
		    gtk_tree_store_append ( GTK_TREE_STORE(model), &child_iter, iter );
		}
		else
		{
		    first = FALSE;
		}

		fill_transaction_row ( model, &child_iter, transaction_number_tmp);
	    }
	    list_tmp_transactions = list_tmp_transactions -> next;
	}
    }
    return FALSE;
}



/**
 * \todo Document this
 *  
 *
 */
gboolean division_activated ( GtkTreeView * treeview, GtkTreePath * path,
			      GtkTreeViewColumn * col, gpointer userdata )
{
    gpointer  operation;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gint no_division, no_sub_division;

    model = gtk_tree_view_get_model(treeview);

    if ( gtk_tree_model_get_iter ( model, &iter, path ) )
    {
	gint transaction_number;
	
	gtk_tree_model_get( model, &iter, 
			    META_TREE_NO_DIV_COLUMN, &no_division,
			    META_TREE_NO_SUB_DIV_COLUMN, &no_sub_division,
			    META_TREE_POINTER_COLUMN, &operation, 
			    -1);

	transaction_number = gsb_data_transaction_get_transaction_number (operation);

	/* We do not jump to a transaction if a division is specified */
	if ( transaction_number && no_division == -1 && no_sub_division == -1 )
	{
	    /* If transaction is reconciled, show reconciled
	     * transactions. */
	    if ( gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE &&
		 !gsb_data_account_get_r (gsb_gui_navigation_get_current_account ()))
	    {
		mise_a_jour_affichage_r ( TRUE );
	    }

	    gsb_data_account_list_gui_change_current_account ( GINT_TO_POINTER ( gsb_data_transaction_get_account_number (transaction_number)));
	    remplissage_details_compte ();
	    gsb_gui_notebook_change_page ( GSB_ACCOUNT_PAGE );
	    gsb_gui_navigation_set_selection ( GSB_ACCOUNT_PAGE, 
					       gsb_data_transaction_get_account_number (transaction_number), 
					       NULL );

	    gsb_transactions_list_set_current_transaction ( transaction_number );
	    gsb_transactions_list_move_to_current_transaction ( gsb_data_transaction_get_account_number (transaction_number) );
	}
	else
	{
	    gtk_tree_view_expand_row ( treeview, path, FALSE );
	}
    }

    return FALSE;
}



/**
 * \todo Document this
 * 
 *
 */
gboolean division_row_drop_possible ( GtkTreeDragDest * drag_dest, GtkTreePath * dest_path,
				      GtkSelectionData * selection_data )
{
    if ( dest_path && selection_data )
    {
	enum meta_tree_row_type orig_type, dest_type;
	GtkTreePath * orig_path;
	GtkTreeModel * model;
	gint orig_no_div, no_div;
	gpointer pointer;

	gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);

	if ( model == GTK_TREE_MODEL(navigation_model) )
	{
	    return navigation_row_drop_possible ( drag_dest, dest_path, selection_data );
	}

	orig_type = metatree_get_row_type ( model, orig_path );

	dest_type = metatree_get_row_type ( model, dest_path );
	if ( ! metatree_get ( model, dest_path, META_TREE_NO_DIV_COLUMN, 
			      (gpointer) &no_div ) ||
	     ! metatree_get ( model, orig_path, META_TREE_NO_DIV_COLUMN, 
			      (gpointer) &orig_no_div ) ||
	     ! metatree_get ( model, dest_path, META_TREE_POINTER_COLUMN, 
			      (gpointer) &pointer ) )
	{
	    return FALSE;
	}

	switch ( orig_type )
	{
	    case META_TREE_SUB_DIV:
		if ( dest_type == META_TREE_DIV && 
		     ! gtk_tree_path_is_ancestor ( dest_path, orig_path ) &&
		     pointer && 
		     orig_no_div != 0 && 
		     no_div != 0 ) /* i.e. ancestor is no "No division" */
		    return TRUE;
		break;

	    case META_TREE_TRANSACTION:
		if ( dest_type == META_TREE_DIV || 
		     dest_type == META_TREE_SUB_DIV )
		    return TRUE;
		break;

	    default:
		break;
	}
    }

    return FALSE;
}



/**
 * \todo Document this
 *  
 *
 */
gboolean division_drag_data_received ( GtkTreeDragDest * drag_dest, GtkTreePath * dest_path,
				       GtkSelectionData * selection_data )
{
    devel_debug ( g_strdup_printf ("division_drag_data_received %p, %p, %p", drag_dest, dest_path, 
				   selection_data));

    if ( dest_path && selection_data )
    {
	GtkTreeModel * model;
	GtkTreePath * orig_path;
	GtkTreeIter iter, iter_parent, orig_iter, orig_parent_iter;
	gchar * name;
	gint no_dest_division, no_dest_sub_division, no_orig_division, no_orig_sub_division;
	gpointer sub_division = NULL, dest_sub_division = NULL;
	gpointer orig_division = NULL, dest_division = NULL, pointer = NULL;
	enum meta_tree_row_type orig_type;
	gpointer  transaction = NULL;
	MetatreeInterface * iface;
	GSList *list_tmp_transactions;

	gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);

	if ( model == GTK_TREE_MODEL(navigation_model) )
	{
	    return navigation_drag_data_received ( drag_dest, dest_path, selection_data );
	}

	iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );
	if ( ! iface )
	    return FALSE;

	metatree_get_row_properties ( model, orig_path,
				      NULL, &no_orig_division, &no_orig_sub_division, 
				      &pointer );
	orig_type = metatree_get_row_type ( model, orig_path );

	metatree_get_row_properties ( model, dest_path,
				      &name, &no_dest_division, &no_dest_sub_division, 
				      NULL );
	if ( ! name )
	{
	    gtk_tree_path_up ( dest_path );
	    metatree_get_row_properties ( model, dest_path,
					  &name, &no_dest_division, &no_dest_sub_division, 
					  NULL );
	}
	
	switch ( orig_type )
	{
	    case META_TREE_TRANSACTION:
		transaction = (gpointer ) pointer;
		if ( transaction )
		{
		    move_transaction_to_sub_division ( transaction, model, 
						       orig_path, dest_path,
						       no_dest_division, no_dest_sub_division );
		}
		break;

	    case META_TREE_SUB_DIV:
		dest_division = iface -> get_div_pointer ( no_dest_division );
		orig_division = iface -> get_div_pointer ( no_orig_division );
		sub_division = iface -> get_sub_div_pointer ( no_orig_division, no_orig_sub_division );

		if ( sub_division )
		    dest_sub_division = iface -> get_sub_div_pointer_from_name ( no_dest_division,
										 iface -> sub_div_name (sub_division),
										 1 );
		if ( dest_sub_division )
		    no_dest_sub_division = iface -> sub_div_id ( dest_sub_division );
		else
		    no_dest_sub_division = 0;

		/* Populate tree */
		gtk_tree_model_get_iter ( model, &iter_parent, dest_path );
		if ( iface -> get_sub_div_pointer ( no_orig_division, 
						    no_orig_sub_division ) )
		{
		    gtk_tree_store_append ( GTK_TREE_STORE(model), &iter, &iter_parent);
		}
		else
		{
		    /* To handle "no categ" ? */
		    GtkTreeIter * p_iter;
		    p_iter = get_iter_from_div ( model, no_dest_division, 0 );
		    if ( p_iter )
		    {
			iter = *p_iter;
		    }
		}
		list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

		while ( list_tmp_transactions )
		{
		    gint transaction_number_tmp;
		    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

		    if ( transaction_number_tmp &&
			 iface -> transaction_div_id (gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp)) == no_orig_division &&
			 iface -> transaction_sub_div_id (gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp)) == no_orig_sub_division )
		    {
			GtkTreePath * path;
			path = gtk_tree_model_get_path ( model, &iter );
			move_transaction_to_sub_division ( gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp), model, 
							   NULL, path,
							   no_dest_division, 
							   no_dest_sub_division );
		    }
		    list_tmp_transactions = list_tmp_transactions -> next;
		}

		gtk_tree_model_get_iter ( model, &orig_iter, orig_path );

		/* Update original parent. */
		if ( gtk_tree_model_iter_parent ( model, &orig_parent_iter, &orig_iter ) )
		{
		    fill_division_row ( model, iface, &orig_parent_iter, orig_division );
		}
		
		/* Remove original division. */
		iface -> remove_sub_div ( no_orig_division, no_orig_sub_division );
		gtk_tree_store_remove ( GTK_TREE_STORE(model), &orig_iter );

		/* If it was no sub-category, recreate it. */
		if ( ! no_orig_sub_division )
		{
		    metatree_fill_new_sub_division ( iface, model, 
						     no_orig_division, no_orig_sub_division );
		}
	
		/* Update dest at last. */
		fill_sub_division_row ( model, iface, &iter, 
					dest_division, dest_sub_division );

		modification_fichier(TRUE);

		break;

	    default:
		break;
	}
    }
    
    return FALSE;
}



/**
 * \todo Document this
 * 
 *
 */
void move_transaction_to_sub_division ( gpointer  transaction,
					GtkTreeModel * model,
					GtkTreePath * orig_path, GtkTreePath * dest_path,
					gint no_division, gint no_sub_division )
{
    GtkTreeIter orig_iter, child_iter, dest_iter, parent_iter, gd_parent_iter;
    gpointer old_division, new_division;
    gpointer old_sub_division, new_sub_division;
    MetatreeInterface * iface;
    gint transaction_number;
	
    if ( ! model )
	return;

    transaction_number = gsb_data_transaction_get_transaction_number (transaction);
    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );

    old_division = iface -> get_div_pointer ( iface -> transaction_div_id (transaction) );
    new_division = iface -> get_div_pointer ( no_division );

    old_sub_division = iface -> get_sub_div_pointer ( iface -> transaction_div_id (transaction),
						      iface -> transaction_sub_div_id (transaction) );
    new_sub_division = iface -> get_sub_div_pointer ( no_division, no_sub_division );

    /* Insert new row */
    if ( dest_path )
    {
	gtk_tree_model_get_iter ( model, &dest_iter, dest_path );
    }
    else
    {
	GtkTreeIter * p_iter = get_iter_from_div ( model, no_division, no_sub_division );
	if ( p_iter )
	{
	    dest_iter = *p_iter;
	}
	else
	{
	    return;
	}
    }

    /* Avoid filling "empty" not yet selected subdivisions */
    if ( gtk_tree_model_iter_children ( model, &child_iter, &dest_iter ) )
    {
	gchar * name;
	gtk_tree_model_get ( model, &child_iter, META_TREE_TEXT_COLUMN, &name, -1 );
	if ( name )
	{
	    gtk_tree_store_insert ( GTK_TREE_STORE (model), &child_iter, &dest_iter, 0 );
	    fill_transaction_row ( model, &child_iter, transaction_number);
	}
    }
    else
    {
	gtk_tree_store_append ( GTK_TREE_STORE (model), &child_iter, &dest_iter );
	fill_transaction_row ( model, &child_iter, transaction_number);
    }

    /* Update new parents */
    if ( iface -> depth > 1 )
    {
	iface -> add_transaction_to_sub_div ( transaction, no_division, no_sub_division );
	fill_sub_division_row ( model, iface, &dest_iter, new_division, new_sub_division );
	if ( gtk_tree_model_iter_parent ( model, &parent_iter, &dest_iter ) )
	    fill_division_row ( model, iface, &parent_iter, new_division );
    }
    else
    {
	iface -> add_transaction_to_div ( transaction, no_division );
	fill_division_row ( model, iface, &dest_iter, new_division );
    }

    /* Update old parents */
    iface -> remove_transaction_from_sub_div ( transaction, 
					       iface -> transaction_div_id (transaction),
					       iface -> transaction_sub_div_id (transaction) );

    /* Change parameters */
    iface -> transaction_set_div_id ( transaction, no_division );
    iface -> transaction_set_sub_div_id ( transaction, no_sub_division );
    gsb_transactions_list_update_transaction (transaction_number);

    if ( orig_path )
    {
	if ( gtk_tree_model_get_iter ( model, &orig_iter, orig_path ) )
	{
	    if ( gtk_tree_model_iter_parent ( model, &parent_iter, &orig_iter ) )
	    {
		if ( iface -> depth > 1 )
		{
		    fill_sub_division_row ( model, iface, &parent_iter, 
					    old_division, old_sub_division );
		    if ( gtk_tree_model_iter_parent ( model, &gd_parent_iter, 
						      &parent_iter ) )
			fill_division_row ( model, iface, &gd_parent_iter, old_division );
		}
		else
		{
		    fill_division_row ( model, iface, &parent_iter, old_division );
		}
	    }
	    
	    /* Remove old row */
	    gtk_tree_store_remove ( GTK_TREE_STORE (model), &orig_iter );
	}
    }
		    
    /* We did some modifications */
    modification_fichier ( TRUE );
}



/**
 * \todo Document this
 * 
 *
 */
gboolean division_node_maybe_expand ( GtkTreeModel *model, GtkTreePath *path, 
				      GtkTreeIter *iter, gpointer data )
{
    GtkTreeView * tree_view = g_object_get_data ( G_OBJECT(model), "tree-view" );

    if ( tree_view )
    {
	if ( (gint) data == gtk_tree_path_get_depth ( path ) )
	    gtk_tree_view_expand_to_path ( tree_view, path );
    }

    return FALSE;
}



/**
 * Iterates over all divisions tree nodes and expand nodes that are
 * not deeper than specified depth.
 *
 * \param bouton	Widget that triggered this callback.  Not used.
 * \param depth		Maximum depth for nodes to expand.
 */
void expand_arbre_division ( GtkWidget *bouton, gint depth )
{
    GtkTreeView * tree_view = g_object_get_data ( G_OBJECT(bouton), "tree-view" );
    GtkTreeModel * model;
    
    if ( tree_view )
    {
	gtk_tree_view_collapse_all ( tree_view );
	model = gtk_tree_view_get_model ( tree_view );
	gtk_tree_model_foreach ( model, division_node_maybe_expand, (gpointer) depth );
    }
}



/**
 * \todo Document this
 * 
 *
 */
gboolean find_destination_blob ( MetatreeInterface * iface, GtkTreeModel * model, 
				 gpointer division, gpointer sub_division, 
				 gint * no_div, gint * no_sub_div )
{
    GtkWidget *dialog, *hbox, *bouton_division_generique, *combofix, *bouton_transfert;
    GSList *liste_combofix, *pointeur, *liste_division_credit, *liste_division_debit;
    gint resultat, nouveau_no_division, nouveau_no_sub_division;
    gpointer nouvelle_division;
    gpointer nouvelle_sub_division;
    gchar **split_division;

    dialog = dialogue_special_no_run ( GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
				       make_hint ( g_strdup_printf ( _("'%s' still contains transactions."), 
								     ( !sub_division ? 
								       iface -> div_name ( division ) :
								       iface -> sub_div_name ( sub_division ) ) ),
						   g_strdup_printf ( _("If you want to remove it but want to keep transactions, you can transfer them to another (sub-)%s.  Otherwise, transactions can be simply deleted along with their division."), 
								     g_ascii_strdown ( iface -> meta_name, -1 ) ) ) );

    /*       mise en place du choix tranfert vers un autre division */

    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox,
			 FALSE, FALSE, 0 );

    bouton_transfert = gtk_radio_button_new_with_label ( NULL,
							 COLON(_("Transfer transactions to division"))  );
    gtk_box_pack_start ( GTK_BOX ( hbox ), bouton_transfert,
			 FALSE, FALSE, 0 );

    pointeur = iface -> div_list ( );
    liste_combofix = NULL;
    liste_division_credit = NULL;
    liste_division_debit = NULL;

    while ( pointeur )
    {
	gpointer p_division;
	GSList *sous_pointeur;

	p_division = pointeur -> data;

	if ( division && ! sub_division && division == p_division )
	{
	    pointeur = pointeur -> next;
	    continue;
	}

	switch ( iface -> div_type ( p_division ) )
	{
	    case 1:
		liste_division_debit = g_slist_append ( liste_division_debit,
							my_strdup ( iface -> div_name ( p_division ) ) );
		break;
	    default:
		liste_division_credit = g_slist_append ( liste_division_credit,
							 my_strdup ( iface -> div_name ( p_division ) ) );
		break;
	}

	sous_pointeur = iface -> div_sub_div_list ( p_division );

	while ( sous_pointeur )
	{
	    gpointer p_sub_division;

	    p_sub_division = sous_pointeur -> data;

	    if ( iface -> sub_div_id (p_sub_division) !=  iface -> sub_div_id (sub_division) )
	    {
		switch ( iface -> div_type ( p_division ) )
		{
		    case 1:
			liste_division_debit = g_slist_append ( liste_division_debit,
								g_strconcat ( "\t",
									      iface -> sub_div_name ( p_sub_division ),
									      NULL ) );
			break;
		    case 0:
			liste_division_credit = g_slist_append ( liste_division_credit,
								 g_strconcat ( "\t",
									       iface -> sub_div_name ( p_sub_division ),
									       NULL ) );
			break;
		    default:
			break;
		}

	    }

	    sous_pointeur = sous_pointeur -> next;
	}

	pointeur = pointeur -> next;
    }

    /*   on ajoute les listes des crÃ©dits / débits à la liste
     *   du combofix du formulaire */
    liste_combofix = g_slist_append ( liste_combofix, liste_division_debit );
    liste_combofix = g_slist_append ( liste_combofix, liste_division_credit );

    combofix = gtk_combofix_new_complex (liste_combofix);
    gtk_box_pack_start ( GTK_BOX ( hbox ), combofix, TRUE, TRUE, 0 );

    /*       mise en place du choix supprimer le division */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox,
			 FALSE, FALSE, 0 );
    
    bouton_division_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )),
								  PRESPACIFY(_("Just remove this sub-division.")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), bouton_division_generique, FALSE, FALSE, 0 );

    gtk_widget_show_all ( dialog );


  retour_dialogue:
    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
	return FALSE;
    }

    nouveau_no_division = 0;
    nouveau_no_sub_division = 0;

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_transfert )) )
    {

	if ( !strlen (gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ))))
	{
	    dialogue_warning_hint ( _("It is compulsory to specify a destination division to move transactions but no division was entered."),
				    _("Please enter a division!"));

	    goto retour_dialogue;
	}

	/* récupère les no de division et sous division */
	split_division = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
				      " : ", 2 );

	nouvelle_division = iface -> get_div_pointer_from_name ( split_division[0], 0 );

	if ( nouvelle_division )
	{
	    nouveau_no_division = iface -> div_id ( nouvelle_division );
	    nouvelle_sub_division =  iface -> get_sub_div_pointer_from_name ( nouveau_no_division, split_division[1], 0 );

	    if ( nouvelle_sub_division )
	    {
		nouveau_no_sub_division = iface -> sub_div_id ( nouvelle_sub_division );
	    }
	}

	g_strfreev ( split_division );
    }
    else
    {
	nouveau_no_division = 0;
	nouveau_no_sub_division = 0;
    }

    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    if ( no_div)
	*no_div = nouveau_no_division;
    if ( no_sub_div )
	*no_sub_div = nouveau_no_sub_division;

    return TRUE;
}



/** 
 * Find transactions that are associated with a div or subdiv.
 *
 * \param iface		A MetatreeInterface to use.
 * \param no_division	Division id to search for.
 * \param no_sub_division Subdivision id to search for, or -1 to search
 *			only on division.
 *
 * \return TRUE if transactions are associated.
 *
 * \todo Return number of transactions instead, to produce nicer
 * dialogs.
 */
gboolean find_associated_transactions ( MetatreeInterface * iface, 
					gint no_division, gint no_sub_division )
{
    GSList *liste_tmp;
    GSList *list_tmp_transactions;

    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( iface -> transaction_div_id ( gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp) ) == no_division &&
	     ( no_sub_division == -1 ||
	       iface -> transaction_sub_div_id ( gsb_data_transaction_get_pointer_to_transaction (transaction_number_tmp)) == no_sub_division ) )
	{
	    return TRUE;
	}
	else
	    list_tmp_transactions = list_tmp_transactions -> next;
    }


    liste_tmp = gsb_data_scheduled_get_scheduled_list ();

    /* fait le tour des échéances pour en trouver une qui a cette catégorie  */
    while ( liste_tmp )
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (liste_tmp -> data);

	if ( iface -> scheduled_div_id (scheduled_number) == no_division && 
	     ( no_sub_division == -1 ||
	       iface -> scheduled_div_id (scheduled_number) == no_sub_division ) )
	{
	    return TRUE;
	}
	else
	    liste_tmp = liste_tmp -> next;
    }

    return FALSE;
}



/**
 * \todo Document this
 *
 *
 */
gboolean search_for_div_or_subdiv ( GtkTreeModel *model, GtkTreePath *path,
				    GtkTreeIter *iter, gpointer * pointers)
{
    int no_div, no_sub_div;
    gpointer text;

    gtk_tree_model_get ( GTK_TREE_MODEL(model), iter,
			 META_TREE_TEXT_COLUMN, &text, 
			 META_TREE_NO_DIV_COLUMN, &no_div, 
			 META_TREE_NO_SUB_DIV_COLUMN, &no_sub_div, 
			 -1 );

    /* This is a kludge because we want to skip "dummy" iters that are
     * here only to provide a slider.*/
    if ( !text )
	return FALSE;

    if ( ! pointers[0] && ( !pointers[1] || pointers[1] == (gpointer) -1 ) && 
	 !no_div && !no_sub_div )
    {
	pointers[2] = gtk_tree_iter_copy (iter);
	return TRUE;
    }

    if ( no_div == (gint) pointers[0] )
    {
	if ( ( (gint) pointers[1] == -1 ) || 
	     ( no_sub_div == (gint) pointers[1] ) )
	{
	    pointers[2] = gtk_tree_iter_copy (iter);
	    return TRUE;
	}
    }

    return FALSE;
}



/**
 * \todo Document this
 *
 *
 */
GtkTreeIter * get_iter_from_div ( GtkTreeModel * model, int div, int sub_div )
{
    gpointer pointers[3] = { (gpointer) div, (gpointer) sub_div, NULL };

    gtk_tree_model_foreach ( model, (GtkTreeModelForeachFunc) search_for_div_or_subdiv, 
			     pointers );

    return (GtkTreeIter *) pointers[2];
}



/**
 * \todo Document this
 *
 *
 */
gboolean search_for_pointer ( GtkTreeModel *model, GtkTreePath *path,
			      GtkTreeIter *iter, gpointer * pointers)
{
    gpointer pointer;

    gtk_tree_model_get ( GTK_TREE_MODEL(model), iter,
			 META_TREE_POINTER_COLUMN, &pointer, 
			 -1 );

    if ( pointer == pointers[0] )
    {
	pointers[1] = gtk_tree_iter_copy (iter);
	return TRUE;
    }

    return FALSE;
}



/**
 * \todo Document this
 *
 *
 */
GtkTreeIter * get_iter_from_pointer ( GtkTreeModel * model, gpointer pointer )
{
    gpointer pointers[2] = { (gpointer) pointer, NULL };

    gtk_tree_model_foreach ( model, (GtkTreeModelForeachFunc) search_for_pointer, pointers );

    return (GtkTreeIter *) pointers[1];
}



/**
 * Update a transaction in a tree model if it is possible to find its
 * associated GtkTreeIter.  This function is not responsible to remove
 * old transaction if transaction has changed in a way that would
 * move it in the tree (i.e, its category has changed and we want to
 * update the category tree).
 *
 * \param iface		A MetatreeInterface to use.
 * \param model		Tree model to update.
 * \param transaction_number   Transaction to update if associated GtkTreeIter exists.
 */
void update_transaction_in_tree ( MetatreeInterface * iface, GtkTreeModel * model, 
				  gint transaction_number )
{
    GtkTreeIter * transaction_iter, * sub_div_iter = NULL, * div_iter;
    GtkTreePath * div_path, * sub_div_path, * transaction_path;
    gpointer div, sub_div;
    gint div_id, sub_div_id;
    gpointer transaction_pointer;
    
    if ( !transaction_number || ! metatree_model_is_displayed ( model ) )
	return;

    /* FIXME : sould remove transaction_pointer for all the metatree */
    transaction_pointer = gsb_data_transaction_get_pointer_to_transaction (transaction_number);
    
    div_id = iface -> transaction_div_id ( transaction_pointer );
    sub_div_id = iface -> transaction_sub_div_id ( transaction_pointer );
    div = iface -> get_div_pointer ( div_id );
    sub_div = iface -> get_sub_div_pointer ( div_id, sub_div_id );

    /* Fill in division if existing. */
    div_iter = get_iter_from_div ( model, div_id, -1 );
    if ( div_iter )
    {
	fill_division_row ( model, iface, div_iter, div );
    }
    else
    {
	metatree_fill_new_division ( iface, model, div_id );
	div_iter = get_iter_from_div ( model, div_id, -1 );
    }

    /* Fill in sub-division if existing. */
    if ( iface -> depth != 1 )
    {
	sub_div_iter = get_iter_from_div ( model, div_id, sub_div_id );
	if ( sub_div_iter )
	{
	    fill_sub_division_row ( model, iface, sub_div_iter, div, sub_div );
	}
	else
	{
	    metatree_fill_new_sub_division ( iface, model, div_id, sub_div_id );
	    sub_div_iter = get_iter_from_div ( model, div_id, sub_div_id );
	}
    }

    /* Fill in transaction if existing. */
    transaction_iter = get_iter_from_pointer ( model, transaction_pointer );

    if ( transaction_iter )
    {
	div_path = gtk_tree_model_get_path ( model, div_iter );
	sub_div_path = gtk_tree_model_get_path ( model, sub_div_iter );
	transaction_path = gtk_tree_model_get_path ( model, transaction_iter );
	if ( ( iface -> depth != 1 &&
	       ! gtk_tree_path_is_ancestor ( sub_div_path, transaction_path ) ) ||
	     ! gtk_tree_path_is_ancestor ( div_path, transaction_path ) )
	{
	    printf (">Removing old\n");
	    gtk_tree_store_remove ( GTK_TREE_STORE(model), transaction_iter );
	    transaction_iter = NULL;
	}
    }

    /* If no transaction iter is found, this either means transactions
     * for this division hasn't been shown yet, so no need to fill it;
     * or that it is a new transaction, so we need to append it to
     * subdivision row. */
    if ( ! transaction_iter )
    {
	GtkTreeIter child_iter;
	gpointer text;
	gtk_tree_model_iter_children ( model, &child_iter, 
				       ( iface -> depth == 1 ? div_iter : sub_div_iter ) );
	gtk_tree_model_get ( model, &child_iter, META_TREE_TEXT_COLUMN, &text, -1 );

	/* Text is set only if division has been expanded previously,
	 * so we can add an iter.  Otherwise, this will be done by the
	 * expanded callback. */
	if ( text )
	{
	    gtk_tree_store_append ( GTK_TREE_STORE(model), &child_iter,
				    ( iface -> depth == 1 ? div_iter : sub_div_iter ) );
	    transaction_iter = &child_iter;
	}
    }

    if ( transaction_iter )
    {

	fill_transaction_row ( model, transaction_iter, transaction_number );
    }
}



/**
 * Performs actions needed when selection of a metatree has changed.
 * First, update the headings bar accordingly.  Then update
 * sensitiveness of linked widgets.
 *
 * \return TRUE
 */
gboolean metatree_selection_changed ( GtkTreeSelection * selection, GtkTreeModel * model )
{
    MetatreeInterface * iface;
    GtkTreeView * tree_view;
    GtkTreeIter iter;
    gboolean selection_is_set = FALSE;
    gpointer div = NULL, sub_div = NULL;
    gint div_id, sub_div_id;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   
    tree_view = g_object_get_data ( G_OBJECT(model), "tree-view" );
    if ( !iface || !tree_view )
	return FALSE;

    if ( selection && gtk_tree_selection_get_selected ( selection, &model, &iter ) )
    {
	gchar * text, * balance = "";
	gpointer pointer;

	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     META_TREE_NO_DIV_COLUMN, &div_id,
			     META_TREE_NO_SUB_DIV_COLUMN, &sub_div_id,
			     META_TREE_POINTER_COLUMN, &pointer,
			     -1);

	div = iface -> get_div_pointer ( div_id );

	if ( ! div && pointer )
	{
	    div_id = iface -> transaction_div_id ( pointer );
	    div = iface -> get_div_pointer ( div_id );

	    sub_div_id = iface -> transaction_sub_div_id ( pointer );
	}

	text = g_strconcat ( _(iface -> meta_name),  " : ", 
			     (div ? iface -> div_name ( div ) : _(iface->no_div_label) ),
			     NULL );

	if ( div ) 
	{
	    balance = gsb_format_amount ( iface -> div_balance ( div ),
					  iface -> tree_currency () );
	}

	if ( sub_div_id >= 0 )
	{
	    sub_div = iface -> get_sub_div_pointer ( div_id, sub_div_id );
	    text = g_strconcat ( text, " : ", 
				 ( sub_div ? iface -> sub_div_name ( sub_div ) :
				   _(iface->no_sub_div_label) ), NULL );
	    balance = gsb_format_amount ( iface -> sub_div_balance ( div, sub_div ),
					  iface -> tree_currency () );
	}

	gsb_gui_headings_update ( text, balance );
	selection_is_set = TRUE;
    }

    /* Update sensitiveness of linked widgets. */
    metatree_set_linked_widgets_sensitive ( model, selection_is_set, "selection" );
    if ( ! div_id )
    {
	metatree_set_linked_widgets_sensitive ( model, FALSE, "selection" );
    }

    if ( selection_is_set && 
	 metatree_get_row_type ( model, gtk_tree_model_get_path ( model, &iter ) ) == META_TREE_DIV &&
	 div_id )
    {
	metatree_set_linked_widgets_sensitive ( model, TRUE, "sub-division" );
    }
    else
    {
	metatree_set_linked_widgets_sensitive ( model, FALSE, "sub-division" );
    }

    return TRUE;
}



/**
 * Link a widget to a metatree.  This means, in some events like
 * selection change, it will be set sensitive or unsensitive.
 *
 * \param model		Model to link widget to.
 * \param widget	Widget to link to metatree.
 * \param link_type	A string representing different links.  Should
 *			be "selection" for a widget that is sensitive
 *			only if a line is selected or "sub-division"
 *			for a widget that is sensitive only if a first
 *			level entry is selected.
 */
void metatree_register_widget_as_linked ( GtkTreeModel * model, GtkWidget * widget,
					  gchar * link_type )
{
    GSList * links;

    g_return_if_fail ( widget != NULL );
    g_return_if_fail ( model != NULL );
    g_return_if_fail ( link_type && strlen ( link_type ) );

    links = g_object_get_data ( G_OBJECT(model), link_type );
    g_object_set_data ( G_OBJECT(model), link_type, g_slist_append ( links, widget ) );
}



/**
 * Set widgets linked to a metatree (normally, buttons) sensitive or
 * unsensitive.
 *
 * \param model		Model widgets are linked to.
 * \param sensitive	Set widgets sensitive if TRUE, unsensitive otherwise.
 * \param link_type	A string representing different links.  Should
 *			be "selection" for a widget that is sensitive
 *			only if a line is selected or "sub-division"
 *			for a widget that is sensitive only if a first
 *			level entry is selected.
 */
void metatree_set_linked_widgets_sensitive ( GtkTreeModel * model, gboolean sensitive,
					     gchar * link_type )
{
    GSList * links = g_object_get_data ( G_OBJECT(model), link_type );

    while ( links )
    {
	if ( links -> data && GTK_IS_WIDGET ( links -> data ) )
	{
	    gtk_widget_set_sensitive ( GTK_WIDGET ( links -> data ), sensitive );
	}
	links = links -> next;
    }
}



/**
 * Compare two iters from a metatree.  If they both contain a date
 * (from the META_TREE_DATE_COLUMN field), then a date comparison is
 * done.  Otherwise, it compares dates.  Normally, it is called as a
 * backend function from gtk_tree_sortable_set_sort_func().
 *
 * \param model		Sortable tree model that triggered the sort.
 * \param a		First iter to compare.
 * \param b		Second iter to compare.
 * \param user_data	Not used.
 *
 * \return		Same as a > b
 */
inline gboolean metatree_sort_column  ( GtkTreeModel * model, 
					GtkTreeIter * a, GtkTreeIter * b, 
					gpointer user_data )
{
    GDate * date_a = NULL, * date_b = NULL;
    gchar * string_a, * string_b;

    gtk_tree_model_get ( model, a, 
			 META_TREE_DATE_COLUMN, &date_a, 
			 META_TREE_TEXT_COLUMN, &string_a,  -1 );
    gtk_tree_model_get ( model, b, 
			 META_TREE_DATE_COLUMN, &date_b, 
			 META_TREE_TEXT_COLUMN, &string_b, -1 );

    if ( ! date_a && ! date_b )
    {
	return g_utf8_collate ( string_a, string_b );
    }

    if ( ! date_b )
    {
	return 1;
    }
    else if ( ! date_a )
    {
	return -1;
    }

    return g_date_compare ( date_a, date_b );
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
