/* ************************************************************************** */
/* work with the struct of form                                               */
/*                                                                            */
/*                                     data_form                              */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
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

/**
 * \file data_form.c
 * work with the form structure, no GUI here
 */


#include "include.h"



/*START_INCLUDE*/
#include "data_form.h"
#include "operations_formulaire.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
extern gint nb_colonnes;
/*END_EXTERN*/





/** create a new form organization
 * \param none
 * \return the struct of form_organization
 * */
struct organisation_formulaire *gsb_form_new_organization ( void )
{
    struct organisation_formulaire *new_form;
    gint tab[4][6] = { 
	{ TRANSACTION_FORM_DATE, TRANSACTION_FORM_PARTY, TRANSACTION_FORM_CREDIT, TRANSACTION_FORM_DEBIT, 0, 0 },
	{ 0, TRANSACTION_FORM_CATEGORY, TRANSACTION_FORM_TYPE, TRANSACTION_FORM_CHEQUE, 0, 0 },
	{ 0, TRANSACTION_FORM_NOTES, TRANSACTION_FORM_CONTRA, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0 }
    };
    gint i, j;
    gint taille[6] = { 15, 50, 15, 15, 0, 0 };

    new_form = g_malloc ( sizeof ( struct organisation_formulaire ));
    
    new_form -> nb_colonnes = 4;
    new_form -> nb_lignes = 3;

    for ( i = 0 ; i<4 ; i++ )
	for ( j = 0 ; j<6 ; j++ )
	    new_form -> tab_remplissage_formulaire[i][j] = tab[i][j];

    for ( i = 0 ; i<6 ; i++ )
	new_form -> taille_colonne_pourcent[i] = taille[i];

    return new_form;
}


/** duplicate the form organization given in param
 * \param the form organization to duplicate
 * \return the new struct of form_organization
 * */
struct organisation_formulaire *gsb_form_dup_organization ( struct organisation_formulaire *form_organisation )
{
    struct organisation_formulaire *new_form;

    if ( !form_organisation )
	return NULL;

    new_form = g_malloc ( sizeof ( struct organisation_formulaire ));
    
    memcpy ( new_form,
	     form_organisation,
	     sizeof ( struct organisation_formulaire ));
 
    return new_form;
}



