#ifndef _IMPORT_CSV_H
#define _IMPORT_CSV_H (1)

/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean csv_import_csv_account ( GtkWidget * assistant, struct imported_file * imported );
GtkWidget * import_create_csv_preview_page ( GtkWidget * assistant );
gboolean import_enter_csv_preview_page ( GtkWidget * assistant );
/* END_DECLARATION */

struct csv_field {
    gchar * name;
    gfloat alignment;
    gboolean (* validate ) ( gchar * );
    gboolean (* parse ) ( struct struct_ope_importation *, gchar * );
    gchar * alias;
};

#endif
