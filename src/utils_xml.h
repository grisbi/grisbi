#ifndef UTILS_XML_H
#define UTILS_XML_H

#include <libxml/parser.h>

/*START_DECLARATION*/
gchar * child_content ( xmlNodePtr node, gchar * child_name );
xmlNodePtr get_child ( xmlNodePtr node, gchar * child_name );
gboolean node_strcmp ( xmlNodePtr node, gchar * name );
/*END_DECLARATION*/

#endif
