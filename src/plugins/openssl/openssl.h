#ifndef GSB_CRYPT_H
#define GSB_CRYPT_H

#include <libintl.h>
#include <locale.h>

#define GTK_ENABLE_BROKEN 1
#define G_BROKEN_FILENAMES 1
#include <gtk/gtk.h>


/* START_INCLUDE_H */
#include "./../../gsb_crypt.h"
#include "./../../include.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gchar *gsb_file_util_ask_for_crypt_key ( gchar * file_name, gchar * additional_message,
					 gboolean encrypt );
gulong gsb_file_util_crypt_file ( gchar * file_name, gchar **file_content,
				  gboolean crypt, gulong length );
extern void openssl_plugin_register ();
extern gboolean openssl_plugin_release ( );
extern gint openssl_plugin_run ( gchar * file_name, gchar **file_content,
					  gboolean crypt, gulong length );
/* END_DECLARATION */

#endif
