/* ************************************************************************** */
/*                                  utils_devises.c                           */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-monpt2.fr)	      */
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
#include "utils_devises.h"
#include "search_glist.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern struct struct_devise *devise_compte;
extern struct struct_devise *devise_operation;
extern GSList *liste_struct_devises;
/*END_EXTERN*/






/* **************************************************************************************************************************** */
/* cette fonction prend en argument un montant, la devise de renvoi (en gÃ©nÃ©ral la devise du compte) */
/*      et la devise du montant donnÃ© en argument */
/* elle renvoie le montant de l'opÃ©ration dans la devise de renvoi */
/* **************************************************************************************************************************** */

gdouble calcule_montant_devise_renvoi ( gdouble montant_init,
					gint no_devise_renvoi,
					gint no_devise_montant,
					gint une_devise_compte_egale_x_devise_ope,
					gdouble taux_change,
					gdouble frais_change )
{
    gdouble montant = 0;

    /* tout d'abord, si les 2 devises sont les mÃÂªmes, on renvoie le montant directement */

    if ( no_devise_renvoi == no_devise_montant )
	return ( montant_init );

    /*   il faut faire une transformation du montant */
    /* on utilise les variables globales devise_compte et devise_operation pour */
    /* gagner du temps */

    /* rÃÂ©cupÃÂšre la devise du compte si nÃÂ©cessaire */

    if ( !devise_compte
	 ||
	 devise_compte -> no_devise != no_devise_renvoi )
	devise_compte = devise_par_no ( no_devise_renvoi );

    /* rÃÂ©cupÃÂšre la devise de l'opÃÂ©ration si nÃÂ©cessaire */

    if ( !devise_operation
	 ||
	 devise_operation -> no_devise != no_devise_montant )
	devise_operation = devise_par_no ( no_devise_montant );

    /* on a maintenant les 2 devises, on peut faire les calculs */

    if ( devise_compte && devise_compte -> passage_euro
	 &&
	 !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
	montant = montant_init * devise_compte -> change;
    else
	if ( devise_operation && devise_operation -> passage_euro
	     &&
	     !strcmp ( devise_compte -> nom_devise, _("Euro") ))
	    montant = montant_init / devise_operation -> change;
	else
	    if ( taux_change )
	    {
		if ( une_devise_compte_egale_x_devise_ope )
		    montant = montant_init / taux_change - frais_change;
		else
		    montant = montant_init * taux_change - frais_change;
	    }
	    else
	    {
		if ( devise_operation &&
		     devise_operation -> no_devise_en_rapport == no_devise_renvoi &&
		     devise_operation -> change )
		{
		    if ( devise_operation -> une_devise_1_egale_x_devise_2 )
			montant = montant_init * devise_operation -> change - frais_change;
		    else
			montant = montant_init / devise_operation -> change - frais_change;
		} 
	    }

    montant = ( rint (montant * 100 )) / 100;

    return ( montant);
}
/* ***************************************************************************************** */



/* ***************************************************************************************** */
/* renvoie la devise correspondant au no */
/* renvoie NULL si pas trouvÃ©e */
/* ***************************************************************************************** */
struct struct_devise *devise_par_no ( gint no_devise )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_devises,
				      GINT_TO_POINTER ( no_devise ),
				      (GCompareFunc) recherche_devise_par_no );

    if ( liste_tmp )
	return ( liste_tmp -> data );
 
    return NULL;
}
/* ***************************************************************************************** */



/* ***************************************************************************************** */
/* renvoie la devise correspondant au nom */
/* renvoie NULL si pas trouvÃ©e */
/* ***************************************************************************************** */
struct struct_devise *devise_par_nom ( gchar *nom_devise )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_devises,
				      g_strstrip ( nom_devise ),
				      (GCompareFunc) recherche_devise_par_nom );

    if ( liste_tmp )
	return ( liste_tmp -> data );
 
    return NULL;
}
/* ***************************************************************************************** */



/* ***************************************************************************************** */
/* renvoie la devise correspondant au code iso */
/* renvoie NULL si pas trouvÃ©e */
/* ***************************************************************************************** */
struct struct_devise *devise_par_code_iso ( gchar *code_iso )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_devises,
				      g_strstrip ( code_iso ),
				      (GCompareFunc) recherche_devise_par_code_iso );

    if ( liste_tmp )
	return ( liste_tmp -> data );
 
    return NULL;
}
/* ***************************************************************************************** */



/**
 * Return either currency's code or currency's ISO4217 nickname if no
 * name is found.
 *
 * \param devise A pointer to a struct_devise holding currency
 * informations.
 *
 * \return name or ISO4217 name of currency.
 * or NULL if devise is NULL
 */
gchar * devise_code ( struct struct_devise * devise )
{
    if ( devise )
    {
	if ( devise -> code_devise && (strlen(devise -> code_devise) > 0))
	    return devise -> code_devise;

	return devise -> code_iso4217_devise;
    }
    return NULL;
}
/* ***************************************************************************************** */




/* ***************************************************************************************** */
/* renvoie le code de la devise correspondante au no */
/* ou null si pas trouvÃ©e */
/* ***************************************************************************************** */

gchar * devise_code_by_no ( gint no_devise )
{
    return ( devise_code ( devise_par_no ( no_devise )));
}
/* ***************************************************************************************** */


/* ***************************************************************************************** */
/* renvoie le nom de la devise donnÃ© en argument */
/* ***************************************************************************************** */
gchar * devise_name ( struct struct_devise * devise )
{
    if ( devise )
    {
	if ( devise -> nom_devise && (strlen(devise -> nom_devise) > 0))
	    return devise -> nom_devise;

	return devise -> code_iso4217_devise;
    }
    return NULL;
}
/* ***************************************************************************************** */

/* ***************************************************************************************** */
gboolean is_euro ( struct struct_devise * currency )
{
    return (gboolean) !strcmp ( currency -> nom_devise, _("Euro"));
}
/* ***************************************************************************************** */




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
