#ifndef GSB_OFX_H
#define GSB_OFX_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean 	recuperation_donnees_ofx 		(GtkWidget *assistant,
											 ImportFile *imported);
/* END_DECLARATION */

#endif /* GSB_OFX_H */
