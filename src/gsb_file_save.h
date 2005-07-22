#ifndef _GSB_FILE_SAVE_H
#define _GSB_FILE_SAVE_H (1)


/* START_INCLUDE_H */
#include "gsb_file_save.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gint gsb_file_save_append_part ( gint iterator,
				 gint *length_calculated,
				 gchar **file_content,
				 gchar *new_string );
gint gsb_file_save_budgetary_part ( gint iterator,
				    gint *length_calculated,
				    gchar **file_content );
gint gsb_file_save_category_part ( gint iterator,
				   gint *length_calculated,
				   gchar **file_content );
gint gsb_file_save_report_part ( gint iterator,
				 gint *length_calculated,
				 gchar **file_content );
gboolean gsb_file_save_save_file ( gchar *filename );
/* END_DECLARATION */

#endif

