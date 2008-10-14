#ifndef _GSB_REPORT_H
#define _GSB_REPORT_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_report_get_current ( void );
gint gsb_report_get_report_from_combobox ( GtkWidget *combo_box );
void gsb_report_init_variables ( void );
GtkWidget *gsb_report_make_combobox ( void );
void gsb_report_set_current ( gint report_number );
/* END_DECLARATION */
#endif
