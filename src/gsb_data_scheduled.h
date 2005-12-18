#ifndef _GSB_DATA_SCHEDULED_H
#define _GSB_DATA_SCHEDULED_H (1)

/* START_INCLUDE_H */
#include "gsb_data_scheduled.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_data_scheduled_get_account_number ( gint no_scheduled );
gint gsb_data_scheduled_get_account_number_transfer ( gint no_scheduled );
gdouble gsb_data_scheduled_get_amount ( gint no_scheduled );
gint gsb_data_scheduled_get_automatic_scheduled ( gint no_scheduled );
gint gsb_data_scheduled_get_breakdown_of_scheduled ( gint no_scheduled );
gint gsb_data_scheduled_get_budgetary_number ( gint no_scheduled );
gint gsb_data_scheduled_get_category_number ( gint no_scheduled );
GSList *gsb_data_scheduled_get_children ( gint scheduled_number );
gint gsb_data_scheduled_get_contra_method_of_payment_number ( gint no_scheduled );
gint gsb_data_scheduled_get_currency_number ( gint no_scheduled );
GDate *gsb_data_scheduled_get_date ( gint no_scheduled );
gint gsb_data_scheduled_get_financial_year_number ( gint no_scheduled );
gint gsb_data_scheduled_get_frequency ( gint no_scheduled );
GDate *gsb_data_scheduled_get_limit_date ( gint no_scheduled );
gchar *gsb_data_scheduled_get_method_of_payment_content ( gint no_scheduled );
gint gsb_data_scheduled_get_method_of_payment_number ( gint no_scheduled );
gint gsb_data_scheduled_get_mother_scheduled_number ( gint no_scheduled );
gchar *gsb_data_scheduled_get_notes ( gint no_scheduled );
gint gsb_data_scheduled_get_party_number ( gint no_scheduled );
GSList *gsb_data_scheduled_get_scheduled_list ( void );
gint gsb_data_scheduled_get_scheduled_number ( gpointer scheduled_pointer );
gint gsb_data_scheduled_get_sub_budgetary_number ( gint no_scheduled );
gint gsb_data_scheduled_get_sub_category_number ( gint no_scheduled );
gint gsb_data_scheduled_get_user_entry ( gint no_scheduled );
gint gsb_data_scheduled_get_user_interval ( gint no_scheduled );
gboolean gsb_data_scheduled_init_variables ( void );
gint gsb_data_scheduled_new_scheduled ( void );
gint gsb_data_scheduled_new_scheduled_with_number ( gint scheduled_number );
gint gsb_data_scheduled_new_white_line ( gint mother_scheduled_number);
gboolean gsb_data_scheduled_remove_scheduled ( gint scheduled_number );
gboolean gsb_data_scheduled_set_account_number ( gint no_scheduled,
						 gint no_account );
gboolean gsb_data_scheduled_set_account_number_transfer ( gint no_scheduled,
							  gint account_number_transfer );
gboolean gsb_data_scheduled_set_amount ( gint no_scheduled,
					 gdouble amount );
gboolean gsb_data_scheduled_set_automatic_scheduled ( gint no_scheduled,
						      gint automatic_scheduled );
gboolean gsb_data_scheduled_set_breakdown_of_scheduled ( gint no_scheduled,
							 gint is_breakdown );
gboolean gsb_data_scheduled_set_budgetary_number ( gint no_scheduled,
						   gint budgetary_number );
gboolean gsb_data_scheduled_set_category_number ( gint no_scheduled,
						  gint no_category );
gboolean gsb_data_scheduled_set_contra_method_of_payment_number ( gint no_scheduled,
								  gint number );
gboolean gsb_data_scheduled_set_currency_number ( gint no_scheduled,
						  gint no_currency );
gboolean gsb_data_scheduled_set_date ( gint no_scheduled,
				       GDate *date );
gboolean gsb_data_scheduled_set_financial_year_number ( gint no_scheduled,
							gint financial_year_number );
gboolean gsb_data_scheduled_set_frequency ( gint no_scheduled,
					    gint number );
gboolean gsb_data_scheduled_set_limit_date ( gint no_scheduled,
					     GDate *date );
gboolean gsb_data_scheduled_set_method_of_payment_content ( gint no_scheduled,
							    const gchar *method_of_payment_content );
gboolean gsb_data_scheduled_set_method_of_payment_number ( gint no_scheduled,
							   gint number );
gboolean gsb_data_scheduled_set_mother_scheduled_number ( gint no_scheduled,
							  gint mother_scheduled_number );
gboolean gsb_data_scheduled_set_notes ( gint no_scheduled,
					const gchar *notes );
gboolean gsb_data_scheduled_set_party_number ( gint no_scheduled,
					       gint no_party );
gboolean gsb_data_scheduled_set_sub_budgetary_number ( gint no_scheduled,
						       gint sub_budgetary_number );
gboolean gsb_data_scheduled_set_sub_category_number ( gint no_scheduled,
						      gint no_sub_category );
gboolean gsb_data_scheduled_set_user_entry ( gint no_scheduled,
					     gint number );
gboolean gsb_data_scheduled_set_user_interval ( gint no_scheduled,
						gint number );
/* END_DECLARATION */
#endif
