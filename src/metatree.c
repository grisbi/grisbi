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
#include "./dialog.h"
#include "./gsb_data_account.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_currency.h"
#include "./utils_dates.h"
#include "./fenetre_principale.h"
#include "./navigation.h"
#include "./gsb_transactions_list.h"
#include "./gtk_combofix.h"
#include "./traitement_variables.h"
#include "./utils_str.h"
#include "./comptes_gestion.h"
#include "./fenetre_principale.h"
#include "./gtk_combofix.h"
#include "./gsb_data_transaction.h"
#include "./gsb_transactions_list.h"
#include "./include.h"
#include "./erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gboolean division_node_maybe_expand ( GtkTreeModel *model, GtkTreePath *path, 
				      GtkTreeIter *iter, gpointer data );
static void fill_transaction_row ( GtkTreeModel * model, GtkTreeIter * iter, 
			    gint transaction_number );
static gboolean find_associated_transactions ( MetatreeInterface * iface, 
					gint no_division, gint no_sub_division );
static gboolean find_destination_blob ( MetatreeInterface * iface, GtkTreeModel * model, 
				 gint division, gint sub_division, 
				 gint * no_div, gint * no_sub_div );
static GtkTreeIter * get_iter_from_transaction ( GtkTreeModel * model, gint transaction_number );
static void metatree_fill_new_division ( MetatreeInterface * iface, GtkTreeModel * model, 
				  gint div_id );
static void metatree_fill_new_sub_division ( MetatreeInterface * iface, GtkTreeModel * model, 
				      gint div_id, gint sub_div_id );
static gboolean metatree_get ( GtkTreeModel * model, GtkTreePath * path,
			gint column, gint *data );
static gboolean metatree_get_row_properties ( GtkTreeModel * tree_model, GtkTreePath * path, 
				       gchar ** text, gint * lvl1, gint * lvl2, 
				       gint * data );
static enum meta_tree_row_type metatree_get_row_type ( GtkTreeModel * tree_model, 
						GtkTreePath * path );
static gboolean metatree_model_is_displayed ( GtkTreeModel * model );
static void metatree_new_sub_division ( GtkTreeModel * model, gint div_id );
static void metatree_remove_iter_and_select_next ( GtkTreeView * tree_view, GtkTreeModel * model,
					    GtkTreeIter * iter );
static void move_transaction_to_sub_division ( gint transaction_number,
					GtkTreeModel * model,
					GtkTreePath * orig_path, GtkTreePath * dest_path,
					gint no_division, gint no_sub_division );
static gboolean search_for_div_or_subdiv ( GtkTreeModel *model, GtkTreePath *path,
				    GtkTreeIter *iter, gpointer * pointers);
static gboolean search_for_transaction ( GtkTreeModel *model, GtkTreePath *path,
				  GtkTreeIter *iter, gpointer * pointers);
static void supprimer_sub_division ( GtkTreeView * tree_view, GtkTreeModel * model,
			      MetatreeInterface * iface, 
			      gint sub_division, gint division );
static void supprimer_transaction ( GtkTreeView * tree_view, GtkTreeModel * model,
			     MetatreeInterface * iface, 
			     gint transaction);
/*END_STATIC*/


/*START_EXTERN*/
extern GtkTreeModel * navigation_model ;
extern GtkTreeSelection * selection ;
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
				       gint * data )
{
    GtkTreeIter iter;
    gint tmp_lvl1, tmp_lvl2;
    gint tmp_data;
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
	    else if ( lvl1 != 0 && lvl2 != 0 )
		return META_TREE_SUB_DIV;
	}
    }

    return META_TREE_INVALID;
}



/**
 * get the content of the column and path in param
 * fill the data field with that content
 *
 * \param model
 * \param path row we want the content
 * \param column column we want the content
 * \param data a pointer to a gint to be filled with the content
 *
 * \return TRUE ok, FALSE problem
 */
gboolean metatree_get ( GtkTreeModel * model, GtkTreePath * path,
			gint column, gint *data )
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
 * \param division		Division structure number.
 */
void fill_division_row ( GtkTreeModel * model, MetatreeInterface * iface, 
			 GtkTreeIter * iter, gint division )
{
    gchar * label = NULL, * balance = NULL;
    const gchar *string_tmp;
    GtkTreeIter dumb_iter;
    GtkTreePath * path;
    enum meta_tree_row_type type;

    if ( ! metatree_model_is_displayed ( model ) )
	return;

    path = gtk_tree_model_get_path ( model, iter );
    type = metatree_get_row_type ( model, path );
    if ( type != META_TREE_DIV && type != META_TREE_INVALID )
    {
	g_free ( path );
	return;
    }
    g_free ( path );

    gchar* strtmp = g_strdup_printf ("fill_division_row %d", division);
    devel_debug ( strtmp  );
    g_free ( strtmp );

    if ( ! division )
	division = iface -> get_without_div_pointer ();

    string_tmp = iface -> div_name (division);
    if (!string_tmp)
	string_tmp = _(iface->no_div_label);

    if ( iface -> div_nb_transactions ( division ))
    {
	gchar* tmpstr = itoa ( iface -> div_nb_transactions (division) );
	label = g_strconcat ( string_tmp, " (",
			      tmpstr , ")",
			      NULL );
	g_free ( tmpstr );
	balance = gsb_format_amount ( iface -> div_balance ( division ),
				      iface -> tree_currency () );
    }
    else
	label = my_strdup (string_tmp);

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
			META_TREE_NO_DIV_COLUMN, division,
			META_TREE_NO_SUB_DIV_COLUMN, -1,
			META_TREE_FONT_COLUMN, 800,
			META_TREE_DATE_COLUMN, NULL,
			-1);
    if (label)
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
 * \param division	Division structure number (parent).
 * \param sub_division	Sub-division structure number.
 */
void fill_sub_division_row ( GtkTreeModel * model, MetatreeInterface * iface, 
			     GtkTreeIter * iter,
			     gint division,
			     gint sub_division )
{
    gchar * balance = NULL, *label = NULL;
    const gchar *string_tmp;
    GtkTreeIter dumb_iter;
    GtkTreePath * path;
    enum meta_tree_row_type type;
    gint nb_ecritures = 0;

    if ( ! metatree_model_is_displayed ( model ) )
	return;

    path = gtk_tree_model_get_path ( model, iter );
    type = metatree_get_row_type ( model, path );
    if ( type != META_TREE_SUB_DIV && type != META_TREE_INVALID )
    {
	g_free ( path );
	return;
    }
    g_free ( path );

    string_tmp = ( sub_division ? iface -> sub_div_name (division, sub_division) : _(iface -> no_sub_div_label) );

    if ( ! division )
	division = iface -> get_without_div_pointer ();

    nb_ecritures = iface -> sub_div_nb_transactions ( division, sub_division );
    
    if ( nb_ecritures )
    {
	gchar* tmpstr = itoa ( nb_ecritures );
	label = g_strconcat ( string_tmp, " (", tmpstr, ")", NULL );
	g_free ( tmpstr );
	
	if ( ! gtk_tree_model_iter_has_child ( model, iter ) )
	{
	    gtk_tree_store_append (GTK_TREE_STORE (model), &dumb_iter, iter );
	}

	balance = gsb_format_amount ( iface -> sub_div_balance ( division, sub_division ),
				      iface -> tree_currency () );
    }
    else
	label = my_strdup (string_tmp);
    
    gtk_tree_store_set ( GTK_TREE_STORE (model), iter,
			 META_TREE_TEXT_COLUMN, label,
			 META_TREE_POINTER_COLUMN, sub_division,
			 META_TREE_BALANCE_COLUMN, balance,
			 META_TREE_XALIGN_COLUMN, 1.0,
			 META_TREE_NO_DIV_COLUMN, division,
			 META_TREE_NO_SUB_DIV_COLUMN, sub_division,
			 META_TREE_FONT_COLUMN, 400,
			 META_TREE_DATE_COLUMN, NULL,
			 -1 );
    if (label)
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
    gchar * account; /* no need to be freed */
    gchar * montant = NULL;
    gchar * label = NULL; 
    gchar * notes = NULL; 
    const gchar *string;
    GtkTreePath * path;
    enum meta_tree_row_type type;

    if ( ! metatree_model_is_displayed ( model ) )
	return;

    path = gtk_tree_model_get_path ( model, iter );
    type = metatree_get_row_type ( model, path );
    if ( type != META_TREE_TRANSACTION && type != META_TREE_INVALID)
    {
	g_free ( path );
	return;
    }
    g_free ( path );

    string = gsb_data_transaction_get_notes ( transaction_number);

    if (string)
    {
	if ( strlen ( gsb_data_transaction_get_notes ( transaction_number)) > 30 )
	{
	    const gchar *tmp;

	    tmp = string + 30;

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
					    (tmp - string));
		notes = g_strconcat ( trunc, " ...", NULL );
		g_free ( trunc );
	    }
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
        gchar* tmpstr = label;
	label = g_strconcat ( tmpstr, " (", _("breakdown"), ")", NULL );
	g_free ( tmpstr );
    }

    montant = gsb_format_amount ( gsb_data_transaction_get_amount (transaction_number),
				  gsb_data_transaction_get_currency_number (transaction_number) );
    account = gsb_data_account_get_name ( gsb_data_transaction_get_account_number (transaction_number));
    gtk_tree_store_set ( GTK_TREE_STORE(model), iter, 
			 META_TREE_POINTER_COLUMN, transaction_number,
			 META_TREE_TEXT_COLUMN, label,
			 META_TREE_ACCOUNT_COLUMN, account,
			 META_TREE_BALANCE_COLUMN, montant,
			 META_TREE_NO_DIV_COLUMN, -1,
			 META_TREE_NO_SUB_DIV_COLUMN, -1,
			 META_TREE_XALIGN_COLUMN, 1.0,
			 META_TREE_FONT_COLUMN, 400,
			 META_TREE_DATE_COLUMN, gsb_data_transaction_get_date ( transaction_number ),
			 -1);
    g_free(montant);
    g_free(label);
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

    gchar* strtmp = g_strdup_printf ("metatree_fill_new_division %d", div_id);
    devel_debug ( strtmp  );
    g_free ( strtmp );

    g_return_if_fail ( iface );
    if ( ! metatree_model_is_displayed ( model ) )
	return;

    gtk_tree_store_append ( GTK_TREE_STORE(model), &iter, NULL );
    fill_division_row ( model, iface, &iter, div_id );

    if ( iface -> depth > 1 )
    {
	gtk_tree_store_append (GTK_TREE_STORE (model), &sub_iter, &iter);
	fill_sub_division_row ( GTK_TREE_MODEL(model), iface, &sub_iter, 
				div_id, 0 );
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

    gchar* strtmp = g_strdup_printf ("metatree_fill_new_sub_division %d %d", div_id, sub_div_id);
    devel_debug ( strtmp  );
    g_free ( strtmp );


    parent_iter = get_iter_from_div ( model, div_id, -1 );

    gtk_tree_store_append ( GTK_TREE_STORE(model), &iter, parent_iter );
    fill_sub_division_row ( model, iface, &iter, 
			    div_id,
			    sub_div_id );

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
    GtkTreePath * path;
    GSList * liste_tmp;
    gint no_division = 0, no_sub_division = 0;
    gint current_number = 0;
    MetatreeInterface * iface;

    selection = gtk_tree_view_get_selection ( tree_view );
    if ( selection && gtk_tree_selection_get_selected(selection, &model, &iter))
    {
	gtk_tree_model_get ( model, &iter, 
			     META_TREE_POINTER_COLUMN, &current_number,
			     META_TREE_NO_DIV_COLUMN, &no_division,
			     META_TREE_NO_SUB_DIV_COLUMN, &no_sub_division,
			     -1 );
    }

    if (!current_number)
    {
	/* FIXME: tell the truth ;-) */
	dialogue_warning_hint ( _("This should not theorically happen."),
				_("Can't remove selected division."));
	return FALSE;
    }

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   

    path = gtk_tree_model_get_path ( model, &iter );
    switch ( metatree_get_row_type ( model, path ) )
    {
	case META_TREE_TRANSACTION:
	    supprimer_transaction ( tree_view, model, iface, current_number );
	    return FALSE;
	case META_TREE_DIV:
	    /* Nothing, do the grunt job after. */
	    break;
	case META_TREE_SUB_DIV:
	    /* We asked to remove a sub-categ or sub-budget and have a
	     * function for that. */
	    supprimer_sub_division ( tree_view, model, iface, no_sub_division, no_division );
	    return FALSE;
	default:
	    warning_debug ( "tried to remove an invalid entry" );
	    return FALSE;
    }
	    
    if ( find_associated_transactions ( iface, no_division, -1 ) )
    {
	gint nouveau_no_division, nouveau_no_sub_division;
	GSList *list_tmp_transactions;
	
	/* fill nouveau_no_division and nouveau_no_sub_division */
	if ( ! find_destination_blob ( iface, model, current_number, 0, 
				       &nouveau_no_division, &nouveau_no_sub_division ) )
	    return FALSE;

	/* on fait le tour des opés pour mettre le nouveau numéro de
	 * division et sub_division */

	/* move the transactions, need to to that for archived transactions too */
	list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( iface -> transaction_div_id (transaction_number_tmp) == no_division )
	    {
		move_transaction_to_sub_division ( transaction_number_tmp, model,
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
				    nouveau_no_division,
				    nouveau_no_sub_division);

	/* Fill division as well */
	it = get_iter_from_div ( model, nouveau_no_division, -1 );
	if ( it )
	    fill_division_row ( model, iface, it, nouveau_no_division );

	/* metatree too complex for me, so instead of re-writing all the transactions, update
	 * just the value we changed, or if we come here, it's for payee, categ or budget i
	 * FIXME benj if you can know here and do only what is necessary ... */
	gsb_transactions_list_update_transaction_value (TRANSACTION_LIST_PARTY);
	gsb_transactions_list_update_transaction_value (TRANSACTION_LIST_BUDGET);
	gsb_transactions_list_update_transaction_value (TRANSACTION_LIST_CATEGORY);
    }

    /* supprime dans la liste des division  */
    iface -> remove_div ( no_division ); 

    metatree_remove_iter_and_select_next ( tree_view, model, &iter );
    modification_fichier(TRUE);
    
    return FALSE;
}



/**
 * remove a sub-division from a metatree
 *
 * \param tree_view
 * \param model
 * \param iface the metatree interface
 * \param sub_division the sub-division number we want to remove
 * \param division the division number (parent)
 *
 * \return
 */
void supprimer_sub_division ( GtkTreeView * tree_view, GtkTreeModel * model,
			      MetatreeInterface * iface, 
			      gint sub_division, gint division )
{
	/*TODO dOm : unused variable selection.
    GtkTreeSelection * selection;
    */
    GtkTreeIter iter, * it;

    if ( find_associated_transactions ( iface, division, 
					sub_division ) )
    {
	gint nouveau_no_division, nouveau_no_sub_division;
	GSList *liste_tmp;
	GSList *list_tmp_transactions;

	/* fill nouveau_no_division and nouveau_no_sub_division */
	if ( ! find_destination_blob ( iface, model, division, sub_division, 
				       &nouveau_no_division, &nouveau_no_sub_division ) )
	    return;

	/* move the transactions to the new division numbers, need to do for archived transactions too */
	list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( ( iface -> transaction_div_id (transaction_number_tmp) == division)
		 &&
		 ( iface -> transaction_sub_div_id (transaction_number_tmp) == sub_division))
	    {
		iface -> add_transaction_to_sub_div ( transaction_number_tmp, nouveau_no_division,
						      nouveau_no_sub_division );
		iface -> transaction_set_div_id (transaction_number_tmp, nouveau_no_division);
		iface -> transaction_set_sub_div_id (transaction_number_tmp, nouveau_no_sub_division);
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

	    if ( iface -> scheduled_div_id (scheduled_number) == division
		 &&
		 iface -> scheduled_sub_div_id (scheduled_number) == sub_division )
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
				    nouveau_no_division,
				    nouveau_no_sub_division );

	/* Fill division as well */
	it = get_iter_from_div ( model, nouveau_no_division, -1 );
	if ( it )
	    fill_division_row ( model, iface, it, nouveau_no_division );

	/* metatree too complex for me, so instead of re-writing all the transactions, update
	 * just the value we changed, or if we come here, it's for payee, categ or budget i
	 * FIXME benj if you can know here and do only what is necessary ... */
	gsb_transactions_list_update_transaction_value (TRANSACTION_LIST_PARTY);
	gsb_transactions_list_update_transaction_value (TRANSACTION_LIST_BUDGET);
	gsb_transactions_list_update_transaction_value (TRANSACTION_LIST_CATEGORY);
    }

    /* supprime dans la liste des division  */
    iface -> remove_sub_div (division, sub_division);

    metatree_remove_iter_and_select_next ( tree_view, model, &iter );
    modification_fichier(TRUE);
}



/**
 * Remove a transaction from a metatree
 *
 * \param tree_view
 * \param model
 * \param iface the metatree interface
 * \param transaction the transaction number we want to remove
 * \param sub_division the parent sub-division number
 * \param division the division number (parent)
 *
 * \return
 */
void supprimer_transaction ( GtkTreeView * tree_view, GtkTreeModel * model,
			     MetatreeInterface * iface, 
			     gint transaction)
{
    GtkTreeIter iter, * it;
    gint division, sub_division;

    division = iface -> transaction_div_id ( transaction );
    sub_division = iface -> transaction_sub_div_id ( transaction );
    iface -> remove_transaction_from_div ( transaction );
    gsb_data_transaction_remove_transaction ( transaction );

    /* Fill parent sub division */
    it = get_iter_from_div ( model, division, sub_division );
    if ( it )
	fill_sub_division_row ( model, iface, it, division, sub_division );

    /* Fill division as well */
    it = get_iter_from_div ( model, division, -1 );
    if ( it )
	fill_division_row ( model, iface, it, division );
    
    metatree_remove_iter_and_select_next ( tree_view, model, &iter );
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
		 iface -> transaction_div_id ( transaction_number_tmp) == no_division &&
		 iface -> transaction_sub_div_id ( transaction_number_tmp) == no_sub_division )
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
			    META_TREE_POINTER_COLUMN, &transaction_number, 
			    -1);

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

	    navigation_change_account ( GINT_TO_POINTER ( gsb_data_transaction_get_account_number (transaction_number)));
	    remplissage_details_compte ();
	    gsb_gui_notebook_change_page ( GSB_ACCOUNT_PAGE );
	    gsb_gui_navigation_set_selection ( GSB_ACCOUNT_PAGE, 
					       gsb_data_transaction_get_account_number (transaction_number), 
					       NULL );

	    gsb_transactions_list_select ( transaction_number );
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
	gint current_number;

	gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);

	if ( model == GTK_TREE_MODEL(navigation_model) )
	{
	    return navigation_row_drop_possible ( drag_dest, dest_path, selection_data );
	}

	orig_type = metatree_get_row_type ( model, orig_path );
	dest_type = metatree_get_row_type ( model, dest_path );

	if ( ! metatree_get ( model, dest_path, META_TREE_NO_DIV_COLUMN, 
			      &no_div ) ||
	     ! metatree_get ( model, orig_path, META_TREE_NO_DIV_COLUMN, 
			      &orig_no_div ) ||
	     ! metatree_get ( model, dest_path, META_TREE_POINTER_COLUMN, 
			      &current_number ) )
	{
	    return FALSE;
	}

	switch ( orig_type )
	{
	    case META_TREE_SUB_DIV:
		if ( dest_type == META_TREE_DIV && 
		     ! gtk_tree_path_is_ancestor ( dest_path, orig_path ) &&
		     current_number && 
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
    gchar* strtmp = g_strdup_printf ("division_drag_data_received %p, %p, %p", 
                                         drag_dest, dest_path, selection_data);
    devel_debug ( strtmp );
    g_free ( strtmp );

    if ( dest_path && selection_data )
    {
	GtkTreeModel * model;
	GtkTreePath * orig_path;
	GtkTreeIter iter, iter_parent, orig_iter, orig_parent_iter;
	gchar * name;
	gint no_dest_division, no_dest_sub_division, no_orig_division, no_orig_sub_division;
	enum meta_tree_row_type orig_type;
	MetatreeInterface * iface;
	GSList *list_tmp_transactions;
	gint transaction_number;

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
				      &transaction_number );
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
		if (transaction_number)
		{
		    move_transaction_to_sub_division ( transaction_number, model, 
						       orig_path, dest_path,
						       no_dest_division, no_dest_sub_division );
		}
		break;

	    case META_TREE_SUB_DIV:
		/* create the new sub-division */
		if (no_orig_sub_division)
		    no_dest_sub_division = iface -> get_sub_div_pointer_from_name ( no_dest_division,
										    iface -> sub_div_name (no_orig_division, no_orig_sub_division),
										    1 );

		/* Populate tree */
		gtk_tree_model_get_iter ( model, &iter_parent, dest_path );
		if (no_orig_sub_division)
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
			 iface -> transaction_div_id (transaction_number_tmp) == no_orig_division &&
			 iface -> transaction_sub_div_id (transaction_number_tmp) == no_orig_sub_division )
		    {
			GtkTreePath * path;
			path = gtk_tree_model_get_path ( model, &iter );
			move_transaction_to_sub_division ( transaction_number_tmp, model, 
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
		    fill_division_row ( model, iface, &orig_parent_iter, no_orig_division );
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
					no_dest_division, no_dest_sub_division );

		modification_fichier(TRUE);

		break;

	    default:
		break;
	}
    }
    
    return FALSE;
}



/**
 * move a transaction to another sub-division
 *
 * \param transaction_number
 * \param model
 * \param orig_path
 * \param dest_path
 * \param no_division new division to the transaction
 * \param no_sub_division new sub-division to the transaction
 *
 * \return	
 */
void move_transaction_to_sub_division ( gint transaction_number,
					GtkTreeModel * model,
					GtkTreePath * orig_path, GtkTreePath * dest_path,
					gint no_division, gint no_sub_division )
{
    GtkTreeIter orig_iter, child_iter, dest_iter, parent_iter, gd_parent_iter;
    MetatreeInterface * iface;
    gint old_div, old_sub_div;
	
    if ( ! model )
	return;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );

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
	iface -> add_transaction_to_sub_div ( transaction_number, no_division, no_sub_division );
	fill_sub_division_row ( model, iface, &dest_iter, no_division, no_sub_division );
	if ( gtk_tree_model_iter_parent ( model, &parent_iter, &dest_iter ) )
	    fill_division_row ( model, iface, &parent_iter, no_division );
    }
    else
    {
	iface -> add_transaction_to_div ( transaction_number, no_division );
	fill_division_row ( model, iface, &dest_iter, no_division );
    }

    /* get the old div */
    old_div = iface -> transaction_div_id (transaction_number);
    old_sub_div = iface -> transaction_sub_div_id (transaction_number);

    /* Update old parents */
    iface -> remove_transaction_from_sub_div ( transaction_number );

    /* Change parameters */
    iface -> transaction_set_div_id ( transaction_number, no_division );
    iface -> transaction_set_sub_div_id ( transaction_number, no_sub_division );
    gsb_transactions_list_update_transaction (transaction_number);

    if ( orig_path
	 &&
	 gtk_tree_model_get_iter ( model, &orig_iter, orig_path ) )
    {
	if ( gtk_tree_model_iter_parent ( model, &parent_iter, &orig_iter ) )
	{
	    if ( iface -> depth > 1 )
	    {
		fill_sub_division_row ( model, iface, &parent_iter, 
					old_div,
					old_sub_div );
		if ( gtk_tree_model_iter_parent ( model, &gd_parent_iter, 
						  &parent_iter ) )
		    fill_division_row ( model, iface, &gd_parent_iter, transaction_number );
	    }
	    else
	    {
		fill_division_row ( model, iface, &parent_iter, transaction_number );
	    }
	}

	/* Remove old row */
	gtk_tree_store_remove ( GTK_TREE_STORE (model), &orig_iter );
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
/**
 * called when there is some transactions associated to the category, budget or payee
 * show a dialog and ask another categ/budget/payee to move the concerned transactions
 * and fill no_div and no_sub_div with the user choice
 *
 * \param iface
 * \param model
 * \param division the current (old) division number
 * \param sub_division the current (old) sub-division number
 * \param no_div a pointer to gint for the choice of division chosen by user
 * \param no_sub_div a pointer to gint for the choice of sub-division chosen by user
 *
 * \return FALSE to stop the process, TRUE to continue
 * */
gboolean find_destination_blob ( MetatreeInterface * iface, GtkTreeModel * model, 
				 gint division, gint sub_division, 
				 gint * no_div, gint * no_sub_div )
{
    GtkWidget *dialog, *hbox, *bouton_division_generique, *combofix, *bouton_transfert;
    GSList *liste_combofix, *division_list, *liste_division_credit, *liste_division_debit;
    gint resultat, nouveau_no_division, nouveau_no_sub_division;
    gchar **split_division;

    /* create the box to move change the division and sub-div of the transactions */
    gchar* tmpstr1 = g_strdup_printf ( _("'%s' still contains transactions or archived transactions."), 
						     ( !sub_division ? 
						       iface -> div_name ( division ) :
						       iface -> sub_div_name ( division, sub_division ) ) );
    gchar* tmpstr2 = g_strdup_printf ( _("If you want to remove it but want to keep transactions, you can transfer them to another (sub-)%s.  Otherwise, transactions can be simply deleted along with their division."), 
							     g_ascii_strdown ( iface -> meta_name, -1 ) );
    dialog = dialogue_special_no_run ( GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
				       make_hint ( tmpstr1 , tmpstr2 ) );

    g_free ( tmpstr1 );
    g_free ( tmpstr2 );

    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox,
			 FALSE, FALSE, 0 );

    tmpstr1 = g_strdup_printf (_("Transfer transactions to %s"), iface -> meta_name);
    bouton_transfert = gtk_radio_button_new_with_label ( NULL, tmpstr1);
    g_free ( tmpstr1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), bouton_transfert,
			 FALSE, FALSE, 0 );

    division_list = iface -> div_list ( );
    liste_combofix = NULL;
    liste_division_credit = NULL;
    liste_division_debit = NULL;

    while ( division_list )
    {
	gint tmp_division;
	GSList *sub_division_list;

	tmp_division = iface -> div_id (division_list -> data);

	/* if we are on the current division and no sub-division, go to the next */
	if ( division && !sub_division && division == tmp_division )
	{
	    division_list = division_list -> next;
	    continue;
	}

	/* get the division */
	switch ( iface -> div_type (tmp_division))
	{
	    case 1:
		liste_division_debit = g_slist_append ( liste_division_debit,
							my_strdup (iface -> div_name (tmp_division)));
		break;
	    default:
		liste_division_credit = g_slist_append ( liste_division_credit,
							 my_strdup (iface -> div_name (tmp_division)));
		break;
	}

	/* get the sub-divisions */
	sub_division_list = iface -> div_sub_div_list (tmp_division);

	while ( sub_division_list )
	{
	    gint tmp_sub_division;

	    tmp_sub_division = iface -> sub_div_id (sub_division_list -> data);

	    if (tmp_sub_division == sub_division)
	    {
		sub_division_list = sub_division_list -> next;
		continue;
	    }

	    switch ( iface -> div_type (tmp_division))
	    {
		case 1:
		    liste_division_debit = g_slist_append ( liste_division_debit,
							    g_strconcat ( "\t",
									  iface -> sub_div_name (tmp_division, tmp_sub_division),
									  NULL ) );
		    break;
		case 0:
		    liste_division_credit = g_slist_append ( liste_division_credit,
							     g_strconcat ( "\t",
									   iface -> sub_div_name (tmp_division, tmp_sub_division),
									   NULL ) );
		    break;
		default:
		    break;
	    }
	    sub_division_list = sub_division_list -> next;
	}
	division_list = division_list -> next;
    }

    /*   on ajoute les listes des crÃ©dits / débits à la liste
     *   du combofix du transaction_form */
    liste_combofix = g_slist_append ( liste_combofix, liste_division_debit );
    liste_combofix = g_slist_append ( liste_combofix, liste_division_credit );

    combofix = gtk_combofix_new_complex (liste_combofix);
    gtk_combofix_set_force_text ( GTK_COMBOFIX (combofix), TRUE );

    gtk_box_pack_start ( GTK_BOX ( hbox ), combofix, TRUE, TRUE, 0 );

    /*       mise en place du choix supprimer le division */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox,
			 FALSE, FALSE, 0 );
    
    gchar* tmpstr = g_strdup_printf(_("Just remove this sub-%s."), iface -> meta_name );
    bouton_division_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )), tmpstr );
    g_free ( tmpstr );
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
	    gchar* tmpstr1 = g_strdup_printf ( _("It is compulsory to specify a destination %s to move transactions but no %s was entered."), iface -> meta_name, iface -> meta_name );
	    gchar* tmpstr2 = g_strdup_printf ( _("Please enter a %s!"), iface -> meta_name );
	    dialogue_warning_hint ( tmpstr1 , tmpstr2 );
	    g_free ( tmpstr1 );
	    g_free ( tmpstr2 );

	    goto retour_dialogue;
	}

	/* récupère les no de division et sous division */
	split_division = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
				      " : ", 2 );

	nouveau_no_division = iface -> get_div_pointer_from_name ( split_division[0], 0 );

	if (nouveau_no_division)
	    nouveau_no_sub_division =  iface -> get_sub_div_pointer_from_name ( nouveau_no_division, split_division[1], 0 );

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
    GSList *tmp_list;

    /* we need to check all the transactions, even in archives */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();

    while ( tmp_list )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	if ( iface -> transaction_div_id (transaction_number_tmp) == no_division &&
	     ( no_sub_division == -1 ||
	       iface -> transaction_sub_div_id (transaction_number_tmp) == no_sub_division ) )
	{
	    return TRUE;
	}
	else
	    tmp_list = tmp_list -> next;
    }


    /* check also the scheduled transactions */
    tmp_list = gsb_data_scheduled_get_scheduled_list ();

    while ( tmp_list )
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (tmp_list -> data);

	if ( iface -> scheduled_div_id (scheduled_number) == no_division && 
	     ( no_sub_division == -1 ||
	       iface -> scheduled_div_id (scheduled_number) == no_sub_division ) )
	{
	    return TRUE;
	}
	else
	    tmp_list = tmp_list -> next;
    }

    return FALSE;
}




/**
 * return the the iter of a div and sub_div
 *
 * \param model
 * \param div
 * \param sub_div
 *
 * \return a newly allocated GtkTreeIter or NULL
 */
GtkTreeIter *get_iter_from_div ( GtkTreeModel * model, int div, int sub_div )
{
    gpointer pointers[3] = { GINT_TO_POINTER (div), GINT_TO_POINTER (sub_div), NULL };

    gtk_tree_model_foreach ( model, (GtkTreeModelForeachFunc) search_for_div_or_subdiv, 
			     pointers );

    return (GtkTreeIter *) pointers[2];
}


/**
 * this is a tree model foreach function
 *
 * \param model
 * \param path
 * \param iter
 * \param pointers a structure { gpointer div, gpointer sub_div, gpointer }
 * 		div and sub-div transformed by GINT_TO_POINTER
 * 
 * \return FALSE not found, TRUE found
 */
gboolean search_for_div_or_subdiv ( GtkTreeModel *model, GtkTreePath *path,
				    GtkTreeIter *iter, gpointer * pointers)
{
    int no_div, no_sub_div;
    gchar *text;

    gtk_tree_model_get ( GTK_TREE_MODEL(model), iter,
			 META_TREE_TEXT_COLUMN, &text, 
			 META_TREE_NO_DIV_COLUMN, &no_div, 
			 META_TREE_NO_SUB_DIV_COLUMN, &no_sub_div, 
			 -1 );

    /* This is a kludge because we want to skip "dummy" iters that are
     * here only to provide a slider.*/
    if ( !text )
	return FALSE;

    if ( ! pointers[0] && ( !pointers[1] || GPOINTER_TO_INT (pointers[1]) == -1) && 
	 !no_div && !no_sub_div )
    {
	pointers[2] = gtk_tree_iter_copy (iter);
	return TRUE;
    }

    if ( no_div == GPOINTER_TO_INT (pointers[0]))
    {
	if ( ( GPOINTER_TO_INT (pointers[1]) == -1 ) || 
	     ( no_sub_div == GPOINTER_TO_INT (pointers[1])))
	{
	    pointers[2] = gtk_tree_iter_copy (iter);
	    return TRUE;
	}
    }
    return FALSE;
}


/**
 * return the the iter of a transaction
 *
 * \param model
 * \param transaction_number
 *
 * \return a newly allocated GtkTreeIter or NULL
 */
GtkTreeIter * get_iter_from_transaction ( GtkTreeModel * model, gint transaction_number )
{
    gpointer pointers[2] = { GINT_TO_POINTER (transaction_number), NULL };

    gtk_tree_model_foreach ( model, (GtkTreeModelForeachFunc) search_for_transaction, pointers );

    return (GtkTreeIter *) pointers[1];
}


/**
 * this is a tree model foreach function
 *
 * \param model
 * \param path
 * \param iter
 * \param pointers a structure { gpointer transaction_number, gpointer }
 * 		transaction_number transformed by GINT_TO_POINTER
 * 
 * \return FALSE not found, TRUE found
 */
gboolean search_for_transaction ( GtkTreeModel *model, GtkTreePath *path,
				  GtkTreeIter *iter, gpointer * pointers)
{
    gint current_number;

    gtk_tree_model_get ( GTK_TREE_MODEL(model), iter,
			 META_TREE_POINTER_COLUMN, &current_number, 
			 -1 );

    if ( current_number == GPOINTER_TO_INT (pointers[0] ))
    {
	pointers[1] = gtk_tree_iter_copy (iter);
	return TRUE;
    }
    return FALSE;
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
    gint div_id, sub_div_id;
    
    if ( !transaction_number || ! metatree_model_is_displayed ( model ) )
	return;

    div_id = iface -> transaction_div_id (transaction_number);
    sub_div_id = iface -> transaction_sub_div_id (transaction_number);

    /* Fill in division if existing. */
    div_iter = get_iter_from_div ( model, div_id, -1 );
    if ( div_iter )
    {
	fill_division_row ( model, iface, div_iter, div_id );
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
	    fill_sub_division_row ( model, iface, sub_div_iter, div_id, sub_div_id );
	}
	else
	{
	    metatree_fill_new_sub_division ( iface, model, div_id, sub_div_id );
	    sub_div_iter = get_iter_from_div ( model, div_id, sub_div_id );
	}
    }

    /* Fill in transaction if existing. */
    transaction_iter = get_iter_from_transaction ( model, transaction_number );

    if ( transaction_iter )
    {
	div_path = gtk_tree_model_get_path ( model, div_iter );
	sub_div_path = gtk_tree_model_get_path ( model, sub_div_iter );
	transaction_path = gtk_tree_model_get_path ( model, transaction_iter );
	if ( ( iface -> depth != 1 &&
	       ! gtk_tree_path_is_ancestor ( sub_div_path, transaction_path ) ) ||
	     ! gtk_tree_path_is_ancestor ( div_path, transaction_path ) )
	{
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
	gchar *text;
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
 * Remove selected iter from metatree and select next iter.
 *
 * \param tree_view	Tree view that contains selection.
 * \param model		Model pertaining to tree view.
 * \param iter		Iter to remove.
 */
void metatree_remove_iter_and_select_next ( GtkTreeView * tree_view, GtkTreeModel * model,
					    GtkTreeIter * iter )
{
    GtkTreeSelection * selection;

    selection = gtk_tree_view_get_selection ( tree_view );
    if ( selection && gtk_tree_selection_get_selected ( selection, &model, iter ) )
    {
	GtkTreeIter * next = gtk_tree_iter_copy ( iter );
	GtkTreePath * path = gtk_tree_model_get_path ( model, iter );

	g_return_if_fail ( path );
	if ( ! gtk_tree_model_iter_next ( model, next ) )
	{
	    gtk_tree_path_up ( path );
	    gtk_tree_path_next ( path );
	}

	gtk_tree_store_remove ( GTK_TREE_STORE(model), iter );
	gtk_tree_selection_select_path ( selection, path );

	gtk_tree_iter_free ( next );
	g_free ( path );
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
    gint div_id, sub_div_id, current_number;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   
    tree_view = g_object_get_data ( G_OBJECT(model), "tree-view" );
    if ( !iface || !tree_view )
	return FALSE;

    if ( selection && gtk_tree_selection_get_selected ( selection, &model, &iter ) )
    {
	gchar * text, * balance = "";

	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     META_TREE_NO_DIV_COLUMN, &div_id,
			     META_TREE_NO_SUB_DIV_COLUMN, &sub_div_id,
			     META_TREE_POINTER_COLUMN, &current_number,
			     -1);

	/* if we are on a transaction, get the div_id of the transaction */
	if (div_id == -1
	    &&
	    current_number )
	{
	    div_id = iface -> transaction_div_id (current_number);
	    sub_div_id = iface -> transaction_sub_div_id (current_number);
	}

	text = g_strconcat ( _(iface -> meta_name),  " : ", 
			     (div_id ? iface -> div_name ( div_id ) : _(iface->no_div_label) ),
			     NULL );

	if ( div_id ) 
	{
	    balance = gsb_format_amount ( iface -> div_balance ( div_id ),
					  iface -> tree_currency () );
	}

	if ( sub_div_id >= 0 )
	{
	    gchar* tmpstr = text;
	    text = g_strconcat ( tmpstr, " : ", 
				 ( sub_div_id ? iface -> sub_div_name ( div_id, sub_div_id ) :
				   _(iface->no_sub_div_label) ), NULL );
            g_free ( tmpstr );
	    balance = gsb_format_amount ( iface -> sub_div_balance ( div_id, sub_div_id ),
					  iface -> tree_currency () );
	}

	gsb_gui_headings_update ( text, balance );
	g_free ( text );
	selection_is_set = TRUE;
    }

    /* Update sensitiveness of linked widgets. */
    metatree_set_linked_widgets_sensitive ( model, selection_is_set, "selection" );
    if ( ! div_id || ( sub_div_id <= 0 && current_number <= 0 ) )
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
