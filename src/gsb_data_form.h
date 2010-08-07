#ifndef _DATA_FORM_H
#define _DATA_FORM_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */


enum transaction_form_widget {
    TRANSACTION_FORM_DATE = 1,
    TRANSACTION_FORM_DEBIT,
    TRANSACTION_FORM_CREDIT,
    TRANSACTION_FORM_VALUE_DATE,
    TRANSACTION_FORM_EXERCICE,      /* 5 */
    TRANSACTION_FORM_PARTY,
    TRANSACTION_FORM_CATEGORY,
    TRANSACTION_FORM_FREE,          /* free for the moment, to be filled with a new thing */
    TRANSACTION_FORM_BUDGET,
    TRANSACTION_FORM_NOTES,         /* 10  */
    TRANSACTION_FORM_TYPE,
    TRANSACTION_FORM_CHEQUE,        /* 12  */
    TRANSACTION_FORM_DEVISE,
    TRANSACTION_FORM_CHANGE,
    TRANSACTION_FORM_VOUCHER,       /* 15 */
    TRANSACTION_FORM_BANK,
    TRANSACTION_FORM_CONTRA,
    TRANSACTION_FORM_OP_NB,
    TRANSACTION_FORM_MODE,
    TRANSACTION_FORM_WIDGET_NB,     /* 20 */
    TRANSACTION_FORM_MAX_WIDGETS,
};

/* the maximum values for the size of the form */
#define MAX_WIDTH 6
#define MAX_HEIGHT 4

/* START_DECLARATION */
gboolean gsb_data_form_check_for_value ( gint value );
gboolean gsb_data_form_dup_organization ( gint origin_account,
					  gint target_account );
gint gsb_data_form_get_nb_columns ( gint account_number );
gint gsb_data_form_get_nb_rows ( gint account_number );
gint gsb_data_form_get_value ( gint account_number,
			       gint column,
			       gint row );
gint gsb_data_form_get_values_total ( gint account_number );
gint gsb_data_form_get_width_column ( gint account_number,
				      gint column );
gboolean gsb_data_form_look_for_value ( gint account_number,
					gint value,
					gint *return_row,
					gint *return_column );
gboolean gsb_data_form_new_organization ( gint account_number );
gboolean gsb_data_form_set_default_organization ( gint account_number );
gboolean gsb_data_form_set_nb_columns ( gint account_number,
					gint columns );
gboolean gsb_data_form_set_nb_rows ( gint account_number,
				     gint rows );
gboolean gsb_data_form_set_value ( gint account_number,
				   gint column,
				   gint row,
				   gint value );
gboolean gsb_data_form_set_width_column ( gint account_number,
					  gint column,
					  gint width );
gboolean gsb_form_config_update_from_account (gint account_number);
/* END_DECLARATION */
#endif
