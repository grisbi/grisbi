/* fichier d'en tête etats_html.h */

struct struct_etat_affichage html_affichage;

/* Html backend functions */
gint html_initialise (GSList * opes_selectionnees);
gint html_finish ();
void html_attach_hsep ( int x, int x2, int y, int y2);
void html_attach_vsep ( int x, int x2, int y, int y2);
void html_attach_label ( gchar * text, gdouble properties, int x, int x2, 
			  int y, int y2, enum alignement align, 
			  struct structure_operation * ope );

/* Utility functions */
void html_safe ( gchar * text );
