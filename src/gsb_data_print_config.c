/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_data_print_config.c
 * set and get the print configuration, no GUI here
 * all the functions here work with gsb_autofunc
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_print_config.h"
#include "structures.h"
/*END_INCLUDE*/


/* first part for transaction list printing */
static gboolean draw_lines;
static gboolean draw_column;
static gboolean draw_background;
static gboolean draw_archives;
static gboolean draw_columns_name;
static gboolean draw_title;
static gboolean draw_interval_dates;
static gboolean draw_dates_are_value_dates;

static PangoFontDescription *font_transactions = NULL;
static PangoFontDescription *font_title = NULL;

/* second part for report printing */
static PangoFontDescription *report_font_transactions = NULL;
static PangoFontDescription *report_font_title = NULL;




/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * initialize the variables to default config
 *
 * \param
 *
 * \return
 * */
void gsb_data_print_config_init (void)
{
    /* init transactions list config */
    draw_lines = TRUE;
    draw_column = TRUE;
    draw_background = FALSE;
    draw_archives = FALSE;
    draw_columns_name = TRUE;
    draw_title = FALSE;

    draw_interval_dates = FALSE;
    draw_dates_are_value_dates = FALSE;

    if (font_transactions)
	pango_font_description_free (font_transactions);
    if ( conf.utilise_fonte_listes && conf.font_string )
	font_transactions = pango_font_description_from_string ( conf.font_string );
    else
	font_transactions = pango_font_description_from_string  ("sans 6");

    if (font_title)
	pango_font_description_free (font_title);
    font_title = pango_font_description_from_string  ("sans 12");

    /* init report config */
    if (report_font_transactions)
	pango_font_description_free (report_font_transactions);
    report_font_transactions = pango_font_description_from_string  ("sans 6");;

    if (report_font_title)
	pango_font_description_free (report_font_title);
    report_font_title = pango_font_description_from_string  ("sans 12");
}


/**
 * return the draw_lines value
 *
 * \param
 *
 * \return the value
 * */
gboolean gsb_data_print_config_get_draw_lines  (void)
{
    return draw_lines;
}

/**
 * set the draw_lines value
 *
 * \param number	not used (here for compatibility with gsb_autofunc)
 * \param value		value to set
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_draw_lines  ( gint number,
						 gboolean value )
{
    draw_lines = value;
    return FALSE;
}

/**
 * return the draw_column value
 *
 * \param
 *
 * \return the value
 * */
gboolean gsb_data_print_config_get_draw_column  (void)
{
    return draw_column;
}

/**
 * set the draw_column value
 *
 * \param number	not used (here for compatibility with gsb_autofunc)
 * \param value		value to set
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_draw_column  ( gint number,
						  gboolean value )
{
    draw_column = value;
    return FALSE;
}

/**
 * return the draw_background value
 *
 * \param
 *
 * \return the value
 * */
gboolean gsb_data_print_config_get_draw_background  (void)
{
    return draw_background;
}

/**
 * set the draw_background value
 *
 * \param number	not used (here for compatibility with gsb_autofunc)
 * \param value		value to set
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_draw_background  ( gint number,
						      gboolean value )
{
    draw_background = value;
    return FALSE;
}

/**
 * return the draw_archives value
 *
 * \param
 *
 * \return the value
 * */
gboolean gsb_data_print_config_get_draw_archives  (void)
{
    return draw_archives;
}

/**
 * set the draw_archives value
 *
 * \param number	not used (here for compatibility with gsb_autofunc)
 * \param value		value to set
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_draw_archives  ( gint number,
						    gboolean value )
{
    draw_archives = value;
    return FALSE;
}

/**
 * return the draw_columns_name value
 *
 * \param
 *
 * \return the value
 * */
gboolean gsb_data_print_config_get_draw_columns_name  (void)
{
    return draw_columns_name;
}

/**
 * set the draw_columns_name value
 *
 * \param number	not used (here for compatibility with gsb_autofunc)
 * \param value		value to set
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_draw_columns_name  ( gint number,
							gboolean value )
{
    draw_columns_name = value;
    return FALSE;
}

/**
 * return the draw_title value
 *
 * \param
 *
 * \return the value
 * */
gboolean gsb_data_print_config_get_draw_title  (void)
{
    return draw_title;
}

/**
 * set the draw_title value
 *
 * \param number	not used (here for compatibility with gsb_autofunc)
 * \param value		value to set
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_draw_title  ( gint number,
						 gboolean value )
{
    draw_title = value;
    return FALSE;
}

/**
 * return the draw_interval_dates value
 *
 * \param
 *
 * \return the value
 * */
gboolean gsb_data_print_config_get_draw_interval_dates  (void)
{
    return draw_interval_dates;
}

/**
 * set the draw_interval_dates value
 *
 * \param number	not used (here for compatibility with gsb_autofunc)
 * \param value		value to set
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_draw_interval_dates  ( gint number,
							  gboolean value )
{
    draw_interval_dates = value;
    return FALSE;
}

/**
 * return the draw_dates_are_value_dates value
 *
 * \param
 *
 * \return the value
 * */
gboolean gsb_data_print_config_get_draw_dates_are_value_dates  (void)
{
    return draw_dates_are_value_dates;
}

/**
 * set the draw_dates_are_value_dates value
 *
 * \param number	not used (here for compatibility with gsb_autofunc)
 * \param value		value to set
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_draw_dates_are_value_dates  ( gint number,
								 gboolean value )
{
    draw_dates_are_value_dates = value;
    return FALSE;
}

/**
 * return the the font transactions value
 *
 * \param
 *
 * \return the value
 * */
PangoFontDescription *gsb_data_print_config_get_font_transactions (void)
{
    return font_transactions;
}


/**
 * set the font_transactions value
 *
 * \param font_desc	the PangoFontDescription to save
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_font_transaction  ( PangoFontDescription *font_desc )
{
    if (font_transactions)
	pango_font_description_free (font_transactions);
    font_transactions = font_desc;
    return FALSE;
}

/**
 * return the the font title value
 *
 * \param
 *
 * \return the value
 * */
PangoFontDescription *gsb_data_print_config_get_font_title (void)
{
    return font_title;
}


/**
 * set the font title value
 *
 * \param font_desc	the PangoFontDescription to save
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_font_title ( PangoFontDescription *font_desc )
{
    if (font_title)
	pango_font_description_free (font_title);
    font_title = font_desc;
    return FALSE;
}

/**
 * return the the font transactions value for report
 *
 * \param
 *
 * \return the value
 * */
PangoFontDescription *gsb_data_print_config_get_report_font_transactions (void)
{
    return report_font_transactions;
}


/**
 * set the font_transactions value for report
 *
 * \param font_desc	the PangoFontDescription to save
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_report_font_transaction ( PangoFontDescription *font_desc )
{
    if (report_font_transactions)
	pango_font_description_free (report_font_transactions);
    report_font_transactions = font_desc;
    return FALSE;
}

/**
 * return the the font title value for report
 *
 * \param
 *
 * \return the value
 * */
PangoFontDescription *gsb_data_print_config_get_report_font_title (void)
{
    return report_font_title;
}


/**
 * set the font title value
 *
 * \param font_desc	the PangoFontDescription to save
 * 
 * \return FALSE
 * */
gboolean gsb_data_print_config_set_report_font_title ( PangoFontDescription *font_desc )
{
    if (report_font_title)
	pango_font_description_free (report_font_title);
    report_font_title = font_desc;
    return FALSE;
}


