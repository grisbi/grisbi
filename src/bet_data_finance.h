#ifndef _BET_DATA_FINANCE_H
#define _BET_DATA_FINANCE_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

#define BET_TAUX_DIGITS 3
#define BET_PERCENTAGE_FEES_DIGITS 5
typedef struct _AmortissementStruct		AmortissementStruct;
typedef struct _EcheanceStruct			EcheanceStruct;
typedef struct _LoanStruct				LoanStruct;

/* structure amortissement */
struct _AmortissementStruct {
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
};

/* structure échéance */
struct _EcheanceStruct {
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
};

/* structure loan */
struct _LoanStruct {
	guint		number;					/* numero du pret */
	gint 		account_number;
	gint		version_number;			/* numero de prêt : - 1 non initialisé, 0 = nouveau prêt, > 1 numéro de renégociation  */
	gboolean	invers_cols_cap_ech;	/* inverse les colonnes capital du et échéance dans les tableaux d'amortissement */
	gint		currency_number;		/* devise du compte de passif et devise des opérations planifiées */
	gdouble		capital;
	gint 		duree;					/* mois */
	GDate *		first_date;				/* date de la première échéance */
	gdouble		amount_fees;			/* frais fixes par échéance */
	gdouble		annual_rate;			/* taux annuel */
	gdouble		fixed_due_amount;		/* echeance fixe pour type de taux = 2 */
	gdouble		percentage_fees;		/* frais en pourcentage pour type de taux = 2 */
	gint 		type_taux;				/* 0 = CAGR 1 = proportionnel 2 = echeance fixe et frais en % du capital restant */
	gboolean	first_is_different;		/* si la 1ère échéance est différente */
	gdouble		first_capital;			/* capital de la 1ère échéance */
	gdouble		first_fees;				/* frais de la première échéance */
	gdouble		first_interests;		/* intérêts de la 1ère échéance */
	gdouble		other_echeance_amount;	/* valeur des autres échéances si first_is_different = TRUE */
	gdouble		capital_du;				/* capital restant du après la dernière échéance payée. */
	gint		associated_account;		/* compte associé de l'opération planifiée */
	gint		associated_scheduled;	/* scheduled number for the associated account */
	gint		associated_frequency;
	gboolean	init_sch_with_loan;		/* Initialise l'opération planifiée avec le tableau d'amortissement */
};


/* START_DECLARATION */
void 					bet_data_finance_data_simulator_init 				(void);
gdouble 				bet_data_finance_get_bet_taux_step 					(gint nbre_digits);
GDate *					bet_data_finance_get_date_last_installment_paid 	(GDate *date_depart);
gdouble 				bet_data_finance_get_last_echeance 					(gdouble capital_du,
																			 gdouble interets,
																			 gdouble frais);
gdouble 				bet_data_finance_get_echeance 						(gdouble capital,
																			 gdouble taux_periodique,
																			 gint duree);
gdouble 				bet_data_finance_get_fees_for_type_taux_2			(gdouble capital_du,
																			 gdouble percentage_fees);
gdouble 				bet_data_finance_get_frais_par_echeance 			(gdouble capital,
																			 gdouble taux_frais,
																			 gint nbre_echeances);
gdouble 				bet_data_finance_get_interets 						(gdouble capital_du,
																			 gdouble taux_periodique);
gdouble 				bet_data_finance_get_principal 						(gdouble echeance,
																			 gdouble interets,
																			 gdouble frais);
gdouble 				bet_data_finance_get_taux_periodique 				(gdouble taux,
																			 gint type_taux);
gdouble 				bet_data_finance_get_total_cost 					(EcheanceStruct *s_echeance);
void 					bet_data_finance_structure_amortissement_free 		(AmortissementStruct *s_amortissement);
AmortissementStruct *	bet_data_finance_structure_amortissement_init 		(void);
gdouble 				bet_data_finance_troncate_number 					(gdouble number,
																			 gint nbre_decimal);
void	 				bet_data_loan_add_item 								(LoanStruct *s_loan);
gboolean 				bet_data_loan_get_loan_first_is_different			(gint account_number);
void 					bet_data_loan_delete_all_loans 						(void);
LoanStruct *			bet_data_loan_get_last_loan_struct_by_account		(gint account_number);
GSList *				bet_data_loan_get_loan_list 						(void);
GSList *				bet_data_loan_get_loan_list_by_account 				(gint account_number);
gdouble 				bet_data_loan_get_other_echeance_amount 			(gint account_number);
gboolean 				bet_data_loan_remove_item 							(LoanStruct *s_loan);
LoanStruct *			bet_data_loan_struct_loan_init 						(void);
void					bet_data_loan_struct_loan_free						(LoanStruct *s_loan);
void 					bet_data_loan_struct_loan_free_by_account 			(gint account_number);
/* END_DECLARATION */


#endif /*_BET_DATA_FINANCE*/
