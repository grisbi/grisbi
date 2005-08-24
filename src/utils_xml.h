#ifndef _UTILS_XML_H
#define _UTILS_XML_H (1)
/* START_INCLUDE_H */
#include "utils_xml.h"
/* END_INCLUDE_H */

#include <libxml/parser.h>

/* START_DECLARATION */
gchar * child_content ( xmlNodePtr node, gchar * child_name );
xmlNodePtr get_child ( xmlNodePtr node, gchar * child_name );
gboolean node_strcmp ( xmlNodePtr node, gchar * name );
/* END_DECLARATION */
#endif
