#ifndef GSB_CRYPT_H
#define GSB_CRYPT_H

#include <libintl.h>
#include <locale.h>

#define G_BROKEN_FILENAMES 1
#include <gtk/gtk.h>


/* START_INCLUDE_H */
#include "./../.././include.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
G_MODULE_EXPORT extern void openssl_plugin_register ();
G_MODULE_EXPORT extern gboolean openssl_plugin_release ( );
G_MODULE_EXPORT extern gint openssl_plugin_run ( gchar * file_name, gchar **file_content,
					  gboolean crypt, gulong length );
/* END_DECLARATION */

#endif
