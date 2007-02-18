#ifndef GSB_G2BANKING_H
#define GSB_G2BANKING_H


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
extern void g2banking_plugin_register ();
extern gboolean g2banking_plugin_release ( );
extern GtkWidget * g2banking_plugin_run ( gchar * file_name, gchar **file_content,
						   gboolean crypt, gulong length );
/* END_DECLARATION */

#endif
