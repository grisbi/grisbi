void affichage_etat ( struct struct_etat *etat, 
		      struct struct_etat_affichage *affichage );
GSList *recupere_opes_etat ( struct struct_etat *etat );
gchar *recupere_texte_test_etat ( struct structure_operation *operation,
				  gint champ );
gint verifie_texte_test_etat ( struct struct_comparaison_textes_etat *comp_textes,
			       gchar *texte_ope );
gint verifie_chq_test_etat ( struct struct_comparaison_textes_etat *comp_textes,
			     gchar *no_chq );
gint compare_cheques_etat ( gint chq_ope, gint chq_test, gint comparateur );
gint compare_montants_etat ( gdouble montant_ope, gdouble montant_test,
			     gint comparateur );
void rafraichissement_etat ( struct struct_etat *etat );
gint classement_liste_opes_etat ( struct structure_operation *operation_1,
				  struct structure_operation *operation_2 );
gint classement_ope_perso_etat ( struct structure_operation *operation_1,
				 struct structure_operation *operation_2 );
void etape_finale_affichage_etat ( GSList *ope_selectionnees, 
				   struct struct_etat_affichage *affichage );
void denote_struct_sous_jaccentes ( gint origine );
void impression_etat ( struct struct_etat *etat );
void impression_etat_courant ( );
