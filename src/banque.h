#define START_DECLARATION
struct struct_banque *banque_par_no ( gint no_banque );
GtkWidget *creation_menu_banques ( void );
void edit_bank ( GtkWidget * button, struct struct_banque * bank );
GtkWidget *onglet_banques ( void );
void view_bank ( GtkWidget * button, gpointer data );
#define END_DECLARATION

