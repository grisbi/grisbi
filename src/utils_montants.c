/* ************************************************************************** */
/*                                  utils_montants.c                         */
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


/*START_INCLUDE*/
#include "utils_montants.h"
#include "utils_devises.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *bouton_ok_equilibrage;
extern GtkWidget *label_equilibrage_ecart;
extern GtkWidget *label_equilibrage_pointe;
extern gdouble operations_pointees;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern gdouble solde_final;
extern gdouble solde_initial;
/*END_EXTERN*/







/******************************************************************************/
/* cette fonction calcule le total des opés P du compte donné en argument */
/* et met à jour les labels d'équilibrage */
/******************************************************************************/
void calcule_total_pointe_compte ( gint no_compte )
{
    GSList *pointeur_liste_ope;

    p_tab_nom_de_compte_variable =  p_tab_nom_de_compte + no_compte;

    pointeur_liste_ope = LISTE_OPERATIONS;
    operations_pointees = 0;

    while ( pointeur_liste_ope )
    {
	struct structure_operation *operation;

	operation = pointeur_liste_ope -> data;

	/* on ne prend en compte l'opé que si c'est pas une opé de ventil */

	if ( operation -> pointe == 1
	     &&
	     !operation -> no_operation_ventilee_associee )
	{
	    gdouble montant;

	    montant = calcule_montant_devise_renvoi ( operation -> montant,
						      DEVISE,
						      operation -> devise,
						      operation -> une_devise_compte_egale_x_devise_ope,
						      operation -> taux_change,
						      operation -> frais_change );

	    operations_pointees = operations_pointees + montant;
	}

	pointeur_liste_ope = pointeur_liste_ope -> next;
    }

    gtk_label_set_text ( GTK_LABEL ( label_equilibrage_pointe ),
			 g_strdup_printf ( "%4.2f", 
					   operations_pointees ));

    if ( fabs ( solde_final - solde_initial - operations_pointees ) < 0.01 )
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			     g_strdup_printf ( "%4.2f",
					       0.0 ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				   TRUE );
    }
    else
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			     g_strdup_printf ( "%4.2f",
					       solde_final - solde_initial - operations_pointees ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				   FALSE );
    }
}
/******************************************************************************/


/******************************************************************************/
/* cette fonction calcule et renvoie le solde du compte donné en argument */
/******************************************************************************/
gdouble calcule_solde_compte ( gint no_compte )
{
    gdouble solde;
    GSList *liste_tmp;

    if ( DEBUG )
	printf ( "calcule_solde_compte %d\n", no_compte );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

    solde = SOLDE_INIT;

    liste_tmp = LISTE_OPERATIONS;

    while ( liste_tmp )
    {
	struct structure_operation *operation;

	operation = liste_tmp -> data;

	/* 	si l'opé est ventilée, on saute */

	if ( !operation -> no_operation_ventilee_associee )
	solde = solde + calcule_montant_devise_renvoi ( operation -> montant,
							DEVISE,
							operation -> devise,
							operation -> une_devise_compte_egale_x_devise_ope,
							operation -> taux_change,
							operation -> frais_change );
	liste_tmp = liste_tmp -> next;
    }

    return ( solde );
}
/******************************************************************************/



/******************************************************************************/
/* cette fonction calcule et renvoie le solde pointé du compte donné en argument */
/******************************************************************************/
gdouble calcule_solde_pointe_compte ( gint no_compte )
{
    gdouble solde;
    GSList *liste_tmp;

    if ( DEBUG )
	printf ( "calcule_solde_pointe_compte %d\n", no_compte );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

    solde = SOLDE_INIT;

    liste_tmp = LISTE_OPERATIONS;

    while ( liste_tmp )
    {
	struct structure_operation *operation;

	operation = liste_tmp -> data;

	/* 	si l'opé est ventilÃ©e ou non pointée, on saute */

	if ( operation -> pointe
	     &&
	     !operation -> no_operation_ventilee_associee )
	solde = solde + calcule_montant_devise_renvoi ( operation -> montant,
							DEVISE,
							operation -> devise,
							operation -> une_devise_compte_egale_x_devise_ope,
							operation -> taux_change,
							operation -> frais_change );
	liste_tmp = liste_tmp -> next;
    }

    return ( solde );
}
/******************************************************************************/


