#define TEXT_NORMAL	(0<<0)
#define TEXT_BOLD	(1<<0)
#define TEXT_ITALIC	(1<<1)
#define TEXT_HUGE	(1<<2)
#define TEXT_LARGE	(1<<3)
#define TEXT_SMALL	(1<<4)


void etat_affiche_attach_hsep ( int x, int x2, int y, int y2);
void etat_affiche_attach_vsep ( int x, int x2, int y, int y2);
void etat_affiche_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
				 enum alignement align, struct structure_operation * ope );
