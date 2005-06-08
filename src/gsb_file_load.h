#ifndef _GSB_FILE_LOAD_H
#define _GSB_FILE_LOAD_H (1)


/* START_INCLUDE_H */
#include "gsb_file_load.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean file_io_fix_xml_corrupted_file_lock_tag(gchar* accounts_filename);
gboolean gsb_file_load_open_file ( gchar *filename );
gboolean modification_etat_ouverture_fichier ( gboolean fichier_ouvert );
void propose_changement_permissions ( void );
void switch_t_r ( void );
/* END_DECLARATION */
#endif

