#define START_DECLARATION
void calcule_total_pointe_compte ( gint no_compte );
GtkWidget *creation_fenetre_equilibrage ( void );
void equilibrage ( void );
void pointe_equilibrage ( int p_ligne );
gchar *rapprochement_name_by_no ( gint no_rapprochement );
struct struct_no_rapprochement *rapprochement_par_nom ( gchar *nom_rapprochement );
GtkWidget * tab_display_reconciliation ( void );
#define END_DECLARATION

