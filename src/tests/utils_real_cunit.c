/* ************************************************************************** */
/*                                                                            */
/*                                utils_real_cunit                            */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <stdlib.h>

/* START_INCLUDE */
#include "utils_real_cunit.h"
#include "gsb_locale.h"
#include "gsb_real.h"
#include "utils_real.h"
/* END_INCLUDE */


static int utils_real_cunit_init_suite ( void )
{
    return 0;
}


static int utils_real_cunit_clean_suite ( void )
{
    return 0;
}


static void utils_real_cunit__utils_real_get_from_string ( void )
{
    char *lc_monetary_orig;
    char *result = setlocale(LC_MONETARY, NULL);
    if (result != NULL)
    {
        lc_monetary_orig = (char *)malloc((strlen(result) + 1) * sizeof(char));
        strcpy(lc_monetary_orig, result);

        /* C test */
        result = setlocale(LC_MONETARY, "C");
        if (result != NULL)
        {
            gsb_locale_init_lconv_struct ();

            gsb_real val = utils_real_get_from_string("123.45");
            CU_ASSERT_EQUAL(12345, val.mantissa);
            CU_ASSERT_EQUAL(2, val.exponent);

            val = utils_real_get_from_string("21000000");
            CU_ASSERT_EQUAL(21000000, val.mantissa);
            CU_ASSERT_EQUAL(0, val.exponent);

            gsb_locale_shutdown();
        }

        /* French test */
        result = setlocale(LC_MONETARY, "fr_FR.UTF-8");
        if (result == NULL)
            result = setlocale(LC_MONETARY, "fr_FR@euro");
        if (result == NULL)
            result = setlocale(LC_MONETARY, "fr_FR");
        if (result != NULL)
        {
            gsb_locale_init_lconv_struct ();

            gsb_real val = utils_real_get_from_string("123,45");
            CU_ASSERT_EQUAL(12345, val.mantissa);
            CU_ASSERT_EQUAL(2, val.exponent);

            val = utils_real_get_from_string("21000000");
            CU_ASSERT_EQUAL(21000000, val.mantissa);
            CU_ASSERT_EQUAL(0, val.exponent);

            gsb_locale_shutdown();
        }

        /* English test */
        result = setlocale(LC_MONETARY, "en_US.UTF-8");
        if (result == NULL)
            result = setlocale(LC_MONETARY, "en_GB.UTF-8");
        if (result == NULL)
            result = setlocale(LC_MONETARY, "en_US");
        if (result == NULL)
            result = setlocale(LC_MONETARY, "en_GB");
        if (result != NULL)
        {
            gsb_locale_init_locale_structure();

            gsb_real val = utils_real_get_from_string("123.45");
            CU_ASSERT_EQUAL(12345, val.mantissa);
            CU_ASSERT_EQUAL(2, val.exponent);

            val = utils_real_get_from_string("21 000 000");
            CU_ASSERT_EQUAL(21000000, val.mantissa);
            CU_ASSERT_EQUAL(0, val.exponent);

            gsb_locale_shutdown();
        }

        /* Restore current locale and free memory */
        setlocale(LC_MONETARY, lc_monetary_orig);
        free(lc_monetary_orig) ;
    }
}


static void utils_real_cunit__utils_real_get_string_with_currency ( void )
{
    gchar *s;
    gsb_real n;
    gint currency_number = 1;

    n.mantissa = 0;
    n.exponent = 0;
    s = utils_real_get_string_with_currency ( n, currency_number, FALSE );
    CU_ASSERT_STRING_EQUAL("0", s);
    g_free(s);

    n.mantissa = 31415;
    n.exponent = 100;
    s = utils_real_get_string_with_currency ( n, currency_number, FALSE );
    CU_ASSERT_STRING_EQUAL(ERROR_REAL_STRING, s);
    g_free(s);

    n.mantissa = 31415;
    n.exponent = -1;
    s = utils_real_get_string_with_currency ( n, currency_number, FALSE );
    CU_ASSERT_STRING_EQUAL(ERROR_REAL_STRING, s);
    g_free(s);

    n.mantissa = G_GINT64_CONSTANT(0x8000000000000000);
    n.exponent = 2;
    s = utils_real_get_string_with_currency ( n, currency_number, FALSE );
    CU_ASSERT_STRING_EQUAL(ERROR_REAL_STRING, s);
    g_free(s);
}


CU_pSuite utils_real_cunit_create_suite ( void )
{
    CU_pSuite pSuite = CU_add_suite("utils_real",
                                    utils_real_cunit_init_suite,
                                    utils_real_cunit_clean_suite);
    if(NULL == pSuite)
        return NULL;

    if ( ! CU_add_test( pSuite, "of utils_real_get_from_string()",     utils_real_cunit__utils_real_get_from_string )
      || ! CU_add_test( pSuite, "of utils_real_get_string_with_currency()", utils_real_cunit__utils_real_get_string_with_currency )
       )
        return NULL;

    return pSuite;
}
