/* ************************************************************************** */
/*                                  utils_exercices.c                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
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
#include "utils_exercices.h"
#include "search_glist.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *formulaire;
extern GSList *liste_struct_exercices;
extern GtkWidget *nom_exercice;
/*END_EXTERN*/







/* ************************************************************************************************************** */
/* Fonction cherche_no_menu_exercice */
/*   argument : le numÃ©ro de l'exercice demandÃ© */
/* renvoie la place demandÃ©e dans l'option menu du formulaire */
/* pour mettre l'history */
/* ************************************************************************************************************** */

gint cherche_no_menu_exercice ( gint no_demande,
				GtkWidget *option_menu )
{
    GList *liste_tmp;
    gint trouve;
    gint non_affiche;
    gint i;

    liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( option_menu ) -> menu ) -> children;
    i= 0;
    non_affiche = 0;

    while ( liste_tmp )
    {

	trouve = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
							 "no_exercice" ));

	/*       si trouve = no demandÃ©, c'est bon, on se barre */

	if ( trouve == no_demande )
	    return ( i );

	/*  si on est sur la position du non affichÃ©, on le sauve */

	if ( trouve == -1 )
	    non_affiche = i;

	i++;
	liste_tmp = liste_tmp -> next;
    }

    /*   l'exo n'est pas affichÃ©, on retourne la position de non affichÃ© */

    return ( non_affiche );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* fonction recherche_exo_correspondant */
/* renvoie l'exercice correspondant la date donnÃ©e en argument */
/* si aucun ne correspond, on renvoie 0 */
/* ************************************************************************************************************** */

gint recherche_exo_correspondant ( GDate *date )
{
    GSList *liste_tmp;

    liste_tmp = liste_struct_exercices;

    while ( liste_tmp )
    {
	struct struct_exercice *exo;

	exo = liste_tmp -> data;

	if ( g_date_compare ( exo -> date_debut,
			      date ) <= 0
	     &&
	     g_date_compare ( exo -> date_fin,
			      date ) >= 0 )
	    return ( exo -> no_exercice );

	liste_tmp = liste_tmp -> next;
    }

    /*   on n'a pas trouvÃ© l'exo, on retourne 0 */

    return ( 0 );

}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
/* renvoie l'adr de l'exo demandÃ© par son no */
/* ou NULL si pas trouvÃ© */
/* ************************************************************************************************************** */
struct struct_exercice *exercice_par_no ( gint no_exo )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_exercices,
				      GINT_TO_POINTER ( no_exo ),
				      (GCompareFunc) recherche_exercice_par_no );

    if ( liste_tmp )
	return ( liste_tmp -> data );
 
    return NULL;
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* renvoie l'adr de l'exo demandÃ© par son nom */
/* ou NULL si pas trouvÃ© */
/* ************************************************************************************************************** */
struct struct_exercice *exercice_par_nom ( gchar *nom_exo )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_exercices,
				      g_strstrip ( nom_exo ),
				      (GCompareFunc) recherche_exercice_par_nom );

    if ( liste_tmp )
	return ( liste_tmp -> data );
 
    return NULL;
}
/* ************************************************************************************************************** */

/* ************************************************************************************************************** */
/* renvoie le nom de l'exercice donnÃ© en argument */
/* ou bien null si non trouvÃ© */
/* ************************************************************************************************************** */

gchar *exercice_name_by_no ( gint no_exo )
{
    struct struct_exercice *exo;

    exo = exercice_par_no ( no_exo );

    if ( exo )
	return ( g_strdup (exo -> nom_exercice ));

    return NULL;
}
/* ************************************************************************************************************** */


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
