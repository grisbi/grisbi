#ifndef _BET_DATA_FINANCE_H
#define _BET_DATA_FINANCE_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

#define BET_TAUX_DIGITS 2

/* structure échéance */
 typedef struct {
    gint duree;
    gint nbre_echeances;
    gint devise;
    gdouble taux;
    gdouble taux_periodique;
    gdouble capital;
    gdouble frais;
    gdouble echeance;
    gdouble total_echeance;
    gdouble total_cost;
}  struct_echeance;

/* structure amortissement */
 typedef struct {
    gint origin;
    gint num_echeance;
    gint devise;
    gdouble taux_periodique;
    gdouble interets;
    gdouble capital_du;
    gdouble principal;
    gdouble frais;
    gdouble echeance;
    gchar *str_date;
    gchar *str_echeance;
    gchar *str_frais;
}  struct_amortissement;


/* START_DECLARATION */
void bet_data_finance_data_simulator_init ( void );
gdouble bet_data_finance_get_bet_taux_step ( gint nbre_digits );
GDate *bet_data_finance_get_date_last_installment_paid ( GDate *date_depart );
gdouble bet_data_finance_get_last_echeance ( gdouble capital_du,
                        gdouble interets,
                        gdouble frais );
gdouble bet_data_finance_get_echeance ( gdouble capital,
                        gdouble taux_periodique,
                        gint duree );
gdouble bet_data_finance_get_frais_par_echeance ( gdouble capital,
                        gdouble taux_frais,
                        gint nbre_echeances );
gdouble bet_data_finance_get_interets ( gdouble capital_du, gdouble taux_periodique );
gdouble bet_data_finance_get_principal ( gdouble echeance,
                        gdouble interets,
                        gdouble frais );
gdouble bet_data_finance_get_taux_periodique ( gdouble taux, gint type_taux );
gdouble bet_data_finance_get_total_cost ( struct_echeance *s_echeance );
void bet_data_finance_structure_amortissement_free ( struct_amortissement *s_amortissement );
struct_amortissement *bet_data_finance_structure_amortissement_init ( void );
gdouble bet_data_finance_troncate_number ( gdouble number, gint nbre_decimal );
/* END_DECLARATION */


#endif /*_BET_DATA_FINANCE*/
