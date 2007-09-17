/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2007 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_category.c
 * fuctions to deal with the categories
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_category.h"
#include "./include.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



/* declarations of the initial general categories */
const gchar *debit_general_category_list [] = {
    N_("Food : Bar"),
    N_("Food : Baker"),
    N_("Food : Canteen"),
    N_("Food : Sweets"),
    N_("Food : Misc"),
    N_("Food : Grocery"),
    N_("Food : Restaurant"),
    N_("Food : Self-service"),
    N_("Food : Supermarket"),
    N_("Pets : Food"),
    N_("Pets : Various supplies"),
    N_("Pets : Grooming"),
    N_("Pets : Veterinary surgeon"),
    N_("Insurance : Car"),
    N_("Insurance : Health"),
    N_("Insurance : House"),
    N_("Insurance : Civil liability"),
    N_("Insurance : Life"),
    N_("Car : Fuel"),
    N_("Car : Repairs"),
    N_("Car : Maintenance"),
    N_("Car : Parking"),
    N_("Car : Fines"),
    N_("Misc."),
    N_("Gifts"),
    N_("Children : Nurse"),
    N_("Children : Misc."),
    N_("Children : Studies"),
    N_("Studies : Lessons"),
    N_("Studies : Scool fees"),
    N_("Studies : Books"),
    N_("Miscelanious : Gifts"),
    N_("Financial expenses : Miscelanious"),
    N_("Financial expenses : Bank charges"),
    N_("Financial expenses : Loan/Mortgage"),
    N_("Financial expenses : Charges"),
    N_("Financial expenses : Refunding"),
    N_("Professionnal expenses : Non refundable"),
    N_("Professionnal expenses : Refundable"),
    N_("Taxes : Miscelanious"),
    N_("Taxes : Income"),
    N_("Taxes : Land"),
    N_("Housing : Hotel"),
    N_("Housing : Rent"),
    N_("Housing : TV"),
    N_("Housing : Furnitures"),
    N_("Housing : Charges"),
    N_("Housing : Heating"),
    N_("Housing : Decoration"),
    N_("Housing : Water"),
    N_("Housing : Electricity"),
    N_("Housing : White products"),
    N_("Housing : Equipment"),
    N_("Housing : Gaz"),
    N_("Housing : Garden"),
    N_("Housing : House keeper"),
    N_("Housing : Phone"),
    N_("Housing : Mobile Phone"),
    N_("Leisures : Snooker"),
    N_("Leisures : Bowling"),
    N_("Leisures : Movies"),
    N_("Leisures : Night club"),
    N_("Leisures : IT"),
    N_("Leisures : Games"),
    N_("Leisures : Books"),
    N_("Leisures : Parks"),
    N_("Leisures : Concerts"),
    N_("Leisures : Sports"),
    N_("Leisures : Video"),
    N_("Leisures : Travels"),
    N_("Leisures : Equipment"),
    N_("Leisures : Museums/Exhibitions"),
    N_("Loan/Mortgage : Capital"),
    N_("Health : Insurance"),
    N_("Health : Dentist"),
    N_("Health : Hospital"),
    N_("Health : Kinesitherapist"),
    N_("Health : Doctor"),
    N_("Health : Ophtalmologist"),
    N_("Health : Osteopath"),
    N_("Health : Chemist"),
    N_("Health : Social security"),
    N_("Care : Hairdresser"),
    N_("Care : Clothing"),
    N_("Transport : Bus"),
    N_("Transport : Metro"),
    N_("Transport : Toll"),
    N_("Transport : Train"),
    N_("Transport : Tramway"),
    N_("Transport : Travels"),
    N_("Transport : Train"),
    N_("Holiday : Housing"),
    N_("Holiday : Visits"),
    N_("Holiday : Travels"),
    NULL };

const gchar *credit_general_category_list [] = {
    N_("Other incomes : Unemployment benefit"),
    N_("Other incomes : Family allowance"),
    N_("Other incomes : Tax credit"),
    N_("Other incomes : Gamble"),
    N_("Other incomes : Mutual insurance"),
    N_("Other incomes : Social security"),
    N_("Retirement : Retirement Fund"),
    N_("Retirement : Pension"),
    N_("Retirement : Supplementary pension"),
    N_("Investment incomes : Dividends"),
    N_("Investment incomes : Interests"),
    N_("Investment incomes : Capital gain"),
    N_("Salary : Overtime"),
    N_("Salary : Leave allowance"),
    N_("Salary : Misc. premiums"),
    N_("Salary : Success fee"),
    N_("Salary : Net salary"),
    N_("Misc. incomes : Gifts"),
    N_("Misc. incomes : Refunds"),
    N_("Misc. incomes : Second hand sales"),
    NULL
};





/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

