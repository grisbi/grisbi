#ifndef GSB_CRYPT_H
#define GSB_CRYPT_H

/* START_INCLUDE_H */
#include <glib.h>
/* END_INCLUDE_H */

/* START_DECLARATION */
G_MODULE_EXPORT extern void openssl_plugin_register ( void );
G_MODULE_EXPORT extern gpointer openssl_plugin_run ( gchar * file_name, gchar **file_content,
					  gboolean crypt, gulong length );
/* END_DECLARATION */

#endif
