/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_fyear.c
 * contains tools to work with the financial years
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_fyear.h"
#include "gsb_data_fyear.h"
#include "utils_dates.h"
#include "echeancier_formulaire.h"
#include "gsb_data_form.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/


GtkWidget *paddingbox_details;	/** Widget handling financial year details */
GtkWidget *clist_exercices_parametres;
GtkWidget *bouton_supprimer_exercice;
GtkWidget *nom_exercice;
GtkWidget *debut_exercice;
GtkWidget *fin_exercice;
GtkWidget *affichage_exercice;
gint ligne_selection_exercice;



/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *formulaire;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
/*END_EXTERN*/



/**
 * Rebuild financial years menus in various forms
 */
gboolean update_financial_year_menus ()
{
/* FIXME : what to do and do it again with the new form */
/*     if ( widget_formulaire_operations[TRANSACTION_FORM_EXERCICE] && */
/* 	 GTK_OPTION_MENU(widget_formulaire_operations[TRANSACTION_FORM_EXERCICE]) -> menu ) */
/*     { */
/* 	gtk_widget_destroy ( GTK_OPTION_MENU(widget_formulaire_operations[TRANSACTION_FORM_EXERCICE]) -> menu ); */
/* 	gtk_option_menu_set_menu ( GTK_OPTION_MENU (widget_formulaire_operations[TRANSACTION_FORM_EXERCICE]), */
/* 				   creation_menu_exercices (0) ); */
/*     } */

    if ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] &&
	 GTK_OPTION_MENU(widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE]) -> menu )
    {
	gtk_widget_destroy ( GTK_OPTION_MENU(widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE]) -> menu );
	gtk_option_menu_set_menu ( GTK_OPTION_MENU (widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE]),
				   creation_menu_exercices (1) );
    }
    return FALSE;
}


/* ************************************************************************************************************ */
/* Fonction creation_menu_exercices */
/* crée un menu qui contient les noms des exercices associés à leur no et adr */
/* et le renvoie */
/* origine = 0 si ça vient des opérations */
/* origine = 1 si ça vient de l'échéancier ; dans ce cas on rajoute automatique */
/* ************************************************************************************************************ */

GtkWidget *creation_menu_exercices ( gint origine )
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    GSList *tmp_list;

    menu = gtk_menu_new ();
    gtk_widget_show ( menu );


    /* si ça vient de l'échéancier, le 1er est automatique */
    /* on lui associe -2 */

    if ( origine )
    {
	menu_item = gtk_menu_item_new_with_label ( _("Automatic") );
	gtk_menu_append ( GTK_MENU ( menu ),
			  menu_item );
	gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			      "no_exercice",
			      GINT_TO_POINTER (-2));
	gtk_widget_show ( menu_item );
    }

    /* le premier nom est Aucun */
    /* on lui associe 0 */

    menu_item = gtk_menu_item_new_with_label ( _("None") );
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_exercice",
			  NULL );
    gtk_widget_show ( menu_item );


    /* le second est non affiché */
    /* on lui associe -1 */

    menu_item = gtk_menu_item_new_with_label ( _("Not displayed") );
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_exercice",
			  GINT_TO_POINTER (-1));
    gtk_widget_show ( menu_item );


    tmp_list = gsb_data_fyear_get_fyears_list ();

    while (tmp_list)
    {
	gint fyear_number;

	fyear_number = GPOINTER_TO_INT (tmp_list -> data);

	if (gsb_data_fyear_get_form_show (fyear_number))
	{
	    menu_item = gtk_menu_item_new_with_label (gsb_data_fyear_get_name(fyear_number));
	    gtk_menu_append ( GTK_MENU ( menu ),
			      menu_item );

	    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				  "no_exercice",
				  GINT_TO_POINTER (fyear_number));
	    gtk_widget_show ( menu_item );
	}
	tmp_list = tmp_list -> next;
    }
    return ( menu );
}
/* ************************************************************************************************************ */


/**
 * get the number of the selected financial year in the option menu
 * in the param
 * that option menu has to be created before with creation_menu_exercices
 *
 * \param financial_year_option_menu
 *
 * \return the number of the financial year
 * */
gint gsb_financial_year_get_number_from_option_menu ( GtkWidget *option_menu )
{
    gint financial_year_number;

    financial_year_number = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU (option_menu) -> menu_item ),
								    "no_exercice" ));
    return financial_year_number;
}


/* ************************************************************************************************************** */
/* fonction affiche_exercice_par_date */
/* met automatiquement l'option menu de l'exercice par rapport */
/* à la date dans le formulaire */
/* ************************************************************************************************************** */

void affiche_exercice_par_date ( GtkWidget *entree_date,
				 GtkWidget *option_menu_exercice )
{
    GDate *date_courante;

    if ( !etat.affichage_exercice_automatique
	 ||
	!option_menu_exercice )
	return;

    date_courante = gsb_parse_date_string (gtk_entry_get_text ( GTK_ENTRY (entree_date)));
    if (!date_courante)
	return;
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_exercice ),
				  cherche_no_menu_exercice ( gsb_data_fyear_get_from_date (date_courante),
							     option_menu_exercice ));
}
/* ************************************************************************************************************** */


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



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
