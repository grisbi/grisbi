#ifndef GSB_CRYPT_H
#define GSB_CRYPT_H

/* START_INCLUDE_H */
#include "gsb_crypt.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gulong gsb_file_util_crypt_file ( gchar * file_name, gchar **file_content,
				gboolean crypt, gulong length );
/* END_DECLARATION */

#endif
