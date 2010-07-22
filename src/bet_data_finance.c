/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2010 Pierre Biava (grisbi@pierre.biava.name)             */
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

#include "include.h"
#include <config.h>

/*START_INCLUDE*/
#include "bet_data_finance.h"
#include "bet_finance_ui.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/**
 *
 *
 *
 *
 * */
gdouble bet_data_finance_get_echeance ( gdouble capital,
                        gdouble taux_periodique,
                        gint nbre_echeances )
{
    gdouble coeff;
    gdouble number;

    coeff = pow ( ( 1 + taux_periodique ), nbre_echeances );

    number = ( capital * taux_periodique * coeff ) / ( coeff - 1 );
    number = bet_data_finance_troncate_number ( number, 2 );

    return number;
}


/**
 *
 *
 *
 *
 * */
gdouble bet_data_finance_get_taux_periodique ( gdouble taux, gint type_taux )
{
    gdouble taux_periodique;

    if ( type_taux )
        taux_periodique = ( taux / 100 ) / 12;
    else
        //~ taux_periodique = pow ( 1 + ( taux / 100 ), (1/12) ) - 1;
        taux_periodique = 0.004868;

    return taux_periodique;
}


/**
 *
 *
 *
 *
 * */
gdouble bet_data_finance_get_frais_par_echeance ( gdouble capital,
                        gdouble taux_frais,
                        gint nbre_echeances )
{
    gdouble number;

    number = capital * taux_frais / 100 / nbre_echeances;
    number = bet_data_finance_troncate_number ( number, 2 );

    return number;
}


/**
 *
 *
 *
 *
 * */
gdouble bet_data_finance_get_interets ( gdouble capital_du, gdouble taux_periodique )
{
    gdouble number;

    number = capital_du * taux_periodique;
    number = bet_data_finance_troncate_number ( number, 2 );

    return number;
}


/**
 *
 *
 *
 *
 * */
gdouble bet_data_finance_get_principal ( gdouble echeance,
                        gdouble interets,
                        gdouble frais )
{
    gdouble number;

    number = echeance - interets - frais;
    number = bet_data_finance_troncate_number ( number, 2 );

    return number;
}


/**
 *
 *
 *
 *
 * */
gdouble bet_data_finance_get_last_echeance ( gdouble capital_du,
                        gdouble interets,
                        gdouble frais )
{
gdouble number;

    number = capital_du + interets + frais;
    number = bet_data_finance_troncate_number ( number, 2 );

    return number;
}


/**
 *
 *
 *
 *
 * */
gdouble bet_data_finance_troncate_number ( gdouble number, gint nbre_decimal )
{
    gchar buffer[256];
    gchar *str_number;
    gchar *format;
    gint nbre_char;
    gdouble result;

    format = g_strconcat ( "%.", utils_str_itoa ( nbre_decimal ), "f", NULL );

    nbre_char = g_sprintf ( buffer, format, number );
    str_number = g_strndup ( buffer, nbre_char + 1 );

    result =  my_strtod ( str_number, NULL );

    return result;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
