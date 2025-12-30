#ifndef GSB_GNUCASH_H
#define GSB_GNUCASH_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean	recuperation_donnees_gnucash 	(GtkWidget *assistant,
											 ImportFile *imported);
/* END_DECLARATION */

#endif
