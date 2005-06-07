#ifndef _FICHIERS_IO_H
#define _FICHIERS_IO_H (1)
/* START_INCLUDE_H */
#include "fichiers_io.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean charge_categ ( gchar *nom_categ );
gboolean charge_ib ( gchar *nom_ib );
gboolean charge_operations ( gchar *nom_fichier );
gboolean enregistre_categ ( gchar *nom_categ );
gboolean enregistre_fichier ( gchar *new_file );
gboolean enregistre_ib ( gchar *nom_ib );
gboolean modification_etat_ouverture_fichier ( gboolean fichier_ouvert );
void propose_changement_permissions ( void );
/* END_DECLARATION */
#endif
