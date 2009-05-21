#ifndef _GSB_DATA_ARCHIVE_H
#define _GSB_DATA_ARCHIVE_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
GSList *gsb_data_archive_get_archives_list ( void );
GDate *gsb_data_archive_get_beginning_date ( gint archive_number );
GDate *gsb_data_archive_get_end_date ( gint archive_number );
gint gsb_data_archive_get_from_date ( const GDate *date );
gint gsb_data_archive_get_from_fyear ( gint fyear_number );
gint gsb_data_archive_get_fyear ( gint archive_number );
const gchar *gsb_data_archive_get_name ( gint archive_number );
gint gsb_data_archive_get_no_archive ( gpointer archive_ptr );
const gchar *gsb_data_archive_get_report_title ( gint archive_number );
gboolean gsb_data_archive_init_variables ( void );
gint gsb_data_archive_new ( const gchar *name );
gboolean gsb_data_archive_remove ( gint archive_number );
gboolean gsb_data_archive_set_beginning_date ( gint archive_number,
                        const GDate *date );
gboolean gsb_data_archive_set_end_date ( gint archive_number,
                        const GDate *date );
gboolean gsb_data_archive_set_fyear ( gint archive_number,
                        gint fyear_number );
gboolean gsb_data_archive_set_name ( gint archive_number,
                        const gchar *name );
gint gsb_data_archive_set_new_number ( gint archive_number,
                        gint new_no_archive );
gboolean gsb_data_archive_set_report_title ( gint archive_number,
                        const gchar *report_title );
/* END_DECLARATION */
#endif
