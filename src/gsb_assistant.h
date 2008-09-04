#ifndef GSB_ASSISTANT_H
#define GSB_ASSISTANT_H

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_assistant_add_page ( GtkWidget * assistant, GtkWidget * widget, gint position,
			      gint prev, gint next, GCallback enter_callback );
void gsb_assistant_change_button_next ( GtkWidget * assistant, gchar * title,
					GtkResponseType response );
GtkWidget * gsb_assistant_new ( const gchar * title, const gchar * explanation,
				gchar * image_filename,
				GCallback enter_callback );
void gsb_assistant_next_page ( GtkWidget *assistant );
GtkResponseType gsb_assistant_run ( GtkWidget * assistant );
gboolean gsb_assistant_sensitive_button_next ( GtkWidget * assistant, gboolean state );
gboolean gsb_assistant_sensitive_button_prev ( GtkWidget * assistant, gboolean state );
void gsb_assistant_set_next ( GtkWidget * assistant, gint page, gint next );
void gsb_assistant_set_prev ( GtkWidget * assistant, gint page, gint prev );
/* END_DECLARATION */

#endif
