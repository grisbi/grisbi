/*  Fonctions utilitaires à destination des plugins d'état  */
/*      etats.c */

/*     Copyright (C) 2002  Benjamin Drieu */
/* 			benj@april.org */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "etats_support.h"
#include "utils_dates.h"
#include "gsb_data_fyear.h"
#include "gsb_data_report.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/




/*START_EXTERN*/
/*END_EXTERN*/




gchar *etats_titre ( gint report_number)
{
    gchar *titre;
    GDate *today_date;

    titre = gsb_data_report_get_report_name (report_number);
    today_date = gdate_today ();

    if ( gsb_data_report_get_use_financial_year (report_number))
    {
	GSList *tmp_list;
	gint fyear_number;
	gint last_fyear_number;

	switch (gsb_data_report_get_financial_year_type (report_number))
	{
	    case 0:
		/* all the financial years */

		titre = g_strconcat ( titre,
				      ", ", _("all financial years"),
				      NULL );
		break;

	    case 1:
		/* current financial year */
		fyear_number = gsb_data_fyear_get_from_date (today_date);

		if (fyear_number)
		    titre = g_strconcat ( titre,
					  ", ", _("current financial year") , " (",
					  gsb_data_fyear_get_name (fyear_number),
					  ")",
					  NULL );
		else
		    titre = g_strconcat ( titre,
					  ", ", _("current financial year"),
					  NULL );
		break;

	    case 2:
		/* last financial year */

		fyear_number = gsb_data_fyear_get_from_date (today_date);
		last_fyear_number = 0;

		tmp_list = gsb_data_fyear_get_fyears_list ();
		while (tmp_list)
		{
		    gint tmp_fyear_number;

		    tmp_fyear_number = gsb_data_fyear_get_no_fyear (tmp_list -> data);

		    if (gsb_data_fyear_compare (fyear_number, tmp_fyear_number) == 1)
		    {
			if (last_fyear_number)
			{
			    if (gsb_data_fyear_compare (last_fyear_number, tmp_fyear_number) == -1)
				last_fyear_number = tmp_fyear_number;
			}
			else
			{
			    last_fyear_number = tmp_fyear_number;
			}
		    }
		    tmp_list = tmp_list -> next;
		}
		
		/* here, last_fyear_number is on the last financial year */

		if (last_fyear_number)
		    titre = g_strconcat ( titre,
					  ", ", _("former financial year") , " (",
					  gsb_data_fyear_get_name (last_fyear_number),
					  ")",
					  NULL );
		else
		    titre = g_strconcat ( titre,
					  ", ", _("former financial year"),
					  NULL );
		break;

	    case 3:
		/* personal selection of financial years */

		tmp_list = gsb_data_report_get_financial_year_list (report_number);

		if ( g_slist_length ( tmp_list ) > 1 )
		    titre = g_strconcat ( titre,
					  ", ", _("financial years"), " ",
					  NULL );
		else
		    titre = g_strconcat ( titre,
					  ", ", _("financial year"), " ",
					  NULL );

		while ( tmp_list )
		{
		    gint fyear_number;

		    fyear_number = GPOINTER_TO_INT (tmp_list -> data);

		    if ( tmp_list == g_slist_last (gsb_data_report_get_financial_year_list (report_number)))
			titre = g_strconcat ( titre,
					      gsb_data_fyear_get_name (fyear_number),
					      NULL );
		    else
			titre = g_strconcat ( titre,
					      gsb_data_fyear_get_name (fyear_number),
					      ", ",
					      NULL );
		    tmp_list = tmp_list -> next;
		}
		break;
	}
    }     
    else
    {
	/* c'est une plage de dates qui a été entrée */

	gchar buffer_date[15];
	gchar buffer_date_2[15];
	GDate *date_tmp;

	switch ( gsb_data_report_get_date_type (report_number))
	{
	    case 0:
		/* toutes */

		titre = g_strconcat ( titre,
				      ", ",
				      _("all dates"),
				      NULL );
		break;

	    case 1:
		/* plage perso */

		if ( gsb_data_report_get_personal_date_start (report_number)
		     &&
		     gsb_data_report_get_personal_date_end (report_number))
		    titre = g_strconcat ( titre,
					  ", ",
					  g_strdup_printf ( _("Result from %s to %s"),
							    gsb_format_gdate ( gsb_data_report_get_personal_date_start (report_number)),
							    gsb_format_gdate ( gsb_data_report_get_personal_date_end (report_number)) ),
					  NULL );
		else
		    titre = g_strconcat ( titre,
					  ", ", _("Custom dates ranges not filled"),
					  NULL );
		break;

	    case 2:
		/* cumul à ce jour */

		titre = g_strconcat ( titre,
				      ", ", 
				      g_strdup_printf ( _("total at %s"),
							gsb_format_gdate (today_date)),
				      NULL );
		break;

	    case 3:
		/* mois en cours */

		g_date_strftime ( buffer_date,
				  14,
				  "%B",
				  today_date );

		titre = g_strconcat ( titre,
				      ", ", 
				      g_strdup_printf ( _("%s %d"),
							buffer_date,
							g_date_get_year (today_date)),
				      NULL );
		break;

	    case 4:
		/* année en cours */

		titre = g_strconcat ( titre,
				      ", ", g_strdup_printf ( _("year %d"),
							      g_date_get_year (today_date)),
				      NULL );
		break;

	    case 5:
		/* cumul mensuel */

		titre = g_strconcat ( titre,
				      ", ", 
				      g_strdup_printf ( _("month total at %s"),
							gsb_format_gdate (today_date)),
				      NULL );
		break;

	    case 6:
		/* cumul annuel */

		titre = g_strconcat ( titre,
				      ", ", 
				      g_strdup_printf ( _("year total at %s"),
							gsb_format_gdate (today_date)),
				      NULL );
		break;

	    case 7:
		/* mois précédent */

		g_date_subtract_months ( today_date,
					 1 );
		g_date_strftime ( buffer_date,
				  14,
				  "%B",
				  today_date );

		titre = g_strconcat ( titre,
				      ", ", 
				      g_strdup_printf ( _("%s %d"),
							buffer_date,
							g_date_get_year (today_date)),
				      NULL );
		break;

	    case 8:
		/* année précédente */

		titre = g_strconcat ( titre,
				      ", ", g_strdup_printf ( _("year %d"),
							      g_date_get_year (today_date) - 1),
				      NULL );
		break;

	    case 9:
		/* 30 derniers jours */

		date_tmp = gdate_today ( );

		g_date_subtract_days ( date_tmp,
				       30 );

		titre = g_strconcat ( titre,
				      ", ", 
				      g_strdup_printf ( _("Result from %s to %s"),
							gsb_format_gdate ( date_tmp ),
							gsb_format_gdate (today_date)),
				      NULL );
		break;

	    case 10:
		/* 3 derniers mois */

		date_tmp = gdate_today ( );
		g_date_subtract_months ( date_tmp,
					 3 );
		g_date_strftime ( buffer_date_2,
				  14,
				  "%B",
				  date_tmp );
		g_date_strftime ( buffer_date,
				  14,
				  "%B",
				  today_date);

		titre = g_strconcat ( titre,
				      ", ", 
				      g_strdup_printf ( _("from %s %d"),
							buffer_date_2,
							g_date_get_year ( date_tmp )),
				      " ",
				      g_strdup_printf ( _("to %s %d"),
							buffer_date,
							g_date_get_year (today_date)),
				      NULL );
		break;

	    case 11:
		/* 6 derniers mois */

		date_tmp = gdate_today ( );
		g_date_subtract_months ( date_tmp,
					 6 );
		g_date_strftime ( buffer_date_2,
				  14,
				  "%B",
				  date_tmp );
		g_date_strftime ( buffer_date,
				  14,
				  "%B",
				  today_date);


		titre = g_strconcat ( titre,
				      ", ", 
				      g_strdup_printf ( _("from %s %d"),
							buffer_date_2,
							g_date_get_year ( date_tmp )),
				      " ",
				      g_strdup_printf ( _("to %s %d"),
							buffer_date,
							g_date_get_year (today_date)),
				      NULL );
		break;

	    case 12:
		/* 12 derniers mois */

		date_tmp = gdate_today ( );
		g_date_subtract_months ( date_tmp,
					 12 );
		g_date_strftime ( buffer_date_2,
				  14,
				  "%B",
				  date_tmp );
		g_date_strftime ( buffer_date,
				  14,
				  "%B",
				  today_date);

		titre = g_strconcat ( titre,
				      ", ", 
				      g_strdup_printf ( _("from %s %d"),
							buffer_date_2,
							g_date_get_year ( date_tmp )),
				      " ",
				      g_strdup_printf ( _("to %s %d"),
							buffer_date,
							g_date_get_year (today_date)),
				      NULL );
		break;
	}
    }

    return titre;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
