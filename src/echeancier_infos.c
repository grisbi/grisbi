/* ************************************************************************** */
/* fichier qui s'occupe de la partie gauche de l'Ã©chÃ©ancier                   */
/* concerne le calendrier, choix des comptes, de l'affichage...               */
/* 			echeancier_infos.c                                     */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (aportal@univ-montp2.fr) 	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)  	      */
/* 			http://www.grisbi.org   			      */
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
#include "echeancier_infos.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void click_sur_jour_calendrier_echeance ( GtkWidget *calendrier,
					  gpointer null );
/*END_STATIC*/

GtkWidget *calendrier_echeances;
GtkWidget *entree_personnalisation_affichage_echeances;
GtkWidget *bouton_personnalisation_affichage_echeances;
GtkWidget *bouton_valider_echeance_perso;
gint affichage_echeances; /* contient 0(mois), 1 (2 mois), 2(annÃ©e), 3(toutes), 4(perso) */
gint affichage_echeances_perso_nb_libre;     /* contient le contenu de l'entrÃ©e */
gint affichage_echeances_perso_j_m_a;        /* contient 0 (jours), 1 (mois), 2 (annÃ©es) */


/*START_EXTERN*/
extern GSList *liste_struct_echeances;
/*END_EXTERN*/





/*****************************************************************************/
GtkWidget *creation_partie_gauche_echeancier ( void )
{
    GtkWidget *vbox;
    GtkWidget *separation;
    GtkWidget *hbox;
    GtkWidget *label;
    time_t temps;
    GtkWidget *menu;
    GtkWidget *bouton;
    GtkWidget *item;

    /* crÃ©ation du calendrier */

    time ( &temps );

    calendrier_echeances = gtk_calendar_new ();
    gtk_calendar_display_options ( GTK_CALENDAR ( calendrier_echeances ),
				   GTK_CALENDAR_SHOW_HEADING |
				   GTK_CALENDAR_SHOW_DAY_NAMES |
				   GTK_CALENDAR_WEEK_START_MONDAY );
    gtk_calendar_select_month ( GTK_CALENDAR ( calendrier_echeances ),
				localtime ( &temps ) -> tm_mon,
				localtime ( &temps ) -> tm_year + 1900 );
    mise_a_jour_calendrier ();
    gtk_signal_connect ( GTK_OBJECT ( calendrier_echeances ),
			 "month-changed",
			 GTK_SIGNAL_FUNC ( mise_a_jour_calendrier ),
			 NULL );
    gtk_signal_connect_after ( GTK_OBJECT ( calendrier_echeances ),
			       "day-selected",
			       GTK_SIGNAL_FUNC ( click_sur_jour_calendrier_echeance ),
			       NULL );

    gtk_widget_show ( calendrier_echeances );

    return ( calendrier_echeances );
}
/*****************************************************************************/


/*****************************************************************************/
/* Fonction : mise_a_jour_calendrier					     */
/* met en gras les jours oÃ¹ il y a une Ã©chÃ©ance				     */
/*****************************************************************************/
void mise_a_jour_calendrier ( void )
{
    GDate *date_calendrier;
    time_t temps;
    GSList *pointeur;


    date_calendrier = g_date_new_dmy ( 1,
				       GTK_CALENDAR ( calendrier_echeances ) -> month + 1,
				       GTK_CALENDAR ( calendrier_echeances ) -> year );

    gtk_calendar_clear_marks ( GTK_CALENDAR ( calendrier_echeances ));


    /* si le jour courant doit y apparaitre, on le sÃ©lectionne */

    time ( &temps );

    if ( ( localtime ( &temps ) -> tm_mon == GTK_CALENDAR ( calendrier_echeances ) -> month )
	 &&
	 ( ( localtime ( &temps ) -> tm_year + 1900 ) == GTK_CALENDAR ( calendrier_echeances ) -> year ) )
	gtk_calendar_select_day ( GTK_CALENDAR ( calendrier_echeances ),
				  localtime ( &temps ) -> tm_mday );
    else
	gtk_calendar_select_day ( GTK_CALENDAR ( calendrier_echeances ),
				  FALSE );

    /* on fait le tour de toutes les Ã©chÃ©ances, les amÃšne au mois du calendrier
       et mise en gras du jour de l'Ã©chÃ©ance */

    pointeur = liste_struct_echeances;

    while ( pointeur )
    {
	GDate *copie_date_ech;
	struct operation_echeance *echeance;

	echeance = pointeur -> data;

	copie_date_ech = g_date_new_dmy ( echeance -> jour,
					  echeance -> mois,
					  echeance -> annee );

	/* si c'est une fois */
	/* ou si c'est personnalisÃ© mais la periodicitÃ© est de 0, */
	/* on passe */

	if ( echeance -> periodicite
	     &&
	     !(
	       echeance -> periodicite == 4
	       &&
	       !echeance -> periodicite_personnalisee ))
	    while ( g_date_compare ( copie_date_ech,
				     date_calendrier ) < 0 )
	    {
		/* pÃ©riodicitÃ© hebdomadaire */
		if ( echeance -> periodicite == 1 )
		    g_date_add_days ( copie_date_ech,
				      7 );
		else
		    /* pÃ©riodicitÃ© mensuelle */
		    if ( echeance -> periodicite == 2 )
			g_date_add_months ( copie_date_ech,
					    1 );
		    else
			/* pÃ©riodicitÃ© annuelle */
			if ( echeance -> periodicite == 3 )
			    g_date_add_years ( copie_date_ech,
					       1 );
			else
			    /* pÃ©riodicitÃ© perso */
			    if ( !echeance -> intervalle_periodicite_personnalisee )
				g_date_add_days ( copie_date_ech,
						  echeance -> periodicite_personnalisee );
			    else
				if ( echeance -> intervalle_periodicite_personnalisee == 1 )
				    g_date_add_months ( copie_date_ech,
							echeance -> periodicite_personnalisee );
				else
				    g_date_add_years ( copie_date_ech,
						       echeance -> periodicite_personnalisee );
	    }  

	/* Ã  ce niveau, soit l'Ã©chÃ©ance est sur le mois du calendrier,
	   soit elle l'a dÃ©passÃ©, soit elle a dÃ©passÃ© sa date limite */

	if ( !( copie_date_ech -> month != date_calendrier -> month
		||
		( echeance -> date_limite
		  &&
		  g_date_compare ( copie_date_ech,
				   echeance -> date_limite ) > 0 )
		||
		( !echeance -> periodicite
		  &&
		  copie_date_ech -> year != date_calendrier -> year )))
	    gtk_calendar_mark_day ( GTK_CALENDAR ( calendrier_echeances ),
				    copie_date_ech -> day );

	pointeur = pointeur -> next;
    }
}
/*****************************************************************************/

/*****************************************************************************/
void click_sur_jour_calendrier_echeance ( GtkWidget *calendrier,
					  gpointer null )
{
    time_t temps;

    time ( &temps );

    gtk_signal_handler_block_by_func ( GTK_OBJECT ( calendrier ),
				       GTK_SIGNAL_FUNC ( click_sur_jour_calendrier_echeance ),
				       NULL );

    if ( ( localtime ( &temps ) -> tm_mon == GTK_CALENDAR ( calendrier_echeances ) -> month )
	 &&
	 ( ( localtime ( &temps ) -> tm_year + 1900 ) == GTK_CALENDAR ( calendrier_echeances ) -> year ) )
	gtk_calendar_select_day ( GTK_CALENDAR ( calendrier_echeances ),
				  localtime ( &temps ) -> tm_mday );
    else
	gtk_calendar_select_day ( GTK_CALENDAR ( calendrier_echeances ),
				  FALSE );

    gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( calendrier ),
					 GTK_SIGNAL_FUNC ( click_sur_jour_calendrier_echeance ),
					 NULL );

}
/*****************************************************************************/



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
