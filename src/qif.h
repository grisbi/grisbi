#ifndef _QIF_H
#define _QIF_H (1)
/* START_INCLUDE_H */
#include "qif.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void exporter_fichier_qif ( void );
gboolean recuperation_donnees_qif ( FILE *fichier, gchar * filename );
/* END_DECLARATION */
#endif
