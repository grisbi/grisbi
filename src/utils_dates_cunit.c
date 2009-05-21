/* ************************************************************************** */
/*                                                                            */
/*                                  utils_dates_cunit                         */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2009 Mickaël Remars (grisbi@remars.com)               */
/*          http://www.grisbi.org                                             */
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

#include "include.h"

/* START_INCLUDE */
#include "utils_dates_cunit.h"
#include "./utils_dates.h"
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
    return 0;
}

/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int utils_dates_cunit_clean_suite(void)
{
    return 0;
}

void utils_dates_cunit__gsb_parse_date_string ( void )
{
    GDate *date = NULL;
    // invalid day
    date = gsb_parse_date_string ( "00/02/2009" );
    CU_ASSERT_EQUAL(G_DATE_BAD_YEAR, g_date_get_year(date));
    CU_ASSERT_EQUAL(G_DATE_BAD_MONTH, g_date_get_month(date));
    CU_ASSERT_EQUAL(G_DATE_BAD_DAY, g_date_get_day(date));

    date = gsb_parse_date_string ( "01/02/2009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(1, g_date_get_day(date));

    // invalid day
    date = gsb_parse_date_string ( "30/02/2009" );
    CU_ASSERT_EQUAL(G_DATE_BAD_YEAR, g_date_get_year(date));
    CU_ASSERT_EQUAL(G_DATE_BAD_MONTH, g_date_get_month(date));
    CU_ASSERT_EQUAL(G_DATE_BAD_DAY, g_date_get_day(date));

    // 2009 is not a leap year
    date = gsb_parse_date_string ( "29/02/2009" );
    CU_ASSERT_EQUAL(G_DATE_BAD_YEAR, g_date_get_year(date));
    CU_ASSERT_EQUAL(G_DATE_BAD_MONTH, g_date_get_month(date));
    CU_ASSERT_EQUAL(G_DATE_BAD_DAY, g_date_get_day(date));

    // 2008 was a leap year
    date = gsb_parse_date_string ( "29/02/2008" );
    CU_ASSERT_EQUAL(2008, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(29, g_date_get_day(date));

    date = gsb_parse_date_string ( "28/02/2009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(2, g_date_get_month(date));
    CU_ASSERT_EQUAL(28, g_date_get_day(date));

    date = gsb_parse_date_string ( "31/12/2009" );
    CU_ASSERT_EQUAL(2009, g_date_get_year(date));
    CU_ASSERT_EQUAL(12, g_date_get_month(date));
    CU_ASSERT_EQUAL(31, g_date_get_day(date));

    // invalid day
    date = gsb_parse_date_string ( "32/12/2009" );
    CU_ASSERT_EQUAL(G_DATE_BAD_YEAR, g_date_get_year(date));
    CU_ASSERT_EQUAL(G_DATE_BAD_MONTH, g_date_get_month(date));
    CU_ASSERT_EQUAL(G_DATE_BAD_DAY, g_date_get_day(date));

    // invalid month
    date = gsb_parse_date_string ( "13/13/2009" );
    CU_ASSERT_EQUAL(G_DATE_BAD_YEAR, g_date_get_year(date));
    CU_ASSERT_EQUAL(G_DATE_BAD_MONTH, g_date_get_month(date));
    CU_ASSERT_EQUAL(G_DATE_BAD_DAY, g_date_get_day(date));
}

CU_pSuite utils_dates_cunit_create_suite ( void )
{
    CU_pSuite pSuite = CU_add_suite("utils_dates",
                                    utils_dates_cunit_init_suite,
                                    utils_dates_cunit_clean_suite);
    if ( NULL == pSuite )
        return NULL;

    if ( ( NULL == CU_add_test ( pSuite, "of gsb_parse_date_string()", utils_dates_cunit__gsb_parse_date_string ) )
       )
        return NULL;

    return pSuite;
}
