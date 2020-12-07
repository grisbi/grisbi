#ifndef _GSB_DATA_CURRENCY_H
#define _GSB_DATA_CURRENCY_H (1)

#include <glib.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/** ISO 4217 currency.  Not specific to Grisbi. */
typedef struct _Iso4217Currency			Iso4217Currency;

struct _Iso4217Currency
{										/* + correspondance struct Iso4217Currency */
    const gchar * continent;			/*                          = Europe       */
	const gchar * currency_name;		/* -> currency_name         = Euro         */
	const gchar * country_name;			/*                          = France       */
    const gchar * currency_code_iso;	/* -> currency_code_iso4217 = EUR          */
    const gchar * currency_nickname;	/* -> currency_code         = €            */
    gboolean active;
    const gchar *flag_filename;
    gint floating_point;  				/* ->floating_point (Number of digits after the point => 2 : 0.00 / 1 : 0.0 )*/
    gboolean main_currency;				/** Skip this when autodetecting base currency, in case there are
										 * several countries using the same currency but a "main" one,
	 									 * like USA and Panama sharing USD as official currency..  */
};

/* Columns numbers for currency list  */
enum CurrencyConfigListColumns {
    CURRENCY_FLAG_COLUMN,
    CURRENCY_HAS_FLAG,
    COUNTRY_NAME_COLUMN,
    CURRENCY_NAME_COLUMN,
    CURRENCY_CODE_ISO_COLUMN,
    CURRENCY_NICKNAME_COLUMN,
    CURRENCY_FLOATING_COLUMN,
    CURRENCY_NUMBER_COLUMN,
    CURRENCY_MAIN_CURRENCY_COLUMN,
	CURRENCY_BACKGROUND_COLOR,
    NUM_CURRENCIES_COLUMNS
};

/* START_DECLARATION */
gboolean			gsb_data_currency_check_and_remove_duplicate	(void);
const gchar *		gsb_data_currency_get_nickname 					(gint currency_number);
gchar *				gsb_data_currency_get_code_iso4217 				(gint currency_number);
gchar *				gsb_data_currency_get_nickname_or_code_iso 		(gint currency_number);
GSList *			gsb_data_currency_get_currency_list				(void);
gint 				gsb_data_currency_get_default_currency 			(void);
gint 				gsb_data_currency_get_floating_point 			(gint currency_number);
gchar *				gsb_data_currency_get_name 						(gint currency_number);
gint 				gsb_data_currency_get_no_currency 				(gpointer currency_ptr);
gint 				gsb_data_currency_get_number_by_code_iso4217 	(const gchar *code);
gint 				gsb_data_currency_get_number_by_name 			(const gchar *name);
Iso4217Currency *	gsb_data_currency_get_tab_iso_4217_currencies	(void);
gboolean 			gsb_data_currency_init_variables 				(void);
gint 				gsb_data_currency_load_currency 				(gint new_no_currency);
gint 				gsb_data_currency_max_number 					(void);
gint 				gsb_data_currency_new 							(const gchar *name);
gint				gsb_data_currency_new_from_iso4217_list			(const gchar *currency_name);
gint				gsb_data_currency_new_with_data					(const gchar *currency_name,
																	 const gchar *currency_nickname,
															   		 const gchar *currency_isocode,
															    	 gint floating_point);
gboolean 			gsb_data_currency_remove 						(gint currency_number);
gboolean 			gsb_data_currency_set_nickname 					(gint currency_number,
																 const gchar *currency_nickname);
gboolean 			gsb_data_currency_set_code_iso4217 			(gint currency_number,
																 const gchar *currency_code_iso4217);
gboolean 			gsb_data_currency_set_floating_point 			(gint currency_number,
																 gint floating_point);
gboolean 			gsb_data_currency_set_name 						(gint currency_number,
																 const gchar *name);
/* END_DECLARATION */
#endif
