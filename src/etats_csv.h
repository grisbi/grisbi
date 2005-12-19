/* fichier d'en tête etats_csv.h */

struct struct_etat_affichage csv_affichage;

/* Csv backend functions */
gint csv_initialise (GSList * opes_selectionnees);
gint csv_finish ();
void csv_attach_hsep ( int x, int x2, int y, int y2);
void csv_attach_vsep ( int x, int x2, int y, int y2);
void csv_attach_label ( gchar * text, gdouble properties, int x, int x2, 
			  int y, int y2, enum alignement align, 
			  struct structure_operation * ope );

/* Utility functions */
void csv_safe ( gchar * text );
