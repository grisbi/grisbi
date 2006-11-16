/* ************************************************************************** */
/*                                  utils_comptes.c                           */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "utils_comptes.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint recherche_compte_dans_option_menu ( GtkWidget *option_menu,
					 gint no_compte );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/






/* ************************************************************************** */
/* cette fonction est appelÃ©e pour mettre un option menu de compte sur le */
/* compte donnÃ© en argument */
/* elle renvoie le no Ã  mettre dans history */
/* ************************************************************************** */
gint recherche_compte_dans_option_menu ( GtkWidget *option_menu,
					 gint no_compte )
{
    GList *liste_menu;
    GList *liste_tmp;

    liste_menu = GTK_MENU_SHELL ( gtk_option_menu_get_menu ( GTK_OPTION_MENU ( option_menu ))) -> children;
    liste_tmp = liste_menu;

    while ( liste_tmp )
    {
	gint *no;

	no = gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				   "account_number" );
	if ( GPOINTER_TO_INT (no) == no_compte )
	    return g_list_position ( liste_menu,
				     liste_tmp );
	liste_tmp = liste_tmp -> next;
    }
    return 0;
}
/* ************************************************************************** */



/* ************************************************************************** */
/* cette fonction renvoie le no de compte sÃ©lectionnÃ© par l'option menu */
/* \param option_menu l'option menu des comptes */
/* \return le no de compte ou -1 si pb */
/* ************************************************************************** */
gint recupere_no_compte ( GtkWidget *option_menu )
{
    gint no_compte;
    
    if ( !option_menu ||
	 !GTK_IS_OPTION_MENU ( option_menu ) ||
	 !GTK_IS_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ))
	return -1;

    no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT (  GTK_OPTION_MENU ( option_menu ) -> menu_item ),
							"account_number" ));
    return no_compte;
}
/* ************************************************************************** */




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
