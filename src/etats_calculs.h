/*START_DECLARATION*/
void affichage_etat ( struct struct_etat *etat, 
		      struct struct_etat_affichage * affichage );
void denote_struct_sous_jaccentes ( gint origine );
void impression_etat ( struct struct_etat *etat );
void impression_etat_courant ( );
void rafraichissement_etat ( struct struct_etat *etat );
GSList *recupere_opes_etat ( struct struct_etat *etat );
/*END_DECLARATION*/

