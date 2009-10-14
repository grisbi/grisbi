#ifndef GSB_GNUCASH_H
#define GSB_GNUCASH_H

/* FIXME : need to be set here because mk_include cannot go into subrep for now */
/* gsb_real gsb_real_double_to_real ( gdouble number ); */

/* START_INCLUDE_H */
#include "./../.././gsb_real.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
G_MODULE_EXPORT extern void gnucash_plugin_register ();
G_MODULE_EXPORT extern gboolean gnucash_plugin_release ( );
G_MODULE_EXPORT extern gint gnucash_plugin_run ( gchar * file_name, gchar **file_content,
					  gboolean crypt, gulong length );
/* END_DECLARATION */

#endif
