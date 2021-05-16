#ifndef _GSB_DATA_REPORT_TEXT_COMPARISON_H
#define _GSB_DATA_REPORT_TEXT_COMPARISON_H (1)

#include <glib.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gint 		gsb_data_report_text_comparison_dup 									(gint last_text_comparison_number);
gint 		gsb_data_report_text_comparison_get_field 								(gint text_comparison_number);
gint 		gsb_data_report_text_comparison_get_first_amount 						(gint text_comparison_number);
gint 		gsb_data_report_text_comparison_get_first_comparison 					(gint text_comparison_number);
gint 		gsb_data_report_text_comparison_get_link_first_to_second_part 			(gint text_comparison_number);
gint 		gsb_data_report_text_comparison_get_link_to_last_text_comparison 		(gint text_comparison_number);
gint 		gsb_data_report_text_comparison_get_operator 							(gint text_comparison_number);
gint 		gsb_data_report_text_comparison_get_second_amount 						(gint text_comparison_number);
gint 		gsb_data_report_text_comparison_get_second_comparison 					(gint text_comparison_number);
gchar *		gsb_data_report_text_comparison_get_text 								(gint text_comparison_number);
gint 		gsb_data_report_text_comparison_get_use_text 							(gint text_comparison_number);
gpointer	gsb_data_report_text_comparison_get_widget								(gint text_comparison_number);
gboolean 	gsb_data_report_text_comparison_init_variables 							(void);
gint 		gsb_data_report_text_comparison_new 									(gint number);
gboolean 	gsb_data_report_text_comparison_remove 									(gint text_comparison_number);
gboolean 	gsb_data_report_text_comparison_set_field 								(gint text_comparison_number,
																					 gint field);
gboolean 	gsb_data_report_text_comparison_set_first_amount 						(gint text_comparison_number,
																					 gint first_amount);
gboolean 	gsb_data_report_text_comparison_set_first_comparison 					(gint text_comparison_number,
																					 gint first_comparison);
gboolean 	gsb_data_report_text_comparison_set_link_first_to_second_part 			(gint text_comparison_number,
																					 gint link_first_to_second_part);
gboolean 	gsb_data_report_text_comparison_set_link_to_last_text_comparison 		(gint text_comparison_number,
																					 gint link_to_last_text_comparison);
gboolean 	gsb_data_report_text_comparison_set_operator 							(gint text_comparison_number,
																					 gint operator);
gboolean 	gsb_data_report_text_comparison_set_report_number 						(gint text_comparison_number,
																					 gint report_number);
gboolean 	gsb_data_report_text_comparison_set_second_amount 						(gint text_comparison_number,
																					 gint second_amount);
gboolean 	gsb_data_report_text_comparison_set_second_comparison 					(gint text_comparison_number,
																					 gint second_comparison);
gboolean 	gsb_data_report_text_comparison_set_text 								(gint text_comparison_number,
																					 const gchar *text);
gboolean 	gsb_data_report_text_comparison_set_use_text 							(gint text_comparison_number,
																					 gint use_text);
gboolean	gsb_data_report_text_comparison_set_widget								(gint text_comparison_number,
																					 gpointer widget);
/* END_DECLARATION */
#endif
