#ifndef _UTILS_XML_H
#define _UTILS_XML_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gchar * child_content ( xmlNodePtr node, gchar * child_name );
xmlNodePtr get_child ( xmlNodePtr node, gchar * child_name );
gboolean node_strcmp ( xmlNodePtr node, gchar * name );
/* END_DECLARATION */
#endif
