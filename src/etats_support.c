/*  Fonctions utilitaires à destination des plugins d'état  */
/*      etats.c */

/*     Copyright (C) 2002  Benjamin Drieu */
/* 			benj@april.org */
/* 			https://www.grisbi.org/*/

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
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>
#include <stdlib.h>

/*START_INCLUDE*/
#include "etats_support.h"
#include "gsb_data_fyear.h"
#include "gsb_data_report.h"
#include "utils_dates.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/




/*START_EXTERN*/
/*END_EXTERN*/


/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * reourne une chaine de deux lignes centrées pour complément de nom
 *
 * \param
 * \param
 * \param
 *
 * \return a new allocated string
 **/
static gchar *etats_support_get_titre_formated (gchar *titre,
												gchar **tab,
												gint function)
{
	gchar *tmp_str;
	gchar *new_titre = NULL;

	if (function == 1)
		tmp_str = g_strconcat ("(", _("Edited"), " ", tab[0], " ", _("at"), " ", tab[1], ")", NULL);
	else
		tmp_str = g_strconcat ("(", _("Edited"), " ", tab[0], ")", NULL);

	new_titre = g_strconcat (titre, "\n", tmp_str, NULL);
	g_free (tmp_str);

	return new_titre;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void etats_support_set_export_pdf_name (gint report_number,
											   gchar **tab)
{
	gchar *report_name;
	gchar *tmp_str;
	gchar *tmp_str_to_free;

	report_name = gsb_data_report_get_report_name (report_number);
	tmp_str_to_free = my_strdelimit (report_name, " ", "_");
	tmp_str = my_strdelimit (tmp_str_to_free, "/\\", ".");
	g_free (tmp_str_to_free);

	if (tmp_str)
	{
		gchar **tmp_tab;
		gchar *export_name;

		tmp_tab = g_strsplit_set (tab[0], "/.-", 3);
		export_name = g_strconcat (tmp_tab[2], ".",tmp_tab[1], ".",tmp_tab[0], "-",tab[1], " ", tmp_str, ".pdf", NULL);
		gsb_data_report_set_export_pdf_name (report_number, export_name);
		g_strfreev (tmp_tab);
		g_free (export_name);
		g_free (tmp_str);
	}
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
gchar *etats_titre ( gint report_number)
{
	gchar **tab;
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
		    gint tmp_fyear_number;

		    tmp_fyear_number = GPOINTER_TO_INT (tmp_list -> data);

		    if ( tmp_list == g_slist_last (gsb_data_report_get_financial_year_list (report_number)))
			titre = g_strconcat ( titre,
					      gsb_data_fyear_get_name (tmp_fyear_number),
					      NULL );
		    else
			titre = g_strconcat ( titre,
					      gsb_data_fyear_get_name (tmp_fyear_number),
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

	gchar buffer_date[256];
	gchar buffer_date_2[256];
	gsize rc;
	GDate *date_tmp;
	gchar *date_str1, *date_str2;

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
		{
			date_str1 = gsb_format_gdate ( gsb_data_report_get_personal_date_start (report_number));
			date_str2 = gsb_format_gdate ( gsb_data_report_get_personal_date_end (report_number));
		    titre = g_strconcat ( titre,
					  ", ",
					  g_strdup_printf ( _("Result from %s to %s"),
								date_str1, date_str2 ),
					  NULL );
			g_free(date_str1);
			g_free(date_str2);
		}
		else
		    titre = g_strconcat ( titre,
					  ", ", _("Custom dates ranges not filled"),
					  NULL );
		break;

	    case 2:
		/* cumul à ce jour */

		date_str1 = gsb_format_gdate (today_date);
		titre = g_strconcat ( titre,
				      ", ",
				      g_strdup_printf ( _("total at %s"),
							date_str1),
				      NULL );
		g_free(date_str1);
		break;

	    case 3:
		/* mois en cours */

		rc = g_date_strftime ( buffer_date,
				  sizeof(buffer_date),
				  "%B",
				  today_date );
		if (rc == 0)
		    strcpy(buffer_date, "???");

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

		date_str1 = gsb_format_gdate (today_date);
		titre = g_strconcat ( titre,
				      ", ",
				      g_strdup_printf ( _("month total at %s"),
							date_str1),
				      NULL );
		g_free(date_str1);
		break;

	    case 6:
		/* cumul annuel */

		date_str1 = gsb_format_gdate (today_date);
		titre = g_strconcat ( titre,
				      ", ",
				      g_strdup_printf ( _("year total at %s"),
							date_str1),
				      NULL );
		g_free(date_str1);
		break;

	    case 7:
		/* mois précédent */

		g_date_subtract_months ( today_date,
					 1 );
		rc = g_date_strftime ( buffer_date,
				  sizeof(buffer_date),
				  "%B",
				  today_date );
		if (rc == 0)
		    strcpy(buffer_date, "???");

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

		date_str1 = gsb_format_gdate ( date_tmp );
		date_str2 = gsb_format_gdate (today_date);
		titre = g_strconcat ( titre,
				      ", ",
				      g_strdup_printf ( _("Result from %s to %s"),
							date_str1, date_str2),
				      NULL );
		g_free(date_str1);
		g_free(date_str2);
		break;

	    case 10:
		/* 3 derniers mois */

		date_tmp = gdate_today ( );
		g_date_subtract_months ( date_tmp,
					 3 );
		rc = g_date_strftime ( buffer_date_2,
				  sizeof(buffer_date_2),
				  "%B",
				  date_tmp );
		if (rc == 0)
		    strcpy(buffer_date_2, "???");
		rc = g_date_strftime ( buffer_date,
				  sizeof(buffer_date),
				  "%B",
				  today_date);
		if (rc == 0)
		    strcpy(buffer_date, "???");

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
		rc = g_date_strftime ( buffer_date_2,
				  sizeof(buffer_date_2),
				  "%B",
				  date_tmp );
		if (rc == 0)
		    strcpy(buffer_date_2, "???");
		rc = g_date_strftime ( buffer_date,
				  sizeof(buffer_date),
				  "%B",
				  today_date);
		if (rc == 0)
		    strcpy(buffer_date, "???");


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
		rc = g_date_strftime ( buffer_date_2,
				  sizeof(buffer_date_2),
				  "%B",
				  date_tmp );
		if (rc == 0)
		    strcpy(buffer_date_2, "???");
		rc = g_date_strftime ( buffer_date,
				  sizeof(buffer_date),
				  "%B",
				  today_date);
		if (rc == 0)
		    strcpy(buffer_date, "???");

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

	/* set complement for the filename of export pdf */
	tab = gsb_date_get_date_time_now_local ();
	etats_support_set_export_pdf_name (report_number, tab);

	/* on ajoute éventuellement le complément de titre */
	if (gsb_data_report_get_compl_name_used (report_number))
	{
		gchar *new_titre;
		gint function;
		gint position;

		function = gsb_data_report_get_compl_name_function (report_number);
		position = gsb_data_report_get_compl_name_position (report_number);

		/* set complement of title */
		switch (position)
		{
			case 1:
				if (function == 1)
					new_titre = g_strconcat (titre, " - ", tab[0], " ", tab[1], NULL);
				else
					new_titre = g_strconcat (titre, " - ", tab[0], NULL);
				break;
			case 2:
				new_titre = etats_support_get_titre_formated (titre, tab, function);
				break;
			default:
				if (function == 1)
					new_titre = g_strconcat (tab[0], " ", tab[1], " - ", titre, NULL);
				else
					new_titre = g_strconcat (tab[0], " - ", titre, NULL);
		}
		g_strfreev (tab);
		g_free (titre);
		g_date_free (today_date);

		return new_titre;
	}
	else
	{
		g_date_free (today_date);

		return titre;
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
