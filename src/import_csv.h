#ifndef _IMPORT_CSV_H
#define _IMPORT_CSV_H (1)

/* START_INCLUDE_H */
#include "import_csv.h"
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean importer_csv ();
gboolean recuperation_donnees_csv ( FILE *fichier );
/* END_DECLARATION */

struct csv_field {
    gboolean * name;
    gfloat alignment;
    gboolean (* validate ) ( gchar * );
    gboolean (* parse ) ( struct struct_ope_importation *, gchar * );
};

#endif
