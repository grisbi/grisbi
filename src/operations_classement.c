/* ************************************************************************** */
/* Contient toutes les fonctions utilisées pour classer la liste des opé      */
/* 			classement_liste.c                                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (dionysos@grisbi.org) 	      */
/*			http://www.grisbi.org   			      */
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
#include "gtkcombofix.h"
#include "operations_classement.h"




/* ************************************************************************** */
/* Fonction de classement par défaut : par ordre de date                      */
/* ************************************************************************** */
gint classement_liste_par_date ( GtkWidget *liste,
				 GtkCListRow *ligne_1,
				 GtkCListRow *ligne_2 )
{
    struct structure_operation *operation_1;
    struct structure_operation *operation_2;
    gint retour;

    operation_1 = ligne_1 -> data;
    operation_2 = ligne_2 -> data;


    if ( operation_1 == GINT_TO_POINTER ( -1 ) )
	return ( 1 );

    if ( operation_2 == GINT_TO_POINTER ( -1 ) )
	return ( -1 );

    if ( etat.classement_par_date )
	/* on classe par date d'opération */
	retour = g_date_compare ( operation_1 -> date, operation_2 -> date );
    else
    {
	/* on classe par date bancaire, si elle existe */

	if ( operation_1 -> date_bancaire )
	{
	    if ( operation_2 -> date_bancaire )
		retour = g_date_compare ( operation_1 -> date_bancaire, operation_2 -> date_bancaire );
	    else
		retour = g_date_compare ( operation_1 -> date_bancaire, operation_2 -> date );
	}
	else
	{
	    if ( operation_2 -> date_bancaire )
		retour = g_date_compare ( operation_1 -> date, operation_2 -> date_bancaire );
	    else
		retour = g_date_compare ( operation_1 -> date, operation_2 -> date );
	}
    }

    if ( retour )
	return ( retour );
    else
	return ( operation_1 -> no_operation - operation_2 -> no_operation );
}
/* ************************************************************************** */

/* ************************************************************************** */
/* classement par no d'opé (donc d'entrée)                                    */
/* ************************************************************************** */
gint classement_liste_par_no_ope ( GtkWidget *liste,
				   GtkCListRow *ligne_1,
				   GtkCListRow *ligne_2 )
{
    struct structure_operation *operation_1;
    struct structure_operation *operation_2;

    operation_1 = ligne_1 -> data;
    operation_2 = ligne_2 -> data;

    if ( operation_1 == GINT_TO_POINTER ( -1 ) )
	return ( 1 );

    if ( operation_2 == GINT_TO_POINTER ( -1 ) )
	return ( -1 );

    if ( operation_1 == NULL )
	return ( 1 );

    if ( operation_2 == NULL )
	return ( -1 );

    return ( operation_1 -> no_operation - operation_2 -> no_operation );
}
/* ************************************************************************** */

/* ************************************************************************** */
/* classement par no d'opé (donc d'entrée)                                    */
/* ************************************************************************** */
gint classement_liste_par_no_ope_ventil ( GtkWidget *liste,
					  GtkCListRow *ligne_1,
					  GtkCListRow *ligne_2 )
{
    struct operation_echeance *operation_1;
    struct operation_echeance *operation_2;

    operation_1 = ligne_1 -> data;
    operation_2 = ligne_2 -> data;

    if ( operation_1 == GINT_TO_POINTER ( -1 ) )
	return ( 1 );

    if ( operation_2 == GINT_TO_POINTER ( -1 ) )
	return ( -1 );

    if ( operation_1 == NULL )
	return ( 1 );

    if ( operation_2 == NULL )
	return ( -1 );

    return ( operation_1 -> no_operation - operation_2 -> no_operation );
}
/* ************************************************************************** */


/* ************************************************************************** */
gint classement_liste_par_tri_courant ( GtkWidget *liste,
					GtkCListRow *ligne_1,
					GtkCListRow *ligne_2 )
{
    struct structure_operation *operation_1;
    struct structure_operation *operation_2;
    gint pos_type_ope_1;
    gint pos_type_ope_2;
    gint buffer;

    operation_1 = ligne_1 -> data;
    operation_2 = ligne_2 -> data;

    if ( operation_1 == GINT_TO_POINTER ( -1 ) )
	return ( 1 );

    if ( operation_2 == GINT_TO_POINTER ( -1 ) )
	return ( -1 );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    /* si l'opération est négative et que le type est neutre et que les types
       neutres sont séparés, on lui met la position du type négatif */

    if ( operation_1 -> montant < 0
	 && ( buffer = g_slist_index ( LISTE_TRI, GINT_TO_POINTER ( -operation_1 -> type_ope ))) != -1 )
	pos_type_ope_1 = buffer;
    else
	pos_type_ope_1 = g_slist_index ( LISTE_TRI, GINT_TO_POINTER ( operation_1 -> type_ope ));

    if ( operation_2 -> montant < 0
	 && ( buffer = g_slist_index ( LISTE_TRI, GINT_TO_POINTER ( -operation_2 -> type_ope ))) != -1 )
	pos_type_ope_2 = buffer;
    else
	pos_type_ope_2 = g_slist_index ( LISTE_TRI, GINT_TO_POINTER ( operation_2 -> type_ope ));

    /* s'elles ont le même type, on les classe par date */

    if ( pos_type_ope_1 == pos_type_ope_2 )
    {
	gint retour;

	if ( etat.classement_par_date )
	    /* on classe par date d'opération */
	    retour = g_date_compare ( operation_1 -> date, operation_2 -> date );
	else
	{
	    /* on classe par date bancaire, si elle existe */
	    if ( operation_1 -> date_bancaire )
	    {
		if ( operation_2 -> date_bancaire )
		    retour = g_date_compare ( operation_1 -> date_bancaire, operation_2 -> date_bancaire );
		else
		    retour = g_date_compare ( operation_1 -> date_bancaire, operation_2 -> date );
	    }
	    else
	    {
		if ( operation_2 -> date_bancaire )
		    retour = g_date_compare ( operation_1 -> date, operation_2 -> date_bancaire );
		else
		    retour = g_date_compare ( operation_1 -> date, operation_2 -> date );
	    }
	}

	if ( retour )
	    return ( retour );
	else
	    return ( operation_1 -> no_operation - operation_2 -> no_operation );
    }

    if ( pos_type_ope_1 < pos_type_ope_2 )
	return ( -1 );
    else
	return ( 1 );
}
/* ************************************************************************** */

/* ************************************************************************** */
/* cette fonction est appelée pour classer les opérations dans la sliste      */
/* avant que cette liste ne soit affichée ; trie automatiquement la liste     */
/* en fonction du moment                                                      */
/* ************************************************************************** */
gint classement_sliste ( struct structure_operation *operation_1,
			 struct structure_operation *operation_2 )
{
    gpointer **save_ptab;
    gint result;

    save_ptab = p_tab_nom_de_compte_variable;

    /* pour l'instant, soit on est en train d'équilibrer et c'est par tri courant,
       uniquement si p_tab... est sur le compte courant
       sinon c'est par date (qui utilise la date de valeur si nécessaire) */

    if ( etat.equilibrage
	 && ( p_tab_nom_de_compte_variable - p_tab_nom_de_compte ) == compte_courant )
	result = classement_sliste_par_tri_courant ( operation_1, operation_2 );
    else
    {
	if ( etat.classement_rp )
	    result = classement_sliste_par_date_rp ( operation_1, operation_2 );
	else
	    result = classement_sliste_par_date ( operation_1, operation_2 );
    }

    p_tab_nom_de_compte_variable = save_ptab;

    return ( result );
}
/* ************************************************************************** */

/* ************************************************************************** */
/* Fonction par défaut : par ordre de date                                    */
/* et classe en fonction de r/p si demandé (FIXME : virer dans l'instable) */
/* ************************************************************************** */
gint classement_sliste_par_date ( struct structure_operation *operation_1,
				  struct structure_operation *operation_2 )
{
    gint retour;

    if ( etat.classement_par_date )
	/* on classe par dates normales */
	retour = g_date_compare ( operation_1 -> date, operation_2 -> date );
    else
    {
	/* on classe par date de valeur, si elle existe */

	if ( operation_1 -> date_bancaire )
	{
	    if ( operation_2 -> date_bancaire )
		retour = g_date_compare ( operation_1 -> date_bancaire, operation_2 -> date_bancaire );
	    else
		retour = g_date_compare ( operation_1 -> date_bancaire, operation_2 -> date );
	}
	else
	{
	    if ( operation_2 -> date_bancaire )
		retour = g_date_compare ( operation_1 -> date, operation_2 -> date_bancaire );
	    else
		retour = g_date_compare ( operation_1 -> date, operation_2 -> date );
	}

	/* on a classé par date de valeur, si c'est la même date,
	   alors on classe par date d'opération */

	if ( !retour )
	    retour = g_date_compare ( operation_1 -> date, operation_2 -> date );
    }

    if ( retour )
	return ( retour );
    else
	return ( operation_1 -> no_operation - operation_2 -> no_operation );
}
/* ************************************************************************** */



/* ************************************************************************** */
/* identique à classement_liste_par_tri_courant sauf que classe une slist     */
/* ************************************************************************** */
gint classement_sliste_par_tri_courant ( struct structure_operation *operation_1,
					 struct structure_operation *operation_2 )
{
    gint pos_type_ope_1;
    gint pos_type_ope_2;
    gint buffer;
    gint sort_result;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    /* On classe soit par le tri désiré, soit par date,
       et dans ce cas on renvoie au tri normal */

    if ( !TRI || !LISTE_TRI )
    {
	if ( etat.classement_rp )
	    sort_result = classement_sliste_par_date_rp ( operation_1, operation_2 );
	else
	    sort_result = classement_sliste_par_date ( operation_1, operation_2 );
    }
    else
    {
	/* si l'opération est négative et que le type est neutre et que les types
	   neutres sont séparés, on lui met la position du type négatif */

	if ( operation_1 -> montant < 0
	     && ( buffer = g_slist_index ( LISTE_TRI, GINT_TO_POINTER ( -operation_1 -> type_ope ))) != -1 )
	    pos_type_ope_1 = buffer;
	else
	    pos_type_ope_1 = g_slist_index ( LISTE_TRI, GINT_TO_POINTER ( operation_1 -> type_ope ));

	if ( operation_2 -> montant < 0
	     && ( buffer = g_slist_index ( LISTE_TRI, GINT_TO_POINTER ( -operation_2 -> type_ope ))) != -1 )
	    pos_type_ope_2 = buffer;
	else
	    pos_type_ope_2 = g_slist_index ( LISTE_TRI, GINT_TO_POINTER ( operation_2 -> type_ope ));

	/* s'elles ont le même type, on les classe par date */

	if ( pos_type_ope_1 == pos_type_ope_2 )
	{
	    if ( etat.classement_rp )
		sort_result = classement_sliste_par_date_rp ( operation_1, operation_2 );
	    else
		sort_result = classement_sliste_par_date ( operation_1, operation_2 );
	}
	else
	{
	    if ( pos_type_ope_1 < pos_type_ope_2 )
		sort_result = -1;
	    else
		sort_result = 1;
	}
    }
    return( sort_result );
}
/* ************************************************************************** */

/* ************************************************************************** */
/* classement d'une liste composée de chaines par ordre alphabétique          */
/* en prenant en compte les accents                                           */
/* ************************************************************************** */
gint classe_liste_alphabetique ( gchar *string_1,
				 gchar *string_2 )
{
    /* comme la fonction g_strcasecmp met les accents derrière,
       on magouille pour les prendre en compte */

    string_1 = g_strdup ( string_1 );
    string_1 = g_strdelimit ( string_1, "éÉèÈêÊ", 'e' );
    string_1 = g_strdelimit ( string_1, "çÇ", 'c' );
    string_1 = g_strdelimit ( string_1, "àÀ", 'a' );
    string_1 = g_strdelimit ( string_1, "ùûÙÛ", 'u' );
    string_1 = g_strdelimit ( string_1, "ôÔ", 'o' );
    string_1 = g_strdelimit ( string_1, "îÎ", 'i' );

    string_2 = g_strdup ( string_2 );
    string_2 = g_strdelimit ( string_2, "éÉèÈêÊ", 'e' );
    string_2 = g_strdelimit ( string_2, "çÇ", 'c' );
    string_2 = g_strdelimit ( string_2, "àÀ", 'a' );
    string_2 = g_strdelimit ( string_2, "ùûÙÛ", 'u' );
    string_2 = g_strdelimit ( string_2, "ôÔ", 'o' );
    string_2 = g_strdelimit ( string_2, "îÎ", 'i' );

    return ( g_strcasecmp ( string_1, string_2 ));
}
/* ************************************************************************** */


/* ************************************************************************** */
/* par ordre de date en tenant compte de l'état         */
/* (pointé, rapproché) des opérations. Les opérations rapprochées seront      */
/* toujours classées en premier, suivies par les opérations pointées, puis    */
/* par les opérations qui ne sont ni l'une, ni l'autre.                       */
/* ************************************************************************** */
gint classement_sliste_par_date_rp ( struct structure_operation *pTransaction1,
				     struct structure_operation *pTransaction2 )
{
    gint sort_result;

    /* si l'opération 1 est rapprochée alors que l'opération 2 ne l'est pas,
       ou si l'opération 1 est pointée et l'opération 2 n'est ni pointée, ni
       rapprochée, alors on dit que l'opération 1 est antérieure */
    if ( ( pTransaction1 -> pointe == 2 && pTransaction2 -> pointe != 2 )
	 || ( pTransaction1 -> pointe == 1 && pTransaction2 -> pointe == 0 ))
    {
	sort_result = -1;
    }
    else
    {
	/* même raisonnement que ci-dessus, sauf que l'on interverti opération 1
	   et opération 2 */
	if ( ( pTransaction2 -> pointe == 2 && pTransaction1 -> pointe != 2 )
	     || ( pTransaction2 -> pointe == 1 && pTransaction1 -> pointe == 0 ))
	{
	    sort_result = 1;
	}
	else
	{
	    /* les deux opérations sont toutes les deux dans le même état
	       (rapprochées, pointées ou ni l'un ni l'autre),
	       alors on les classe par date */
	    sort_result = classement_sliste_par_date( pTransaction1, pTransaction2);
	}
    }
    return( sort_result );
}
/* ************************************************************************** */

