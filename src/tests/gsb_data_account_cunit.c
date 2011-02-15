/* ************************************************************************** */
/*                                                                            */
/*                                  gsb_data_account_cunit                    */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	          */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2009 Mickaël Remars (grisbi@remars.com)               */
/* 			http://www.grisbi.org				                              */
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
 * \file gsb_data_account_cunit.c
 * cunit tests for gsb_data_account
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/* START_INCLUDE */
#include "gsb_data_account_cunit.h"
#include "utils_dates.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
/* END_INCLUDE */

/* START_STATIC */
static void gsb_data_account_cunit__gsb_data_account_calculate_current_and_marked_balances();
static int gsb_data_account_cunit_clean_suite(void);
static int gsb_data_account_cunit_init_suite(void);
/* END_STATIC */

/* START_EXTERN */
/* END_EXTERN */

/* The suite initialization function.
 * Returns zero on success, non-zero otherwise.
 */
int gsb_data_account_cunit_init_suite(void)
{
    return 0;
}

/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int gsb_data_account_cunit_clean_suite(void)
{
    return 0;
}

void gsb_data_account_cunit__gsb_data_account_calculate_current_and_marked_balances()
{
    GDate *date = gdate_today() ;

    gint account_number = gsb_data_account_new(GSB_TYPE_BANK);
    CU_ASSERT_EQUAL(1, account_number);

    gint cur_number = gsb_data_currency_new("EUR");
    CU_ASSERT_EQUAL(1, cur_number);
    CU_ASSERT_EQUAL(TRUE, gsb_data_currency_set_floating_point(cur_number, 2));
    CU_ASSERT_EQUAL(TRUE, gsb_data_account_set_currency(account_number, cur_number));
    
    gint tr_number_1 = gsb_data_transaction_new_transaction(account_number);
    CU_ASSERT_EQUAL(1, tr_number_1);
    gsb_real amount = { 2100000021, 2 };
    CU_ASSERT_EQUAL(TRUE, gsb_data_transaction_set_amount(tr_number_1, amount));
    CU_ASSERT_EQUAL(TRUE, gsb_data_transaction_set_date(tr_number_1, date));
    gsb_real balance = gsb_data_account_calculate_current_and_marked_balances(account_number);
    CU_ASSERT_EQUAL(2100000021, balance.mantissa);
    CU_ASSERT_EQUAL(2, balance.exponent);
    
    gint tr_number_2 = gsb_data_transaction_new_transaction(account_number);
    CU_ASSERT_EQUAL(2, tr_number_2);
    amount.mantissa = 100000000;
    CU_ASSERT_EQUAL(TRUE, gsb_data_transaction_set_amount(tr_number_2, amount));
    CU_ASSERT_EQUAL(TRUE, gsb_data_transaction_set_date(tr_number_2, date));
    balance = gsb_data_account_calculate_current_and_marked_balances(account_number);
    /* Lose of precision but no overflow */
    /* Previous test
    CU_ASSERT_EQUAL(0x80000000, balance.mantissa);
    CU_ASSERT_EQUAL(0, balance.exponent);
    */
    CU_ASSERT_EQUAL(2200000021, balance.mantissa);
    CU_ASSERT_EQUAL(2, balance.exponent);
    
    gint tr_number_3 = gsb_data_transaction_new_transaction(account_number);
    CU_ASSERT_EQUAL(3, tr_number_3);
    amount.mantissa = -100000000;
    CU_ASSERT_EQUAL(TRUE, gsb_data_transaction_set_amount(tr_number_3, amount));
    CU_ASSERT_EQUAL(TRUE, gsb_data_transaction_set_date(tr_number_3, date));
    balance = gsb_data_account_calculate_current_and_marked_balances(account_number);
    CU_ASSERT_EQUAL(2100000021, balance.mantissa);
    CU_ASSERT_EQUAL(2, balance.exponent);

    gsb_data_transaction_remove_transaction( tr_number_3 );
    gsb_data_transaction_remove_transaction( tr_number_2 );
    gsb_data_transaction_remove_transaction( tr_number_1 );

    gsb_data_account_delete( account_number );
    g_date_free(date);
}

CU_pSuite gsb_data_account_cunit_create_suite()
{
    CU_pSuite pSuite = CU_add_suite("gsb_data_account",
                                    gsb_data_account_cunit_init_suite,
                                    gsb_data_account_cunit_clean_suite);
    if(NULL == pSuite)
        return NULL;

    if((NULL == CU_add_test(pSuite, "of gsb_data_account()", gsb_data_account_cunit__gsb_data_account_calculate_current_and_marked_balances))
       )
        return NULL;

    return pSuite;
}
