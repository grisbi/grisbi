/* ************************************************************************** */
/*                                  utils_tiers.c                             */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (dionysos@grisbi.org)	      */
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
#include "utils_tiers.h"
#include "utils_devises.h"
#include "tiers_onglet.h"
#include "search_glist.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern GSList *liste_struct_tiers;
extern struct struct_tiers * without_payee;
extern gint nb_comptes;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern gint no_devise_totaux_tiers;
/*END_EXTERN*/






/* **************************************************************************************************** */
/* renvoie le tiers demandÃ© par son no */
/* ou NULL si pas trouvÃ© */
/* **************************************************************************************************** */
struct struct_tiers *tiers_par_no ( gint no_tiers )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_tiers,
				      GINT_TO_POINTER ( no_tiers ),
				      (GCompareFunc) recherche_tiers_par_no );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* renvoie le tiers demandÃ© par son no */
/* si creer = 1, crÃ©e le tiers si pas trouvÃ© */
/* ou NULL si pb */
/* **************************************************************************************************** */
struct struct_tiers *tiers_par_nom ( gchar *nom_tiers,
				     gboolean creer )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_tiers,
				      g_strstrip ( nom_tiers ),
				      (GCompareFunc) recherche_tiers_par_nom );

    if ( liste_tmp )
	return ( liste_tmp -> data );
    else
    {
	if ( creer )
	{
	    struct struct_tiers *tiers;

	    tiers = ajoute_nouveau_tiers ( nom_tiers );
	    return ( tiers );
	}
    }

    return NULL;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* retourne le tiers en donnant comme argument son numÃ©ro */
/* retour : soit le nom du tiers
 * 	    soit No third party defined si return_null est FALSE et pas de tiers trouvÃ©,
 * 	    soit NULL si return_null est TRUE et pas de tiers trouvÃ© */
/* **************************************************************************************************** */

gchar *tiers_name_by_no ( gint no_tiers,
			  gboolean return_null )
{
    struct struct_tiers *tiers;

    tiers = tiers_par_no ( no_tiers );

    if (tiers)
	return ( tiers->nom_tiers );
    else
	if ( return_null )
	    return NULL;
	else
	    return ( g_strdup (_("No third party defined")));
}



/**
 *
 *
 */
void calcule_total_montant_payee ( void )
{
    gint i;

    reset_payee_counters();

    without_payee = calloc ( 1, sizeof ( struct struct_tiers ));
    without_payee -> no_tiers = 0;
    without_payee -> nom_tiers = _("No payee");

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	GSList *liste_tmp;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	liste_tmp = LISTE_OPERATIONS;
	while ( liste_tmp )
	{
	    struct structure_operation *operation;

	    operation = liste_tmp -> data;

	    if ( operation -> tiers )
	    {
		struct struct_tiers * payee = NULL;

		/* il y a une catégorie */
		payee = tiers_par_no ( operation -> tiers );

		add_transaction_to_payee ( operation, payee );
	    }
	    else if ( ! operation -> operation_ventilee && 
		      ! operation -> relation_no_operation )
	    {
		add_transaction_to_payee ( operation, without_payee );
	    }

	    liste_tmp = liste_tmp -> next;
	}
    }
}



/**
 *
 *
 */
void add_transaction_to_payee ( struct structure_operation * transaction,
				struct struct_tiers * payee )
{
    gdouble amount = 
	calcule_montant_devise_renvoi ( transaction -> montant, no_devise_totaux_tiers,
					transaction -> devise,
					transaction -> une_devise_compte_egale_x_devise_ope,
					transaction -> taux_change,
					transaction -> frais_change );

    if ( payee )
    {
	payee -> nb_transactions ++;
	payee -> balance += amount;
    }
    else 
    {
	without_payee -> nb_transactions ++;
	without_payee -> balance ++;
    }
}



/**
 *
 *
 */
void remove_transaction_from_payee ( struct structure_operation * transaction,
				     struct struct_tiers * payee )
{
    gdouble amount = 
	calcule_montant_devise_renvoi ( transaction -> montant, no_devise_totaux_tiers,
					transaction -> devise,
					transaction -> une_devise_compte_egale_x_devise_ope,
					transaction -> taux_change,
					transaction -> frais_change );

    if ( payee )
    {
	payee -> nb_transactions --;
	payee -> balance -= amount;
	if ( !payee -> nb_transactions ) /* Cope with float errors */
	    payee -> balance = 0.0;
    }
    else
    {
	without_payee -> nb_transactions --;
	without_payee -> balance -= amount;
	if ( !without_payee -> nb_transactions ) /* Cope with float errors */
	    without_payee -> balance = 0.0;
    }
}



/**
 *
 *
 */
void reset_payee_counters ()
{
    GSList * tmp;

    free ( without_payee );

    tmp = liste_struct_tiers;
    while ( tmp )
    {
	struct struct_tiers * payee = tmp -> data;

	payee -> balance = 0.0;
	payee -> nb_transactions = 0;

	tmp = tmp -> next;
    }
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
