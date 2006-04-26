#ifndef GSB_CRYPT_H
#define GSB_CRYPT_H

#include <libintl.h>
#include <locale.h>

#define GTK_ENABLE_BROKEN 1
#define G_BROKEN_FILENAMES 1
#include <gtk/gtk.h>


/* START_INCLUDE_H */
#include "openssl.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gulong gsb_file_util_crypt_file ( gchar * file_name, gchar **file_content,
				  gboolean crypt, gulong length );
/* END_DECLARATION */

#endif
