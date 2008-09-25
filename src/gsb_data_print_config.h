#ifndef _GSB_DATA_PRINT_CONFIG_H
#define _GSB_DATA_PRINT_CONFIG_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean gsb_data_print_config_get_draw_archives  (void);
gboolean gsb_data_print_config_get_draw_background  (void);
gboolean gsb_data_print_config_get_draw_column  (void);
gboolean gsb_data_print_config_get_draw_columns_name  (void);
gboolean gsb_data_print_config_get_draw_dates_are_value_dates  (void);
gboolean gsb_data_print_config_get_draw_interval_dates  (void);
gboolean gsb_data_print_config_get_draw_lines  (void);
gboolean gsb_data_print_config_get_draw_title  (void);
PangoFontDescription *gsb_data_print_config_get_font_title (void);
PangoFontDescription *gsb_data_print_config_get_font_transactions (void);
PangoFontDescription *gsb_data_print_config_get_report_font_title (void);
PangoFontDescription *gsb_data_print_config_get_report_font_transactions (void);
void gsb_data_print_config_init (void);
gboolean gsb_data_print_config_set_draw_archives  ( gint number,
						    gboolean value );
gboolean gsb_data_print_config_set_draw_background  ( gint number,
						      gboolean value );
gboolean gsb_data_print_config_set_draw_column  ( gint number,
						  gboolean value );
gboolean gsb_data_print_config_set_draw_columns_name  ( gint number,
							gboolean value );
gboolean gsb_data_print_config_set_draw_dates_are_value_dates  ( gint number,
								 gboolean value );
gboolean gsb_data_print_config_set_draw_interval_dates  ( gint number,
							  gboolean value );
gboolean gsb_data_print_config_set_draw_lines  ( gint number,
						 gboolean value );
gboolean gsb_data_print_config_set_draw_title  ( gint number,
						 gboolean value );
gboolean gsb_data_print_config_set_font_title ( PangoFontDescription *font_desc );
gboolean gsb_data_print_config_set_font_transaction  ( PangoFontDescription *font_desc );
gboolean gsb_data_print_config_set_report_font_title ( PangoFontDescription *font_desc );
gboolean gsb_data_print_config_set_report_font_transaction ( PangoFontDescription *font_desc );
/* END_DECLARATION */
#endif
