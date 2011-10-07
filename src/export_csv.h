#ifndef EXPORT_CSV_H
#define EXPORT_CSV_H

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/*START_DECLARATION*/
gboolean gsb_csv_export_account ( const gchar *filename, gint account_number );
gboolean gsb_csv_export_archive ( const gchar *filename, gint archive_number );
gboolean gsb_csv_export_tree_view_list ( const gchar *filename, GtkTreeView *tree_view );
/*END_DECLARATION*/

#endif

