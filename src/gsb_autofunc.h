#ifndef _GSB_AUTOFUNC_H
#define _GSB_AUTOFUNC_H (1)


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
GtkWidget *gsb_autofunc_checkbutton_new ( const gchar *label,
					  gboolean value,
					  GCallback hook,
					  gpointer data,
					  GCallback default_func,
					  gint number_for_func );
void gsb_autofunc_checkbutton_set_value ( GtkWidget *button,
					  gboolean value,
					  gint number_for_func );
GtkWidget *gsb_autofunc_combobox_new ( GSList *list,
				       gint index,
				       GCallback hook,
				       gpointer data,
				       GCallback default_func,
				       gint number_for_func );
void gsb_autofunc_combobox_set_index ( GtkWidget *combobox,
				       gint index,
				       gint number_for_func );
GtkWidget *gsb_autofunc_currency_new ( gboolean set_name,
				       gint currency_number,
				       GCallback hook,
				       gpointer data,
				       GCallback default_func,
				       gint number_for_func );
void gsb_autofunc_currency_set_currency_number ( GtkWidget *combobox,
						 gint currency_number,
						 gint number_for_func );
GtkWidget *gsb_autofunc_date_new ( const GDate *date,
				   GCallback hook,
				   gpointer data,
				   GCallback default_func,
				   gint number_for_func );
void gsb_autofunc_date_set ( GtkWidget *entry,
			     const GDate *date,
			     gint number_for_func );
GtkWidget *gsb_autofunc_entry_new ( const gchar *value,
				    GCallback hook,
				    gpointer data,
				    GCallback default_func,
				    gint number_for_func );
void gsb_autofunc_entry_set_value ( GtkWidget *entry,
				    const gchar *value,
				    gint number_for_func );
void gsb_autofunc_int_erase_entry ( GtkWidget *entry );
GtkWidget *gsb_autofunc_int_new ( gint value,
				  GCallback hook,
				  gpointer data,
				  GCallback default_func,
				  gint number_for_func );
void gsb_autofunc_int_set_value ( GtkWidget *entry,
				  gint value,
				  gint number_for_func );
GtkWidget *gsb_autofunc_radiobutton_new ( const gchar *choice1,
					  const gchar *choice2,
					  gboolean value,
					  GCallback hook,
					  gpointer data,
					  GCallback default_func,
					  gint number_for_func );
GtkWidget *gsb_autofunc_real_new ( gsb_real real,
				   GCallback hook,
				   gpointer data,
				   GCallback default_func,
				   gint number_for_func );
void gsb_autofunc_real_set ( GtkWidget *entry,
			     gsb_real real,
			     gint number_for_func );
GtkWidget *gsb_autofunc_spin_new ( gint value,
				   GCallback hook,
				   gpointer data,
				   GCallback default_func,
				   gint number_for_func );
void gsb_autofunc_spin_set_value ( GtkWidget *spin_button,
				   gint value,
				   gint number_for_func );
GtkWidget *gsb_autofunc_textview_new ( const gchar *value,
				       GCallback hook,
				       gpointer data,
				       GCallback default_func,
				       gint number_for_func );
void gsb_autofunc_textview_set_value ( GtkWidget *text_view,
				       const gchar *value,
				       gint number_for_func );
/* END_DECLARATION */
#endif
