#ifndef PRINT_CONFIG_H
#define PRINT_CONFIG_H

#include <glib.h>

/*START_DECLARATION*/
gboolean print_config ( );
struct print_config * print_config_dup ();
void print_config_set ( struct print_config * config );
/*END_DECLARATION*/

#endif /* PRINT_CONFIG_H */
