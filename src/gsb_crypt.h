#ifndef GSB_CRYPT_H
#define GSB_CRYPT_H

/* START_INCLUDE_H */
#include "././plugins/openssl/openssl.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gchar *gsb_file_util_ask_for_crypt_key ( gchar * file_name, gboolean encrypt );
gulong gsb_file_util_crypt_file ( gchar * file_name, gchar **file_content,
				  gboolean crypt, gulong length );
/* END_DECLARATION */

#endif
