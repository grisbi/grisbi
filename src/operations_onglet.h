#define START_DECLARATION
GtkWidget *creation_onglet_operations ( void );
struct structure_operation *operation_par_cheque ( gint no_cheque,
						   gint no_compte );
struct structure_operation *operation_par_id ( gchar *no_id,
					       gint no_compte );
struct structure_operation *operation_par_no ( gint no_operation,
					       gint no_compte );
#define END_DECLARATION

