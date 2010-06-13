#ifndef _GSB_DATA_REPORT_AMOUT_COMPARISON_H
#define _GSB_DATA_REPORT_AMOUT_COMPARISON_H (1)

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_data_report_amount_comparison_dup ( gint last_amount_comparison_number );
gpointer gsb_data_report_amount_comparison_get_button_first_comparison ( gint amount_comparison_number );
gpointer gsb_data_report_amount_comparison_get_button_link ( gint amount_comparison_number );
gpointer gsb_data_report_amount_comparison_get_button_link_first_to_second_part ( gint amount_comparison_number );
gpointer gsb_data_report_amount_comparison_get_button_second_comparison ( gint amount_comparison_number );
gpointer gsb_data_report_amount_comparison_get_entry_first_amount ( gint amount_comparison_number );
gpointer gsb_data_report_amount_comparison_get_entry_second_amount ( gint amount_comparison_number );
gsb_real gsb_data_report_amount_comparison_get_first_amount ( gint amount_comparison_number );
gint gsb_data_report_amount_comparison_get_first_comparison ( gint amount_comparison_number );
gpointer gsb_data_report_amount_comparison_get_hbox_line ( gint amount_comparison_number );
gpointer gsb_data_report_amount_comparison_get_hbox_second_part ( gint amount_comparison_number );
gint gsb_data_report_amount_comparison_get_link_first_to_second_part ( gint amount_comparison_number );
gint gsb_data_report_amount_comparison_get_link_to_last_amount_comparison ( gint amount_comparison_number );
gsb_real gsb_data_report_amount_comparison_get_second_amount ( gint amount_comparison_number );
gint gsb_data_report_amount_comparison_get_second_comparison ( gint amount_comparison_number );
gboolean gsb_data_report_amount_comparison_init_variables ( void );
gint gsb_data_report_amount_comparison_new ( gint number );
gboolean gsb_data_report_amount_comparison_remove ( gint amount_comparison_number );
gboolean gsb_data_report_amount_comparison_set_button_first_comparison ( gint amount_comparison_number,
									 gpointer button_first_comparison);
gboolean gsb_data_report_amount_comparison_set_button_link ( gint amount_comparison_number,
							     gpointer button_link);
gboolean gsb_data_report_amount_comparison_set_button_link_first_to_second_part ( gint amount_comparison_number,
										  gpointer button_link_first_to_second_part);
gboolean gsb_data_report_amount_comparison_set_button_second_comparison ( gint amount_comparison_number,
									  gpointer button_second_comparison);
gboolean gsb_data_report_amount_comparison_set_entry_first_amount ( gint amount_comparison_number,
								    gpointer entry_first_amount);
gboolean gsb_data_report_amount_comparison_set_entry_second_amount ( gint amount_comparison_number,
								     gpointer entry_second_amount);
gboolean gsb_data_report_amount_comparison_set_first_amount ( gint amount_comparison_number,
							      gsb_real first_amount);
gboolean gsb_data_report_amount_comparison_set_first_comparison ( gint amount_comparison_number,
								  gint first_comparison);
gboolean gsb_data_report_amount_comparison_set_hbox_line ( gint amount_comparison_number,
							   gpointer hbox_line);
gboolean gsb_data_report_amount_comparison_set_hbox_second_part ( gint amount_comparison_number,
								  gpointer hbox_second_part);
gboolean gsb_data_report_amount_comparison_set_link_first_to_second_part ( gint amount_comparison_number,
									   gint link_first_to_second_part);
gboolean gsb_data_report_amount_comparison_set_link_to_last_amount_comparison ( gint amount_comparison_number,
										gint link_to_last_amount_comparison);
gboolean gsb_data_report_amount_comparison_set_report_number ( gint amount_comparison_number,
							       gint report_number);
gboolean gsb_data_report_amount_comparison_set_second_amount ( gint amount_comparison_number,
							       gsb_real second_amount);
gboolean gsb_data_report_amount_comparison_set_second_comparison ( gint amount_comparison_number,
								   gint second_comparison);
/* END_DECLARATION */
#endif
