#ifndef UTILS_XML_H
#define UTILS_XML_H

#include <libxml/parser.h>

/*START_DECLARATION*/
gboolean node_strcmp ( xmlNodePtr node, gchar * name );
xmlNodePtr get_child ( xmlNodePtr node, gchar * child_name );
gchar * child_content ( xmlNodePtr node, gchar * child_name );
/*END_DECLARATION*/

#endif
