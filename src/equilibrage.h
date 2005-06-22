#ifndef _EQUILIBRAGE_H
#define _EQUILIBRAGE_H (1)

enum reconciliation_columns {
    RECONCILIATION_NAME_COLUMN = 0,
    RECONCILIATION_VISIBLE_COLUMN,
    RECONCILIATION_SORT_COLUMN,
    RECONCILIATION_SPLIT_NEUTRAL_COLUMN,
    RECONCILIATION_ACCOUNT_COLUMN,
    RECONCILIATION_TYPE_COLUMN,
    NUM_RECONCILIATION_COLUMNS,
};

/* START_INCLUDE_H */
#include "equilibrage.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *creation_fenetre_equilibrage ( void );
void equilibrage ( void );
gboolean gsb_reconcile_mark_transaction ( gpointer transaction );
GtkWidget * tab_display_reconciliation ( void );
/* END_DECLARATION */
#endif
