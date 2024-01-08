#ifndef _IMPORT_CSV_H
#define _IMPORT_CSV_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean 		csv_import_change_separator			(GtkEntry *entry,
													 GtkWidget *assistant);
gboolean 		csv_import_csv_account 				(GtkWidget *assistant,
													 struct ImportFile *imported);
gboolean		csv_import_file_by_rule				(gint rule,
													 struct ImportFile *imported);
GSList *		csv_import_get_columns_list			(GtkWidget *assistant);
void			csv_import_set_first_data_line		(gint first_data_line);
GtkWidget * 	import_create_csv_preview_page 		(GtkWidget *assistant);
gboolean 		import_enter_csv_preview_page 		(GtkWidget *assistant);
/* END_DECLARATION */

#endif
