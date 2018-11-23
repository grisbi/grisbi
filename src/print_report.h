#ifndef _PRINT_REPORT_H
#define _PRINT_REPORT_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean	print_report				(GtkWidget *button,
										 gpointer null);
void		print_report_export_pdf		(const gchar *pdf_name);
/* END_DECLARATION */
#endif
