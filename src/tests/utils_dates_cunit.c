/* ************************************************************************** */
/*                                                                            */
/*                                  utils_dates_cunit                         */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2009 Mickaël Remars (grisbi@remars.com)               */
/*          https://www.grisbi.org/                                            */
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
 * \file utils_dates_cunit.c
 * cunit tests for utils_dates
 */

#include "config.h"

#include "include.h"
#include <stdlib.h>
#include <locale.h>

/* START_INCLUDE */
#include "gsb_regex.h"
#include "utils_dates_cunit.h"
#include "utils_dates.h"
/* END_INCLUDE */

/* START_STATIC */
static void utils_dates_cunit__gsb_parse_date_string ( void );
static int utils_dates_cunit_clean_suite ( void );
static int utils_dates_cunit_init_suite ( void );
/* END_STATIC */

/* START_EXTERN */
/* END_EXTERN */


/* The suite initialization function.
 * Returns zero on success, non-zero otherwise.
 */
int utils_dates_cunit_init_suite ( void )
{
    gsb_regex_init_variables ( );
    return 0;
}


/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int utils_dates_cunit_clean_suite ( void )
{
    gsb_regex_destroy ( );
    return 0;
}


void utils_dates_cunit__gsb_parse_date_string ( void )
{
    GDate *date = NULL;

    /**
     * month-first tests
     */
    gsb_date_set_format_date ( "%m/%d/%Y" );

    /* invalid day  */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "02/00/2009" ) );

    date = gsb_parse_date_string ( "01/02/2009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(1, g_date_get_month(date));
    CU_ASSERT_EQUAL(2, g_date_get_day(date));
    g_date_free( date );

    /* invalid day  */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "02/30/2009" ) );

    /* 2009 is not a leap year */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "02/29/2009" ) );

    /* 2008 was a leap year */
    date = gsb_parse_date_string ( "02:29:2008" );
    CU_ASSERT_EQUAL(2008, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(29, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "02-28-2009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(28, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "12312009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(12, g_date_get_month(date));
    CU_ASSERT_EQUAL(31, g_date_get_day(date));
    g_date_free( date );

    /* implicit year */
    date = gsb_parse_date_string ( "12/31/09" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(12, g_date_get_month(date));
    CU_ASSERT_EQUAL(31, g_date_get_day(date));
    g_date_free( date );

    /* invalid day  */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "12/32/2009" ) );

    /* invalid month  */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "13/13/2009" ) );


    /**
     * day-first tests
     */
    gsb_date_set_format_date ( "%d/%m/%Y" );

    /* invalid day */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "00/02/2009" ) );

    date = gsb_parse_date_string ( "01/02/2009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(1, g_date_get_day(date));
    g_date_free( date );

    /* invalid day */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "30/02/2009" ) );

    /* 2009 is not a leap year */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "29/02/2009" ) );

    /* 2008 was a leap year */
    date = gsb_parse_date_string ( "29/02/2008" );
    CU_ASSERT_EQUAL(2008, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(29, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "28.02.2009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(28, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "31122009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(12, g_date_get_month(date));
    CU_ASSERT_EQUAL(31, g_date_get_day(date));
    g_date_free( date );

    /* implicit year */
    date = gsb_parse_date_string ( "010209" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(1, g_date_get_day(date));
    g_date_free( date );

    /* invalid day */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "32/12/2009" ) );

    /* invalid month */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "13/13/2009" ) );

    /* month-first tests */
    gsb_date_set_format_date ( "%m/%d/%Y" );

    /* invalid day */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "02/00/2009" ) );

    /* invalid day */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "02/30/2009" ) );

    /* 2009 is not a leap year */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "02/29/2009" ) );

    /* 209 is not a valid year */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "02/29/209" ) );

    /* 2008 was a leap year */
    date = gsb_parse_date_string ( "02/29/2008" );
    CU_ASSERT_EQUAL(2008, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(29, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "02/28/2009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(28, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "12/31/2009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(12, g_date_get_month(date));
    CU_ASSERT_EQUAL(31, g_date_get_day(date));
    g_date_free( date );

    /* invalid day */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "12/32/2009" ) );

    /* invalid month */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "13/13/2009" ) );


    /**
     * short date tests
     */
    gsb_date_set_format_date ( "%d/%m/%Y" );

    date = gsb_parse_date_string ( "1" );
    CU_ASSERT_EQUAL(1, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "10" );
    CU_ASSERT_EQUAL(10, g_date_get_day(date));
    g_date_free( date );


    /**
     * date tests with string as months instead of integers
     */

    gsb_date_set_format_date ( "%m/%d/%Y" );

    date = gsb_parse_date_string ( "Jan/31/2011" );
    CU_ASSERT_EQUAL(2011, g_date_get_year(date));
    CU_ASSERT_EQUAL(1, g_date_get_month(date));
    CU_ASSERT_EQUAL(31, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "JAN/31/2011" );
    CU_ASSERT_EQUAL(2011, g_date_get_year(date));
    CU_ASSERT_EQUAL(1, g_date_get_month(date));
    CU_ASSERT_EQUAL(31, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "jan/31/2011" );
    CU_ASSERT_EQUAL(2011, g_date_get_year(date));
    CU_ASSERT_EQUAL(1, g_date_get_month(date));
    CU_ASSERT_EQUAL(31, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "Dec/31/2011" );
    CU_ASSERT_EQUAL(2011, g_date_get_year(date));
    CU_ASSERT_EQUAL(12, g_date_get_month(date));
    CU_ASSERT_EQUAL(31, g_date_get_day(date));
    g_date_free( date );

    /* invalid months */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "" ) );
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "January/01/2011" ) );
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "December/01/2011" ) );
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "MARCH/01/2011" ) );
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "MAR /01/2011" ) );
    /* wrong order */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "01/Jan/2011" ) );
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "31/Dec/2011" ) );

    gsb_date_set_format_date ( "%d/%m/%Y" );

    date = gsb_parse_date_string ( "14/Jul/2011" );
    CU_ASSERT_EQUAL(2011, g_date_get_year(date));
    CU_ASSERT_EQUAL(7, g_date_get_month(date));
    CU_ASSERT_EQUAL(14, g_date_get_day(date));
    g_date_free( date );

    date = gsb_parse_date_string ( "31/Dec/2011" );
    CU_ASSERT_EQUAL(2011, g_date_get_year(date));
    CU_ASSERT_EQUAL(12, g_date_get_month(date));
    CU_ASSERT_EQUAL(31, g_date_get_day(date));
    g_date_free( date );

    /* invalid months */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "" ) );
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "01/MAR /2011" ) );
    /* wrong order */
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "Jan/01/2011" ) );
    CU_ASSERT_EQUAL ( NULL, gsb_parse_date_string ( "Dec/31/2011" ) );


    /**
     * tear down
     */
    gsb_date_set_format_date ( NULL );
}



CU_pSuite utils_dates_cunit_create_suite ( void )
{
    CU_pSuite pSuite = CU_add_suite("utils_dates",
                                    utils_dates_cunit_init_suite,
                                    utils_dates_cunit_clean_suite);
    if ( NULL == pSuite )
        return NULL;

#if 0
    if ( NULL == CU_add_test ( pSuite, "of gsb_parse_date_string()",
                               utils_dates_cunit__gsb_parse_date_string ) )
        return NULL;
#endif

    return pSuite;
}


