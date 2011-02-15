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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "bet_data_finance.h"
#include "bet_finance_ui.h"
#include "gsb_data_currency.h"
#include "gsb_file_save.h"
#include "structures.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
extern gint no_devise_totaux_categ;
/*END_EXTERN*/

gdouble bet_taux_step[] = { 0, 0.1, 0.01, 0.001, 0.0001 };

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
        taux_periodique = exp ( log ( 1.0 + ( taux / 100 ) ) / 12 ) - 1;

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

    str_number = utils_str_dtostr ( number, nbre_decimal, TRUE );
    result =  utils_str_safe_strtod ( str_number, NULL );

    g_free ( str_number );

    return result;
}

GDate *bet_data_finance_get_date_last_installment_paid ( GDate *date_depart )
{
    GDate *date_jour;
    GDateDay jour;

    date_jour = gdate_today ( );
    jour = g_date_get_day ( date_depart );

    if ( g_date_get_day ( date_jour ) > jour )
        g_date_add_months  ( date_jour, 1 );
    
    g_date_set_day ( date_jour, jour );

    return date_jour;
}
/**
 *
 *
 *
 *
 * */
gdouble bet_data_finance_get_total_cost ( struct_echeance *s_echeance )
{
    gdouble capital_du;
    gdouble cost;
    gdouble echeance = 0.0;
    gdouble interets;
    gdouble principal;
    gint index;


    capital_du = s_echeance -> capital;

    for ( index = 1; index <= s_echeance -> nbre_echeances; index++ )
    {
        interets = bet_data_finance_get_interets ( capital_du, s_echeance -> taux_periodique );

        if ( index == s_echeance -> nbre_echeances )
        {
            echeance = bet_data_finance_get_last_echeance (
                        capital_du,
                        interets,
                        s_echeance -> frais );
            principal = capital_du;
        }
        else
            principal = bet_data_finance_get_principal (
                        s_echeance -> total_echeance,
                        interets,
                        s_echeance -> frais );

        capital_du -= principal;
    }

    cost = ( s_echeance -> total_echeance * ( s_echeance -> nbre_echeances - 1 ) )
                        + echeance - s_echeance -> capital;

    return cost;
}


/**
 *
 *
 *
 *
 * */
void bet_data_finance_structure_amortissement_free ( struct_amortissement *s_amortissement )
{
    if ( s_amortissement -> str_date )
        g_free ( s_amortissement -> str_date );
    if ( s_amortissement -> str_echeance )
        g_free ( s_amortissement -> str_echeance );
    if ( s_amortissement -> str_frais )
        g_free ( s_amortissement -> str_frais );

    g_free ( s_amortissement );
}


/**
 *
 *
 *
 *
 * */
struct_amortissement *bet_data_finance_structure_amortissement_init ( void )
{
    struct_amortissement *s_amortissement;

    s_amortissement = g_malloc0 ( sizeof ( struct_amortissement ) );

    s_amortissement -> str_date = NULL;
    s_amortissement -> str_echeance = NULL;
    s_amortissement -> str_frais = NULL;

    return s_amortissement;
}


/**
 *
 *
 *
 *
 * */
void bet_data_finance_data_simulator_init ( void )
{
    etat.bet_capital = 1000.0;
    etat.bet_currency = no_devise_totaux_categ;
    etat.bet_taux_annuel = 4.0;
    etat.bet_index_duree = 0;
    etat.bet_frais = 0;
    etat.bet_type_taux = 1;
}


/**
 *
 *
 *
 *
 * */
gdouble bet_data_finance_get_bet_taux_step ( gint nbre_digits )
{
    return bet_taux_step[BET_TAUX_DIGITS];
}


/**
 *
 *
 *
 *
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
