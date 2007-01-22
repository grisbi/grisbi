#ifndef EXPORT_CSV_H
#define EXPORT_CSV_H

/* START_INCLUDE_H */
#include "print_config.h"
/* END_INCLUDE_H */


/*START_DECLARATION*/
void csv_export ( gchar * filename, gint account_nb );
void export_accounts_to_csv ( GSList*  );
/*END_DECLARATION*/

#endif

