gint recherche_categorie_par_nom ( struct struct_categ *categorie,
			     gchar *categ );
gint recherche_sous_categorie_par_nom ( struct struct_sous_categ *sous_categorie,
				  gchar *sous_categ );
gint recherche_categorie_par_no ( struct struct_categ *categorie,
				  gint *no_categorie );
gint recherche_sous_categorie_par_no ( struct struct_sous_categ *sous_categorie,
				       gint *no_sous_categorie );
gint recherche_banque_par_no ( struct struct_banque *banque,
			       gint *no_banque );
gint recherche_devise_par_nom ( struct struct_devise *devise,
				gchar *nom );
gint recherche_devise_par_code_iso ( struct struct_devise *devise,
				     gchar *nom );
gint recherche_devise_par_no ( struct struct_devise *devise,
			       gint *no_devise );
gint recherche_echeance_par_no ( struct operation_echeance *echeance,
				 gint no_echeance );
gint recherche_echeance_par_no_compte ( struct operation_echeance *echeance,
					gint no_compte );
gint recherche_rapprochement_par_nom ( struct struct_no_rapprochement *rapprochement,
				       gchar *no_rap );
gint recherche_rapprochement_par_no ( struct struct_no_rapprochement *rapprochement,
				      gint *no_rap );
gint recherche_nom_dans_liste ( gchar *nom_liste,
				gchar *nom_test );
gint recherche_exercice_par_nom ( struct struct_exercice *exercice,
				  gchar *nom );
gint recherche_exercice_par_no ( struct struct_exercice *exercice,
				 gint *no_exercice );
gint recherche_imputation_par_nom ( struct struct_imputation *imputation,
				    gchar *nom_imputation );
gint recherche_sous_imputation_par_nom ( struct struct_sous_imputation *sous_imputation,
					 gchar *nom_sous_imputation );
gint recherche_imputation_par_no ( struct struct_imputation *imputation,
				   gint *no_imputation );
gint recherche_sous_imputation_par_no ( struct struct_sous_imputation *sous_imputation,
					gint *no_sous_imputation );
gint recherche_operation_par_no ( struct structure_operation *operation,
				  gint *no_ope );
gint recherche_operation_par_cheque ( struct structure_operation *operation,
				  gint *no_chq );
gint recherche_operation_par_id ( struct structure_operation *operation,
				      gchar *id_recherchee );
gint recherche_tiers_par_no ( struct struct_tiers *tiers,
			      gint *no_tiers );
gint recherche_tiers_par_nom ( struct struct_tiers *tiers,
			       gchar *ancien_tiers );
gint recherche_type_ope_par_no ( struct struct_type_ope *type_ope,
				 gint *no_type );
gint cherche_string_equivalente_dans_slist ( gchar *string_list,
					     gchar *string_cmp );

