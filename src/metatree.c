/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2004-2008 Benjamin Drieu (bdrieu@april.org)           */
/*                      2008-2011 Pierre Biava (grisbi@pierre.biava.name)     */
/*          http://www.grisbi.org                                             */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "metatree.h"
#include "categories_onglet.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "gsb_account_property.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
#include "gsb_transactions_list.h"
#include "gtk_combofix.h"
#include "imputation_budgetaire.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "transaction_list_select.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static void button_delete_div_sub_div_clicked ( GtkWidget *togglebutton, GdkEventButton *event, GtkWidget *button );
static gboolean division_node_maybe_expand ( GtkTreeModel *model, GtkTreePath *path, 
                        GtkTreeIter *iter, gpointer data );
static void fill_division_zero ( GtkTreeModel * model,
                        MetatreeInterface * iface,
                        GtkTreeIter * iter );
static void fill_sub_division_zero ( GtkTreeModel * model,
                        MetatreeInterface * iface,
                        GtkTreeIter * iter,
                        gint division );
static void fill_transaction_row ( GtkTreeModel * model, GtkTreeIter * iter, 
                        gint transaction_number );
static gboolean find_associated_transactions ( MetatreeInterface * iface, 
                        gint no_division, gint no_sub_division );
static gboolean find_destination_blob ( MetatreeInterface * iface, GtkTreeModel * model, 
                        gint division, gint sub_division, 
                        gint * no_div, gint * no_sub_div );
static GtkTreeIter *get_iter_from_sub_div_zero ( GtkTreeModel *model,
                        MetatreeInterface *iface,
                        GtkTreeIter *parent_iter );
static GtkTreeIter * get_iter_from_transaction ( GtkTreeModel * model,
                        gint transaction_number );
static void metatree_button_action_sub_div_clicked ( GtkWidget *togglebutton,
                        GdkEventButton *event,
                        gint *pointeur );
static gint metatree_create_division_from_sub_division ( MetatreeInterface *iface,
                        gint no_division,
                        gint no_sub_division );
static void metatree_fill_new_division ( MetatreeInterface * iface, GtkTreeModel * model,
                        gint div_id );
static void metatree_fill_new_sub_division ( MetatreeInterface * iface,
                        GtkTreeModel * model,
                        gint div_id, gint sub_div_id );
static void metatree_division_set_name ( MetatreeInterface *iface,
                        gint no_division,
                        gint no_sub_division,
                        gint new_division );
static gboolean metatree_find_destination_blob ( MetatreeInterface *iface,
                        GtkTreeModel *model,
                        gint division,
                        gint sub_division,
                        gint *no_div,
                        gint *no_sub_div,
                        enum meta_tree_row_type type_division );
static gboolean metatree_get ( GtkTreeModel * model, GtkTreePath * path,
                        gint column, gint *data );
static GtkWidget *metatree_get_combofix ( MetatreeInterface *iface,
                        gint division,
                        gint sub_division,
                        enum meta_tree_row_type type_division );
static GSList *metatree_get_combofix_list ( MetatreeInterface *iface,
                        gint division,
                        gint sub_division,
                        gboolean show_sub_division );
static gboolean metatree_get_row_properties ( GtkTreeModel * tree_model, GtkTreePath * path,
                        gchar ** text, gint * no_div, gint * no_sub_div,
                        gint * no_transaction, gint * data );
static enum meta_tree_row_type metatree_get_row_type ( GtkTreeModel * tree_model,
                        GtkTreePath * path );
static gboolean metatree_model_is_displayed ( GtkTreeModel * model );
static void metatree_move_scheduled_with_div_sub_div ( MetatreeInterface *iface,
                        GtkTreeModel *model,
                        gint no_division,
                        gint no_sub_division,
                        gint new_division,
                        gint new_sub_division );
static gint metatree_move_sub_division_to_division ( MetatreeInterface *iface,
                        gint no_division,
                        gint no_sub_division,
                        gint new_division );
static void metatree_move_transactions_to_sub_div ( MetatreeInterface *iface,
                        GtkTreeModel *model,
                        gint no_division,
                        gint no_sub_division,
                        gint new_division,
                        gint new_sub_division );
static void metatree_new_sub_division ( GtkTreeModel * model, gint div_id );
static void metatree_remove_iter_and_select_next ( GtkTreeView * tree_view,
                        GtkTreeModel * model,
                        GtkTreeIter * iter );
static void metatree_reset_transactions_without_div_sub_div ( MetatreeInterface *iface,
                        GtkTreeModel *model,
                        gint no_division,
                        gint new_division,
                        gint new_sub_division );
static void metatree_sub_division_set_name ( MetatreeInterface *iface,
                        gint no_division,
                        gint no_sub_division,
                        gint new_division,
                        gint new_sub_division );
static void metatree_update_tree_view ( MetatreeInterface *iface );
static void move_all_sub_divisions_to_division ( GtkTreeModel *model,
                        gint orig_division,
                        gint dest_division );
static void move_sub_division_to_division ( GtkTreeModel *model,
                        gint no_dest_division,
                        gint no_orig_division,
                        gint no_orig_sub_division );
static void move_transaction_to_sub_division ( gint transaction_number,
                        GtkTreeModel * model,
                        GtkTreePath * orig_path, GtkTreePath * dest_path,
                        gint no_division, gint no_sub_division );
static void move_transaction_to_sub_division_zero ( gint transaction_number,
                        MetatreeInterface * iface,
                        GtkTreeModel * model, gint no_division );
static void move_transactions_to_division_payee (GtkTreeModel * model,
                        MetatreeInterface * iface,
                        gint orig_div, gint dest_div );
static gboolean search_for_div_or_subdiv ( GtkTreeModel *model, GtkTreePath *path,
                        GtkTreeIter *iter, gpointer * pointers);
static gboolean search_for_transaction ( GtkTreeModel *model, GtkTreePath *path,
                        GtkTreeIter *iter, gpointer * pointers);
static void supprimer_sub_division ( GtkTreeView * tree_view, GtkTreeModel * model,
                        MetatreeInterface * iface,
                        gint sub_division, gint division );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkTreeModel * navigation_model;
/*END_EXTERN*/

/* Save the choice for the deleting of division */
static gint button_move_selected = 0;
static gint button_action_selected = 0;

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
 * \param no_div	A pointer to an int that will be filled
 *			with the division of entry if non null (aka
 *			META_TREE_NO_DIV_COLUMN).
 * \param no_sub_div	A pointer to an int that will be filled
 *			with the sub division of entry if non null
 *			(aka META_TREE_NO_SUB_DIV_COLUMN).
 * \param no_transaction	A pointer to an int that will be filled
 *			with the sub division of entry if non null
 *			(aka META_TREE_NO_SUB_DIV_COLUMN).
 * \param data		A pointer to an int that will be filled
 *			with the pointer associated to  entry if non
 *			null (aka META_TREE_NO_SUB_DIV_COLUMN).
 *
 * \return		TRUE on success.
 */
gboolean metatree_get_row_properties ( GtkTreeModel * tree_model, GtkTreePath * path, 
                        gchar ** text, gint * no_div, gint * no_sub_div, 
                        gint * no_transaction, gint * data )
{
    GtkTreeIter iter;
    gint tmp_lvl1, tmp_lvl2, tmp_lvl3;
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
			 META_TREE_NO_TRANSACTION_COLUMN, &tmp_lvl3,
			 META_TREE_POINTER_COLUMN, &tmp_data,
			 -1);

    if ( text ) *text = tmp_text;
    if ( no_div ) *no_div = tmp_lvl1;
    if ( no_sub_div ) *no_sub_div = tmp_lvl2;
    if ( no_transaction ) *no_transaction = tmp_lvl3;
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
    gint no_div, no_sub_div, no_transaction;
  
    if ( metatree_get_row_properties ( tree_model, path, NULL, &no_div, &no_sub_div, &no_transaction, NULL ) )
    {
	if (no_div)
	{
	    if (no_sub_div)
		return META_TREE_SUB_DIV;
	    else
		return META_TREE_DIV;
	}
	else
	    if (no_transaction)
		return META_TREE_TRANSACTION;
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
 * \param division		Division structure number. (category number, payee number or budget number)
 */
void fill_division_row ( GtkTreeModel * model, MetatreeInterface * iface,
                        GtkTreeIter * iter, gint division )
{
    gchar *balance = NULL;
    gchar *string_tmp;
    GtkTreeIter dumb_iter;
    GtkTreePath * path;
    enum meta_tree_row_type type;
    gint number_transactions;

    if ( ! metatree_model_is_displayed ( model ) )
	return;

    path = gtk_tree_model_get_path ( model, iter );
    type = metatree_get_row_type ( model, path );

    gtk_tree_path_free ( path );
    if ( type != META_TREE_DIV && type != META_TREE_INVALID )
	return;

    string_tmp = iface -> div_name (division);
    number_transactions = iface -> div_nb_transactions (division);

    if (number_transactions)
    {
	gchar *label;

	label = g_strdup_printf ( "%s (%d)", string_tmp, number_transactions);
	g_free (string_tmp);
	string_tmp = label;
	balance = gsb_real_get_string_with_currency ( iface -> div_balance ( division ),
						      iface -> tree_currency (), TRUE );

	/* add a white child to show the arrow to open it */
	if ( ! gtk_tree_model_iter_has_child ( model, iter )
	     &&
	     (iface -> depth == 1 || !division ))
	    gtk_tree_store_append (GTK_TREE_STORE (model), &dumb_iter, iter );
    }

    /* set 0 for the sub-div, so no categ/no budget have 0 for div and 0 for sub-div */
    gtk_tree_store_set (GTK_TREE_STORE(model), iter,
			META_TREE_TEXT_COLUMN, string_tmp,
			META_TREE_POINTER_COLUMN, division,
			META_TREE_BALANCE_COLUMN, balance,
			META_TREE_XALIGN_COLUMN, 1.0,
			META_TREE_NO_DIV_COLUMN, division,
			META_TREE_NO_SUB_DIV_COLUMN, 0,
			META_TREE_NO_TRANSACTION_COLUMN, 0,
			META_TREE_FONT_COLUMN, 800,
			META_TREE_DATE_COLUMN, NULL,
			-1);
    g_free (string_tmp);
    if (balance)
	g_free (balance);
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
void fill_sub_division_row ( GtkTreeModel *model,
                        MetatreeInterface *iface,
                        GtkTreeIter *iter,
                        gint division,
                        gint sub_division )
{
    gchar *balance = NULL;
    gchar *string_tmp;
    GtkTreeIter dumb_iter;
    GtkTreePath * path;
    enum meta_tree_row_type type;
    gint number_transactions = 0;

    if ( ! metatree_model_is_displayed ( model ) )
	return;

    /* if no division, there is no sub division */
    if (!division)
	return;

    path = gtk_tree_model_get_path ( model, iter );
    type = metatree_get_row_type ( model, path );
    gtk_tree_path_free ( path );

    if ( type != META_TREE_SUB_DIV && type != META_TREE_INVALID )
	return;

    string_tmp = iface -> sub_div_name (division, sub_division);
    number_transactions = iface -> sub_div_nb_transactions ( division, sub_division );

    if ( number_transactions )
    {
	gchar *label;

	label = g_strdup_printf ( "%s (%d)",
				  string_tmp,
				  number_transactions );
	g_free (string_tmp);
	string_tmp = label;

	if ( ! gtk_tree_model_iter_has_child ( model, iter ) )
	    gtk_tree_store_append (GTK_TREE_STORE (model), &dumb_iter, iter );

	balance = gsb_real_get_string_with_currency ( iface -> sub_div_balance ( division, sub_division ),
						      iface -> tree_currency (), TRUE );
    }
    
    gtk_tree_store_set ( GTK_TREE_STORE (model), iter,
			 META_TREE_TEXT_COLUMN, string_tmp,
			 META_TREE_POINTER_COLUMN, sub_division,
			 META_TREE_BALANCE_COLUMN, balance,
			 META_TREE_XALIGN_COLUMN, 1.0,
			 META_TREE_NO_DIV_COLUMN, division,
			 META_TREE_NO_SUB_DIV_COLUMN, sub_division,
			 META_TREE_NO_TRANSACTION_COLUMN, 0,
			 META_TREE_FONT_COLUMN, 400,
			 META_TREE_DATE_COLUMN, NULL,
			 -1 );
    g_free (string_tmp);
    if (balance)
	g_free (balance);
}



/**
 *  fill the line transaction for each category/sub-category
 *
 * \param model		The GtkTreeModel that contains iter.
 */
void fill_transaction_row ( GtkTreeModel *model,
                        GtkTreeIter *iter,
                        gint transaction_number )
{
    gchar * account; /* no need to be freed */
    gchar * amount = NULL;
    gchar * label = NULL; 
    gchar * notes = NULL; 
    const gchar *string;
    GtkTreePath * path;
    enum meta_tree_row_type type;

    if ( ! metatree_model_is_displayed ( model ) )
	return;

    path = gtk_tree_model_get_path ( model, iter );
    type = metatree_get_row_type ( model, path );
    gtk_tree_path_free (path);
    if ( type != META_TREE_TRANSACTION && type != META_TREE_INVALID)
	return;

    string = gsb_data_transaction_get_notes ( transaction_number);

    if ( string && strlen ( string ) > 0 )
    {
        if ( strlen ( string ) > 30 )
        {
            const gchar *tmp;

            tmp = string + 30;

            tmp = strchr ( tmp, ' ' );
            if ( !tmp )
            {
                /* We do not risk splitting the string
                   in the middle of a UTF-8 accent
                   ... the end is probably near btw. */
                notes = my_strdup ( string );
            }
            else 
            {
                gchar * trunc = g_strndup ( string, ( tmp - string ) );
                notes = g_strconcat ( trunc, " ...", NULL );
                g_free ( trunc );
            }
        }
        else 
        {
            notes = my_strdup ( string );
        }
    }
    else
    {
        notes = my_strdup (gsb_data_payee_get_name (
                        gsb_data_transaction_get_party_number ( transaction_number),
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
	label = g_strconcat ( tmpstr, " (", _("split"), ")", NULL );
	g_free ( tmpstr );
    }

    amount = gsb_real_get_string_with_currency ( gsb_data_transaction_get_amount (transaction_number),
						 gsb_data_transaction_get_currency_number (transaction_number), TRUE );
    account = gsb_data_account_get_name ( gsb_data_transaction_get_account_number (transaction_number));
    gtk_tree_store_set ( GTK_TREE_STORE(model), iter, 
			 META_TREE_POINTER_COLUMN, transaction_number,
			 META_TREE_TEXT_COLUMN, label,
			 META_TREE_ACCOUNT_COLUMN, account,
			 META_TREE_BALANCE_COLUMN, amount,
			 META_TREE_NO_DIV_COLUMN, 0,
			 META_TREE_NO_SUB_DIV_COLUMN, 0,
			 META_TREE_NO_TRANSACTION_COLUMN, transaction_number,
			 META_TREE_XALIGN_COLUMN, 1.0,
			 META_TREE_FONT_COLUMN, 400,
			 META_TREE_DATE_COLUMN, gsb_data_transaction_get_date ( transaction_number ),
			 -1);
    g_free(amount);
    g_free(label);
}



/**
 * Handle request for a new division.  Normally called when user
 * clicked on the "New foo" button.
 *
 * \param model		Model to create a new division for.
 */
void metatree_new_division ( GtkTreeModel *model )
{
    MetatreeInterface * iface;
    gint div_id;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   
    g_return_if_fail ( iface );

    div_id = iface -> add_div ();

    metatree_fill_new_division ( iface, model, div_id );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}



/**
 * \todo Document this
 *
 *
 *
 */
void metatree_fill_new_division ( MetatreeInterface * iface,
                        GtkTreeModel *model,
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
void metatree_new_sub_division ( GtkTreeModel *model, gint div_id )
{
    MetatreeInterface * iface;
    gint sub_div_id;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   
    g_return_if_fail ( iface );

    sub_div_id = iface -> add_sub_div ( div_id );
    if ( !sub_div_id )
	return;

    metatree_fill_new_sub_division ( iface, model, div_id, sub_div_id );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}



/**
 * \todo Document this
 *
 *
 *
 */
void metatree_fill_new_sub_division ( MetatreeInterface *iface,
                        GtkTreeModel *model,
                        gint div_id,
                        gint sub_div_id )
{
    GtkTreeIter iter, * parent_iter;
    GtkTreeView * tree_view;
	gchar* strtmp;

    g_return_if_fail ( iface );
    if ( ! metatree_model_is_displayed ( model ) )
	return;

    strtmp = g_strdup_printf ("div : %d - sub-div : %d", div_id, sub_div_id);
    devel_debug ( strtmp  );
    g_free ( strtmp );

    parent_iter = get_iter_from_div ( model, div_id, 0 );

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
	metatree_get_row_properties ( model, path, NULL, &div_id, NULL, NULL, NULL ) ;
	if ( !div_id )
	    return;

	metatree_new_sub_division ( model, div_id );
			     
	if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

	gtk_tree_path_free ( path );
    }
}



/**
 * remove a division or a sub-division from a metatree
 *
 * \param tree_view
 *
 * \return FALSE
 */
gboolean supprimer_division ( GtkTreeView * tree_view )
{
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeIter iter;
    GtkTreePath * path;
    gint no_division = 0, no_sub_division = 0;
    gint current_number = 0;
    gint nbre_trans_s_s_div = 0; /* nbre de transactions sans sous-division */

    MetatreeInterface * iface;

    devel_debug (NULL);

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

    /* get the type of the delete */
    path = gtk_tree_model_get_path ( model, &iter );
    switch ( metatree_get_row_type ( model, path ) )
    {
	case META_TREE_TRANSACTION:
	    metatree_remove_transaction ( tree_view, iface, current_number, TRUE );
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

    /* ok, now we know that we really want to delete a division */
    /* is the division contains some transactions ? */
    if ( find_associated_transactions ( iface, no_division, 0 ) )
    {
        gint new_division, new_sub_division;

        /* some transactions have that division, we ask to move them
         * to another division or juste erase the division, and it
         * will become blanck for the transactions */
        
        /* fill new_division and new_sub_division (nota = 0)*/
        if ( ! find_destination_blob ( iface, model, no_division, 0,
                           &new_division, &new_sub_division ) )
            return FALSE;

        /* now we have new_division filled, new_sub_division is always 0 because
         * here we remove only a division
         * so only division choice to move the transactions
         * to remove a sub-division, go to see supprimer_sub_division */

        /* two ways now :
         * either we have no div, so we just remove the division and all the
         * sub-divisions and set the transactions to no division
         * either we have a new div, we have to move the sub-div of the
         * last div to the new div, and move the transactions too */

        if (new_division)
        {
            /* there is a new division, we have to move the sub-division to another division */
            move_all_sub_divisions_to_division (model, no_division, new_division);

            /* on regarde s'il y a des transactions sans sous-division */
            nbre_trans_s_s_div = iface -> sub_div_nb_transactions ( no_division, 0 );
            if ( nbre_trans_s_s_div > 0 )
                metatree_move_transactions_to_sub_div ( iface, model,
                        no_division, no_sub_division, new_division, new_sub_division );
        }
        else
        {
            /* there is no new div so juste set the transactions to 0 */
            metatree_reset_transactions_without_div_sub_div ( iface, model, no_division,
                            new_division, new_sub_division );
        }

        /* fait le tour des échéances pour mettre le nouveau numéro de division et sub_division  */
        metatree_move_scheduled_with_div_sub_div ( iface, model,
                            no_division, no_sub_division,
                            new_division, new_sub_division );

        /* update value in the tree view */
        metatree_update_tree_view ( iface );

        if ( etat.modification_fichier == 0 )
            modification_fichier ( TRUE );
    }

    /* supprime dans la liste des division  */
    iface -> remove_div ( no_division ); 
    metatree_remove_iter_and_select_next ( tree_view, model, &iter );

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
    GtkTreeIter iter, *parent_iter, * it;
    GtkTreePath *path;

    devel_debug (NULL);

    if ( find_associated_transactions ( iface, division, 
                        sub_division ) )
    {
	gint nouveau_no_division, nouveau_no_sub_division;
	GSList *list_tmp_transactions;
    GSList *list_num = NULL; /* mémorise les transactions à transferer */

	/* fill nouveau_no_division and nouveau_no_sub_division */
	if ( ! find_destination_blob ( iface, model, division, sub_division, 
                        &nouveau_no_division, &nouveau_no_sub_division ) )
	    return;

	/* move the transactions to the new division numbers, need to do for 
     * archived transactions too */
	list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (
                        list_tmp_transactions -> data);

	    if ( ( iface -> transaction_div_id (transaction_number_tmp) == division)
		 &&
		 ( iface -> transaction_sub_div_id (transaction_number_tmp) == sub_division))
	    {
		iface -> add_transaction_to_sub_div ( transaction_number_tmp, 
                        nouveau_no_division,
                        nouveau_no_sub_division );
		iface -> transaction_set_div_id (transaction_number_tmp, nouveau_no_division);
		iface -> transaction_set_sub_div_id (transaction_number_tmp, nouveau_no_sub_division);
        list_num = g_slist_append ( list_num, GINT_TO_POINTER (
                        transaction_number_tmp) );
        }
	    list_tmp_transactions = list_tmp_transactions -> next;
	}

        /* fait le tour des échéances pour mettre le nouveau numéro de division et sub_division  */
        metatree_move_scheduled_with_div_sub_div ( iface, model,
                            division, sub_division,
                            nouveau_no_division, nouveau_no_sub_division );
	/* Fill sub division */
    if ( nouveau_no_division && nouveau_no_sub_division == 0 )
    {
        parent_iter = get_iter_from_div ( model, nouveau_no_division, 0 );
        it = get_iter_from_sub_div_zero ( model, iface, parent_iter );
    }
    else
        it = get_iter_from_div ( model, nouveau_no_division, nouveau_no_sub_division );

	if ( it )
    {
        GtkTreeIter child_iter;
        gint transaction_number;

        if ( nouveau_no_division && nouveau_no_sub_division == 0 )
            fill_sub_division_zero ( model, iface, it,nouveau_no_division );
        else
            fill_sub_division_row ( model, iface, it,
                            nouveau_no_division,
                            nouveau_no_sub_division );

        path = gtk_tree_model_get_path ( model, it );
        if ( gtk_tree_view_row_expanded ( tree_view, path ) )
        {
            while ( list_num )
            {
                transaction_number = GPOINTER_TO_INT ( list_num -> data );
                gtk_tree_store_append ( GTK_TREE_STORE(model), &child_iter, it );
                fill_transaction_row ( model, &child_iter, transaction_number);
                list_num = list_num -> next;
            }
            g_slist_free ( list_num );
            gtk_tree_path_free ( path );
        }
    }

        /* Fill division as well */
        it = get_iter_from_div ( model, nouveau_no_division, 0 );
        if ( it )
            fill_division_row ( model, iface, it, nouveau_no_division );

        /* update value in the tree view */
        metatree_update_tree_view ( iface );
    }

    /* supprime dans la liste des divisions  */
    iface -> remove_sub_div (division, sub_division);

    metatree_remove_iter_and_select_next ( tree_view, model, &iter );

    /* Fill old division */
	it = get_iter_from_div ( model, division, 0 );
	if ( it )
	    fill_division_row ( model, iface, it, division );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}



/**
 * Remove a transaction from a metatree
 * if delete_transaction is TRUE, will delete the transaction
 * 	before removing it from the metatree
 * for now, the transaction MUST NOT be deleted at this call
 * 	(if we don't want that function deletes the transaction)
 *
 * \param tree_view
 * \param iface the metatree interface
 * \param transaction the transaction number we want to remove
 * \param sub_division the parent sub-division number
 * \param division the division number (parent)
 *
 * \return
 */
void metatree_remove_transaction ( GtkTreeView * tree_view,
                        MetatreeInterface * iface,
                        gint transaction,
                        gboolean delete_transaction )
{
    GtkTreeModel *model;
    gint division, sub_division;
    GtkTreeIter iter, *it;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    division = iface -> transaction_div_id ( transaction );
    sub_division = iface -> transaction_sub_div_id ( transaction );

    /* remove the transaction from memory and list, and show the warnings */
    if (delete_transaction)
	if (!gsb_transactions_list_delete_transaction (transaction, TRUE))
	    return;

    /* Fill parent sub division */
    it = get_iter_from_div ( model, division, sub_division );
    if ( it )
	fill_sub_division_row ( model, iface, it, division, sub_division );

    /* Fill division as well */
    it = get_iter_from_div ( model, division, 0 );
    if ( it )
	fill_division_row ( model, iface, it, division );
    
    metatree_remove_iter_and_select_next ( tree_view, model, &iter );
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}



/**
 * callback when expand a row
 *
 * \param treeview
 * \param iter
 * \param tree_path
 * \param user_data not used
 *
 * \return FALSE
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
    if ( etat.metatree_sort_transactions )
        list_tmp_transactions = gsb_data_transaction_get_transactions_list_by_date ();
    else if ( etat.add_archive_in_total_balance )
        list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
    else
        list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (
                        list_tmp_transactions -> data);

	    /* set the transaction if the same div/sub-div
	     * or if no categ (must check if no transfer or split) */
	    if ( transaction_number_tmp &&
		 ( (iface -> transaction_div_id ( transaction_number_tmp) == no_division &&
		    iface -> transaction_sub_div_id ( transaction_number_tmp) == no_sub_division )
		   ||
		   ( !no_division &&
		     !iface -> transaction_div_id ( transaction_number_tmp) &&
		     !gsb_data_transaction_get_split_of_transaction (transaction_number_tmp) &&
		     gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp) == 0)))

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
	if ( transaction_number && !no_division && !no_sub_division )
	{
        /* If transaction is an archive return */
        if ( gsb_data_transaction_get_archive_number ( transaction_number ) )
        {
            dialogue_warning ( _("This transaction is archived.\n\n"
                        "You must view the transactions in this archive for access.") );
            return FALSE;
        }
	    /* If transaction is reconciled, show reconciled
	     * transactions. */
	    if ( gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE &&
		 !gsb_data_account_get_r (gsb_gui_navigation_get_current_account ()))
	    {
		mise_a_jour_affichage_r ( TRUE );
	    }

	    navigation_change_account ( GINT_TO_POINTER ( gsb_data_transaction_get_account_number (transaction_number)));
	    gsb_account_property_fill_page ();
	    gsb_gui_notebook_change_page ( GSB_ACCOUNT_PAGE );
	    gsb_gui_navigation_set_selection ( GSB_ACCOUNT_PAGE, 
					       gsb_data_transaction_get_account_number (transaction_number), 
					       NULL );

	    transaction_list_select ( transaction_number );
	}
    }

    return FALSE;
}



/**
 * \todo Document this
 *
 *
 */
gboolean division_row_drop_possible ( GtkTreeDragDest *drag_dest,
                        GtkTreePath *dest_path,
                        GtkSelectionData *selection_data )
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
 * callback when a metatree receive a drag and drop signal
 *
 * \param drag_dest
 * \param dest_path
 * \param selection_data
 *
 * \return FALSE
 */
gboolean division_drag_data_received ( GtkTreeDragDest *drag_dest,
                        GtkTreePath *dest_path,
                        GtkSelectionData *selection_data )
{
    gchar *tmpstr = gtk_tree_path_to_string (dest_path);
    gchar *tmpstr2 = g_strdup_printf ( "Dest path : %s", tmpstr);
    devel_debug (tmpstr2);
    g_free (tmpstr);
    g_free (tmpstr2);

    if ( dest_path && selection_data )
    {
	GtkTreeModel * model;
	GtkTreePath * orig_path;
	gchar * name;
	gint no_dest_division, no_dest_sub_division, no_orig_division, no_orig_sub_division;
	enum meta_tree_row_type orig_type;
	MetatreeInterface * iface;
	gint transaction_number;

	/* get the orig_path */
	gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);

	/* if we are on the navigation list, work with it */
	if ( model == GTK_TREE_MODEL(navigation_model) )
	{
	    return navigation_drag_data_received ( drag_dest, dest_path, selection_data );
	}

	/* get metatree interface */
	iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );
	if ( ! iface )
	    return FALSE;

	metatree_get_row_properties ( model, orig_path,
				      NULL, &no_orig_division, &no_orig_sub_division, 
				      &transaction_number, NULL );

	/* get the type of row (div, sub-div, transaction) */
	orig_type = metatree_get_row_type ( model, orig_path );

	/* get the destination param */
	metatree_get_row_properties ( model, dest_path,
				      &name, &no_dest_division, &no_dest_sub_division, 
				      NULL, NULL );
	if ( ! name )
	{
	    gtk_tree_path_up ( dest_path );
	    metatree_get_row_properties ( model, dest_path,
					  &name, &no_dest_division, &no_dest_sub_division, 
					  NULL, NULL );
	}
	
	switch ( orig_type )
	{
		/* move a transaction */
	    case META_TREE_TRANSACTION:
		if (transaction_number)
		    move_transaction_to_sub_division ( transaction_number, model, 
						       orig_path, dest_path,
						       no_dest_division, no_dest_sub_division );
		break;

		/* move a sub-division */
	    case META_TREE_SUB_DIV:
		move_sub_division_to_division (model,
					       no_dest_division,
					       no_orig_division, no_orig_sub_division );
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
 * \param no_sub_division new sub-division to the transaction or 0 for no sub-division
 *
 * \return
 */
void move_transaction_to_sub_division ( gint transaction_number,
                        GtkTreeModel *model,
                        GtkTreePath *orig_path,
                        GtkTreePath *dest_path,
                        gint no_division,
                        gint no_sub_division )
{
    GtkTreeIter orig_iter, child_iter, dest_iter, parent_iter, gd_parent_iter;
    MetatreeInterface * iface;
    gint old_div, old_sub_div;

    if ( !model )
        return;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );

    /* Insert new row */
    if ( dest_path )
        gtk_tree_model_get_iter ( model, &dest_iter, dest_path );
    else
    {
        GtkTreeIter * p_iter = get_iter_from_div ( model,
                        no_division, no_sub_division );
        if ( p_iter )
            dest_iter = *p_iter;
    }

    /* Avoid filling "empty" not yet selected subdivisions */
    if ( gtk_tree_model_iter_children ( model, &child_iter, &dest_iter ) )
    {
        gchar * name;

        gtk_tree_model_get ( model, &child_iter,
                        META_TREE_TEXT_COLUMN, &name, -1 );
        if ( name )
            gtk_tree_store_insert ( GTK_TREE_STORE (model),
                        &child_iter, &dest_iter, 0 );
    }
    else
        gtk_tree_store_append ( GTK_TREE_STORE (model),
                        &child_iter, &dest_iter );

    /* get the old div */
    old_div = iface -> transaction_div_id (transaction_number);
    old_sub_div = iface -> transaction_sub_div_id (transaction_number);

    /* Update old parents */
    iface -> remove_transaction_from_sub_div ( transaction_number );

    /* Change parameters of the transaction */
    iface -> transaction_set_div_id ( transaction_number, no_division );
    iface -> transaction_set_sub_div_id ( transaction_number, no_sub_division );
    gsb_transactions_list_update_transaction (transaction_number);

    /* met à jour la transaction dans la liste corrige bug d'affichage */
    fill_transaction_row ( model, &child_iter, transaction_number);

    /* Update new parents */
    if ( iface -> depth > 1 )
    {
        iface -> add_transaction_to_sub_div ( transaction_number,
                        no_division, no_sub_division );
        if ( no_sub_division == 0 )
            fill_sub_division_zero ( model, iface, &dest_iter, 
                        no_division );
        else
            fill_sub_division_row ( model, iface, &dest_iter,
                        no_division, no_sub_division );
        if ( gtk_tree_model_iter_parent ( model, &parent_iter, &dest_iter ) )
            fill_division_row ( model, iface, &parent_iter, no_division );
    }
    else
    {
	iface -> add_transaction_to_div ( transaction_number, no_division );
	fill_division_row ( model, iface, &dest_iter, no_division );
    }

    /* update the old parent division and sub-division */
    if ( orig_path
	 &&
	 gtk_tree_model_get_iter ( model, &orig_iter, orig_path ) )
    {
        if ( gtk_tree_model_iter_parent ( model, &parent_iter, &orig_iter ) )
        {
            if ( iface -> depth > 1 )
            {
                if ( old_div == 0 )
                    fill_division_zero ( model, iface, &parent_iter );
                else if ( old_sub_div == 0 )
                    fill_sub_division_zero ( model, iface, &parent_iter, 
                        old_div );
                else
                    fill_sub_division_row ( model, iface, &parent_iter, 
                        old_div, old_sub_div );
                if ( gtk_tree_model_iter_parent ( model, &gd_parent_iter, 
                        &parent_iter ) )
                    fill_division_row ( model, iface, &gd_parent_iter, 
                        old_div );
            }
            else
                fill_division_row ( model, iface, &parent_iter, old_div );
        }
        /* Remove old row */
        gtk_tree_store_remove ( GTK_TREE_STORE (model), &orig_iter );
    }

    /* We did some modifications */
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}

/**
 * move a sub-division to a new division
 *
 * \param model
 * \param no_dest_division number of the division where we want to move the sub-division
 * \param no_orig_division number of the initial division containing the sub-division we want to move
 * \param no_orig_sub_division number of the initial sub-division we want to move to another division or 0 if no sub-division
 *
 * \return	
 */
void move_sub_division_to_division ( GtkTreeModel *model,
                        gint no_dest_division,
                        gint no_orig_division,
                        gint no_orig_sub_division )
{
    GtkTreeIter iter, orig_parent_iter;
    GtkTreeIter *orig_iter, *iter_parent;
    MetatreeInterface * iface;
    gint no_dest_sub_division = 0;
    GSList *list_tmp_transactions;

    if ( !model )
	return;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );

    /* create the new sub-division in memory with the same name of the origin sub-division */
    if (no_orig_sub_division)
	no_dest_sub_division = iface -> get_sub_div_pointer_from_name ( no_dest_division,
									iface -> sub_div_name (no_orig_division, no_orig_sub_division),
									1 );

    /* if no_orig_sub_division is 0, we are on payee tree, so the transactions are
     * directly added to the division */
    iter_parent = get_iter_from_div (model, no_dest_division, 0);

    if (no_orig_sub_division)
    {
	/* there is a sub-division, append a new one to the new division
	 * to add the transactions */
	gtk_tree_store_append ( GTK_TREE_STORE(model), &iter, iter_parent);
    }
    else
    {
	/* we are on payee tree, just set the iter on the new division to add the transactions */
	iter = *iter_parent;
    }

    /* fill the new sub-division (or dest division for payee) with the transactions */
    list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

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

    /* set orig_iter to the initial sub-division */
    orig_iter = get_iter_from_div ( model, no_orig_division, no_orig_sub_division );

    /* Update original parent. */
    if ( gtk_tree_model_iter_parent ( model, &orig_parent_iter, orig_iter ) )
    {
	fill_division_row ( model, iface, &orig_parent_iter, no_orig_division );
    }

    /* Remove original division. */
    iface -> remove_sub_div ( no_orig_division, no_orig_sub_division );
    gtk_tree_store_remove ( GTK_TREE_STORE(model), orig_iter );
    gtk_tree_iter_free (orig_iter);

    /* If it was no sub-division, recreate it. */
    if ( !no_orig_sub_division )
    {
	metatree_fill_new_sub_division ( iface, model, 
					 no_orig_division, no_orig_sub_division );
    }

    /* Update dest at last. */
    fill_sub_division_row ( model, iface, &iter, 
			    no_dest_division, no_dest_sub_division );
    gtk_tree_iter_free (iter_parent);
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}

/**
 * get all the sub-divisions of the orig_division and move them to the dest_division
 * move only the sub-divisions with some transactions
 *
 * \param model
 * \param orig_division
 * \param dest_division
 *
 * \return
 * */
void move_all_sub_divisions_to_division ( GtkTreeModel *model,
                        gint orig_division,
                        gint dest_division )
{
    MetatreeInterface *iface;
    GSList *sub_div_list;
    GSList *tmp_list;

    if (!model)
	return;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );

    /* the easyest way first, if we have no sub-division (ie we are on payee),
     * directly move the sub-division (transactions) the the new division */
    if (!iface -> content)
    {
        move_transactions_to_division_payee (model, iface,
                        orig_division, dest_division );
        return;
    }

    /* if we come here, we are on metatree with sub-division */
    sub_div_list = iface -> div_sub_div_list (orig_division);
    tmp_list = sub_div_list;
    while (tmp_list)
    {
        gint orig_sub_division;
        GtkTreeIter *iter;

        orig_sub_division = iface -> sub_div_id (tmp_list -> data);

        /* check if transactions in the sub-division, move only when there 
         * is some transactions  */
        iter = get_iter_from_div (model, orig_division, orig_sub_division);

        /* go to the next before erase it */
        tmp_list = tmp_list -> next;

        if (gtk_tree_model_iter_has_child (model, iter))
            move_sub_division_to_division (model, dest_division,
                           orig_division, orig_sub_division );
        gtk_tree_iter_free (iter);
    }
}


/**
 * \todo Document this
 *
 *
 */
gboolean division_node_maybe_expand ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    GtkTreeView * tree_view = g_object_get_data ( G_OBJECT(model), "tree-view" );

    if ( tree_view )
    {
	if ( GPOINTER_TO_INT( data ) == gtk_tree_path_get_depth ( path ) )
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
	gtk_tree_model_foreach ( model, division_node_maybe_expand, GINT_TO_POINTER ( depth ) );
    }
}



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
 * 	no_div will contain the new division, no_sub_div the new sub-division
 * 	if no move of transactions, no_div and no_sub_div are set to 0
 * 	if move to another division but no sub-division, no_sub_div is set to 0
 * */
gboolean find_destination_blob ( MetatreeInterface * iface,
                        GtkTreeModel * model,
                        gint division,
                        gint sub_division,
                        gint *no_div,
                        gint *no_sub_div )
{
    GtkWidget *dialog, *hbox, *button_delete, *combofix, *button_move;
    GSList *liste_combofix, *division_list, *liste_division_credit, *liste_division_debit;
    gint resultat, nouveau_no_division, nouveau_no_sub_division;
    gchar **split_division;
    gchar *tmpstr;

    /* create the box to move change the division and sub-div of the transactions */
    gchar* tmpstr1 = g_strdup_printf ( _("'%s' still contains transactions or archived transactions."), 
				       ( !sub_division ? 
					 iface -> div_name ( division ) :
					 iface -> sub_div_name ( division, sub_division ) ) );
    gchar* tmpstr2 = g_strdup_printf ( _("If you want to remove it but want to keep transactions, you can transfer them to another (sub-)%s.  Otherwise, transactions can be simply deleted along with their division."), 
				       _( iface -> meta_name ) );
    dialog = dialogue_special_no_run ( GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
				       make_hint ( tmpstr1 , tmpstr2 ) );

    g_free ( tmpstr1 );
    g_free ( tmpstr2 );

    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox,
			 FALSE, FALSE, 0 );

    if ( iface -> content == 0 )
        tmpstr1 = g_strdup_printf ( _("Transfer transactions to payee") );
    else
        tmpstr1 = g_strdup_printf (_("Transfer transactions to %s"), _(iface -> meta_name));

    button_move = gtk_radio_button_new_with_label ( NULL, tmpstr1);
    g_free ( tmpstr1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_move,
			 FALSE, FALSE, 0 );

    /* create the list containing division and sub-division
     * without the current division to delete */
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

	/* we add the sub-divisions only if we remove the sub-divisions,
	 * else we let only the division */
	if (!sub_division)
	{
	    division_list = division_list -> next;
	    continue;
	}

	/* get the sub-divisions */
	sub_division_list = iface -> div_sub_div_list (tmp_division);

	while ( sub_division_list )
	{
	    gint tmp_sub_division;

	    tmp_sub_division = iface -> sub_div_id (sub_division_list -> data);
	    if (division == tmp_division && tmp_sub_division == sub_division)
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

    /* create the combofix complex with the divisions and sub-divisions */
    liste_combofix = g_slist_append ( liste_combofix, liste_division_debit );
    liste_combofix = g_slist_append ( liste_combofix, liste_division_credit );

    combofix = gtk_combofix_new ( liste_combofix );
    gtk_combofix_set_force_text ( GTK_COMBOFIX (combofix), TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), combofix, TRUE, TRUE, 0 );

    /* other choice, just remove the division */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox,
			 FALSE, FALSE, 0 );

    if ( iface -> content == 0 )
        tmpstr = g_strdup_printf( _("Just remove this payee.") );
    else if ( !sub_division )
	    tmpstr = g_strdup_printf(_("Just remove this %s."), _(iface -> meta_name) );
    else
	tmpstr = g_strdup_printf(_("Just remove this sub-%s."), _(iface -> meta_name) );

    button_delete = gtk_radio_button_new_with_label (
                        gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( button_move ) ),
                        tmpstr );
    g_free ( tmpstr );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_delete, FALSE, FALSE, 0 );

    if ( button_move_selected == 1 )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_delete ), TRUE );

    /* set the signals */
    g_signal_connect ( G_OBJECT ( button_move ),
                        "button-release-event",
                        G_CALLBACK ( button_delete_div_sub_div_clicked ),
                        NULL );

    g_signal_connect ( G_OBJECT ( button_delete ),
                        "button-release-event",
                        G_CALLBACK ( button_delete_div_sub_div_clicked ),
                        NULL );

    gtk_widget_show_all ( dialog );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    if ( resultat != GTK_RESPONSE_OK )
    {
        gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
        return FALSE;
    }

    nouveau_no_division = 0;
    nouveau_no_sub_division = 0;

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_move )) )
    {
	/* we want to move the transactions */
	if ( !strlen (gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ))))
	{
	    gchar* tmpstr1 = g_strdup_printf (
                        _("It is compulsory to specify a destination %s "
                        "to move transactions but no %s was entered."),
                        _(iface -> meta_name), _(iface -> meta_name) );
	    gchar* tmpstr2 = g_strdup_printf ( _("Please enter a %s!"),
                        _(iface -> meta_name) );
	    dialogue_warning_hint ( tmpstr1 , tmpstr2 );
	    g_free ( tmpstr1 );
	    g_free ( tmpstr2 );

	    gtk_widget_destroy (dialog);
	    return (find_destination_blob ( iface, model, 
					    division, sub_division, 
					    no_div, no_sub_div ));
	}

	/* get the new (sub-)divisions */
	split_division = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
				      " : ", 2 );

	nouveau_no_division = iface -> get_div_pointer_from_name ( split_division[0], 0 );

	if (nouveau_no_division)
	    nouveau_no_sub_division =  iface -> get_sub_div_pointer_from_name ( 
                        nouveau_no_division, split_division[1], 0 );

	g_strfreev ( split_division );
    }

    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    /* return the new number of division and sub-division
     * if don't want to move the transactions, 0 and 0 will be returned */
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
 * \param no_sub_division Subdivision id to search for, or 0 to search
 *			only on division.
 *
 * \return TRUE if transactions are associated.
 *
 * \todo Return number of transactions instead, to produce nicer
 * dialogs.
 */
gboolean find_associated_transactions ( MetatreeInterface * iface,
                        gint no_division,
                        gint no_sub_division )
{
    GSList *tmp_list;

    /* we need to check all the transactions, even in archives */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();

    while ( tmp_list )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	if ( iface -> transaction_div_id (transaction_number_tmp) == no_division &&
	     ( !no_sub_division ||
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
	     ( !no_sub_division ||
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
 * \param sub_div 0 if just div
 *
 * \return a newly allocated GtkTreeIter or NULL
 */
GtkTreeIter *get_iter_from_div ( GtkTreeModel * model, int div, int sub_div )
{
    gpointer pointeurs[3] = { GINT_TO_POINTER (div), GINT_TO_POINTER (sub_div), NULL };

    gtk_tree_model_foreach ( model, (GtkTreeModelForeachFunc) search_for_div_or_subdiv, 
			     pointeurs );

    return (GtkTreeIter *) pointeurs[2];
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
gboolean search_for_div_or_subdiv ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer *pointers )
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

    if ( ! pointers[0] && ( !pointers[1] || !GPOINTER_TO_INT (pointers[1]))
	 && 
	 !no_div && !no_sub_div )
    {
	pointers[2] = gtk_tree_iter_copy (iter);
	return TRUE;
    }

    if ( no_div == GPOINTER_TO_INT (pointers[0]))
    {
	if ( ( !GPOINTER_TO_INT (pointers[1])) || 
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
GtkTreeIter *get_iter_from_transaction ( GtkTreeModel * model, 
                        gint transaction_number )
{
    gpointer pointeurs[2] = { GINT_TO_POINTER (transaction_number), NULL };

    gtk_tree_model_foreach ( model, (GtkTreeModelForeachFunc) 
                        search_for_transaction, pointeurs );

    return (GtkTreeIter *) pointeurs[1];
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
gboolean search_for_transaction ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer *pointers )
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
void update_transaction_in_tree ( MetatreeInterface * iface,
                        GtkTreeModel * model,
                        gint transaction_number )
{
    GtkTreeIter *transaction_iter;
    GtkTreeIter *div_iter;
    GtkTreeIter *sub_div_iter = NULL;
    GtkTreePath *transaction_path;
    GtkTreePath *div_path;
    GtkTreePath *sub_div_path = NULL;
    gint div_id;
    gint sub_div_id;

    if ( !transaction_number || !metatree_model_is_displayed ( model ) )
        return;

    div_id = iface -> transaction_div_id (transaction_number);
    sub_div_id = iface -> transaction_sub_div_id (transaction_number);

    /* Fill in division if existing. */
    div_iter = get_iter_from_div ( model, div_id, 0 );
    if ( div_iter )
    {
        fill_division_row ( model, iface, div_iter, div_id );
    }
    else
    {
        metatree_fill_new_division ( iface, model, div_id );
        div_iter = get_iter_from_div ( model, div_id, 0 );
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
        if ( sub_div_iter )
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

	if ( ! gtk_tree_model_iter_children ( model, &child_iter, 
                        ( iface -> depth == 1 ? div_iter : sub_div_iter ) ) )
	    /* Panic, something went wrong. */
	    return;
	
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
        fill_transaction_row ( model, transaction_iter, transaction_number );
}



/**
 * Remove selected iter from metatree and select next iter.
 *
 * \param tree_view	Tree view that contains selection.
 * \param model		Model pertaining to tree view.
 * \param iter		Iter to remove.
 */
void metatree_remove_iter_and_select_next ( GtkTreeView * tree_view, 
                        GtkTreeModel * model,
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
	gtk_tree_path_free ( path );
    }    
}



/**
 * Performs actions needed when selection of a metatree has changed.
 * First, update the headings bar accordingly.  Then update
 * sensitiveness of linked widgets.
 *
 * \return TRUE
 */
gboolean metatree_selection_changed ( GtkTreeSelection *selection, GtkTreeModel *model )
{
    MetatreeInterface *iface;
    GtkTreeView *tree_view;
    GtkTreeIter iter;
    GtkTreePath *path;
    gboolean selection_is_set = FALSE;
    gint div_id = 0, sub_div_id = 0, current_number = 0;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   
    tree_view = g_object_get_data ( G_OBJECT(model), "tree-view" );
    if ( !iface || !tree_view || ! model)
	return FALSE;

    if ( selection && gtk_tree_selection_get_selected ( selection, &model, &iter ) )
    {
	gchar *text, *balance = "";

	if ( !model)
	    return FALSE;

	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     META_TREE_NO_DIV_COLUMN, &div_id,
			     META_TREE_NO_SUB_DIV_COLUMN, &sub_div_id,
			     META_TREE_POINTER_COLUMN, &current_number,
			     -1);

    /* save the new_position */
    path = gtk_tree_model_get_path ( model, &iter );
    gtk_tree_path_to_string ( path);
    iface -> hold_position_set_path ( path );

	/* if we are on a transaction, get the div_id of the transaction */
	if (!div_id
	    &&
	    current_number )
	{
	    div_id = iface -> transaction_div_id (current_number);
	    sub_div_id = iface -> transaction_sub_div_id (current_number);
        metatree_set_linked_widgets_sensitive ( model, FALSE, "selection" );
        /* save the new expand */
        iface -> hold_position_set_expand ( TRUE );
	}
    else
    {
        metatree_set_linked_widgets_sensitive ( model, TRUE, "selection" );
        iface -> hold_position_set_expand ( FALSE );
    }
	text = g_strconcat ( _(iface -> meta_name),  " : ", 
			     (div_id ? iface -> div_name ( div_id ) : _(iface->no_div_label) ),
			     NULL );

	if ( div_id ) 
	{
	    balance = gsb_real_get_string_with_currency ( iface -> div_balance ( div_id ),
							  iface -> tree_currency (), TRUE );
	}

	if ( sub_div_id >= 0 )
	{
	    gchar* tmpstr = text;
	    text = g_strconcat ( tmpstr, " : ", 
				 ( sub_div_id ? iface -> sub_div_name ( div_id, sub_div_id ) :
				   _(iface->no_sub_div_label) ), NULL );
            g_free ( tmpstr );
	    balance = gsb_real_get_string_with_currency ( iface -> sub_div_balance ( div_id, sub_div_id ),
							  iface -> tree_currency (), TRUE );
	}

	gsb_gui_headings_update_title ( text );
    gsb_gui_headings_update_suffix ( balance );
	g_free ( text );
	selection_is_set = TRUE;
    }

    if ( ! model )
	return FALSE;

    /* Update sensitiveness of linked widgets. */
    /* metatree_set_linked_widgets_sensitive ( model, selection_is_set, "selection" ); */
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
void metatree_register_widget_as_linked ( GtkTreeModel * model,
                        GtkWidget * widget,
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
void metatree_set_linked_widgets_sensitive ( GtkTreeModel * model,
                        gboolean sensitive,
                        gchar * link_type )
{
    GSList * links = g_object_get_data ( G_OBJECT(model), link_type );

    while ( links )
    {
        if ( links -> data && GTK_IS_WIDGET ( links -> data ) )
        {
            gtk_widget_set_sensitive ( GTK_WIDGET (
                        links -> data ), sensitive );
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
gboolean metatree_sort_column  ( GtkTreeModel * model,
                        GtkTreeIter * a, GtkTreeIter * b,
                        gpointer user_data )
{
    GDate * date_a = NULL, * date_b = NULL;
    gchar * string_a, * string_b;
    gint no_div_a, no_sous_div_a, no_div_b, no_sous_div_b;

    gtk_tree_model_get ( model, a, 
			 META_TREE_DATE_COLUMN, &date_a, 
			 META_TREE_TEXT_COLUMN, &string_a,
             META_TREE_NO_DIV_COLUMN, &no_div_a,
			 META_TREE_NO_SUB_DIV_COLUMN, &no_sous_div_a, -1 );
    gtk_tree_model_get ( model, b, 
			 META_TREE_DATE_COLUMN, &date_b, 
			 META_TREE_TEXT_COLUMN, &string_b,
             META_TREE_NO_DIV_COLUMN, &no_div_b,
			 META_TREE_NO_SUB_DIV_COLUMN, &no_sous_div_b,  -1 );

    /* on affiche en premier les opérations sans div sous_division */
    if ( no_div_a == 0 )
        return -1;
    if ( no_div_b == 0 )
        return 1;
    if ( no_div_a == no_div_b && no_sous_div_a == 0 )
        return -1;
    if ( no_div_a == no_div_b && no_sous_div_b == 0 )
        return 1;
    
    if (!string_b)
        return 1;
    if (!string_a)
        return -1;

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


/**
 * Fill an empty division row with textual representation of a division
 * structure, in the form: "Name (num transactions) Balance".
 *
 * \param model		The GtkTreeModel that contains iter.
 * \param iface		A pointer to the metatree interface to use
 * \param iter		Iter to fill with division data.
 * \param division		Division structure number. (category number, payee number or budget number)
 */
void fill_division_zero ( GtkTreeModel * model,
                        MetatreeInterface * iface,
                        GtkTreeIter * iter )
{
    gchar *balance = NULL;
    gchar *string_tmp;
    GtkTreeIter dumb_iter;
    gint number_transactions;

    devel_debug ( NULL );
    if ( ! metatree_model_is_displayed ( model ) )
	return;

    string_tmp = iface -> div_name ( 0 );
    number_transactions = iface -> div_nb_transactions ( 0 );

    if (number_transactions)
    {
	gchar *label;

	label = g_strdup_printf ( "%s (%d)",
				  string_tmp,
				  number_transactions);
	g_free (string_tmp);
	string_tmp = label;

	balance = gsb_real_get_string_with_currency ( iface -> div_balance ( 0 ),
						      iface -> tree_currency (), TRUE );

	/* add a white child to show the arrow to open it */
	if ( ! gtk_tree_model_iter_has_child ( model, iter )
	     &&
	     (iface -> depth == 1 ))
	    gtk_tree_store_append (GTK_TREE_STORE (model), &dumb_iter, iter );
    }

    /* set 0 for the sub-div, so no categ/no budget have 0 for div and 0 for sub-div */
    gtk_tree_store_set (GTK_TREE_STORE(model), iter,
			META_TREE_TEXT_COLUMN, string_tmp,
			META_TREE_POINTER_COLUMN, 0,
			META_TREE_BALANCE_COLUMN, balance,
			META_TREE_XALIGN_COLUMN, 1.0,
			META_TREE_NO_DIV_COLUMN, 0,
			META_TREE_NO_SUB_DIV_COLUMN, 0,
			META_TREE_NO_TRANSACTION_COLUMN, 0,
			META_TREE_FONT_COLUMN, 800,
			META_TREE_DATE_COLUMN, NULL,
			-1);
    g_free (string_tmp);
    if (balance)
	g_free (balance);
}


/**
 * Fill an empty sub-division row with textual representation of a
 * sub-division structure, in the form: "Name (num transactions)
 * Balance".
 *
 * \param model		The GtkTreeModel that contains iter.
 * \param iface		A pointer to the metatree interface to use
 * \param iter		Iter to fill with sub-division data.
 * \param division	Division structure number (parent).
 * \param sub_division	Sub-division structure number.
 */
void fill_sub_division_zero ( GtkTreeModel * model,
                        MetatreeInterface * iface,
                        GtkTreeIter * iter,
                        gint division )
{
    gchar *balance = NULL;
    gchar *string_tmp;
    GtkTreeIter dumb_iter;
    gint number_transactions = 0;

/*     devel_debug_int ( division );  */

    if ( ! metatree_model_is_displayed ( model ) )
	return;

    /* if no division, there is no sub division */
    if (!division)
	return;

    string_tmp = iface -> sub_div_name (division, 0);
    /* on a affaire à un tiers */
    if ( string_tmp == NULL)
        return;

    number_transactions = iface -> sub_div_nb_transactions ( division, 0 );

    if ( number_transactions )
    {
	gchar *label;

	label = g_strdup_printf ( "%s (%d)",
				  string_tmp,
				  number_transactions );
	g_free (string_tmp);
	string_tmp = label;

	if ( ! gtk_tree_model_iter_has_child ( model, iter ) )
	    gtk_tree_store_append (GTK_TREE_STORE (model), &dumb_iter, iter );

	balance = gsb_real_get_string_with_currency ( iface -> sub_div_balance ( division, 0 ),
						      iface -> tree_currency (), TRUE );
    }
    
    gtk_tree_store_set ( GTK_TREE_STORE (model), iter,
			 META_TREE_TEXT_COLUMN, string_tmp,
			 META_TREE_POINTER_COLUMN, 0,
			 META_TREE_BALANCE_COLUMN, balance,
			 META_TREE_XALIGN_COLUMN, 1.0,
			 META_TREE_NO_DIV_COLUMN, division,
			 META_TREE_NO_SUB_DIV_COLUMN, 0,
			 META_TREE_NO_TRANSACTION_COLUMN, 0,
			 META_TREE_FONT_COLUMN, 400,
			 META_TREE_DATE_COLUMN, NULL,
			 -1 );
    g_free (string_tmp);
    if (balance)
        g_free (balance);
}


/**
 * 
 *
 * \param
 *
 * \return
 * */
void move_transaction_to_sub_division_zero ( gint transaction_number,
                        MetatreeInterface * iface,
                        GtkTreeModel * model, gint no_division )
{
    GtkTreeIter child_iter, * parent_iter;
    GtkTreeIter * p_iter;

     if ( !model )
        return;

    p_iter = get_iter_from_div ( model, no_division, 0 );
    if ( ! p_iter )
        return;

    parent_iter = get_iter_from_sub_div_zero ( model, iface, p_iter );
    if ( parent_iter )
    {
        gtk_tree_store_append ( GTK_TREE_STORE (model),
                &child_iter, parent_iter );
        iface -> transaction_set_div_id (
                transaction_number, no_division );
        iface -> transaction_set_sub_div_id (
                transaction_number, 0 );
        gsb_transactions_list_update_transaction (
                transaction_number);
        iface -> add_transaction_to_sub_div ( transaction_number,
                no_division, 0 );
        fill_transaction_row ( model, &child_iter, transaction_number );
    }
}


/**
 *
 *
 *
 *
 *
 * */
GtkTreeIter *get_iter_from_sub_div_zero ( GtkTreeModel *model,
                        MetatreeInterface *iface,
                        GtkTreeIter *parent_iter )
{
    GtkTreeIter dest_iter;
    GtkTreeIter child_iter;
    gint i;

    dest_iter = *parent_iter;
    for (i = 0; i< gtk_tree_model_iter_n_children (model, &dest_iter); i++ )
    {
        if ( gtk_tree_model_iter_nth_child (model, &child_iter, &dest_iter, i) )
        {
            gchar *name;

            gtk_tree_model_get ( model, &child_iter,
                            META_TREE_TEXT_COLUMN, &name, -1 );
            if ( name && g_utf8_collate (
                        name, _(iface->no_sub_div_label) ) == 0 )
                break;
        }
    }
    
    return gtk_tree_iter_copy (&child_iter);
}


/**
 *
 *
 *
 *
 *
 * */
void move_transactions_to_division_payee (GtkTreeModel * model,
                        MetatreeInterface * iface,
                        gint orig_div, gint dest_div )
{
    GSList *list_tmp_transactions;

    devel_debug_int ( dest_div);

    if ( !model )
        return;
    
    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );

    /* fill the dest division for payee with the transactions */
    list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

    while ( list_tmp_transactions )
    {
        gint transaction_number_tmp;
        transaction_number_tmp = gsb_data_transaction_get_transaction_number (
                        list_tmp_transactions -> data);

        if ( transaction_number_tmp &&
             iface -> transaction_div_id (transaction_number_tmp) == orig_div )
        {
            iface -> transaction_set_div_id (
                    transaction_number_tmp, dest_div );
            iface -> transaction_set_sub_div_id (
                    transaction_number_tmp, 0 );
            gsb_transactions_list_update_transaction (
                    transaction_number_tmp );
            iface -> add_transaction_to_div ( transaction_number_tmp,
                    dest_div );
        }
        list_tmp_transactions = list_tmp_transactions -> next;
    }

    /* We did some modifications */
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}


/**
 *
 *
 *
 *
 * */
static void button_delete_div_sub_div_clicked ( GtkWidget *togglebutton, GdkEventButton *event, GtkWidget *button )
{
    button_move_selected = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( togglebutton ) );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( togglebutton ),
                        button_move_selected );
}


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
 * 	no_div will contain the new division, no_sub_div the new sub-division
 * 	if no move of transactions, no_div and no_sub_div are set to 0
 * 	if move to another division but no sub-division, no_sub_div is set to 0
 * */
gboolean metatree_find_destination_blob ( MetatreeInterface *iface,
                        GtkTreeModel *model,
                        gint division,
                        gint sub_division,
                        gint *no_div,
                        gint *no_sub_div,
                        enum meta_tree_row_type type_division )
{
    GtkWidget *dialog;
    GtkWidget *hbox;
    GtkWidget *button;
    GtkWidget *label;
    GtkWidget *combofix;
    gint nouveau_no_division;
    gint nouveau_no_sub_division;
    gint resultat;
    gchar **split_division;
    gchar *tmp_str;
    gchar* tmp_str_1;
    gchar* tmp_str_2;

    if ( type_division == META_TREE_TRANS_S_S_DIV )
    {
        tmp_str_1 = g_strdup_printf ( _("Transfer all transactions in a \n%s."),
                        gettext ( iface -> meta_sub_name ) );

        tmp_str_2 = NULL;
    }
    else
    {
        /* create the box to move change the division and sub-div of the transactions */
        tmp_str_1 = g_strdup_printf ( _("Choose action for \"%s\"."),
                        ( !sub_division ?
                        iface -> div_name ( division ) :
                        iface -> sub_div_name ( division, sub_division ) ) );

        tmp_str_2 = g_strdup_printf (
                        _("You can transfer content from \"%s\" in another %s or %s.\n"
                        "Otherwise you can transfer \"%s\" in another %s "
                        "or transform \"%s\" to %s."),
                        iface -> sub_div_name ( division, sub_division ),
                        gettext ( iface -> meta_name_minus ),
                        gettext ( iface -> meta_sub_name ),
                        iface -> sub_div_name ( division, sub_division ),
                        gettext (iface -> meta_name_minus),
                        iface -> sub_div_name ( division, sub_division ),
                        gettext ( iface -> meta_name_minus ) );
    }

    dialog = dialogue_special_no_run ( GTK_MESSAGE_OTHER,
                        GTK_BUTTONS_OK_CANCEL,
                        make_hint ( tmp_str_1 , tmp_str_2 ) );

    gtk_widget_set_size_request ( dialog, 450, -1 );

    g_free ( tmp_str_1 );
    g_free ( tmp_str_2 );

    if ( sub_division )
    {
        GtkWidget *vbox;
        gchar *tmp_str_1;
        gchar *tmp_str_2;
        gchar *tmp_str_3;

        tmp_str_1 = g_strdup_printf ( _("Transfer the transactions in a %s or %s"),
                        gettext ( iface -> meta_name_minus ),
                        gettext ( iface -> meta_sub_name ) );
        tmp_str_2 = g_strdup_printf(_("Transfer \"%s\" in other %s"),
                        iface -> sub_div_name ( division, sub_division ),
                        gettext ( iface -> meta_name_minus ) );
        tmp_str_3 = tmp_str = g_strdup_printf(_("Convert \"%s\" in new %s"),
                        iface -> sub_div_name ( division, sub_division ),
                        gettext ( iface -> meta_name_minus ) );

        button = gsb_automem_radiobutton3_new ( tmp_str_1,
					    tmp_str_2,
					    tmp_str_3,
					    &button_action_selected,
					    G_CALLBACK ( metatree_button_action_sub_div_clicked ),
					    &button_action_selected,
                        GTK_ORIENTATION_VERTICAL );

        g_free ( tmp_str_1 );
        g_free ( tmp_str_2 );
        g_free ( tmp_str_3 );

        gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), button, FALSE, FALSE, 0 );

        /* create the list containing division and sub-division without the current division */
        hbox = gtk_hbox_new ( FALSE, 6 );
        gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox, FALSE, FALSE, 0 );

        label = gtk_label_new ( _("Select the destination: ") );
        gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0 );
        gtk_box_pack_start ( GTK_BOX ( hbox ), label, TRUE, TRUE, 0 );

        vbox = gtk_vbox_new ( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX ( hbox ), vbox, TRUE, TRUE, 0 );

        combofix = metatree_get_combofix ( iface, division, sub_division, type_division );
        gtk_combofix_set_force_text ( GTK_COMBOFIX ( combofix ), FALSE );
        gtk_box_pack_start ( GTK_BOX ( vbox ), combofix, TRUE, TRUE, 0 );
        g_object_set_data ( G_OBJECT ( button ), "label", label );
        g_object_set_data ( G_OBJECT ( button ), "combofix", combofix );
    }
    else
    {
        /* on reset le choix */
        button_action_selected = 0;

        hbox = gtk_hbox_new ( FALSE, 6 );
        gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox, FALSE, FALSE, 0 );

        label = gtk_label_new ( _("Select the destination: ") );
        gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0 );
        gtk_box_pack_start ( GTK_BOX ( hbox ), label, TRUE, TRUE, 0 );

        combofix = metatree_get_combofix ( iface, division, sub_division,  type_division );
        gtk_combofix_set_force_text ( GTK_COMBOFIX ( combofix ), FALSE );
        gtk_box_pack_start ( GTK_BOX ( hbox ), combofix, TRUE, TRUE, 0 );
    }

    gtk_widget_show_all ( dialog );
    gtk_widget_grab_focus ( GTK_WIDGET ( ( GTK_COMBOFIX ( combofix ) ) -> entry ) );
    gtk_editable_set_position ( GTK_EDITABLE ( ( GTK_COMBOFIX ( combofix ) ) -> entry ), 0 );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    if ( resultat != GTK_RESPONSE_OK )
    {
        gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

        return FALSE;
    }

    nouveau_no_division = 0;
    nouveau_no_sub_division = 0;

    if ( button_action_selected < 2 )
    {
        /* we want to move the content */
        if ( !strlen ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ) ) ) )
        {
            tmp_str_1 = g_strdup_printf (
                            _("It is compulsory to specify a destination %s "
                            "to move content but no %s was entered."),
                            gettext ( iface -> meta_name_minus ),
                            gettext ( iface -> meta_name_minus ) );
            tmp_str_2 = g_strdup_printf ( _("Please enter a %s!"), _(iface -> meta_name_minus) );

            dialogue_warning_hint ( tmp_str_1 , tmp_str_2 );

            g_free ( tmp_str_1 );
            g_free ( tmp_str_2 );

            gtk_widget_destroy (dialog);

            return ( metatree_find_destination_blob ( iface, model,
                            division, sub_division,
                            no_div, no_sub_div, type_division ) );
        }

        /* get the new (sub-)divisions */
        split_division = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ) ), " : ", 2 );

        nouveau_no_division = iface -> get_div_pointer_from_name ( split_division[0], 0 );

        if (nouveau_no_division)
        {
            nouveau_no_sub_division =  iface -> get_sub_div_pointer_from_name (
                        nouveau_no_division, split_division[1], 0 );

            if ( split_division[1] && nouveau_no_sub_division == 0 )
            {
                tmp_str_1 = g_strdup_printf ( _("Warning you can not create %s."),
                        _(iface -> meta_name_minus) );
                dialogue_warning( tmp_str_1 );

                g_free ( tmp_str_1 );

                gtk_widget_destroy ( dialog );

                return ( metatree_find_destination_blob ( iface, model,
                            division, sub_division,
                            no_div, no_sub_div, type_division ) );
            }
        }

        g_strfreev ( split_division );
    }

    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    /* return the new number of division and sub-division
     * if don't want to move the transactions, 0 and 0 will be returned */
    if ( no_div)
        *no_div = nouveau_no_division;
    if ( no_sub_div )
        *no_sub_div = nouveau_no_sub_division;

    return TRUE;
}


/**
 *
 *
 * \param
 *
 */
void metatree_manage_sub_divisions ( GtkWidget *tree_view )
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeIter *it;
    GtkTreePath *path = NULL;
    gint no_division = 0;
    gint no_sub_division = 0;
    gint new_division;
    gint new_sub_division;
    gint profondeur;
    MetatreeInterface *iface;
    enum meta_tree_row_type type_division;

    devel_debug (NULL);

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    if ( selection && gtk_tree_selection_get_selected ( selection, &model, &iter ) )
    {
        gtk_tree_model_get ( model, &iter,
                        META_TREE_NO_DIV_COLUMN, &no_division,
                        META_TREE_NO_SUB_DIV_COLUMN, &no_sub_division,
                        -1 );
    }
    else
        return;

    iface = g_object_get_data ( G_OBJECT ( model ), "metatree-interface" );
    type_division = metatree_get_row_type_from_tree_view ( tree_view );

	if ( !metatree_find_destination_blob ( iface, model, no_division, no_sub_division,
				       &new_division, &new_sub_division, type_division ) )
	    return;

/*     printf ("no_division = %d no_sub_division = %d\n", no_division, no_sub_division);
 *     printf ("new_division = %d new_sub_division = %d\n", new_division, new_sub_division);
 * 
 */
    /* Transfert d'opérations dans une catégorie ou une sous catégorie */
    if ( button_action_selected == 0 && no_division && new_division )
    {
        metatree_move_transactions_to_sub_div ( iface, model,
                        no_division, no_sub_division, new_division, new_sub_division );

        /* fait le tour des échéances pour mettre le nouveau numéro de division et sub_division  */
        metatree_move_scheduled_with_div_sub_div ( iface, model,
                            no_division, no_sub_division,
                            new_division, new_sub_division );

        /* update value in the tree view */
        metatree_update_tree_view ( iface );
	}

    if ( button_action_selected == 1 && no_division && no_sub_division && new_division )
    {
        new_sub_division = metatree_move_sub_division_to_division ( iface,
                        no_division, no_sub_division, new_division );

        /* fait le tour des échéances pour mettre le nouveau numéro de division et sub_division  */
        metatree_move_scheduled_with_div_sub_div ( iface, model,
                            no_division, no_sub_division,
                            new_division, new_sub_division );

        /* update value in the tree view */
        metatree_update_tree_view ( iface );
    }

    if ( button_action_selected == 2 && new_division == 0 && new_sub_division == 0 )
    {
        new_division = metatree_create_division_from_sub_division ( iface, no_division, no_sub_division );

        /* fait le tour des échéances pour mettre le nouveau numéro de division et sub_division  */
        metatree_move_scheduled_with_div_sub_div ( iface, model,
                            no_division, no_sub_division,
                            new_division, new_sub_division );

        /* update value in the tree view */
        metatree_update_tree_view ( iface );
    }
        /* restitue l'état du tree_view */
        /* old path */
        if ( iface -> content == 1 )
            path = category_hold_position_get_path ( );
        else if ( iface -> content == 2 )
            path = budgetary_hold_position_get_path ( );

        profondeur = gtk_tree_path_get_depth ( path );
        while ( profondeur > 1 )
        {
            gtk_tree_path_up ( path );
            profondeur = gtk_tree_path_get_depth ( path );
        }
        gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( tree_view ), path );
        gtk_tree_path_free ( path );

        /* new path */
        it = get_iter_from_div ( model, new_division, 0 );
        if ( it )
        {
            path = gtk_tree_model_get_path ( model, it );
            gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( tree_view ), path );
            gtk_tree_path_free ( path );
        }

    /* We did some modifications */
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}


/**
 *
 *
 * \param
 *
 */
enum meta_tree_row_type metatree_get_row_type_from_tree_view ( GtkWidget *tree_view )
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    enum meta_tree_row_type type_division;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    if ( selection && gtk_tree_selection_get_selected ( selection, &model, &iter ) )
    {
        GtkTreePath *path;
        GtkTreeIter parent;
        gint no_division = 0;
        gint no_sub_division = 0;

        gtk_tree_model_get ( model, &iter,
                        META_TREE_NO_DIV_COLUMN, &no_division,
                        META_TREE_NO_SUB_DIV_COLUMN, &no_sub_division,
                        -1 );

        if ( no_sub_division == 0
         &&
         gtk_tree_model_iter_parent ( GTK_TREE_MODEL ( model ), &parent, &iter ) )
        {
            gint nbre_trans_s_s_div = 0; /* nbre de transactions sans sous-division */

            nbre_trans_s_s_div = metatree_get_nbre_transactions_sans_sub_div ( tree_view );
            if ( nbre_trans_s_s_div > 0 )
                return META_TREE_TRANS_S_S_DIV;
            else
                return META_TREE_INVALID;
        }

        path = gtk_tree_model_get_path ( model, &iter );
        type_division = metatree_get_row_type ( model, path );

        gtk_tree_path_free ( path );

        return type_division;
    }

    return META_TREE_INVALID;
}


/**
 *
 *
 *
 *
 * */
void metatree_button_action_sub_div_clicked ( GtkWidget *togglebutton,
                        GdkEventButton *event,
                        gint *pointeur )
{
    if ( pointeur )
    {
        GtkWidget *parent = NULL;
        GtkWidget *label;
        GtkWidget *combofix = NULL;
        GSList *list;
        gint value = 0;

        value = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( togglebutton ), "pointer" ) );
        *pointeur = value;
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( togglebutton ), TRUE );

        parent = gtk_widget_get_parent ( togglebutton );
        if ( parent )
        {
            label = g_object_get_data ( G_OBJECT ( parent ), "label" );
            combofix = g_object_get_data ( G_OBJECT ( parent ), "combofix" );
        }
        if ( ! combofix )
            return;

        switch ( value )
        {
            case 0:
                gtk_widget_set_sensitive ( label, TRUE );
                gtk_widget_set_sensitive ( combofix, TRUE );
                list = g_object_get_data ( G_OBJECT ( combofix ), "list_1" );
                gtk_combofix_set_list ( GTK_COMBOFIX ( combofix ), list );
                gtk_widget_grab_focus ( GTK_WIDGET ( ( GTK_COMBOFIX ( combofix ) ) -> entry ) );
                gtk_editable_set_position ( GTK_EDITABLE ( ( GTK_COMBOFIX ( combofix ) ) -> entry ), 0 );
            break;
            case 1:
                gtk_widget_set_sensitive ( label, TRUE );
                gtk_widget_set_sensitive ( combofix, TRUE );
                list = g_object_get_data ( G_OBJECT ( combofix ), "list_2" );
                gtk_combofix_set_list ( GTK_COMBOFIX ( combofix ), list );
                gtk_widget_grab_focus ( GTK_WIDGET ( ( GTK_COMBOFIX ( combofix ) ) -> entry ) );
                gtk_editable_set_position ( GTK_EDITABLE ( ( GTK_COMBOFIX ( combofix ) ) -> entry ), 0 );
            break;
            case 2:
                gtk_widget_set_sensitive ( label, FALSE );
                gtk_widget_set_sensitive ( combofix, FALSE );
            break;
        }
    }
}


/**
 *
 *
 *
 *
 * */
void metatree_move_scheduled_with_div_sub_div ( MetatreeInterface *iface,
                        GtkTreeModel *model,
                        gint no_division,
                        gint no_sub_division,
                        gint new_division,
                        gint new_sub_division )
{
    GSList *list_tmp;

    /* fait le tour des échéances pour mettre le nouveau numéro de division et sub_division  */
    list_tmp = gsb_data_scheduled_get_scheduled_list ( );

    while ( list_tmp )
    {
        gint scheduled_number;

        scheduled_number = gsb_data_scheduled_get_scheduled_number ( list_tmp -> data );

        if ( iface -> scheduled_div_id ( scheduled_number ) == no_division )
        {
            if ( ( no_sub_division && iface -> scheduled_sub_div_id (scheduled_number) == no_sub_division )
             ||
             no_sub_division == 0 )
            {
                iface -> scheduled_set_div_id ( scheduled_number, new_division );
                iface -> scheduled_set_sub_div_id ( scheduled_number, new_sub_division );
            }
        }

        list_tmp = list_tmp -> next;
    }
}


/**
 *
 *
 *
 *
 * */
void metatree_reset_transactions_without_div_sub_div ( MetatreeInterface *iface,
                        GtkTreeModel *model,
                        gint no_division,
                        gint new_division,
                        gint new_sub_division )
{
    GSList *list_tmp;

    /* move the transactions, need to to that for archived transactions too */
    list_tmp = gsb_data_transaction_get_complete_transactions_list ();

    while ( list_tmp )
    {
        gint transaction_number_tmp;

        transaction_number_tmp =
                gsb_data_transaction_get_transaction_number ( list_tmp -> data );

        if ( iface -> transaction_div_id ( transaction_number_tmp ) == no_division )
            move_transaction_to_sub_division ( transaction_number_tmp, model,
                                NULL, NULL,
                                new_division, new_sub_division );

        list_tmp = list_tmp -> next;
    }
}


/**
 *
 *
 *
 *
 * */
void metatree_update_tree_view ( MetatreeInterface *iface )
{
	/* update value in the tree view */
	switch ( iface -> content )
	{
	    case 0:
		transaction_list_update_element ( ELEMENT_PARTY );
        gsb_data_payee_update_counters ( );
		break;
	    case 1:
		transaction_list_update_element ( ELEMENT_CATEGORY );
        remplit_arbre_categ ( );
		break;
	    case 2:
		transaction_list_update_element ( ELEMENT_BUDGET );
        gsb_data_budget_update_counters ( );
		break;
	}
}


/**
 *
 *
 *
 *
 * */
gint metatree_get_nbre_transactions_sans_sub_div ( GtkWidget *tree_view )
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint nbre_trans_s_s_div = 0; /* nbre de transactions sans sous-division */

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    if ( selection && gtk_tree_selection_get_selected ( selection, &model, &iter ) )
    {
        gint no_division = 0;
        MetatreeInterface *iface;

        gtk_tree_model_get ( model, &iter,
                        META_TREE_NO_DIV_COLUMN, &no_division,
                        -1 );

        iface = g_object_get_data ( G_OBJECT ( model ), "metatree-interface" );
        nbre_trans_s_s_div = iface -> sub_div_nb_transactions ( no_division, 0 );
    }

    return nbre_trans_s_s_div;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *metatree_get_combofix ( MetatreeInterface *iface,
                        gint division,
                        gint sub_division,
                        enum meta_tree_row_type type_division )
{
    GtkWidget *combofix;
    GSList *liste_combofix;

    liste_combofix = metatree_get_combofix_list ( iface, division, sub_division, TRUE );

    combofix = gtk_combofix_new ( liste_combofix );
    g_object_set_data ( G_OBJECT ( combofix ), "list_1", liste_combofix );

    liste_combofix = metatree_get_combofix_list ( iface, division, 0, FALSE );
    g_object_set_data ( G_OBJECT ( combofix ), "list_2", liste_combofix );

    return combofix;
}


GSList *metatree_get_combofix_list ( MetatreeInterface *iface,
                        gint division,
                        gint sub_division,
                        gboolean show_sub_division )
{
    GSList *liste_combofix;
    GSList *division_list;
    GSList *liste_division_credit;
    GSList *liste_division_debit;
    gint show_division;

    division_list = iface -> div_list ( );
    liste_combofix = NULL;
    liste_division_credit = NULL;
    liste_division_debit = NULL;
    show_division = g_slist_length ( iface -> div_sub_div_list ( division ) );

    while ( division_list )
    {
        gint tmp_division;
        GSList *sub_division_list;

        tmp_division = iface -> div_id ( division_list -> data );

        /* if we are on the current division and no sub-(  go to the next */
        if ( division && show_division == 0 && division == tmp_division )
        {
            division_list = division_list -> next;
            continue;
        }

        /* get the division */
        switch ( iface -> div_type ( tmp_division ) )
        {
            case 1:
            liste_division_debit = g_slist_append ( liste_division_debit,
                                my_strdup (iface -> div_name ( tmp_division ) ) );
            break;
            default:
            liste_division_credit = g_slist_append ( liste_division_credit,
                                my_strdup (iface -> div_name ( tmp_division ) ) );
            break;
        }

        sub_division_list = iface -> div_sub_div_list ( tmp_division );

        if ( show_sub_division == 0 )
        {
            division_list = division_list -> next;
            continue;
        }

        /* add the sub-divisions if necessary */
        while ( sub_division_list )
        {
            gint tmp_sub_division;

            tmp_sub_division = iface -> sub_div_id (sub_division_list -> data);
            if ( division == tmp_division && tmp_sub_division == sub_division )
            {
                sub_division_list = sub_division_list -> next;
                continue;
            }

            switch ( iface -> div_type (tmp_division))
            {
            case 1:
                liste_division_debit = g_slist_append ( liste_division_debit,
                                        g_strconcat ( "\t",
                                        iface -> sub_div_name ( tmp_division, tmp_sub_division ),
                                        NULL ) );
                break;
            case 0:
                liste_division_credit = g_slist_append ( liste_division_credit,
                                        g_strconcat ( "\t",
                                        iface -> sub_div_name ( tmp_division, tmp_sub_division ),
                                        NULL ) );
                break;
            default:
                break;
            }

            sub_division_list = sub_division_list -> next;
        }

        division_list = division_list -> next;
    }

    /* create the combofix complex with the divisions and sub-divisions */
    liste_combofix = g_slist_append ( liste_combofix, liste_division_debit );
    liste_combofix = g_slist_append ( liste_combofix, liste_division_credit );

    return liste_combofix;
}


/**
 *
 *
 *
 *
 * */
void metatree_move_transactions_to_sub_div ( MetatreeInterface *iface,
                        GtkTreeModel *model,
                        gint no_division,
                        gint no_sub_division,
                        gint new_division,
                        gint new_sub_division )
{
    GSList *list_tmp;

    /* move the transactions, need to to that for archived transactions too */
    list_tmp = gsb_data_transaction_get_complete_transactions_list ();

    while ( list_tmp )
    {
        gint transaction_number_tmp;

        transaction_number_tmp =
                gsb_data_transaction_get_transaction_number ( list_tmp -> data );

        if ( iface -> transaction_div_id ( transaction_number_tmp ) == no_division
         &&
         iface -> transaction_sub_div_id ( transaction_number_tmp ) == no_sub_division )
        {
            /* Change parameters of the transaction */
            iface -> transaction_set_div_id ( transaction_number_tmp, new_division );
            iface -> transaction_set_sub_div_id ( transaction_number_tmp, new_sub_division );
            gsb_transactions_list_update_transaction ( transaction_number_tmp );
        }

        list_tmp = list_tmp -> next;
    }
}


gint metatree_move_sub_division_to_division ( MetatreeInterface *iface,
                        gint no_division,
                        gint no_sub_division,
                        gint new_division )
{
    GSList *list_tmp;
    gint new_sub_division = 0;

    /* create the new_sub_division */
    new_sub_division = iface -> add_sub_div ( new_division );

    /* set name */
    metatree_sub_division_set_name ( iface,
                        no_division, no_sub_division,
                        new_division, new_sub_division );

    /* remove old subdivision */
    iface -> remove_sub_div ( no_division, no_sub_division );

    /* move the transactions, need to to that for archived transactions too */
    list_tmp = gsb_data_transaction_get_complete_transactions_list ();

    while ( list_tmp )
    {
        gint transaction_number_tmp;

        transaction_number_tmp =
                gsb_data_transaction_get_transaction_number ( list_tmp -> data );

        if ( iface -> transaction_div_id ( transaction_number_tmp ) == no_division
         &&
         iface -> transaction_sub_div_id ( transaction_number_tmp ) == no_sub_division )
        {
            /* Change parameters of the transaction */
            iface -> transaction_set_div_id ( transaction_number_tmp, new_division );
            iface -> transaction_set_sub_div_id ( transaction_number_tmp, new_sub_division );
            gsb_transactions_list_update_transaction ( transaction_number_tmp );
        }

        list_tmp = list_tmp -> next;
    }

    return new_sub_division;
}


/**
 *
 *
 *
 *
 * */
gint metatree_create_division_from_sub_division ( MetatreeInterface *iface,
                        gint no_division,
                        gint no_sub_division )
{
    GSList *list_tmp;
    gint new_division = 0;

    /* create the new_sub_division */
    new_division = iface -> add_div ( );

    /* set name */
    metatree_division_set_name ( iface,
                        no_division, no_sub_division,
                        new_division );

    /* remove old subdivision */
    iface -> remove_sub_div ( no_division, no_sub_division );

    /* move the transactions, need to to that for archived transactions too */
    list_tmp = gsb_data_transaction_get_complete_transactions_list ();

    while ( list_tmp )
    {
        gint transaction_number_tmp;

        transaction_number_tmp =
                gsb_data_transaction_get_transaction_number ( list_tmp -> data );

        if ( iface -> transaction_div_id ( transaction_number_tmp ) == no_division
         &&
         iface -> transaction_sub_div_id ( transaction_number_tmp ) == no_sub_division )
        {
            /* Change parameters of the transaction */
            iface -> transaction_set_div_id ( transaction_number_tmp, new_division );
            iface -> transaction_set_sub_div_id ( transaction_number_tmp, 0 );
            gsb_transactions_list_update_transaction ( transaction_number_tmp );
        }

        list_tmp = list_tmp -> next;
    }

    return new_division;
}

/**
 *
 *
 *
 *
 * */
void metatree_division_set_name ( MetatreeInterface *iface,
                        gint no_division,
                        gint no_sub_division,
                        gint new_division )
{
	/* update value in the tree view */
	switch ( iface -> content )
	{
	    case 1:
            gsb_data_category_set_name ( new_division,
						   iface -> sub_div_name ( no_division, no_sub_division ) );
		break;
	    case 2:
            gsb_data_budget_set_name ( new_division,
						   iface -> sub_div_name ( no_division, no_sub_division ) );
		break;
	}
}


/**
 *
 *
 *
 *
 * */
void metatree_sub_division_set_name ( MetatreeInterface *iface,
                        gint no_division,
                        gint no_sub_division,
                        gint new_division,
                        gint new_sub_division )
{
	/* update value in the tree view */
	switch ( iface -> content )
	{
	    case 1:
            gsb_data_category_set_sub_category_name ( new_division,
						   new_sub_division,
						   iface -> sub_div_name ( no_division, no_sub_division ) );
		break;
	    case 2:
            gsb_data_budget_set_sub_budget_name ( new_division,
						   new_sub_division,
						   iface -> sub_div_name ( no_division, no_sub_division ) );
		break;
	}
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
