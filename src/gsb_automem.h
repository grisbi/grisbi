#ifndef _GSB_AUTOMEM_H
#define _GSB_AUTOMEM_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "utils_buttons.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
GtkWidget *gsb_automem_checkbutton_new 					(const gchar *label,
														 gboolean *value,
														 GCallback hook,
														 gpointer data);
GtkWidget *	gsb_automem_entry_new 						(gchar **value,
														 GCallback hook,
														 gpointer data);
void		gsb_automem_entry_new_from_ui				(GtkWidget *entry,
														 gchar **value,
														 GCallback hook,
														 gpointer data);
GtkWidget *	gsb_automem_radiobutton3_gsettings_new 		(const gchar *choice1,
														 const gchar *choice2,
														 const gchar *choice3,
														 gint *value,
														 GCallback callback,
														 gpointer data,
														 gint orientation);
GtkWidget *	gsb_automem_radiobutton3_new 				(const gchar *choice1,
														 const gchar *choice2,
														 const gchar *choice3,
														 gint *value,
														 GCallback callback,
														 gpointer data,
														 gint orientation);
GtkWidget *	gsb_automem_radiobutton3_new_with_title 	(GtkWidget *parent,
														 const gchar *title,
														 const gchar *choice1,
														 const gchar *choice2,
														 const gchar *choice3,
														 gint *value,
														 GCallback hook,
														 gpointer data,
														 gint orientation);
GtkWidget *	gsb_automem_checkbutton_gsettings_new 		(const gchar *label,
														 gboolean *value,
														 GCallback hook,
														 gpointer data);
GtkWidget *	gsb_automem_radiobutton_gsettings_new 		(const gchar *choice1,
														 const gchar *choice2,
														 gboolean *value,
														 GCallback hook,
														 gpointer data);
GtkWidget *	gsb_automem_radiobutton_new 				(const gchar *choice1,
														 const gchar *choice2,
														 gboolean *value,
														 GCallback hook,
														 gpointer data);
GtkWidget *	gsb_automem_radiobutton_new_with_title 		(GtkWidget *parent,
														 const gchar *title,
														 const gchar *choice1,
														 const gchar *choice2,
														 gboolean *value,
														 GCallback hook,
														 gpointer data);
GtkWidget *	gsb_automem_spin_button_new 				(gint *value,
														 GCallback hook,
														 gpointer data);
GtkWidget *	gsb_automem_textview_new 					(gchar **value,
														 GCallback hook,
														 gpointer data);
/* END_DECLARATION */
#endif
