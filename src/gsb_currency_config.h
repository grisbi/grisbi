#ifndef _GSB_CURRENCY_CONFIG_H
#define _GSB_CURRENCY_CONFIG_H (1)

/** This structure holds informations needed for exchange rates
  cache. */
struct cached_exchange_rate {
    gint currency1_number;	/** First currency */
    gint currency2_number;	/** Second currency */
    gdouble rate;				/** Exchange rate betweend
						  currency1 and currency 2 */
    gdouble fees;				/** Fees associated with
						  exchange rate */
};

/** Columns numbers for currencies list  */
enum currency_list_column {
    CURRENCY_FLAG_COLUMN,
    CURRENCY_HAS_FLAG,
    COUNTRY_NAME_COLUMN,
    CURRENCY_NAME_COLUMN,
    CURRENCY_ISO_CODE_COLUMN,
    CURRENCY_NICKNAME_COLUMN,
    CURRENCY_POINTER_COLUMN,
    NUM_CURRENCIES_COLUMNS,
};

/* Handle an ISO 4217 currency.  Not specific to Grisbi. */
struct iso_4217_currency 
{
    gchar * continent;
    gchar * currency_name;
    gchar * country_name;
    gchar * currency_code;
    gchar * currency_nickname;
    gboolean active;
    gchar *flag_filename;
};



/* START_INCLUDE_H */
#include "gsb_currency_config.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean ajout_devise ( GtkWidget *widget );
struct cached_exchange_rate *cached_exchange_rate ( gint currency1_number, 
						    gint currency2_number );
gint find_currency_from_iso4217_list ( gchar * currency_name );
GtkWidget *onglet_devises ( void );
GtkWidget *tab_display_totals ( void );
void update_exchange_rate_cache ( gint currency1_number, 
				  gint currency2_number,
				  gdouble change, gdouble fees );
/* END_DECLARATION */
#endif
