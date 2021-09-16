#ifndef _BALANCE_ESTIMATE_DATA_H
#define _BALANCE_ESTIMATE_DATA_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* nombre de colonnes du tableau des prévisions */
#define BET_ARRAY_COLUMNS 		 5
#define PREV_MONTH_MAX	 		60.0

typedef struct _BetRange					BetRange;
typedef struct _HistData					HistData;
typedef struct _FutureData					FuturData;
typedef struct _TransfertData				TransfertData;
typedef struct _TransactionCurrentFyear		TransactionCurrentFyear;

struct _BetRange					/* sbr */
{
	gboolean 		first_pass;
	GsbReal 		current_fyear;
	GsbReal 		current_balance;
};

/* utilisée pour gérer la liste des données historiques  */
struct _HistData					/* shd */
{
	gint 			account_nb;
	gint 			div_number;
	gint 			origin;
	gboolean 		div_edited;
	GHashTable *	sub_div_list;
	BetRange *		sbr;			/* ajout pour fusionner avec HistList */
	GsbReal 		amount;
};

/* utilisée pour créer des pseudos opérations planifiées dans le tableau des prévisions */
struct _FutureData					/* sbr */
{
	gint 			number;
	gint 			account_number;
	GDate *			date;
	GsbReal 		amount;
	guint 			fyear_number;
	gint 			payment_number;
	gint 			party_number;
	gint 			category_number;
	gint 			sub_category_number;
	gint 			budgetary_number;
	gint 			sub_budgetary_number;
	gchar *			notes;

	gboolean 		is_transfert;
	gint 			account_transfert;

	gint 			frequency;					/*  0=once, 1=week, 2=month, 5=year, 6=perso */
	gint 			user_interval;				/*  0=days, 1= week 2=month, 3=years */
	gint 			user_entry;
	GDate *			limit_date;
	gint			mother_row;					/* if frequency > 0 */
};

/* utilisée pour gérer les comptes à débit différé dans le tableau des prévisions */
struct _TransfertData				/* std */
{
	gint 			number;
	gint 			account_number;				/* numéro du compte principal concerné */
	gint 			type;						/* type de compte carte : 0 = account 1 = partial balance */
	gint			card_account_number;		/* Account number or partial balance number */
	gint 			replace_transaction;	   	/* remplace la transaction plannifiée dans le compte principal */
	gint 			direct_debit;			  	/* si = 1 création de la transaction dans le compte principal */
	GDate *			date_debit;					/* date de l'opération du compte principal */
	GDate *			date_bascule;				/* date de début de nouveau mois dans le compte à débit différé */
	gint 			main_choice_debit_day;		/* selection du calcul de la date de prélèvement */
	gint 			main_payee_number;		 	/* tiers de l'opération du compte principal */
	gint 			main_payment_number;	   	/* moyen de payement de l'opération du compte principal */
	gint 			main_category_number;	  	/* catégorie de l'opération du compte principal */
	gint 			main_sub_category_number;  	/* sous-catégorie de l'opération du compte principal */
	gint 			main_budgetary_number;	 	/* IB de l'opération du compte principal */
	gint 			main_sub_budgetary_number; 	/* sous IB de l'opération du compte principal */
	gint			card_choice_bascule_day;	/* si date de bascule est non ouvrable force le premier jour ouvrable suivant */
	gint 			card_payee_number;		 	/* tiers de l'opération du compte à débit différé */
	gint 			card_payment_number;	   	/* moyen de payement de l'opération du compte  à débit différé */
	gint 			card_category_number;	  	/* catégorie de l'opération du compte à débit différé */
	gint 			card_sub_category_number;  	/* sous-catégorie de l'opération du compte à débit différé */
	gint 			card_budgetary_number;	 	/* IB de l'opération du compte à débit différé */
	gint 			card_sub_budgetary_number; 	/* sous IB de l'opération du compte à débit différé */
};

/* utilisée pour créer les opérations du tableau des prévisions */
struct _TransactionCurrentFyear
{
	gint 		transaction_number;
	gint 		type_de_transaction;			/* 0 = historique 1 = current fyear 2 = hist and current fyear */
	gint 		div_nb;
	gint 		sub_div_nb;
	GDate *		date;
	GsbReal 	amount;
};

/* noms des colonnes du tree_view des previsions */
enum BetEstimationTreeColumns
{
	SPP_ESTIMATE_TREE_SELECT_COLUMN,			/* select column for the balance */
	SPP_ESTIMATE_TREE_ORIGIN_DATA,				/* origin of data : transaction, scheduled, hist, future */
	SPP_ESTIMATE_TREE_DIVISION_COLUMN,			/* div_number, transaction_number, futur_number, scheduled_number*/
	SPP_ESTIMATE_TREE_SUB_DIV_COLUMN,			/* sub_div_nb */
	SPP_ESTIMATE_TREE_DATE_COLUMN,
	SPP_ESTIMATE_TREE_DESC_COLUMN,
	SPP_ESTIMATE_TREE_DEBIT_COLUMN,
	SPP_ESTIMATE_TREE_CREDIT_COLUMN,
	SPP_ESTIMATE_TREE_BALANCE_COLUMN,
	SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
	SPP_ESTIMATE_TREE_AMOUNT_COLUMN,			/* the amount without currency */
	SPP_ESTIMATE_TREE_BALANCE_COLOR,
	SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
	SPP_ESTIMATE_TREE_COLOR_STRING,
	SPP_ESTIMATE_TREE_NUM_COLUMNS
};

/* noms des colonnes du tree_view des données historiques */
enum BetHistoricalTreeColumns {
	SPP_HISTORICAL_SELECT_COLUMN,
	SPP_HISTORICAL_DESC_COLUMN,
	SPP_HISTORICAL_CURRENT_COLUMN,				/* Accumulation of the current year */
	SPP_HISTORICAL_BALANCE_COLUMN,
	SPP_HISTORICAL_BALANCE_AMOUNT,				/* balance column without currency */
	SPP_HISTORICAL_AVERAGE_COLUMN,
	SPP_HISTORICAL_AVERAGE_AMOUNT,  			/* average column without currency */
	SPP_HISTORICAL_RETAINED_COLUMN,
	SPP_HISTORICAL_RETAINED_AMOUNT, 			/* retenaid column without currency */
	SPP_HISTORICAL_BALANCE_COLOR,
	SPP_HISTORICAL_ACCOUNT_NUMBER,
	SPP_HISTORICAL_DIV_NUMBER,
	SPP_HISTORICAL_SUB_DIV_NUMBER,
	SPP_HISTORICAL_EDITED_COLUMN,
	SPP_HISTORICAL_BACKGROUND_COLOR,
	SPP_HISTORICAL_NUM_COLUMNS
};


/* START_DECLARATION */
void						bet_data_bet_range_struct_free					(BetRange *sbr);
BetRange *					bet_data_bet_range_struct_init 					(void);
gchar *						bet_data_get_div_name 							(gint div_num,
																		 gint sub_div,
																		 const gchar *return_value_error);
gint 						bet_data_get_div_number 						(gint transaction_number,
																		 gboolean is_transaction);
gint 						bet_data_get_div_type 							(gint div_number);
gint 						bet_data_get_selected_currency 					(void);
gchar *						bet_data_get_str_amount_in_account_currency		(GsbReal amount,
																		 gint account_number,
																		 gint line_number,
																		 gint origin);
GPtrArray *					bet_data_get_strings_to_save 					(void);
gint 						bet_data_get_sub_div_nb 						(gint transaction_number,
																		 gboolean is_transaction);
gboolean 					bet_data_remove_all_bet_data 					(gint account_number);
void						bet_data_renum_account_number_0					(gint new_account_number);
void 						bet_data_select_bet_pages 						(gint account_number);
gboolean 					bet_data_set_div_amount 						(gint account_nb,
																		 gint div_number,
																		 gint sub_div_nb,
																		 GsbReal amount);
gboolean 					bet_data_set_div_edited 						(gint account_nb,
																		 gint div_number,
																		 gint sub_div_nb,
																		 gboolean edited);
gboolean 					bet_data_set_div_ptr 							(gint type_div);
void 						bet_data_set_maj 								(gint account_number,
																		 gint type_maj);
void 						bet_data_update_bet_module 						(gint account_number,
																		 gint page);

void 						bet_data_struct_transaction_current_fyear_free 	(TransactionCurrentFyear *self);
TransactionCurrentFyear *	bet_data_struct_transaction_current_fyear_init	(void);
void 						bet_data_variables_free 						(void);
gboolean 					bet_data_variables_init 						(void);

/* ARRAY_DATA */
GDate *						bet_data_array_get_date_max 					(gint account_number);

/* FUTURE_DATA */
gboolean 					bet_data_future_add_lines 						(FuturData *scheduled);
GHashTable *				bet_data_future_get_list 						(void);
FuturData *					bet_data_future_get_struct 						(gint account_number,
																		 gint number);
gboolean 					bet_data_future_modify_lines 					(FuturData *scheduled);
gboolean 					bet_data_future_remove_line 					(gint account_number,
																		 gint number,
																		 gboolean maj);
gboolean 					bet_data_future_remove_lines 					(gint account_number,
																		 gint number,
																		 gint mother_row);
gboolean 					bet_data_future_set_lines_from_file 			(FuturData *scheduled);
FuturData *					bet_data_future_struct_init 					(void);

/* HISTORIQUE_DATA */
GsbReal 					bet_data_hist_get_div_amount 					(gint account_nb,
																		 gint div_number,
																		 gint sub_div_nb);
gboolean 					bet_data_hist_get_div_edited 					(gint account_number,
																		 gint div_number,
																		 gint sub_div_nb);
gboolean 					bet_data_hist_div_add 							(gint account_number,
																		 gint div_number,
																		 gint sub_div_nb);
void 						bet_data_hist_div_insert 						(HistData *shd,
																		 HistData *sub_shd);
gboolean 					bet_data_hist_div_populate 						(gint transaction_number,
																		 gboolean is_transaction,
																		 GHashTable  *list_div,
																		 gint type_de_transaction,
																		 TransactionCurrentFyear *tcf);
gboolean 					bet_data_hist_div_remove 						(gint account_number,
																		 gint div_number,
																		 gint sub_div_nb);
gboolean 					bet_data_hist_div_search 						(gint account_number,
																		 gint div_number,
																		 gint sub_div_nb);
void 						bet_data_hist_reset_all_amounts 				(gint account_number);
HistData *					bet_data_hist_struct_init 						(void);
void 						bet_data_hist_struct_free						(HistData *shd);

/* TRANSFERT_DATA */
gboolean 					bet_data_transfert_add_line 					(TransfertData *transfert);
void 						bet_data_transfert_create_new_transaction 		(TransfertData *transfert);
TransfertData *				bet_data_transfert_get_struct_from_number		(gint number);
GHashTable *				bet_data_transfert_get_list 					(void);
gboolean 					bet_data_transfert_modify_line 					(TransfertData *transfert);
gboolean 					bet_data_transfert_remove_line 					(gint account_number,
																		 gint number);
gboolean 					bet_data_transfert_set_line_from_file 			(TransfertData *transfert);
void 						bet_data_transfert_update_date_if_necessary		(TransfertData *transfert,
												  						 GDate *date_bascule,
																		 gboolean force);
void						struct_free_bet_transfert						(TransfertData *transfert);
TransfertData *				struct_initialise_bet_transfert 				(void);
/* END_DECLARATION */


#endif
