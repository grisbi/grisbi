#ifndef _BET_DATA_FINANCE_H
#define _BET_DATA_FINANCE_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
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
gdouble bet_data_finance_get_total_cost ( gdouble capital,
                        gdouble mensualite,
                        gdouble duree );
gdouble bet_data_finance_troncate_number ( gdouble number, gint nbre_decimal );
/* END_DECLARATION */


#endif /*_BET_DATA_FINANCE*/
