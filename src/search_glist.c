/* ************************************************************************** */
/*   									      */
/*  					                                      */
/*                                                                            */
/*                                  search_glist.c                            */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
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
#include "structures.h"
#include "variables-extern.c"

#include "devises.h"
#include "search_glist.h"
#include "utils.h"






/***********************************************************************************************************/
/* Fonction recherche_categorie_par_nom */
/***********************************************************************************************************/

gint recherche_categorie_par_nom ( struct struct_categ *categorie,
				   gchar *categ )
{
    return ( g_strcasecmp ( categorie -> nom_categ,
			    categ ) );
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* Fonction recherche_sous_categorie_par_nom */
/***********************************************************************************************************/

gint recherche_sous_categorie_par_nom ( struct struct_sous_categ *sous_categorie,
					gchar *sous_categ )
{
    return ( g_strcasecmp ( sous_categorie -> nom_sous_categ,
			    sous_categ ) );
}
/***********************************************************************************************************/



/***************************************************************************************************/
/* Fonction  recherche_categorie_par_no */
/* appelée par un g_slist_find_custom */
/* donne en arg la struct du tiers et le no du tiers recherché */
/***************************************************************************************************/

gint recherche_categorie_par_no ( struct struct_categ *categorie,
				  gint *no_categorie )
{
    return (  categorie ->  no_categ != GPOINTER_TO_INT ( no_categorie ) );
}
/***************************************************************************************************/


/***************************************************************************************************/
/* Fonction  recherche_sous_categorie_par_no */
/* appelée par un g_slist_find_custom */
/* donne en arg la struct du tiers et le no du tiers recherché */
/***************************************************************************************************/

gint recherche_sous_categorie_par_no ( struct struct_sous_categ *sous_categorie,
				       gint *no_sous_categorie )
{
    return (  sous_categorie ->  no_sous_categ != GPOINTER_TO_INT ( no_sous_categorie ) );
}
/***************************************************************************************************/





/* ************************************************************************************************************ */
/* Fonction recherche_banque_par_no */
/* appelée par g_slist_find_custom */
/* ************************************************************************************************************ */

gint recherche_banque_par_no ( struct struct_banque *banque,
			       gint *no_banque )
{
    return ( !( banque -> no_banque == GPOINTER_TO_INT ( no_banque )) );
}
/* ************************************************************************************************************ */




/* ************************************************************************************************************ */
gint recherche_devise_par_nom ( struct struct_devise *devise,
				gchar *nom )
{
    return ( g_strcasecmp ( g_strstrip ( devise_name(devise) ), nom ) );
}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
gint recherche_devise_par_code ( struct struct_devise *devise,
				gchar *code )
{
    return ( g_strcasecmp ( g_strstrip ( devise_code(devise) ), code ) );
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
gint recherche_devise_par_code_iso ( struct struct_devise *devise,
				     gchar *nom )
{
    if ( devise -> code_iso4217_devise )
	return ( g_strcasecmp ( g_strstrip ( devise -> code_iso4217_devise ),
				nom ) );
    else
	return (-1);
}
/* ************************************************************************************************************ */



/***********************************************************************************************************/
/* Fonction recherche_devise_par_no */
/* appelÃ©e par un g_slist_find_custom */
/***********************************************************************************************************/

gint recherche_devise_par_no ( struct struct_devise *devise,
			       gint *no_devise )
{
    return ( devise -> no_devise != GPOINTER_TO_INT ( no_devise ));
}
/***********************************************************************************************************/




/*****************************************************************************/
gint recherche_echeance_par_no ( struct operation_echeance *echeance,
				 gint no_echeance )
{
    return ( !(echeance -> no_operation == no_echeance ));
}
/*****************************************************************************/


/* *********************************************************************************************************** */
/* cette fonction est appelée pour chercher dans les échéances si certaines sont  */
/* associées au compte en train d'être supprimé */
/* *********************************************************************************************************** */

gint recherche_echeance_par_no_compte ( struct operation_echeance *echeance,
					gint no_compte )
{
    return ( echeance -> compte != no_compte );
}
/* *********************************************************************************************************** */



/******************************************************************************/
gint recherche_rapprochement_par_nom ( struct struct_no_rapprochement *rapprochement,
				       gchar *no_rap )
{
    return ( strcmp ( rapprochement -> nom_rapprochement,
		      no_rap ));
}
/******************************************************************************/

/******************************************************************************/
gint recherche_rapprochement_par_no ( struct struct_no_rapprochement *rapprochement,
				      gint *no_rap )
{
    return ( !(rapprochement -> no_rapprochement == GPOINTER_TO_INT ( no_rap )));
}
/******************************************************************************/




/* ************************************************************************************************************ */
gint recherche_exercice_par_nom ( struct struct_exercice *exercice,
				  gchar *nom )
{
    return ( g_strcasecmp ( g_strstrip ( exercice -> nom_exercice ),
			    nom ) );
}
/* ************************************************************************************************************ */



/***********************************************************************************************************/
/* Fonction recherche_exercice_par_no */
/* appelée par un g_slist_find_custom */
/***********************************************************************************************************/

gint recherche_exercice_par_no ( struct struct_exercice *exercice,
				 gint *no_exercice )
{
    return ( exercice -> no_exercice != GPOINTER_TO_INT ( no_exercice ));
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* Fonction recherche_imputation_par_nom */
/***********************************************************************************************************/

gint recherche_imputation_par_nom ( struct struct_imputation *imputation,
				    gchar *nom_imputation )
{
    return ( g_strcasecmp ( imputation -> nom_imputation,
			    nom_imputation ) );
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* Fonction recherche_sous_imputation_par_nom */
/***********************************************************************************************************/

gint recherche_sous_imputation_par_nom ( struct struct_sous_imputation *sous_imputation,
					 gchar *nom_sous_imputation )
{
    return ( g_strcasecmp ( sous_imputation -> nom_sous_imputation,
			    nom_sous_imputation ) );
}
/***********************************************************************************************************/



/***************************************************************************************************/
/* Fonction  recherche_imputation_par_no */
/* appelée par un g_slist_find_custom */
/* donne en arg la struct du tiers et le no du tiers recherché */
/***************************************************************************************************/

gint recherche_imputation_par_no ( struct struct_imputation *imputation,
				   gint *no_imputation )
{
    return (  imputation ->  no_imputation != GPOINTER_TO_INT ( no_imputation ) );
}
/***************************************************************************************************/


/***************************************************************************************************/
/* Fonction  recherche_imputation_par_no */
/* appelée par un g_slist_find_custom */
/* donne en arg la struct du tiers et le no du tiers recherché */
/***************************************************************************************************/

gint recherche_sous_imputation_par_no ( struct struct_sous_imputation *sous_imputation,
					gint *no_sous_imputation )
{
    return (  sous_imputation ->  no_sous_imputation != GPOINTER_TO_INT ( no_sous_imputation ) );
}
/***************************************************************************************************/




/******************************************************************************/
/* Fonction recherche_operation_par_no					      */
/* appelée par un slist_find_custom					      */
/* recherche une opé par son numéro d'opé dans la liste des opérations	      */
/******************************************************************************/
gint recherche_operation_par_no ( struct structure_operation *operation,
				  gint *no_ope )
{
    return ( ! ( operation -> no_operation == GPOINTER_TO_INT ( no_ope ) ));
}
/******************************************************************************/



/******************************************************************************/
/* Fonction recherche_operation_par_cheque					      */
/* appelée par un slist_find_custom					      */
/* recherche une opé par son numéro de cheque dans la liste des opérations	      */
/******************************************************************************/
gint recherche_operation_par_cheque ( struct structure_operation *operation,
				      gint *no_chq )
{
  if ( operation -> contenu_type )
    return ( ! ( my_atoi (operation -> contenu_type) == GPOINTER_TO_INT ( no_chq ) ));
  return -1;
}
/******************************************************************************/





/* *******************************************************************************/
gint recherche_operation_par_id ( struct structure_operation *operation,
				      gchar *id_recherchee )
{
    if ( operation -> id_operation )
	return ( strcmp ( id_recherchee,
			  operation -> id_operation ));
    else
	return -1;
}
/* *******************************************************************************/


/***************************************************************************************************/
/* Fonction recherche tiers par no */
/* appelée par un g_slist_find_custom */
/* donne en arg la struct du tiers et le no du tiers recherché */
/***************************************************************************************************/

gint recherche_tiers_par_no ( struct struct_tiers *tiers,
			      gint *no_tiers )
{
    return ( tiers -> no_tiers != GPOINTER_TO_INT ( no_tiers ) );
}
/***************************************************************************************************/



/* **************************************************************************************************** */
/* Fonction recherche_tiers_par_nom */
/* appelée par un g_slist_find_custom */
/* prend en arg la struct du tiers et le nom du tiers recherché */
/* **************************************************************************************************** */

gint recherche_tiers_par_nom ( struct struct_tiers *tiers,
			       gchar *ancien_tiers )
{
    return ( g_strcasecmp ( tiers -> nom_tiers,
			    ancien_tiers ) );
}
/* **************************************************************************************************** */


/* ************************************************************************************************************** */
gint recherche_type_ope_par_no ( struct struct_type_ope *type_ope,
				 gint *no_type )
{
    return ( !(type_ope->no_type == GPOINTER_TO_INT(no_type)) );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
gint cherche_string_equivalente_dans_slist ( gchar *string_list,
					     gchar *string_cmp )
{
    return ( strcmp ( string_list,
		      string_cmp ));
}
/* ************************************************************************************************************** */

