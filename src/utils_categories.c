/* ************************************************************************** */
/*                                  utils_categories.c			      */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
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
#include "utils_categories.h"
#include "metatree.h"
#include "gsb_account.h"
#include "gsb_transaction_data.h"
#include "search_glist.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void reset_category_counters ();
static void update_category_in_trees ( struct struct_categ * category, 
				struct struct_sous_categ * sub_category );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkTreeStore * categ_tree_model;
extern MetatreeInterface * category_interface ;
extern GSList *liste_struct_categories;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint nb_enregistrements_categories, no_derniere_categorie;
extern gint nb_enregistrements_categories, no_derniere_categorie;
extern struct struct_categ * without_category;
/*END_EXTERN*/

struct struct_categ * without_category;



/* **************************************************************************************************** */
/* retourne l'adr de la categ dont le nom est donnÃ© en argument */
/* si pas trouvÃ©e : */
/* renvoie NULL si creer = 0*/
/* la crÃ©e et renvoie son adr avec le type_categ et la derniere sous_categ donnÃ©s si creer = 1 */
/* type_categ = 0=crÃ©dit ; 1 = dÃ©bit ; 2 = spÃ©cial */
/* si on ajoute une categ, on met mise_a_jour_combofix_categ_necessaire Ã  1 */
/* **************************************************************************************************** */

struct struct_categ *categ_par_nom ( gchar *nom_categ,
				     gboolean creer,
				     gint type_categ,
				     gint no_derniere_sous_categ )
{
    if ( nom_categ
	 &&
	 strlen ( nom_categ ) )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_categories, nom_categ,
					  (GCompareFunc) recherche_categorie_par_nom );

	if ( liste_tmp )
	    return liste_tmp -> data;
	else
	{
	    if ( creer )
	    {
		struct struct_categ *nouvelle_categorie;

		nouvelle_categorie = calloc ( 1,
					      sizeof ( struct struct_categ ));

		nouvelle_categorie -> no_categ = ++no_derniere_categorie;
		nouvelle_categorie -> nom_categ = g_strdup ( nom_categ );
		nouvelle_categorie -> type_categ = type_categ;
		nouvelle_categorie -> no_derniere_sous_categ = no_derniere_sous_categ;

		liste_struct_categories = g_slist_append ( liste_struct_categories,
							   nouvelle_categorie );
		nb_enregistrements_categories++;
		mise_a_jour_combofix_categ_necessaire = 1;

/* 		update_category_in_trees ( nouvelle_categorie, NULL ); */

		return ( nouvelle_categorie );
	    }
	}
    }
    return NULL;
}
/* **************************************************************************************************** */
   



/* **************************************************************************************************** */
/* retourne l'adr de la sous categ dont le nom est donnÃ© en argument */
/* si pas trouvÃ©e : */
/* la crÃ©e et renvoie son adr si creer=1 */
/* renvoie NULL si creer = 0 */
/* si on ajoute une categ, on met mise_a_jour_combofix_categ_necessaire Ã  1 */
/* **************************************************************************************************** */

struct struct_sous_categ *sous_categ_par_nom ( struct struct_categ *categ,
					       gchar *nom_sous_categ,
					       gboolean creer )
{
    if ( categ &&
	 nom_sous_categ &&
	 strlen ( nom_sous_categ ))
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( categ -> liste_sous_categ,
					  nom_sous_categ,
					  (GCompareFunc) recherche_sous_categorie_par_nom );

	if ( liste_tmp )
	    return liste_tmp -> data;
	else
	{
	    if ( creer )
	    {
		struct struct_sous_categ *nouvelle_sous_categorie;

		nouvelle_sous_categorie = malloc ( sizeof ( struct struct_sous_categ ));

		nouvelle_sous_categorie -> no_sous_categ = ++( categ -> no_derniere_sous_categ );
		nouvelle_sous_categorie -> nom_sous_categ = g_strdup ( nom_sous_categ );
		nouvelle_sous_categorie -> nb_transactions = 0;
		nouvelle_sous_categorie -> balance = 0.0;

		categ -> liste_sous_categ = g_slist_append ( categ -> liste_sous_categ,
							     nouvelle_sous_categorie );

		mise_a_jour_combofix_categ_necessaire = 1;

/* 		update_category_in_trees ( categ, nouvelle_sous_categorie ); */

		return ( nouvelle_sous_categorie );
	    }
	}
    }

    return NULL;
}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
/* cette fonction renvoie l'adr de la categ demandÃ©e en argument */
/* et NULL si pas trouvÃ©e */
/* **************************************************************************************************** */
struct struct_categ *categ_par_no ( gint no_categorie )
{
    if ( no_categorie )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_categories,
					  GINT_TO_POINTER ( no_categorie ),
					  (GCompareFunc) recherche_categorie_par_no );

	if ( liste_tmp )
	    return ( liste_tmp -> data );
    }
    return NULL;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* cette fonction renvoie l'adr de la sous categ demandÃ©e */
/* ou NULL si la categ ou la sous categ n'existe pas */
/* **************************************************************************************************** */
struct struct_sous_categ *sous_categ_par_no ( gint no_categorie,
					      gint no_sous_categorie )
{
    if ( no_categorie
	 &&
	 no_sous_categorie )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_categories,
					  GINT_TO_POINTER ( no_categorie ),
					  (GCompareFunc) recherche_categorie_par_no );

	if ( liste_tmp )
	{
	    struct struct_categ *categ;

	    categ = liste_tmp -> data;

	    liste_tmp = g_slist_find_custom ( categ -> liste_sous_categ,
					      GINT_TO_POINTER ( no_sous_categorie ),
					      (GCompareFunc) recherche_sous_categorie_par_no );

	    if ( liste_tmp )
		return ( liste_tmp -> data );
	}
    }
    return NULL;
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* cette fonction renvoie une chaine de la forme */
/* soit categ : sous categ */
/* soit categ si no_sous_categorie=0 ou si la sous_categ n'existe pas */
/* ou NULL si la categ n'existe pas */
/* **************************************************************************************************** */
gchar *nom_categ_par_no ( gint no_categorie,
			  gint no_sous_categorie )
{
    struct struct_categ *categ;
    struct struct_sous_categ *sous_categ;

    categ = categ_par_no ( no_categorie );
    sous_categ = sous_categ_par_no ( no_categorie,
				     no_sous_categorie );

    if ( sous_categ )
	/* 	s'il y a une sous categ, c'est qu'il y a une categ... */
	return ( g_strconcat ( categ -> nom_categ,
			       " : ",
			       sous_categ -> nom_sous_categ,
			       NULL ));
    else
	if ( categ )
	    return ( g_strdup ( categ -> nom_categ ));
	
    return NULL;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* cette fonction renvoie une chaine contenant la sous categ */
/* ou NULL si la categ ou la sous categ n'existe pas */
/* **************************************************************************************************** */
gchar *nom_sous_categ_par_no ( gint no_categorie,
			       gint no_sous_categorie )
{
    struct struct_sous_categ *sous_categ;

    sous_categ = sous_categ_par_no ( no_categorie,
				     no_sous_categorie );

    if ( sous_categ )
	    return ( g_strdup ( sous_categ -> nom_sous_categ ));
	
    return NULL;
}
/* **************************************************************************************************** */



/**
 *
 *
 */
void calcule_total_montant_categ ( void )
{
    GSList *list_tmp;

    reset_category_counters();

    without_category = calloc ( 1, sizeof ( struct struct_categ ));
    without_category -> no_categ = 0;
    without_category -> nom_categ = _("No category");
    without_category -> type_categ = 0;
    without_category -> no_derniere_sous_categ = 0;

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	GSList *liste_tmp;

	i = gsb_account_get_no_account ( list_tmp -> data );

	liste_tmp = gsb_account_get_transactions_list (i);
	while ( liste_tmp )
	{
	    struct structure_operation *operation;

	    operation = liste_tmp -> data;

	    if ( gsb_transaction_data_get_category_number ( gsb_transaction_data_get_transaction_number (operation )))
	    {
		struct struct_categ *categorie = NULL;
		struct struct_sous_categ *sous_categorie = NULL;

		/* il y a une catégorie */
		categorie = categ_par_no ( gsb_transaction_data_get_category_number ( gsb_transaction_data_get_transaction_number (operation )));

		/* on ajoute maintenant le montant à la sous categ si elle existe */
		if ( gsb_transaction_data_get_sub_category_number ( gsb_transaction_data_get_transaction_number (operation )))
		    sous_categorie = sous_categ_par_no ( gsb_transaction_data_get_category_number ( gsb_transaction_data_get_transaction_number (operation )), 
							 gsb_transaction_data_get_sub_category_number ( gsb_transaction_data_get_transaction_number (operation )));

		add_transaction_to_category ( operation, categorie, sous_categorie );
	    }
	    else if ( ! gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (operation ))
		      && 
		      ! operation -> relation_no_operation )
	    {
		add_transaction_to_category ( operation, without_category, NULL );
	    }

	    liste_tmp = liste_tmp -> next;
	}
	list_tmp = list_tmp -> next;
    }

}



/**
 *
 *
 */
void remove_transaction_from_category ( struct structure_operation * transaction,
					struct struct_categ * category,
					struct struct_sous_categ * sub_category )
{
    gdouble amount = 
	gsb_transaction_data_get_adjusted_amount ( gsb_transaction_data_get_transaction_number (transaction));

    if ( category )
    {
	category -> nb_transactions --;
	category -> balance -= amount;
	if ( !category -> nb_transactions ) /* Cope with float errors */
	    category -> balance = 0.0;
    }
   
    if ( sub_category )
    {
	sub_category -> nb_transactions --;
	sub_category -> balance -= amount;
	if ( !sub_category -> nb_transactions ) /* Cope with float errors */
	    sub_category -> balance = 0.0;
    }
    else if ( category )
    {
	category -> nb_direct_transactions --;
	category -> direct_balance -= amount;
    }
}



/**
 *
 *
 */
void add_transaction_to_category ( struct structure_operation * transaction,
				   struct struct_categ * category,
				   struct struct_sous_categ * sub_category )
{
    gdouble amount = 
	gsb_transaction_data_get_adjusted_amount ( gsb_transaction_data_get_transaction_number (transaction));

    if ( category )
    {
	category -> nb_transactions ++;
	category -> balance += amount;
    }
   
    if ( sub_category )
    {
	sub_category -> nb_transactions ++;
	sub_category -> balance += amount;
    }
    else if ( category )
    {
	category -> nb_direct_transactions ++;
	category -> direct_balance += amount;
    }
}



/**
 *
 *
 */
void reset_category_counters ()
{
    GSList * tmp;

    free ( without_category );

    tmp = liste_struct_categories;
    while ( tmp )
    {
	struct struct_categ * categ = tmp -> data;
	GSList * sous_categ_list;

	categ -> balance = 0.0;
	categ -> nb_transactions = 0;
	categ -> direct_balance = 0;
	categ -> nb_direct_transactions = 0;

	sous_categ_list = categ -> liste_sous_categ;
	while ( sous_categ_list )
	{
	    struct struct_sous_categ * sous_categ = sous_categ_list -> data;

	    sous_categ -> balance = 0.0;
	    sous_categ -> nb_transactions = 0;

	    sous_categ_list = sous_categ_list -> next;
	}
	
	tmp = tmp -> next;
    }
}



/**
 *
 *
 */
void update_category_in_trees ( struct struct_categ * category, 
				struct struct_sous_categ * sub_category )
{
    GtkTreeIter * categ_iter, * sub_categ_iter, iter;

    if ( category &&
	 gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (categ_tree_model), &iter ) )
    {

	categ_iter = get_iter_from_pointer ( GTK_TREE_MODEL (categ_tree_model), category );
	if ( ! categ_iter )
	{
	    gtk_tree_store_append ( GTK_TREE_STORE(categ_tree_model), &iter, NULL );
	    categ_iter = &iter;
	}
	fill_division_row ( GTK_TREE_MODEL (categ_tree_model), category_interface, categ_iter, category );

	sub_categ_iter = get_iter_from_div ( GTK_TREE_MODEL (categ_tree_model), category -> no_categ,
					     ( sub_category ? 
					       sub_category -> no_sous_categ : 0 ) );
	if ( ! sub_categ_iter )
	{
	    gtk_tree_store_append ( GTK_TREE_STORE(categ_tree_model), &iter, 
				    categ_iter );
	    sub_categ_iter = &iter;
	}
	fill_sub_division_row ( GTK_TREE_MODEL (categ_tree_model), category_interface, sub_categ_iter, 
				category, sub_category );
    }
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
