/* ************************************************************************** */
/*                                                                            */
/*                                  gsb_real_cunit                            */
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

/**
 * \file gsb_real_cunit.c
 * cunit tests for gsb_real
 */

#include "include.h"

/* START_INCLUDE */
#include "gsb_real_cunit.h"
#include "./gsb_real.h"
/* END_INCLUDE */

/* START_STATIC */
static void gsb_real_cunit__gsb_real_add ( void );
static void gsb_real_cunit__gsb_real_get_from_string ( void );
static void gsb_real_cunit__gsb_real_raw_format_string ( void );
static int gsb_real_cunit_clean_suite ( void );
static int gsb_real_cunit_init_suite ( void );
/* END_STATIC */

/* START_EXTERN */
/* END_EXTERN */

/* The suite initialization function.
 * Returns zero on success, non-zero otherwise.
 */
int gsb_real_cunit_init_suite ( void )
{
    return 0;
}

/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int gsb_real_cunit_clean_suite ( void )
{
    return 0;
}

void gsb_real_cunit__gsb_real_get_from_string ( void )
{
    gsb_real val = gsb_real_get_from_string("123,45");
    CU_ASSERT_EQUAL(12345, val.mantissa);
    CU_ASSERT_EQUAL(2, val.exponent);

    val = gsb_real_get_from_string("21000000");
    CU_ASSERT_EQUAL(21000000, val.mantissa);
    CU_ASSERT_EQUAL(0, val.exponent);
}

void gsb_real_cunit__gsb_real_raw_format_string ( void )
{
    gchar *s;
    gsb_real n;
    struct lconv conv;
    memset(&conv, 0, sizeof(conv));
    conv.positive_sign = "<+>";
    conv.negative_sign = "<->";
    conv.mon_thousands_sep = "< >";
    conv.mon_decimal_point = "<.>";
    gchar *currency_symbol = "<¤>";
        
    n.mantissa = 1;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>0<.>01<¤>", s);
    g_free(s);

    n.mantissa = 10;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>0<.>10<¤>", s);
    g_free(s);

    n.mantissa = 31415;
    n.exponent = 1;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>3< >141<.>5<¤>", s);
    g_free(s);

    n.mantissa = 31415;
    n.exponent = 9;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>0<.>000031415<¤>", s);
    g_free(s);

    n.mantissa = 31415;
    n.exponent = 10;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("###ERR###", s);
    g_free(s);

    n.mantissa = 31415;
    n.exponent = -1;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("###ERR###", s);
    g_free(s);

    n.mantissa = 0x7FFFFFFF;
    n.exponent = 0;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>2< >147< >483< >647<.>0<¤>", s);
    g_free(s);

    n.mantissa = 0x7FFFFFFF;
    n.exponent = 1;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>214< >748< >364<.>7<¤>", s);
    g_free(s);

    n.mantissa = 0x7FFFFFFF;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>21< >474< >836<.>47<¤>", s);
    g_free(s);

    n.mantissa = 0x80000001;
    n.exponent = 0;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<->2< >147< >483< >647<.>0<¤>", s);
    g_free(s);

    n.mantissa = 0x80000001;
    n.exponent = 1;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<->214< >748< >364<.>7<¤>", s);
    g_free(s);

    n.mantissa = 0x80000001;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<->21< >474< >836<.>47<¤>", s);
    g_free(s);

    n.mantissa = 0x80000000;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("###ERR###", s);
    g_free(s);

    n.mantissa = 2100000000;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>21< >000< >000<.>00<¤>", s);
    g_free(s);

    conv.p_sep_by_space = 1;
    n.mantissa = 123;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>1<.>23 <¤>", s);
    g_free(s);

    conv.p_sep_by_space = 1;
    n.mantissa = 123;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>1<.>23 <¤>", s);
    g_free(s);
}

void gsb_real_cunit__gsb_real_add ( void )
{
    gsb_real a = {1, 0};
    gsb_real b = {31415, 4};
    gsb_real r = gsb_real_add(a, b);
    CU_ASSERT_EQUAL(41415, r.mantissa);
    CU_ASSERT_EQUAL(4, r.exponent);
    
    a.mantissa = 0x7FFFFFFE;
    a.exponent = 0;
    b.mantissa = 1;
    b.exponent = 0;
    r = gsb_real_add(a, b);
    CU_ASSERT_EQUAL(0x7FFFFFFF, r.mantissa);
    CU_ASSERT_EQUAL(0, r.exponent);
    
    a.mantissa = 0x7FFFFFFF;
    a.exponent = 0;
    b.mantissa = 2;
    b.exponent = 0;
    r = gsb_real_add(a, b);
    CU_ASSERT_EQUAL(0x80000000, r.mantissa);
    CU_ASSERT_EQUAL(0, r.exponent);
    
    a.mantissa = 0x80000001;
    a.exponent = 0;
    b.mantissa = -2;
    b.exponent = 0;
    r = gsb_real_add(a, b);
    CU_ASSERT_EQUAL(0x80000000, r.mantissa);
    CU_ASSERT_EQUAL(0, r.exponent);

    a.mantissa = 0x80000000;
    a.exponent = 0;
    b.mantissa = 100;
    b.exponent = 0;
    r = gsb_real_add(a, b);
    CU_ASSERT_EQUAL(0x80000000, r.mantissa);
    CU_ASSERT_EQUAL(0, r.exponent);

    a.mantissa = 100;
    a.exponent = 0;
    b.mantissa = 0x80000000;
    b.exponent = 0;
    r = gsb_real_add(a, b);
    CU_ASSERT_EQUAL(0x80000000, r.mantissa);
    CU_ASSERT_EQUAL(0, r.exponent);
}

CU_pSuite gsb_real_cunit_create_suite ( void )
{
    CU_pSuite pSuite = CU_add_suite("gsb_real",
                                    gsb_real_cunit_init_suite,
                                    gsb_real_cunit_clean_suite);
    if(NULL == pSuite)
        return NULL;

    if((NULL == CU_add_test(pSuite, "of gsb_real_get_from_string()",   gsb_real_cunit__gsb_real_get_from_string))
    || (NULL == CU_add_test(pSuite, "of gsb_real_raw_format_string()", gsb_real_cunit__gsb_real_raw_format_string))
    || (NULL == CU_add_test(pSuite, "of gsb_real_add()",               gsb_real_cunit__gsb_real_add))
       )
        return NULL;

    return pSuite;
}
