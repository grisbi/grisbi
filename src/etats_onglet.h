#ifndef _ETATS_ONGLET_H
#define _ETATS_ONGLET_H (1)
/* START_INCLUDE_H */
#include "etats_onglet.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean ajout_etat ( void );
void changement_etat ( gint report_number );
GtkWidget *creation_onglet_etats ( void );
void dupliquer_etat ( void );
void efface_etat ( void );
void export_etat_courant_vers_csv ( );
void export_etat_courant_vers_html ( );
void exporter_etat ( void );
void importer_etat ( void );
/* END_DECLARATION */
#endif
