/*START_DECLARATION*/
GtkWidget *creation_liste_organisation_formulaire ( void );
gboolean recherche_place_element_formulaire ( struct organisation_formulaire *structure_formulaire,
					      gint no_element,
					      gint *ligne,
					      gint *colonne );
gboolean verifie_element_formulaire_existe ( gint no_element );
/*END_DECLARATION*/

