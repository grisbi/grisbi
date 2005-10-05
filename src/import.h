#ifndef _IMPORT_H
#define _IMPORT_H (1)
/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean affichage_recapitulatif_importation ( GtkWidget * assistant );
GSList * import_selected_files ( GtkWidget * assistant );
void importer_fichier ( void );
GtkWidget *onglet_importation (void);
/* END_DECLARATION */

enum import_type {
  TYPE_UNKNOWN = 0,
  TYPE_QIF,
  TYPE_OFX,
  TYPE_GNUCASH,
  TYPE_CSV,
  TYPE_MAX,
};

struct imported_file {
    gchar * name;
    enum import_type type;
};


#endif
