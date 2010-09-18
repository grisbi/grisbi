#ifndef GSB_GNUCASH_H
#define GSB_GNUCASH_H

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
G_MODULE_EXPORT extern void gnucash_plugin_register ();
G_MODULE_EXPORT extern gboolean gnucash_plugin_release ( );
G_MODULE_EXPORT extern gint gnucash_plugin_run ( gchar * file_name, gchar **file_content,
					  gboolean crypt, gulong length );
/* END_DECLARATION */

#endif
