/* fichier d'en tête etats_latex.h */

gint latex_initialise (GSList * opes_selectionnees);
gint latex_finish ();
void latex_attach_hsep ( int x, int x2, int y, int y2);
void latex_attach_vsep ( int x, int x2, int y, int y2);
void latex_attach_label ( gchar * text, int x, int x2, int y, int y2, 
			     enum alignement align, struct structure_operation * ope );

struct struct_etat_affichage latex_affichage;
