#ifndef _GSB_DATA_PARTIAL_BALANCE_H
#define _GSB_DATA_PARTIAL_BALANCE_H (1)


/* START_INCLUDE_H */
#include "././gsb_data_account.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_data_partial_balance_get_currency ( gint partial_balance_number );
gchar *gsb_data_partial_balance_get_current_balance ( gint partial_balance_number );
kind_account gsb_data_partial_balance_get_kind ( gint partial_balance_number );
GSList *gsb_data_partial_balance_get_list ( void );
const gchar *gsb_data_partial_balance_get_liste_cptes ( gint partial_balance_number );
gchar *gsb_data_partial_balance_get_marked_balance ( gint partial_balance_number );
const gchar *gsb_data_partial_balance_get_name ( gint partial_balance_number );
gint gsb_data_partial_balance_get_number ( gpointer balance_ptr );
gboolean gsb_data_partial_balance_init_variables ( void );
gint gsb_data_partial_balance_new ( const gchar *name );
gboolean gsb_data_partial_balance_set_currency ( gint partial_balance_number,
                        gint currency );
gboolean gsb_data_partial_balance_set_kind ( gint partial_balance_number,
                        kind_account kind );
gboolean gsb_data_partial_balance_set_liste_cptes ( gint partial_balance_number,
                        const gchar *liste_cptes );
gboolean gsb_data_partial_balance_set_name ( gint partial_balance_number,
                        const gchar *name );
gint gsb_data_partial_balance_set_new_number ( gint partial_balance_number,
                        gint new_no_partial_balance );
/* END_DECLARATION */
#endif
