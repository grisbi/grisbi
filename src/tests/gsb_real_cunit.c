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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/* START_INCLUDE */
#include "gsb_real_cunit.h"
#include "gsb_real.h"
/* END_INCLUDE */

/* START_STATIC */
static void gsb_real_cunit__gsb_real_add ( void );
static void gsb_real_cunit__gsb_real_get_from_string ( void );
static void gsb_real_cunit__gsb_real_mul();
static void gsb_real_cunit__gsb_real_normalize();
static void gsb_real_cunit__gsb_real_get_string_with_currency ( void );
static void gsb_real_cunit__gsb_real_raw_format_string ( void );
static void gsb_real_cunit__gsb_real_raw_get_from_string();
static void gsb_real_cunit__gsb_real_raw_get_from_string__locale();
static void gsb_real_cunit__gsb_real_sub();
static void gsb_real_cunit__gsb_real_adjust_exponent ( void );
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
            gsb_real val = gsb_real_get_from_string("123.45");
            CU_ASSERT_EQUAL(12345, val.mantissa);
            CU_ASSERT_EQUAL(2, val.exponent);

            val = gsb_real_get_from_string("21000000");
            CU_ASSERT_EQUAL(21000000, val.mantissa);
            CU_ASSERT_EQUAL(0, val.exponent);
        }

        /* French test */
        result = setlocale(LC_MONETARY, "fr_FR.UTF-8");
        if (result == NULL)
            result = setlocale(LC_MONETARY, "fr_FR@euro");
        if (result == NULL)
            result = setlocale(LC_MONETARY, "fr_FR");
        if (result != NULL)
        {
            gsb_real val = gsb_real_get_from_string("123,45");
            CU_ASSERT_EQUAL(12345, val.mantissa);
            CU_ASSERT_EQUAL(2, val.exponent);

            val = gsb_real_get_from_string("21000000");
            CU_ASSERT_EQUAL(21000000, val.mantissa);
            CU_ASSERT_EQUAL(0, val.exponent);
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
            gsb_real val = gsb_real_get_from_string("123.45");
            CU_ASSERT_EQUAL(12345, val.mantissa);
            CU_ASSERT_EQUAL(2, val.exponent);

            val = gsb_real_get_from_string("21 000 000");
            CU_ASSERT_EQUAL(21000000, val.mantissa);
            CU_ASSERT_EQUAL(0, val.exponent);
        }

        /* Restore current locale and free memory */
        setlocale(LC_MONETARY, lc_monetary_orig);
        free(lc_monetary_orig) ;
    }
}

void gsb_real_cunit__gsb_real_raw_get_from_string()
{
    gsb_real val;
    /*  NULL ==> error */
    val = gsb_real_raw_get_from_string ( NULL, NULL, NULL );
    CU_ASSERT_EQUAL ( G_MININT64, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

    val = gsb_real_raw_get_from_string ( "", NULL, NULL );
/*     empty ==> 0  */
    CU_ASSERT_EQUAL ( 0, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

    val = gsb_real_raw_get_from_string ( "0", NULL, NULL );
    CU_ASSERT_EQUAL ( 0, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

    val = gsb_real_raw_get_from_string ( "1", NULL, NULL );
    CU_ASSERT_EQUAL ( 1, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

    val = gsb_real_raw_get_from_string ( "12", NULL, NULL );
    CU_ASSERT_EQUAL ( 12, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // ignore left spaces  */
    val = gsb_real_raw_get_from_string ( "   12", NULL, NULL );
    CU_ASSERT_EQUAL ( 12, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // ignore right spaces  */
    val = gsb_real_raw_get_from_string ( "12   ", NULL, NULL );
    CU_ASSERT_EQUAL ( 12, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // ignore left-middle-right spaces  */
    val = gsb_real_raw_get_from_string ( "    1   2   ", NULL, NULL );
    CU_ASSERT_EQUAL ( 12, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // significant zeros  */
    val = gsb_real_raw_get_from_string ( "12000", NULL, NULL );
    CU_ASSERT_EQUAL ( 12000, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // useless zeros  */
    val = gsb_real_raw_get_from_string ( "00012", NULL, NULL );
    CU_ASSERT_EQUAL ( 12, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // comma decimal separator  */
    val = gsb_real_raw_get_from_string ( "123,45", NULL, NULL );
    CU_ASSERT_EQUAL ( 12345, val.mantissa );
    CU_ASSERT_EQUAL ( 2, val.exponent );

/*     // dot decimal separator  */
    val = gsb_real_raw_get_from_string ( "123.45", NULL, NULL );
    CU_ASSERT_EQUAL ( 12345, val.mantissa );
    CU_ASSERT_EQUAL ( 2, val.exponent );

/*     // decimal separator with spaces  */
    val = gsb_real_raw_get_from_string ( "  123  .  45  ", NULL, NULL );
    CU_ASSERT_EQUAL ( 12345, val.mantissa );
    CU_ASSERT_EQUAL ( 2, val.exponent );

/*     // decimal separator with space, and zeroes  */
    val = gsb_real_raw_get_from_string ( "0 0123.4500", NULL, NULL );
    CU_ASSERT_EQUAL ( 1234500, val.mantissa );
    CU_ASSERT_EQUAL ( 4, val.exponent );

/*     // decimal separator with space, zero, and value less than 1  */
    val = gsb_real_raw_get_from_string ( "  0.123  45 ", NULL, NULL );
    CU_ASSERT_EQUAL ( 12345, val.mantissa );
    CU_ASSERT_EQUAL ( 5, val.exponent );

/*     // decimal separator with space, no zero, and value less than 1  */
    val = gsb_real_raw_get_from_string ( "  .123  45 ", NULL, NULL );
    CU_ASSERT_EQUAL ( 12345, val.mantissa );
    CU_ASSERT_EQUAL ( 5, val.exponent );

/*     // multiple decimal separators ==> error  */
    val = gsb_real_raw_get_from_string ( "  123  .  45 . ", NULL, NULL );
    CU_ASSERT_EQUAL ( G_MININT64, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // large number  */
    val = gsb_real_raw_get_from_string ( "21000000", NULL, NULL );
    CU_ASSERT_EQUAL ( 21000000, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // large number with thousands separators  */
    val = gsb_real_raw_get_from_string ( "21 000 000", NULL, NULL );
    CU_ASSERT_EQUAL ( 21000000, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // large number (largest-1) with thousands separators  */
    val = gsb_real_raw_get_from_string ( " 2 147 483 646 ", NULL, NULL );
    CU_ASSERT_EQUAL ( 2147483646, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // largest positive number with thousands separators  */
    val = gsb_real_raw_get_from_string ( " 2 147 483 647 ", NULL, NULL );
    CU_ASSERT_EQUAL ( 2147483647, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // too large positive number ==> error  */
    val = gsb_real_raw_get_from_string ( " 2 147 483 648 ", NULL, NULL );
    CU_ASSERT_EQUAL ( 2147483648, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // too large positive number ==> error  */
    val = gsb_real_raw_get_from_string ( " 2 147 483 649 ", NULL, NULL );
    CU_ASSERT_EQUAL ( 2147483649, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // very large positive number ==> error  */
    val = gsb_real_raw_get_from_string ( " 112 147 483 649 ", NULL, NULL );
    CU_ASSERT_EQUAL ( 112147483649, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // 0 with positive sign  */
    val = gsb_real_raw_get_from_string ( "+0", NULL, NULL );
    CU_ASSERT_EQUAL ( 0, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // 0 with negative sign  */
    val = gsb_real_raw_get_from_string ( "-0", NULL, NULL );
    CU_ASSERT_EQUAL ( 0, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // negative number  */
    val = gsb_real_raw_get_from_string ( "-1", NULL, NULL );
    CU_ASSERT_EQUAL ( -1, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // positive number  */
    val = gsb_real_raw_get_from_string ( "+1", NULL, NULL );
    CU_ASSERT_EQUAL ( 1, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // negative two digits  */
    val = gsb_real_raw_get_from_string ( "-12", NULL, NULL );
    CU_ASSERT_EQUAL ( -12, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // positive two digits  */
    val = gsb_real_raw_get_from_string ( "+12", NULL, NULL );
    CU_ASSERT_EQUAL ( 12, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // negative sign at unexpected position ==> error  */
    val = gsb_real_raw_get_from_string ( "1-2", NULL, NULL );
    CU_ASSERT_EQUAL ( G_MININT64, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // positive sign at unexpected position ==> error  */
    val = gsb_real_raw_get_from_string ( "1+2", NULL, NULL );
    CU_ASSERT_EQUAL ( G_MININT64, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // two signs ==> error  */
    val = gsb_real_raw_get_from_string ( "-1+2", NULL, NULL );
    CU_ASSERT_EQUAL ( G_MININT64, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // two signs ==> error  */
    val = gsb_real_raw_get_from_string ( "+1-2", NULL, NULL );
    CU_ASSERT_EQUAL ( G_MININT64, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // two signs ==> error  */
    val = gsb_real_raw_get_from_string ( "--1", NULL, NULL );
    CU_ASSERT_EQUAL ( G_MININT64, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // decimal separator, no zero, and value less than 1  */
    val = gsb_real_raw_get_from_string ( "-.12345", NULL, NULL );
    CU_ASSERT_EQUAL ( -12345, val.mantissa );
    CU_ASSERT_EQUAL ( 5, val.exponent );

/*     // large negative number (largest+1) with thousands separators  */
    val = gsb_real_raw_get_from_string ( " -2 147 483 646 ", NULL, NULL );
    CU_ASSERT_EQUAL ( -2147483646, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // largest negative number with thousands separators  */
    val = gsb_real_raw_get_from_string ( " -2 147 483 647 ", NULL, NULL );
    CU_ASSERT_EQUAL ( -2147483647, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // too large negative number ==> error  */
    val = gsb_real_raw_get_from_string ( " -2 147 483 648 ", NULL, NULL );
    CU_ASSERT_EQUAL ( -2147483648, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // too large negative number ==> error  */
    val = gsb_real_raw_get_from_string ( " -2 147 483 649 ", NULL, NULL );
    CU_ASSERT_EQUAL ( -2147483649, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // too large negative number ==> error  */
    val = gsb_real_raw_get_from_string ( " -112 147 483 649 ", NULL, NULL );
    CU_ASSERT_EQUAL ( -112147483649, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );

/*     // error number as string ==> error  */
    val = gsb_real_raw_get_from_string ( ERROR_REAL_STRING, NULL, NULL );
    CU_ASSERT_EQUAL ( G_MININT64, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );
}

void gsb_real_cunit__gsb_real_raw_get_from_string__locale()
{
    gsb_real val;
/*     // locale's thousands separator override comma  */
    val = gsb_real_raw_get_from_string ( "1,000.00", ",", "." );
    CU_ASSERT_EQUAL ( 100000, val.mantissa );
    CU_ASSERT_EQUAL ( 2, val.exponent );

/*     // utf8 thousands separator (non-break space)  */
    gchar tmp[] = { '1', 0xC2, 0xA0, '0', '0', '0', '.', '0', '0', 0x0 };
    val = gsb_real_raw_get_from_string ( tmp, "\xC2\xA0", NULL );
    CU_ASSERT_EQUAL ( 100000, val.mantissa );
    CU_ASSERT_EQUAL ( 2, val.exponent );

/*     // non-break space used as separator, but locale says somthing else  */
    val = gsb_real_raw_get_from_string ( tmp, "\xC2\xA1", NULL );
    CU_ASSERT_EQUAL ( G_MININT64, val.mantissa );
    CU_ASSERT_EQUAL ( 0, val.exponent );
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
    gchar *currency_symbol = "<€>";
        
    n.mantissa = 1;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>0<.>01<€>", s);
    g_free(s);

    n.mantissa = 10;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>0<.>10<€>", s);
    g_free(s);

    n.mantissa = 31415;
    n.exponent = 1;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>3< >141<.>5<€>", s);
    g_free(s);

    n.mantissa = 31415;
    n.exponent = 9;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>0<.>000031415<€>", s);
    g_free(s);

    n.mantissa = 0x7FFFFFFF;
    n.exponent = 0;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>2< >147< >483< >647<.>0<€>", s);
    g_free(s);

    n.mantissa = 0x7FFFFFFF;
    n.exponent = 1;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>214< >748< >364<.>7<€>", s);
    g_free(s);

    n.mantissa = 0x7FFFFFFF;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>21< >474< >836<.>47<€>", s);
    g_free(s);

    n.mantissa = -2147483649;
    n.exponent = 0;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<->2< >147< >483< >649<.>0<€>", s);
    g_free(s);

    n.mantissa = -2147483649;
    n.exponent = 1;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<->214< >748< >364<.>9<€>", s);
    g_free(s);

    n.mantissa = -2147483649;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<->21< >474< >836<.>49<€>", s);
    g_free(s);

    n.mantissa = 2100000000;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>21< >000< >000<.>00<€>", s);
    g_free(s);

    conv.p_sep_by_space = 1;
    n.mantissa = 123;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>1<.>23 <€>", s);
    g_free(s);

    conv.p_sep_by_space = 1;
    n.mantissa = 123;
    n.exponent = 2;
    s = gsb_real_raw_format_string(n, &conv, currency_symbol);
    CU_ASSERT_STRING_EQUAL("<+>1<.>23 <€>", s);
    g_free(s);
}


void gsb_real_cunit__gsb_real_get_string_with_currency ( void )
{
    gchar *s;
    gsb_real n;
    gint currency_number = 1;

    n.mantissa = 0;
    n.exponent = 0;
    s = gsb_real_get_string_with_currency ( n, currency_number, FALSE );
    CU_ASSERT_STRING_EQUAL("0", s);
    g_free(s);

    n.mantissa = 31415;
    n.exponent = 100;
    s = gsb_real_get_string_with_currency ( n, currency_number, FALSE );
    CU_ASSERT_STRING_EQUAL(ERROR_REAL_STRING, s);
    g_free(s);

    n.mantissa = 31415;
    n.exponent = -1;
    s = gsb_real_get_string_with_currency ( n, currency_number, FALSE );
    CU_ASSERT_STRING_EQUAL(ERROR_REAL_STRING, s);
    g_free(s);

    n.mantissa = 0x7fffffffffffffff + 1;
    n.exponent = 2;
    s = gsb_real_get_string_with_currency ( n, currency_number, FALSE );
    CU_ASSERT_STRING_EQUAL(ERROR_REAL_STRING, s);
}


void gsb_real_cunit__gsb_real_normalize()
{
    gsb_real a;
    gsb_real b;
    a.mantissa = 1;
    a.exponent = 0;
    b.mantissa = 31415;
    b.exponent = 4;
    CU_ASSERT_EQUAL ( TRUE, gsb_real_normalize ( &a, &b ) );
    CU_ASSERT_EQUAL ( 10000, a.mantissa );
    CU_ASSERT_EQUAL ( 4, a.exponent );
    CU_ASSERT_EQUAL ( 31415, b.mantissa );
    CU_ASSERT_EQUAL ( 4, b.exponent );

    a.mantissa = 0x7FFFFFFF;
    a.exponent = 1;
    b.mantissa = 11;
    b.exponent = 0;
    CU_ASSERT_EQUAL ( TRUE, gsb_real_normalize ( &a, &b ) );
    CU_ASSERT_EQUAL ( 0x7FFFFFFF, a.mantissa );
    CU_ASSERT_EQUAL ( 1, a.exponent );
    CU_ASSERT_EQUAL ( 110, b.mantissa );
    CU_ASSERT_EQUAL ( 1, b.exponent );

    a.mantissa = 11;
    a.exponent = 0;
    b.mantissa = 0x7FFFFFFF;
    b.exponent = 1;
    CU_ASSERT_EQUAL ( TRUE, gsb_real_normalize ( &a, &b ) );
    CU_ASSERT_EQUAL ( 110, a.mantissa );
    CU_ASSERT_EQUAL ( 1, a.exponent );
    CU_ASSERT_EQUAL ( 0x7FFFFFFF, b.mantissa );
    CU_ASSERT_EQUAL ( 1, b.exponent );

    a.mantissa = 0x7FFFFFFF;
    a.exponent = 1;
    b.mantissa = 0x7FFFFFFF;
    b.exponent = 0;
    CU_ASSERT_EQUAL ( TRUE, gsb_real_normalize ( &a, &b ) );
    CU_ASSERT_EQUAL ( 0x7FFFFFFF, a.mantissa );
    CU_ASSERT_EQUAL ( 1, a.exponent );
    CU_ASSERT_EQUAL ( 0x4FFFFFFF6, b.mantissa );
    CU_ASSERT_EQUAL ( 1, b.exponent );
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
    CU_ASSERT_EQUAL(0x80000001, r.mantissa);
    CU_ASSERT_EQUAL(0, r.exponent);
    
    a.mantissa = 0x80000001;
    a.exponent = 0;
    b.mantissa = -2;
    b.exponent = 0;
    r = gsb_real_add(a, b);
    CU_ASSERT_EQUAL(0x7FFFFFFF, r.mantissa);
    CU_ASSERT_EQUAL(0, r.exponent);

    a.mantissa = G_MININT64;
    a.exponent = 0;
    b.mantissa = 100;
    b.exponent = 0;
    r = gsb_real_add(a, b);
    CU_ASSERT_EQUAL(G_MININT64, r.mantissa);
    CU_ASSERT_EQUAL(0, r.exponent);

    a.mantissa = 100;
    a.exponent = 0;
    b.mantissa = G_MININT64;
    b.exponent = 0;
    r = gsb_real_add(a, b);
    CU_ASSERT_EQUAL(G_MININT64, r.mantissa);
    CU_ASSERT_EQUAL(0, r.exponent);
}

void gsb_real_cunit__gsb_real_sub()
{
    gsb_real a = { -1, 0 };
    gsb_real b = { 31415, 4 };
    gsb_real r = gsb_real_sub ( a, b );
    CU_ASSERT_EQUAL ( -41415, r.mantissa );
    CU_ASSERT_EQUAL ( 4, r.exponent );
    
    a.mantissa = 0x7FFFFFFE;
    a.exponent = 0;
    b.mantissa = -1;
    b.exponent = 0;
    r = gsb_real_sub ( a, b );
    CU_ASSERT_EQUAL ( 0x7FFFFFFF, r.mantissa );
    CU_ASSERT_EQUAL ( 0, r.exponent );
    
    a.mantissa = 0x7FFFFFFF;
    a.exponent = 0;
    b.mantissa = -2;
    b.exponent = 0;
    r = gsb_real_sub ( a, b );
    CU_ASSERT_EQUAL ( 0x80000001, r.mantissa );
    CU_ASSERT_EQUAL ( 0, r.exponent );
    
    a.mantissa = 0x80000001;
    a.exponent = 0;
    b.mantissa = 2;
    b.exponent = 0;
    r = gsb_real_sub ( a, b );
    CU_ASSERT_EQUAL ( 0x7FFFFFFF, r.mantissa );
    CU_ASSERT_EQUAL  (0, r.exponent );

}

void gsb_real_cunit__gsb_real_mul()
{
    gsb_real a = { 12, 1 };
    gsb_real b = { 31415, 4 };
    gsb_real r = gsb_real_mul ( a, b );
    CU_ASSERT_EQUAL ( 37698, r.mantissa );
    CU_ASSERT_EQUAL ( 4, r.exponent );
    
    a.mantissa = 0x7FFFFFFF;
    a.exponent = 0;
    b.mantissa = 2;
    b.exponent = 0;
    r = gsb_real_mul ( a, b );
    CU_ASSERT_EQUAL ( 4294967294, r.mantissa );
    CU_ASSERT_EQUAL ( 0, r.exponent );
    
    a.mantissa = 0x7FFFFFFF;
    a.exponent = 0;
    b.mantissa = 9;
    b.exponent = 1;
    r = gsb_real_mul ( a, b );
    CU_ASSERT_EQUAL ( 19327352823, r.mantissa );
    CU_ASSERT_EQUAL ( 1, r.exponent );
    
    a.mantissa = G_MININT64;
    a.exponent = 0;
    b.mantissa = 2;
    b.exponent = 0;
    r = gsb_real_mul ( a, b );
    CU_ASSERT_EQUAL ( G_MININT64, r.mantissa );
    CU_ASSERT_EQUAL ( 0, r.exponent );

    a.mantissa = 100;
    a.exponent = 0;
    b.mantissa = G_MININT64;
    b.exponent = 0;
    r = gsb_real_mul(a, b);
    CU_ASSERT_EQUAL ( G_MININT64, r.mantissa );
    CU_ASSERT_EQUAL ( 0, r.exponent );

    a.mantissa = 22000;
    a.exponent = 0;
    b.mantissa = 100000;
    b.exponent = 0;
    r = gsb_real_mul ( a, b );
    CU_ASSERT_EQUAL ( 2200000000, r.mantissa );
    CU_ASSERT_EQUAL ( 0, r.exponent );

    a.mantissa = -22000;
    a.exponent = 0;
    b.mantissa = 100000;
    b.exponent = 0;
    r = gsb_real_mul ( a, b );
    CU_ASSERT_EQUAL ( -2200000000, r.mantissa );
    CU_ASSERT_EQUAL ( 0, r.exponent );
}


void gsb_real_cunit__gsb_real_adjust_exponent ( void )
{
    gsb_real a = {1, 0};
    gint b = 4;
    gsb_real r = gsb_real_adjust_exponent ( a, b );
    CU_ASSERT_EQUAL(10000, r.mantissa);
    CU_ASSERT_EQUAL(4, r.exponent);

    a.mantissa = -11926672494897;
    a.exponent = 9;
    b = 2;
    r = gsb_real_adjust_exponent ( a, b );
    CU_ASSERT_EQUAL(-1192667, r.mantissa);
    CU_ASSERT_EQUAL(2, r.exponent);
}


CU_pSuite gsb_real_cunit_create_suite ( void )
{
    CU_pSuite pSuite = CU_add_suite("gsb_real",
                                    gsb_real_cunit_init_suite,
                                    gsb_real_cunit_clean_suite);
    if(NULL == pSuite)
        return NULL;

    if ( ( NULL == CU_add_test( pSuite, "of gsb_real_get_from_string()",     gsb_real_cunit__gsb_real_get_from_string ) )
      || ( NULL == CU_add_test( pSuite, "of gsb_real_raw_get_from_string()", gsb_real_cunit__gsb_real_raw_get_from_string ) )
      || ( NULL == CU_add_test( pSuite, "of gsb_real_raw_get_from_string() with locale", gsb_real_cunit__gsb_real_raw_get_from_string__locale ) )
      || ( NULL == CU_add_test( pSuite, "of gsb_real_raw_format_string()",   gsb_real_cunit__gsb_real_raw_format_string ) )
      || ( NULL == CU_add_test( pSuite, "of gsb_real_gsb_real_normalize()",  gsb_real_cunit__gsb_real_normalize ) )
      || ( NULL == CU_add_test( pSuite, "of gsb_real_add()",                 gsb_real_cunit__gsb_real_add ) )
      || ( NULL == CU_add_test( pSuite, "of gsb_real_sub()",                 gsb_real_cunit__gsb_real_sub ) )
      || ( NULL == CU_add_test( pSuite, "of gsb_real_mul()",                 gsb_real_cunit__gsb_real_mul ) )
      || ( NULL == CU_add_test( pSuite, "of gsb_real_adjust_exponent()",     gsb_real_cunit__gsb_real_adjust_exponent ) )
      || ( NULL == CU_add_test( pSuite, "of gsb_real_get_string_with_currency()", gsb_real_cunit__gsb_real_get_string_with_currency ) )
       )
        return NULL;

    return pSuite;
}
