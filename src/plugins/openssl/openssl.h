#ifndef GSB_CRYPT_H
#define GSB_CRYPT_H

/* START_INCLUDE_H */
#include <glib.h>
/* END_INCLUDE_H */

/* START_DECLARATION */
void openssl_plugin_register ( void );
gpointer openssl_plugin_run ( gchar * file_name, gchar **file_content,
					  gboolean crypt, gulong length );
/* END_DECLARATION */

#endif
