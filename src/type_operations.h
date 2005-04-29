#ifndef _TYPE_OPERATIONS_H
#define _TYPE_OPERATIONS_H (1)

/** Columns for payment methods tree */
enum payment_methods_columns {
    PAYMENT_METHODS_NAME_COLUMN = 0,
    PAYMENT_METHODS_NUMBERING_COLUMN,
    PAYMENT_METHODS_DEFAULT_COLUMN,
    PAYMENT_METHODS_TYPE_COLUMN,
    PAYMENT_METHODS_VISIBLE_COLUMN,
    PAYMENT_METHODS_ACTIVABLE_COLUMN,
    PAYMENT_METHODS_POINTER_COLUMN,
    NUM_PAYMENT_METHODS_COLUMNS,
};

/* START_INCLUDE_H */
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gchar * automatic_numbering_get_new_number ( struct struct_type_ope * type );
gint cherche_no_menu_type ( gint demande );
gint cherche_no_menu_type_associe ( gint demande );
gint cherche_no_menu_type_echeancier ( gint demande );
GtkWidget *creation_menu_types ( gint demande,
				 gint compte,
				 gint origine );
GtkWidget *onglet_types_operations ( void );
/* END_DECLARATION */
#endif
