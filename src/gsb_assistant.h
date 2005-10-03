#ifndef GSB_ASSISTANT_H
#define GSB_ASSISTANT_H

/* START_INCLUDE_H */
#include "gsb_assistant.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_assistant_add_page ( GtkWidget * assistant, GtkWidget * widget, gint position,
			      gint prev, gint next, GCallback enter_callback );
GtkWidget * gsb_assistant_new ( gchar * title, gchar * explanation,
				gchar * image_filename );
void gsb_assistant_run ( GtkWidget * assistant );
/* END_DECLARATION */

#endif
