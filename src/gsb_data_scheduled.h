#ifndef _GSB_DATA_SCHEDULED_H
#define _GSB_DATA_SCHEDULED_H (1)


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean gsb_data_scheduled_copy_scheduled ( gint source_scheduled_number,
                        gint target_scheduled_number );
gint gsb_data_scheduled_get_account_number ( gint scheduled_number );
gint gsb_data_scheduled_get_account_number_transfer ( gint scheduled_number );
gsb_real gsb_data_scheduled_get_adjusted_amount_for_currency ( gint scheduled_number,
                        gint return_currency_number,
                        gint return_exponent );
gsb_real gsb_data_scheduled_get_amount ( gint scheduled_number );
gint gsb_data_scheduled_get_automatic_scheduled ( gint scheduled_number );
gint gsb_data_scheduled_get_budgetary_number ( gint scheduled_number );
gint gsb_data_scheduled_get_category_number ( gint scheduled_number );
GSList *gsb_data_scheduled_get_children ( gint scheduled_number,
                        gboolean return_number );
gint gsb_data_scheduled_get_contra_method_of_payment_number ( gint scheduled_number );
gint gsb_data_scheduled_get_currency_floating_point ( gint scheduled_number );
gint gsb_data_scheduled_get_currency_number ( gint scheduled_number );
GDate *gsb_data_scheduled_get_date ( gint scheduled_number );
gint gsb_data_scheduled_get_financial_year_number ( gint scheduled_number );
gint gsb_data_scheduled_get_frequency ( gint scheduled_number );
GDate *gsb_data_scheduled_get_limit_date ( gint scheduled_number );
gchar *gsb_data_scheduled_get_method_of_payment_content ( gint scheduled_number );
gint gsb_data_scheduled_get_method_of_payment_number ( gint scheduled_number );
gint gsb_data_scheduled_get_mother_scheduled_number ( gint scheduled_number );
gchar *gsb_data_scheduled_get_notes ( gint scheduled_number );
gint gsb_data_scheduled_get_party_number ( gint scheduled_number );
GSList *gsb_data_scheduled_get_scheduled_list ( void );
gint gsb_data_scheduled_get_scheduled_number ( gpointer scheduled_pointer );
gint gsb_data_scheduled_get_split_of_scheduled ( gint scheduled_number );
gint gsb_data_scheduled_get_sub_budgetary_number ( gint scheduled_number );
gint gsb_data_scheduled_get_sub_category_number ( gint scheduled_number );
gint gsb_data_scheduled_get_user_entry ( gint scheduled_number );
gint gsb_data_scheduled_get_user_interval ( gint scheduled_number );
gboolean gsb_data_scheduled_get_variance ( gint mother_scheduled_number );
gint gsb_data_scheduled_get_white_line ( gint scheduled_number);
gboolean gsb_data_scheduled_init_variables ( void );
gboolean gsb_data_scheduled_is_transfer ( gint scheduled_number );
gint gsb_data_scheduled_new_scheduled ( void );
gint gsb_data_scheduled_new_scheduled_with_number ( gint scheduled_number );
gint gsb_data_scheduled_new_white_line ( gint mother_scheduled_number);
gboolean gsb_data_scheduled_remove_scheduled ( gint scheduled_number );
gboolean gsb_data_scheduled_set_account_number ( gint scheduled_number,
						 gint no_account );
gboolean gsb_data_scheduled_set_account_number_transfer ( gint scheduled_number,
							  gint account_number_transfer );
gboolean gsb_data_scheduled_set_amount ( gint scheduled_number,
					 gsb_real amount );
gboolean gsb_data_scheduled_set_automatic_scheduled ( gint scheduled_number,
						      gint automatic_scheduled );
gboolean gsb_data_scheduled_set_budgetary_number ( gint scheduled_number,
						   gint budgetary_number );
gboolean gsb_data_scheduled_set_category_number ( gint scheduled_number,
						  gint no_category );
gboolean gsb_data_scheduled_set_contra_method_of_payment_number ( gint scheduled_number,
								  gint number );
gboolean gsb_data_scheduled_set_currency_number ( gint scheduled_number,
						  gint no_currency );
gboolean gsb_data_scheduled_set_date ( gint scheduled_number,
				       const GDate *date );
gboolean gsb_data_scheduled_set_financial_year_number ( gint scheduled_number,
							gint financial_year_number );
gboolean gsb_data_scheduled_set_frequency ( gint scheduled_number,
					    gint number );
gboolean gsb_data_scheduled_set_limit_date ( gint scheduled_number,
					     const GDate *date );
gboolean gsb_data_scheduled_set_method_of_payment_content ( gint scheduled_number,
							    const gchar *method_of_payment_content );
gboolean gsb_data_scheduled_set_method_of_payment_number ( gint scheduled_number,
							   gint number );
gboolean gsb_data_scheduled_set_mother_scheduled_number ( gint scheduled_number,
							  gint mother_scheduled_number );
gboolean gsb_data_scheduled_set_notes ( gint scheduled_number,
					const gchar *notes );
gboolean gsb_data_scheduled_set_party_number ( gint scheduled_number,
					       gint no_party );
gboolean gsb_data_scheduled_set_split_of_scheduled ( gint scheduled_number,
							 gint is_split );
gboolean gsb_data_scheduled_set_sub_budgetary_number ( gint scheduled_number,
						       gint sub_budgetary_number );
gboolean gsb_data_scheduled_set_sub_category_number ( gint scheduled_number,
						      gint no_sub_category );
gboolean gsb_data_scheduled_set_user_entry ( gint scheduled_number,
					     gint number );
gboolean gsb_data_scheduled_set_user_interval ( gint scheduled_number,
						gint number );
/* END_DECLARATION */
#endif
