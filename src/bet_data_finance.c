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
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/**
 * retourne l'échéance hors frais
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

    if ( taux_periodique == 0 )
        number = capital / nbre_echeances;
    else
        number = ( capital * taux_periodique * coeff ) / ( coeff - 1 );
    number = bet_data_finance_troncate_number ( number, 2 );

    return number;
}


/**
 * Calcule le taux périodique d'un intérêt
 *
 * /param taux d'intérêt
 * /param type de taux : actuariel ou proportionnel
 *
 * /return a double
 *
 * */
gdouble bet_data_finance_get_taux_periodique ( gdouble taux, gint type_taux )
{
    gdouble taux_periodique;

    if ( type_taux )
        taux_periodique = ( taux / 100 ) / 12;
    else
        /* taux_periodique = pow ( 1 + ( taux / 100 ), (1/12) ) - 1; */
        taux_periodique = 0.004868;

    return taux_periodique;
}


/**
 * Calcule le montant des frais par échéances ( en général des assurances)
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
 * Calcule les intérêts par période
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
 * retourne le capital remboursé
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
 * Calcule la dernière échéance du prêt
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
 * arrondit à la précision demandée
 *
 * /param nombre à arrondir
 * /param nombre de chiffres significatifs
 *
 * /return a double number
 *
 * */
gdouble bet_data_finance_troncate_number ( gdouble number, gint nbre_decimal )
{
    gchar *str_number;
    gdouble result;

    str_number = utils_str_dtostr ( number, nbre_decimal, FALSE );
    result =  my_strtod ( str_number, NULL );

    g_free ( str_number );

    return result;
}

GDate *bet_data_finance_get_date_last_installment_paid ( GDate *date_depart )
{
    GDate *date_jour;
    GDateDay jour;

    date_jour = gdate_today ( );
    jour = g_date_get_day ( date_depart );

    if ( g_date_get_day ( date_jour ) < jour )
        g_date_subtract_months  ( date_jour, 1 );
    
    g_date_set_day ( date_jour, jour );

    return date_jour;
}
/**
 *
 *
 *
 *
 * */
gdouble bet_data_finance_get_total_cost ( gdouble capital,
                        gdouble mensualite,
                        gdouble duree )
{
    gdouble cost;

    
    cost = ( ( mensualite + 0.01) * duree ) - capital;

    return cost;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
