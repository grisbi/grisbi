/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	     2004 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "barre_outils.h"
#include "operations_comptes.h"
#include "utils_comptes.h"
#include "operations_liste.h"
#include "utils_devises.h"
#include "dialog.h"
#include "data_account.h"
#include "gtk_combofix.h"
#include "utils_str.h"
#include "traitement_variables.h"
#include "utils_tiers.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gboolean division_node_maybe_expand ( GtkTreeModel *model, GtkTreePath *path, 
				      GtkTreeIter *iter, gpointer data );
static void fill_transaction_row ( GtkTreeModel * model, GtkTreeIter * iter, 
			    struct structure_operation * operation );
static gboolean find_associated_transactions ( MetatreeInterface * iface, 
					gint no_division, gint no_sub_division );
static gboolean find_destination_blob ( MetatreeInterface * iface, GtkTreeModel * model, 
				 gpointer division, gpointer sub_division, 
				 gint * no_div, gint * no_sub_div );
static gboolean metatree_get ( GtkTreeModel * model, GtkTreePath * path,
			gint column, gpointer * data );
static gboolean metatree_get_row_properties ( GtkTreeModel * tree_model, GtkTreePath * path, 
				       gchar ** text, gint * lvl1, gint * lvl2, 
				       gpointer * data );
static enum meta_tree_row_type metatree_get_row_type ( GtkTreeModel * tree_model, 
						GtkTreePath * path );
static void move_transaction_to_sub_division ( struct structure_operation * transaction,
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
extern gint compte_courant;
extern GtkWidget *formulaire;
extern GSList *list_struct_accounts;
extern GSList *liste_struct_echeances;
extern GtkTreeStore *model;
extern GtkTreeSelection * selection;
extern GtkWidget *tree_view;
extern GtkWidget *treeview;
/*END_EXTERN*/



/**
 *
 *
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
 *
 *
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
 * TODO: document this
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
    GtkTreeIter dumb_iter;

    label = ( division ? iface -> div_name (division) : _(iface->no_div_label) );
    
    if ( ! division )
	division = iface -> get_without_div_pointer ();

    if ( etat.affiche_nb_ecritures_listes && 
	 division &&
	 iface -> div_nb_transactions ( division ) )
	label = g_strconcat ( label, " (",
			      itoa ( iface -> div_nb_transactions (division) ), ")",
			      NULL );

    if ( division && iface -> div_balance ( division ) )
	balance = g_strdup_printf ( _("%4.2f %s"), iface -> div_balance ( division ),
				    devise_code ( iface -> tree_currency () ) );
    
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
			-1);
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
    GtkTreeIter dumb_iter;
    gint nb_ecritures = 0;

    label = ( sub_division ? iface -> sub_div_name (sub_division) : _(iface -> no_sub_div_label) );

    if ( ! division )
	division = iface -> get_without_div_pointer ();

    nb_ecritures = iface -> sub_div_nb_transactions ( division, sub_division );
    
    if ( nb_ecritures )
    {
	if ( etat.affiche_nb_ecritures_listes )
	    label = g_strconcat ( label, " (", itoa ( nb_ecritures ), ")", NULL );
	
	if ( ! gtk_tree_model_iter_has_child ( model, iter ) )
	{
	    gtk_tree_store_append (GTK_TREE_STORE (model), &dumb_iter, iter );
	}

	balance = g_strdup_printf ( _("%4.2f %s"),
				    iface -> sub_div_balance ( division, sub_division ),
				    devise_code ( iface -> tree_currency () ) );
    }
    
    gtk_tree_store_set ( GTK_TREE_STORE (model), iter,
			 META_TREE_TEXT_COLUMN, label,
			 META_TREE_POINTER_COLUMN, sub_division,
			 META_TREE_BALANCE_COLUMN, balance,
			 META_TREE_XALIGN_COLUMN, 1.0,
			 META_TREE_NO_DIV_COLUMN, iface -> div_id ( division ),
			 META_TREE_NO_SUB_DIV_COLUMN, iface -> sub_div_id ( sub_division ),
			 META_TREE_FONT_COLUMN, 400,
			 -1 );
}



/**
 *
 * \param model		The GtkTreeModel that contains iter.
 */
void fill_transaction_row ( GtkTreeModel * model, GtkTreeIter * iter, 
			    struct structure_operation * operation )
{
    gchar *montant, * label, * notes = NULL; /* free */

    if ( operation -> notes )
    {
	if ( strlen ( operation -> notes ) > 30 )
	{
	    gchar * tmp = (operation -> notes) + 30;

	    tmp = strchr ( tmp, ' ' );
	    if ( !tmp )
	    {
		/* We do not risk splitting the string
		   in the middle of a UTF-8 accent
		   ... the end is probably near btw. */
		notes = operation -> notes;
	    }
	    else 
	    {
		gchar * trunc = g_strndup ( operation -> notes, 
					    ( tmp - operation -> notes ) );
		notes = g_strconcat ( trunc, " ...", NULL );
		free ( trunc );
	    }
	}
	else 
	{
	    notes = operation -> notes;
	}
    }
    else
    {
	struct struct_tiers * tiers = tiers_par_no ( operation -> tiers );
	if ( tiers )
	{
	    notes = tiers -> nom_tiers;
	}
    }

    label = g_strdup_printf ( _("%02d/%02d/%04d"),
			      operation -> jour,
			      operation -> mois,
			      operation -> annee );
 
    if ( notes )
    {
	label = g_strconcat ( label, " : ", notes, NULL );
    }

    if ( operation -> no_operation_ventilee_associee )
    {
	label = g_strconcat ( label, " (", _("breakdown"), ")", NULL );
    }

    montant = g_strdup_printf ( "%4.2f %s", operation -> montant,
				devise_code ( devise_par_no ( operation -> devise ) ) );
    gtk_tree_store_set ( GTK_TREE_STORE(model), iter, 
			 META_TREE_POINTER_COLUMN, operation,
			 META_TREE_TEXT_COLUMN, label,
			 META_TREE_ACCOUNT_COLUMN, compte_name_by_no(operation->no_compte),
			 META_TREE_BALANCE_COLUMN, montant,
			 META_TREE_NO_DIV_COLUMN, -1,
			 META_TREE_NO_SUB_DIV_COLUMN, -1,
			 META_TREE_XALIGN_COLUMN, 1.0,
			 -1);
    
}



/**
 * FIXME: shouldn't this be partly done via metatree?  (at least the
 * tree stuff).
 */
void appui_sur_ajout_division ( GtkWidget * button, GtkTreeModel * model )
{
    MetatreeInterface * iface;
    GtkTreeIter iter, sub_iter;
    gint div_id;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );   
    if ( ! iface )
	return;

    div_id = iface -> add_div ();

    gtk_tree_store_append ( GTK_TREE_STORE(model), &iter, NULL );
    fill_division_row ( model, iface, &iter, iface -> get_div_pointer ( div_id ) );
    
    gtk_tree_store_append (GTK_TREE_STORE (model), &sub_iter, &iter);
    fill_sub_division_row ( GTK_TREE_MODEL(model), iface, &sub_iter, 
			    iface -> get_div_pointer ( div_id ), NULL );

    modification_fichier ( TRUE );
}



/**
 *
 *
 */
void appui_sur_ajout_sub_division ( GtkWidget * button, GtkTreeModel * model )
{
    MetatreeInterface * iface;
    GtkTreeIter iter, parent_iter;
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
	gint div_id, sub_div_id;

	/* Get parent division id */
	metatree_get_row_properties ( model, path, NULL, &div_id, NULL, NULL ) ;
	if ( div_id == -1 )
	    return;

	sub_div_id = iface -> add_sub_div ( div_id );
	if ( sub_div_id == -1 )
	    return;

	gtk_tree_store_append ( GTK_TREE_STORE(model), &iter, &parent_iter );
	fill_sub_division_row ( model, iface, &iter, 
				iface -> get_div_pointer ( div_id ),
				iface -> get_sub_div_pointer ( div_id, sub_div_id ) );
			     
	modification_fichier ( TRUE );

	gtk_tree_path_free ( path );
    }
}



/**
 * 
 *
 */
gboolean supprimer_division ( GtkWidget * button, GtkTreeView * tree_view )
{
    GtkTreeSelection * selection;
    GtkTreeModel * model;
    GtkTreeIter iter, * it;
    GSList * liste_tmp;
    gint no_division = 0, no_sub_division = 0;
    gpointer pointer = NULL;
    MetatreeInterface * iface;
    GSList *list_tmp;

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
	
	if ( ! find_destination_blob ( iface, model, pointer, NULL, 
				       &nouveau_no_division, &nouveau_no_sub_division ) )
	    return FALSE;

	/* on fait le tour des opés pour mettre le nouveau numéro de
	 * division et sub_division */

	list_tmp = list_struct_accounts;

	while ( list_tmp )
	{
	    gint i;

	    i = gsb_account_get_no_account ( list_tmp -> data );

	    liste_tmp = gsb_account_get_transactions_list (i);

	    while ( liste_tmp )
	    {
		struct structure_operation *operation;

		operation = liste_tmp -> data;

		if ( iface -> transaction_div_id (operation) == no_division )
		{
		    iface -> add_transaction_to_sub_div ( operation, nouveau_no_division,
							  nouveau_no_sub_division );
		    iface -> transaction_set_div_id ( operation, nouveau_no_division );
		    iface -> transaction_set_sub_div_id ( operation, nouveau_no_sub_division );
		}

		liste_tmp = liste_tmp -> next;
	    }

	    list_tmp = list_tmp -> next;
	}


	/* fait le tour des échéances pour mettre le nouveau numéro
	 * de division et sub_division  */

	liste_tmp = liste_struct_echeances;

	while ( liste_tmp )
	{
	    struct operation_echeance *echeance;

	    echeance = liste_tmp -> data;

	    if ( iface -> scheduled_div_id ( echeance ) == no_division )
	    {
		iface -> scheduled_set_div_id ( echeance, nouveau_no_division );
		iface -> scheduled_set_sub_div_id ( echeance, nouveau_no_sub_division );
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
    GSList *list_tmp;

    division = iface -> get_div_pointer ( no_division );

    if ( find_associated_transactions ( iface, no_division, 
					iface -> sub_div_id ( sub_division ) ) )
    {
	gint nouveau_no_division, nouveau_no_sub_division;
	GSList *liste_tmp;

	if ( ! find_destination_blob ( iface, model, division, sub_division, 
				       &nouveau_no_division, &nouveau_no_sub_division ) )
	    return;

	/* on fait le tour des opés pour mettre le nouveau numéro de
	 * division et sub_division */

	list_tmp = list_struct_accounts;

	while ( list_tmp )
	{
	    gint i;

	    i = gsb_account_get_no_account ( list_tmp -> data );

	    liste_tmp = gsb_account_get_transactions_list (i);

	    while ( liste_tmp )
	    {
		struct structure_operation *operation;

		operation = liste_tmp -> data;

		if ( ( iface -> transaction_div_id (operation) == 
		       iface -> div_id (division) ) &&
		     ( iface -> transaction_sub_div_id (operation) == 
		       iface -> sub_div_id (sub_division) ) )
		{
		    iface -> add_transaction_to_sub_div ( operation, nouveau_no_division,
							  nouveau_no_sub_division );
		    iface -> transaction_set_div_id (operation, nouveau_no_division);
		    iface -> transaction_set_sub_div_id (operation, nouveau_no_sub_division);
		}

		liste_tmp = liste_tmp -> next;
	    }

	    list_tmp = list_tmp -> next;
	}


	/* fait le tour des échéances pour mettre le nouveau numéro
	 * de division et sub_division  */

	liste_tmp = liste_struct_echeances;

	while ( liste_tmp )
	{
	    struct operation_echeance *echeance;

	    echeance = liste_tmp -> data;

	    if ( iface -> scheduled_div_id ( echeance ) == no_division &&
		 ( iface -> scheduled_sub_div_id ( echeance ) == 
		   iface -> sub_div_id ( sub_division ) ) )
	    {
		iface -> scheduled_set_div_id ( echeance, nouveau_no_division );
		iface -> scheduled_set_sub_div_id ( echeance, nouveau_no_sub_division );
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
	GSList *list_tmp;

	gtk_tree_model_get ( model, iter,
			     META_TREE_NO_DIV_COLUMN, &no_division,
			     META_TREE_NO_SUB_DIV_COLUMN, &no_sub_division,
			     -1 );

	list_tmp = list_struct_accounts;

	while ( list_tmp )
	{
	    gint account;
	    GSList *pointeur_ope;

	    account = gsb_account_get_no_account ( list_tmp -> data );

	    pointeur_ope = gsb_account_get_transactions_list (GPOINTER_TO_INT(account));

	    while ( pointeur_ope )
	    {
		struct structure_operation *operation;

		operation = pointeur_ope -> data;

		if ( operation &&
		     iface -> transaction_div_id ( operation ) == no_division &&
		     iface -> transaction_sub_div_id ( operation ) == no_sub_division/*  && */
/* 		     !operation -> relation_no_operation && */
/* 		     !operation -> operation_ventilee */ )
		{
		    if ( !first )
		    {
			gtk_tree_store_append ( GTK_TREE_STORE(model), &child_iter, iter );
		    }
		    else
		    {
			first = FALSE;
		    }

		    fill_transaction_row ( model, &child_iter, operation );
		}

		pointeur_ope = pointeur_ope -> next;
	    }

	    list_tmp = list_tmp -> next;
	}
    }
    return FALSE;
}



/**
 *  
 *
 */
gboolean division_activated ( GtkTreeView * treeview, GtkTreePath * path,
			      GtkTreeViewColumn * col, gpointer userdata )
{
    struct structure_operation * operation;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gint no_division, no_sub_division;

    model = gtk_tree_view_get_model(treeview);

    if ( gtk_tree_model_get_iter ( model, &iter, path ) )
    {
	gtk_tree_model_get( model, &iter, 
			    META_TREE_NO_DIV_COLUMN, &no_division,
			    META_TREE_NO_SUB_DIV_COLUMN, &no_sub_division,
			    META_TREE_POINTER_COLUMN, &operation, 
			    -1);

	/* We do not jump to a transaction if a division is specified */
	if ( operation && no_division == -1 && no_sub_division == -1 )
	{
	    changement_compte ( GINT_TO_POINTER ( operation -> no_compte ));
	    if ( operation -> pointe == 3 && !gsb_account_get_r (compte_courant) )
		change_aspect_liste ( 5 );
	    gsb_transactions_list_set_current_transaction ( operation,
							    compte_courant );
	}
    }

    return FALSE;
}



/**
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
	gint no_div;
	gpointer pointer;

	gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);
	orig_type = metatree_get_row_type ( model, orig_path );

	dest_type = metatree_get_row_type ( model, dest_path );
	if ( ! metatree_get ( model, dest_path, META_TREE_NO_DIV_COLUMN, 
			      (gpointer) &no_div ) ||
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
		     pointer && no_div != 0 ) /* i.e. ancestor is no "No division" */
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
 *  
 *
 */
gboolean division_drag_data_received ( GtkTreeDragDest * drag_dest, GtkTreePath * dest_path,
				       GtkSelectionData * selection_data )
{
    if ( DEBUG )
	printf (">>> division_drag_data_received %p, %p, %p\n", drag_dest, dest_path, 
		selection_data);

    if ( dest_path && selection_data )
    {
	GtkTreeModel * model;
	GtkTreePath * orig_path;
	GtkTreeIter iter, iter_parent, orig_iter;
	gchar * name;
	gint no_dest_division, no_dest_sub_division, no_orig_division, no_orig_sub_division;
	gpointer sub_division, dest_sub_division, orig_division, dest_division, pointer;
	enum meta_tree_row_type orig_type;
	struct structure_operation * transaction = NULL;
	MetatreeInterface * iface;
	GSList *list_tmp;

	gtk_tree_get_row_drag_data (selection_data, &model, &orig_path);
	iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );

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
		transaction = (struct structure_operation *) pointer;
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

		dest_sub_division = iface -> get_sub_div_pointer_from_name ( no_dest_division,
									     iface -> sub_div_name (sub_division),
									     1 );
		if ( dest_sub_division )
		    no_dest_sub_division = iface -> sub_div_id ( dest_sub_division );
		else
		    no_dest_sub_division = 0;

		/* Populate tree */
		gtk_tree_model_get_iter ( model, &iter_parent, dest_path );
		gtk_tree_store_append ( GTK_TREE_STORE(model), &iter, &iter_parent);
		fill_sub_division_row ( model, iface, &iter, 
					dest_division, dest_sub_division );

		list_tmp = list_struct_accounts;

		while ( list_tmp )
		{
		    gint account;
		    GSList *pointeur_ope;

		    account = gsb_account_get_no_account ( list_tmp -> data );

		    pointeur_ope = gsb_account_get_transactions_list (account);
	    
		    while ( pointeur_ope )
		    {
			transaction = pointeur_ope -> data;
		
			if ( transaction &&
			     iface -> transaction_div_id (transaction) == no_orig_division &&
			     iface -> transaction_sub_div_id (transaction) == no_orig_sub_division /* && */
/* 			     !transaction -> relation_no_operation && */
/* 			     !transaction -> operation_ventilee  */)
			{
			    GtkTreePath * path;
			    path = gtk_tree_model_get_path ( model, &iter );
			    move_transaction_to_sub_division ( transaction, model, 
							       NULL, path,
							       no_dest_division, 
							       no_dest_sub_division );
			}
			pointeur_ope = pointeur_ope -> next;
		    }
		    list_tmp = list_tmp -> next;
		}

		gtk_tree_model_get_iter ( model, &orig_iter, orig_path );
		gtk_tree_store_remove ( GTK_TREE_STORE(model), &orig_iter );

		iface -> remove_sub_div ( no_orig_division, no_orig_sub_division );

		modification_fichier(TRUE);

		break;

	    default:
		break;
	}

    }
    
    return FALSE;
}



/**
 * 
 *
 */
void move_transaction_to_sub_division ( struct structure_operation * transaction,
					GtkTreeModel * model,
					GtkTreePath * orig_path, GtkTreePath * dest_path,
					gint no_division, gint no_sub_division )
{
    GtkTreeIter orig_iter, child_iter, dest_iter, parent_iter, gd_parent_iter;
    gpointer old_division, new_division;
    gpointer old_sub_division, new_sub_division;
    MetatreeInterface * iface;
	
    if ( ! model )
	return;

    iface = g_object_get_data ( G_OBJECT(model), "metatree-interface" );

    old_division = iface -> get_div_pointer ( iface -> transaction_div_id (transaction) );
    new_division = iface -> get_div_pointer ( no_division );

    old_sub_division = iface -> get_sub_div_pointer ( iface -> transaction_div_id (transaction),
						      iface -> transaction_sub_div_id (transaction) );
    new_sub_division = iface -> get_sub_div_pointer ( no_division, no_sub_division );

    /* Insert new row */
    gtk_tree_model_get_iter ( model, &dest_iter, dest_path );
    /* Avoid filling "empty" not yet selected subdivisions */
    if ( gtk_tree_model_iter_children ( model, &child_iter, &dest_iter ) )
    {
	gchar * name;
	gtk_tree_model_get ( model, &child_iter, META_TREE_TEXT_COLUMN, &name, -1 );
	if ( name )
	{
	    gtk_tree_store_insert ( GTK_TREE_STORE (model), &child_iter, &dest_iter, 0 );
	    fill_transaction_row ( model, &child_iter, transaction );
	}
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
								     ( no_sub_div <= 0 ? 
								       iface -> div_name ( division ) :
								       iface -> sub_div_name ( sub_division ) ) ),
						   _("If you want to remove this sub-division but want to keep transactions, you can transfer them to another (sub-)division.  Otherwise, transactions can be simply deleted along with their division.") ));

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
							g_strdup ( iface -> div_name ( p_division ) ) );
		break;
	    default:
		liste_division_credit = g_slist_append ( liste_division_credit,
							 g_strdup ( iface -> div_name ( p_division ) ) );
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

    combofix = gtk_combofix_new_complex ( liste_combofix, TRUE, TRUE, TRUE, 0 );
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
    GSList *list_tmp;

    list_tmp = list_struct_accounts;

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	liste_tmp = gsb_account_get_transactions_list (i);

	while ( liste_tmp )
	{
	    struct structure_operation *operation;

	    operation = liste_tmp -> data;

	    if ( iface -> transaction_div_id ( operation ) == no_division &&
		 ( no_sub_division == -1 ||
		   iface -> transaction_sub_div_id ( operation) == no_sub_division ) )
	    {
		return TRUE;
	    }
	    else
		liste_tmp = liste_tmp -> next;
	}

	list_tmp = list_tmp -> next;
    }

    liste_tmp = liste_struct_echeances;

    /* fait le tour des échéances pour en trouver une qui a cette catégorie  */
    while ( liste_tmp )
    {
	struct operation_echeance *echeance = liste_tmp -> data;

	if ( iface -> scheduled_div_id ( echeance ) == no_division && 
	     ( no_sub_division == -1 ||
	       iface -> scheduled_div_id ( echeance ) == no_sub_division ) )
	{
	    return TRUE;
	}
	else
	    liste_tmp = liste_tmp -> next;
    }

    return FALSE;
}



/**
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
 * \param transaction   Transaction to update if associated GtkTreeIter exists.
 */
void update_transaction_in_tree ( MetatreeInterface * iface, GtkTreeModel * model, 
				  struct structure_operation * transaction )
{
    GtkTreeIter * transaction_iter, * sub_div_iter = NULL, * div_iter, dummy_iter;
    gpointer div, sub_div;
    gint div_id, sub_div_id;
    
    if ( ! transaction )
	return;

    if ( ! gtk_tree_model_get_iter_first ( model, &dummy_iter ) )
	return;

    div_id = iface -> transaction_div_id ( transaction );
    sub_div_id = iface -> transaction_sub_div_id ( transaction );
    div = iface -> get_div_pointer ( div_id );
    sub_div = iface -> get_sub_div_pointer ( div_id, sub_div_id );

    /* Fill in division if existing. */
    div_iter = get_iter_from_div ( model, div_id, -1 );
    if ( div_iter )
    {
	fill_division_row ( model, iface, div_iter, div );
    }

    /* Fill in sub-division if existing. */
    if ( iface -> depth != 1 )
    {
	sub_div_iter = get_iter_from_div ( model, div_id, sub_div_id );
	if ( sub_div_iter )
	{
	    fill_sub_division_row ( model, iface, sub_div_iter, div, sub_div );
	}
    }

    /* Fill in transaction if existing. */
    transaction_iter = get_iter_from_pointer ( model, transaction );

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
	fill_transaction_row ( model, transaction_iter, transaction );
    }
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
