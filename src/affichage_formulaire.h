GtkWidget *creation_liste_organisation_formulaire ( void );
gboolean modification_formulaire_distinct_par_compte ( void );
gboolean modification_compte_choix_formulaire ( GtkWidget *menu_item );
gboolean recherche_place_element_formulaire ( struct organisation_formulaire *structure_formulaire,
					      gint no_element,
					      gint *ligne,
					      gint *colonne );
gboolean verifie_element_formulaire_existe ( gint no_element );

