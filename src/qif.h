#ifndef _QIF_H
#define _QIF_H (1)
/* START_INCLUDE_H */
#include "qif.h"
#include "import.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void qif_export ( gchar * filename, gint account_nb );
gboolean recuperation_donnees_qif ( FILE * fichier, struct imported_file * imported );
/* END_DECLARATION */
#endif
