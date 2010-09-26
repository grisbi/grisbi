#ifndef _GSB_TRANSACTIONS_LIST_SORT_H
#define _GSB_TRANSACTIONS_LIST_SORT_H (1)
/* START_INCLUDE_H */
#include "custom_list.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gint gsb_transactions_list_sort (CustomRecord **a,
                        CustomRecord **b,
                        CustomList *custom_list);
gint gsb_transactions_list_sort_check_archive (  CustomRecord *record_1,
                        CustomRecord *record_2 );
gint gsb_transactions_list_sort_general_test ( CustomRecord *record_1,
                        CustomRecord *record_2 );
gint gsb_transactions_list_sort_initial (CustomRecord **a,
                        CustomRecord **b,
                        CustomList *custom_list);
/* END_DECLARATION */
#endif
