#ifndef _BALANCE_ESTIMATE_DATA_H
#define _BALANCE_ESTIMATE_DATA_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* nombre de colonnes du tableau des prévisions */
#define BET_ARRAY_COLUMNS 5

typedef struct _BetRange                    BetRange;
typedef struct _BetHist                     BetHist;
typedef struct _HistDiv                     HistDiv;
typedef struct _FutureData                  FuturData;
typedef struct _TransfertData               TransfertData;
typedef struct _TransactionCurrentFyear     TransactionCurrentFyear;

struct _BetRange
{
    gboolean 		first_pass;
    gsb_real 		current_fyear;
    gsb_real 		current_balance;
};

/* utilisée pour gérer la liste des données historiques dans la vue historique */
struct _BetHist
{
    gint 			div;
    gint 			account_nb;
    BetRange	*			sbr;
    GHashTable *	list_sub_div;
};

/* utilisée pour gérer la liste des données historiques dans le tableau des prévisions */
struct _HistDiv
{
    gint 			account_nb;
    gint 			div_number;
    gint 			origin;
    gboolean 		div_edited;
    GHashTable *	sub_div_list;
    gsb_real 		amount;
};


struct _FutureData
{
    gint 			number;
    gint 			account_number;
    GDate *			date;
    gsb_real 		amount;
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


struct _TransfertData
{
    gint 			number;
    gint 			account_number;				/* numéro du compte principal concerné */
    gint 			type;						/* type de compte carte : 0 = account 1 = partial balance */
    gint			replace_account;           	/* Account number or partial balance number */
    gint 			replace_transaction;       	/* remplace la transaction plannifiée dans le compte principal */
    gint 			direct_debit;              	/* si = 1 création de la transaction dans le compte principal */
    GDate *			date;                    	/* date de l'opération du compte principal */
    GDate *			date_bascule;            	/* date de début de nouveau de mois dans le compte à débit différé */
    gint 			main_last_banking_date;    	/* force la date au dernier jour bancaire ouvrable du mois */
    gint 			main_payee_number;         	/* tiers de l'opération du compte principal */
    gint 			main_payment_number;       	/* moyen de payement de l'opération du compte principal */
    gint 			main_category_number;      	/* catégorie de l'opération du compte principal */
    gint 			main_sub_category_number;  	/* sous-catégorie de l'opération du compte principal */
    gint 			main_budgetary_number;     	/* IB de l'opération du compte principal */
    gint 			main_sub_budgetary_number; 	/* sous IB de l'opération du compte principal */
    gint 			card_payee_number;         	/* tiers de l'opération du compte à débit différé */
    gint 			card_category_number;      	/* catégorie de l'opération du compte à débit différé */
    gint 			card_sub_category_number;  	/* sous-catégorie de l'opération du compte à débit différé */
    gint 			card_budgetary_number;     	/* IB de l'opération du compte à débit différé */
    gint 			card_sub_budgetary_number; 	/* sous IB de l'opération du compte à débit différé */
};

struct _TransactionCurrentFyear
{
    gint 		transaction_number;
    gint 		type_de_transaction;			/* 0 = historique 1 = current fyear 2 = hist and current fyear */
    gint 		div_nb;
    gint 		sub_div_nb;
    GDate *		date;
    gsb_real 	amount;
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
enum BetHistoricalDataColumns {
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
void 						bet_data_free_variables 					(void);
GDate *						bet_data_array_get_date_max 				(gint account_number);
gboolean 					bet_data_future_add_lines 					(FuturData *scheduled);
GHashTable *				bet_data_future_get_list 					(void);
FuturData *					bet_data_future_get_struct 					(gint account_number, gint number);
gboolean 					bet_data_future_modify_lines 				(FuturData *scheduled);
gboolean 					bet_data_future_remove_line 				(gint account_number, gint number, gboolean maj);
gboolean 					bet_data_future_remove_lines 				(gint account_number,
																		 gint number,
																		 gint mother_row);
gboolean 					bet_data_future_set_lines_from_file 		(FuturData *scheduled);
gboolean 					bet_data_get_div_edited 					(gint account_number, gint div_number, gint sub_div_nb);
gchar *						bet_data_get_div_name 						(gint div_num,
																		 gint sub_div,
																		 const gchar *return_value_error);
gint 						bet_data_get_div_number 					(gint transaction_number, gboolean is_transaction);
gint 						bet_data_get_div_type 						(gint div_number);
gint 						bet_data_get_selected_currency 				(void);
gchar *						bet_data_get_str_amount_in_account_currency	(gsb_real amount,
																		 gint account_number,
																		 gint line_number,
																		 gint origin);
GPtrArray *					bet_data_get_strings_to_save 				(void);
gint 						bet_data_get_sub_div_nb 					(gint transaction_number,
																		 gboolean is_transaction);
gboolean 					bet_data_hist_add_div 						(gint account_number,
																		 gint div_number,
																		 gint sub_div_nb);
gsb_real 					bet_data_hist_get_div_amount 				(gint account_nb,
																		 gint div_number,
																		 gint sub_div_nb);
void 						bet_data_hist_reset_all_amounts 			(gint account_number);
gboolean 					bet_data_init_variables 					(void);
void 						bet_data_insert_div_hist 					(HistDiv *shd,
																		 HistDiv *sub_shd);
gboolean 					bet_data_populate_div 						(gint transaction_number,
																		 gboolean is_transaction,
																		 GHashTable  *list_div,
																		 gint type_de_transaction,
																		 TransactionCurrentFyear *tcf);
gboolean 					bet_data_remove_all_bet_data 				(gint account_number);
gboolean 					bet_data_remove_div_hist 					(gint account_number,
																		 gint div_number,
																		 gint sub_div_nb);
gboolean 					bet_data_search_div_hist 					(gint account_number,
																		 gint div_number,
																		 gint sub_div_nb);
void 						bet_data_select_bet_pages 					(gint account_number);
gboolean 					bet_data_set_div_amount 					(gint account_nb,
																		 gint div_number,
																		 gint sub_div_nb,
																		 gsb_real amount);
gboolean 					bet_data_set_div_edited 					(gint account_nb,
																		 gint div_number,
																		 gint sub_div_nb,
																		 gboolean edited);
gboolean 					bet_data_set_div_ptr 						(gint type_div);
void 						bet_data_set_maj 							(gint account_number,
																		 gint type_maj);
void 						bet_data_synchronise_hist_div_list 			(GHashTable  *list_div);
gboolean 					bet_data_transfert_add_line 				(TransfertData *transfert);
void 						bet_data_transfert_create_new_transaction 	(TransfertData *transfert);
GHashTable *				bet_data_transfert_get_list 				(void);
gboolean 					bet_data_transfert_modify_line 				(TransfertData *transfert);
gboolean 					bet_data_transfert_remove_line 				(gint account_number,
																		 gint number);
gboolean 					bet_data_transfert_set_line_from_file 		(TransfertData *transfert);
void 						bet_data_transfert_update_date_if_necessary	(TransfertData *transfert);
void 						bet_data_update_bet_module 					(gint account_number, gint page);
void 						struct_free_bet_historical 					(BetHist *sh);
FuturData *					struct_initialise_bet_future 				(void);
BetRange *					struct_initialise_bet_range 				(void);
TransfertData *				struct_initialise_bet_transfert 			(void);
HistDiv *					struct_initialise_hist_div 					(void);
void 						struct_free_bet_transaction_current_fyear 	(TransactionCurrentFyear *self);
TransactionCurrentFyear *	struct_initialise_transaction_current_fyear (void);
/* END_DECLARATION */


#endif
