#ifndef _H_UTILS_FILES
#define _H_UTILS_FILES 1

#include "config.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <libxml/parser.h>
/*START_DECLARATION*/
FILE* utf8_fopen(gchar* utf8filename,gchar* mode);
xmlDocPtr utf8_xmlParseFile(const gchar *utf8filename);
/*END_DECLARATION*/

#endif
