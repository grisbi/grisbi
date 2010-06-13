#ifndef _QIF_H
#define _QIF_H (1)
/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_qif_export_archive ( const gchar *filename,
                        gint archive_number );
gboolean qif_export ( const gchar *filename,
                        gint account_nb,
                        gint archive_number );
gboolean recuperation_donnees_qif ( GtkWidget *assistant, struct imported_file *imported );
/* END_DECLARATION */
#endif
