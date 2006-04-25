#ifndef _H_UTILS_FILES
#define _H_UTILS_FILES 1

#include "config.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <libxml/parser.h>
/*START_DECLARATION*/
gint get_line_from_file ( FILE *fichier,
			  gchar **string );
GtkWidget * my_file_chooser ();
gchar* my_get_grisbirc_dir(void);
gchar* my_get_gsb_file_default_dir(void);
FILE*       utf8_fopen(gchar* utf8filename,gchar* mode);
gint        utf8_open(gchar* utf8filename,gint mode);
gint        utf8_stat(gchar* utf8filename,struct stat * filestat);
xmlDocPtr   utf8_xmlParseFile(const gchar *utf8filename);
gint        utf8_xmlSaveFormatFile(const gchar *utf8filename, xmlDocPtr cur, gint format);
gint        utf8_remove(const gchar* utf8filename);
gchar *     safe_file_name ( gchar * filename );
gchar*      utf8_long_file_name (gchar* utf8_short_file_name);

/*END_DECLARATION*/

#endif
