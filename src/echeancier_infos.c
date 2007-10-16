/* ************************************************************************** */
/* fichier qui s'occupe de la partie gauche de l'échéancier                   */
/* concerne le calendrier, choix des comptes, de l'affichage...               */
/* 			echeancier_infos.c                                     */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (aportal@univ-montp2.fr) 	      */
/*			2004-2007 Benjamin Drieu (bdrieu@april.org)  	      */
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
#include "./gsb_data_scheduled.h"
#include "./utils_dates.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void click_sur_jour_calendrier_echeance ( GtkWidget *calendrier,
					  gpointer null );
/*END_STATIC*/


GtkWidget *calendrier_echeances;
gint affichage_echeances;
gint affichage_echeances_perso_nb_libre;     /* contient le contenu de l'entrÃ©e */

/*START_EXTERN*/
/*END_EXTERN*/





/*****************************************************************************/
GtkWidget *creation_partie_gauche_echeancier ( void )
{
    time_t temps;

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

    pointeur = gsb_data_scheduled_get_scheduled_list ();

    while ( pointeur )
    {
	GDate *copie_date_ech;
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (pointeur -> data);

	copie_date_ech = gsb_date_copy (gsb_data_scheduled_get_date (scheduled_number));

	if ( !gsb_data_scheduled_get_date (scheduled_number) ||
	     !g_date_valid (gsb_data_scheduled_get_date (scheduled_number)))
	{
	    pointeur = pointeur -> next;
	    continue;
	}

	/* si c'est une fois */
	/* ou si c'est personnalisÃ© mais la periodicitÃ© est de 0, */
	/* on passe */

	if ( gsb_data_scheduled_get_frequency (scheduled_number)
	     &&
	     !(
	       gsb_data_scheduled_get_frequency (scheduled_number) == 4
	       &&
	       !gsb_data_scheduled_get_user_interval (scheduled_number)))
	    while ( g_date_compare ( copie_date_ech,
				     date_calendrier ) < 0 )
	    {
		/* périodicité hebdomadaire */
		if ( gsb_data_scheduled_get_frequency (scheduled_number) == 1 )
		    g_date_add_days ( copie_date_ech,
				      7 );
		else
		    /* périodicité mensuelle */
		    if ( gsb_data_scheduled_get_frequency (scheduled_number) == 2 )
			g_date_add_months ( copie_date_ech,
					    1 );
		    else
			/* périodicité annuelle */
			if ( gsb_data_scheduled_get_frequency (scheduled_number) == 3 )
			    g_date_add_years ( copie_date_ech,
					       1 );
			else
			{
			    /* pÃ©riodicitÃ© perso */
			    if ( ! gsb_data_scheduled_get_user_entry (scheduled_number) )
			    {
				return;
			    }

			    if ( !gsb_data_scheduled_get_user_interval (scheduled_number))
				g_date_add_days ( copie_date_ech,
						  gsb_data_scheduled_get_user_entry (scheduled_number));
			    else
				if ( gsb_data_scheduled_get_user_interval (scheduled_number) == 1 )
				    g_date_add_months ( copie_date_ech,
							gsb_data_scheduled_get_user_entry (scheduled_number));
				else
				    g_date_add_years ( copie_date_ech,
						       gsb_data_scheduled_get_user_entry (scheduled_number));
			}
	    }  

	/* Ã  ce niveau, soit l'Ã©chÃ©ance est sur le mois du calendrier,
	   soit elle l'a dÃ©passÃ©, soit elle a dÃ©passÃ© sa date limite */

	if ( !( copie_date_ech -> month != date_calendrier -> month
		||
		( gsb_data_scheduled_get_limit_date (scheduled_number)
		  &&
		  g_date_compare ( copie_date_ech,
				   gsb_data_scheduled_get_limit_date (scheduled_number)) > 0 )
		||
		( !gsb_data_scheduled_get_frequency (scheduled_number)
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
