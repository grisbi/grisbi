/* fichier d'en tête en_tete.h */
 


/********************/ 
/* fichier main.c */
/********************/ 


int main (int argc, char *argv[]);
gint tente_modif_taille ( GtkWidget *win,
			  GtkAllocation *requisition,
			  gpointer null );




/********************/ 
/* fichier menu.c */
/********************/ 

void init_menus ( GtkWidget * );
void efface_derniers_fichiers_ouverts ( void );
void affiche_derniers_fichiers_ouverts ( void );
void lien_web ( GtkWidget *widget,
		gint origine );

/********************/ 
/* fichier erreurs.c */
/********************/ 

gboolean fermeture_grisbi ( void );
void dialogue ( gchar * texte_dialogue );
gboolean blocage_boites_dialogues ( GtkWidget *dialog,
				    gpointer null );
gboolean question ( gchar *texte );
gboolean question_yes_no ( gchar *texte );
gchar *demande_texte ( gchar *titre_fenetre,
		       gchar *question );
void affiche_log_message ( void );
void traitement_sigsegv ( gint signal_nb );



/***********************************/ 
/* fichier fichier_configuration.c */
/***********************************/ 

void charge_configuration ( void );
void raz_configuration ( void );
void sauve_configuration (void);



/********************/ 
/* fichier traitement_variables.c */
/********************/ 

void modification_fichier ( gboolean modif );
void init_variables ( gboolean ouverture );
void initialise_tab_affichage_ope ( void );



/********************/ 
/* fichier fichiers_io.c */
/********************/ 


gboolean charge_operations ( void );
gboolean charge_operations_version_0_3_2 ( xmlDocPtr doc );
void supprime_operations_orphelines ( void );
gboolean charge_operations_version_0_4_0 ( xmlDocPtr doc );
gboolean charge_operations_version_0_4_1 ( xmlDocPtr doc );
gboolean enregistre_fichier ( gboolean force );
gchar *itoa ( gint integer );
void fichier_marque_ouvert ( gint ouvert );
gboolean enregistre_categ ( gchar *nom_categ );
gboolean charge_categ ( gchar *nom_categ );
gboolean charge_categ_version_0_4_0 ( xmlDocPtr doc );
gboolean enregistre_ib ( gchar *nom_ib );
gboolean charge_ib ( gchar *nom_ib );
gboolean charge_ib_version_0_4_0 ( xmlDocPtr doc );
void propose_changement_permissions ( void );
double my_strtod ( char *nptr, char **endptr );




/************************/ 
/* fichier fichiers_gestion.c */
/************************/ 


void nouveau_fichier ( void );
void ouvrir_fichier ( void );
void ouverture_fichier_par_menu ( GtkWidget *menu,
				  gchar *nom );
void fichier_selectionne ( GtkWidget *selection_fichier);
void ouverture_confirmee ( void );
gboolean enregistrement_fichier ( gint origine );
gboolean enregistrer_fichier_sous ( void );
gboolean fermer_fichier ( void );
void fermeture_confirmee ( void );
void affiche_titre_fenetre ( void );
gboolean enregistrement_backup ( void );
void ajoute_nouveau_fichier_liste_ouverture ( gchar *path_fichier );




/*********************/ 
/* fichier fenetre_principale.c */
/*********************/ 


GtkWidget *creation_fenetre_principale ( void );
void change_page_notebook ( GtkNotebook *notebook,
			    GtkNotebookPage *page,
			    guint numero_page,
			    gpointer null );



/***********************************/ 
/* fichier accueil.c */
/***********************************/ 

GtkWidget *creation_onglet_accueil ( void );
void change_temps ( GtkWidget *label_temps );
void saisie_echeance_accueil ( GtkWidget *event_box,
			       GdkEventButton *event,
			       struct operation_echeance *echeance );
void met_en_prelight ( GtkWidget *event_box,
		       GdkEventMotion *event,
		       gpointer pointeur );
void met_en_normal ( GtkWidget *event_box,
		     GdkEventMotion *event,
		     gpointer pointeur );
void update_liste_comptes_accueil ( void );
void update_liste_echeances_manuelles_accueil ( void );
void update_liste_echeances_auto_accueil ( void );
void mise_a_jour_soldes_minimaux ( void );
void mise_a_jour_fin_comptes_passifs ( void );



/*********************/ 
/* fichier onglet_operations.c */
/*********************/ 

GtkWidget *creation_onglet_operations ( void );



/************************/ 
/* fichier operations_comptes.c */
/************************/ 

GtkWidget *creation_liste_comptes (void);
GtkWidget *comptes_appel ( gint no_de_compte );
void changement_compte_par_menu ( GtkWidget *menu,
				  gint *compte );
void changement_compte ( gint *compte);
void reaffiche_liste_comptes ( void );



/************************/ 
/* fichier operations_liste.c */
/************************/ 

GtkWidget *creation_fenetre_operations ( void );
GtkWidget *initialisation_notebook_operations ( void );
void creation_listes_operations ( void );
gint empeche_pression_titre_colonne ( GtkWidget *bouton );
void ajoute_nouvelle_liste_operation ( gint no_compte );
void onglet_compte_realize ( GtkWidget *onglet,
			     GtkWidget *liste );
void remplissage_liste_operations ( gint compte );
gchar *recherche_contenu_cellule ( struct structure_operation *operation,
				   gint no_affichage );
void selectionne_ligne_souris ( GtkCList *liste_operations,
				GdkEventButton *evenement,
				gpointer data);
gboolean traitement_clavier_liste ( GtkCList *liste_operations,
				     GdkEventKey *evenement,
				     gpointer data);
void selectionne_ligne ( gint compte );
void edition_operation ( void );
void p_press (void);
void r_press (void);
void supprime_operation ( struct structure_operation *operation );
gint recherche_operation_par_no ( struct structure_operation *operation,
				  gint *no_ope );
void changement_taille_liste_ope ( GtkWidget *clist,
				   GtkAllocation *allocation,
				   gint *compte );
void focus_a_la_liste ( void );
void demande_mise_a_jour_tous_comptes ( void );
void verification_mise_a_jour_liste ( void );
void mise_a_jour_solde ( gint compte );
void verifie_ligne_selectionnee_visible ( void );
void changement_taille_colonne ( GtkWidget *clist,
				 gint colonne,
				 gint largeur );


/************************/ 
/* fichier operations_formulaire.c */
/************************/ 

GtkWidget *creation_formulaire ( void );
void echap_formulaire ( void );
void entree_prend_focus ( GtkWidget *entree );
void entree_perd_focus ( GtkWidget *entree,
			 GdkEventFocus *ev,
			 gint *no_origine );
void clique_champ_formulaire ( GtkWidget *entree,
			       GdkEventButton *ev,
			       gint *no_origine );
gboolean touches_champ_formulaire ( GtkWidget *widget,
				    GdkEventKey *ev,
				    gint *no_origine );
void ferme_calendrier ( GtkWidget *popup );
void  touche_calendrier ( GtkWidget *popup,
			  GdkEventKey *ev,
			  gpointer null );
void date_selectionnee ( GtkCalendar *calendrier,
			 GtkWidget *popup );
gboolean modifie_date ( GtkWidget *entree );
void date_bancaire_selectionnee ( GtkCalendar *calendrier,
			 GtkWidget *popup );
void  completion_operation_par_tiers ( void );
void fin_edition ( void );
void ajout_operation ( struct structure_operation *operation );
gint verification_validation_operation ( struct structure_operation *operation );
void recuperation_donnees_generales_formulaire ( struct structure_operation *operation );
void validation_virement_operation ( struct structure_operation *operation,
				     gint modification,
				     gchar *nom_compte_vire );
void recuperation_categorie_formulaire ( struct structure_operation *operation,
					 gint modification );
gint comparaison_date_list_ope ( struct structure_operation *ope_1,
				 struct structure_operation *ope_2);
void formulaire_a_zero (void);
gchar *date_jour ( void );
void affiche_cache_le_formulaire ( void );

/*dOm fonction callback */
void affiche_cache_commentaire_echeancier( void );

void allocation_taille_formulaire ( GtkWidget *widget,
				    gpointer null );
void efface_formulaire ( GtkWidget *widget,
			 gpointer null );
void basculer_vers_ventilation ( GtkWidget *bouton,
				 gpointer null );
void click_sur_bouton_voir_change ( void );
void degrise_formulaire_operations ( void );
void incremente_decremente_date ( GtkWidget *entree,
				  gint demande );
gint place_type_choix_type ( GtkWidget *option_menu,
			     gint type );
void increment_decrement_champ ( GtkWidget *entry, gint increment );




/************************/ 
/* fichier comptes_onglet.c */
/************************/ 


GtkWidget *creation_onglet_comptes ( void );
GtkWidget *creation_liste_comptes_onglet ( void );
GtkWidget *comptes_appel_onglet ( gint no_de_compte );
void changement_compte_onglet ( GtkWidget *bouton,
				gint compte );
void reaffiche_liste_comptes_onglet ( void );




/***********************************/ 
/* fichier comptes_gestion.c */
/***********************************/ 

GtkWidget *creation_details_compte ( void );
GtkWidget *creation_menu_type_compte ( void );
void changement_bouton_adresse_commune_perso ( void );
void modif_detail_compte ( GtkWidget *hbox );
void remplissage_details_compte ( void );
gint recherche_banque_par_no ( struct struct_banque *banque,
			       gint *no_banque );
void modification_details_compte ( void );
void sort_du_detail_compte ( void );
void passage_a_l_euro ( GtkWidget *bouton,
			gpointer null );
void changement_de_banque ( void );


/************************/ 
/* fichier comptes_traitements.c */
/************************/ 

void nouveau_compte (void);
gint initialisation_nouveau_compte ( gint type_de_compte );
void supprimer_compte ( void );
void compte_choisi_destruction ( GtkWidget *item_list,
				   GdkEvent *evenement,
				   GtkWidget *liste_comptes);
gint cherche_compte_dans_echeances ( struct operation_echeance *echeance,
				     gint no_compte );
GtkWidget *creation_option_menu_comptes ( void );
void changement_choix_compte_echeancier ( void );
void creation_types_par_defaut ( gint no_compte,
				 gulong dernier_cheque );
gint demande_type_nouveau_compte ( void );


/************************/ 
/* fichier help.c */
/************************/ 

void a_propos ( GtkWidget *bouton,
		gint data );




/************************/ 
/* fichier equilibrage.c */
/************************/ 

GtkWidget *creation_fenetre_equilibrage ( void );
void equilibrage ( void );
void sortie_entree_date_equilibrage ( void );
void modif_entree_solde_init_equilibrage ( void );
void modif_entree_solde_final_equilibrage ( void );
void annuler_equilibrage ( GtkWidget *bouton_ann,
			   gpointer data);
void pointe_equilibrage ( int p_ligne );
void fin_equilibrage ( GtkWidget *bouton_ok,
		       gpointer data );
gint recherche_no_rapprochement_par_nom ( struct struct_no_rapprochement *rapprochement,
					  gchar *no_rap );
gint recherche_no_rapprochement_par_no ( struct struct_no_rapprochement *rapprochement,
					 gint *no_rap );
void calcule_total_pointe_compte ( gint no_compte );



/************************/ 
/* fichier parametres.c */
/************************/ 

void preferences ( GtkWidget *widget,
		   gint page_demandee );
GtkWidget *onglet_general ( void );
GtkWidget *onglet_fichier ( void );
GtkWidget *onglet_echeances ( void );
GtkWidget *onglet_applet ( void );
void change_selection_verif ( GtkWidget *liste_comptes,
			       gint ligne,
			       gint colonne,
			       GdkEventButton *event,
			      gint * data );
void changement_utilisation_applet ( void );
void ajouter_verification ( GtkWidget *bouton_add,
			    GtkWidget *fenetre_preferences );
void supprimer_verification ( GtkWidget *bouton_supp,
			      GtkWidget *fenetre_preferences );
void changement_choix_backup ( GtkWidget *bouton,
			       gpointer pointeur );
void changement_preferences ( GtkWidget *fenetre_preferences,
			      gint page,
			      gpointer data );
gint verifie_affichage_applet ( void );



/***********************************/ 
/* fichier echeancier_liste.c */
/***********************************/ 

GtkWidget *creation_onglet_echeancier ( void );
GtkWidget *creation_partie_gauche_echeancier ( void );
GtkWidget *creation_liste_echeances ( void );
gboolean traitement_clavier_liste_echeances ( GtkCList *liste_echeances,
					      GdkEventKey *evenement,
					      gpointer null );
void click_sur_saisir_echeance ( void );
void remplissage_liste_echeance ( void );
void click_ligne_echeance ( GtkCList *liste,
			    GdkEventButton *evenement,
			    gpointer data );
void selectionne_echeance ( void );
void edition_echeance ( void );
void supprime_echeance ( void );
void changement_taille_liste_echeances ( GtkWidget *clist,
					 GtkAllocation *allocation,
					 gpointer null );
void mise_a_jour_calendrier ( void );
void click_sur_jour_calendrier_echeance ( GtkWidget *calendrier,
					  gpointer null );
void verification_echeances_a_terme ( void );
gint recherche_echeance_par_no ( struct operation_echeance *echeance,
				 gint no_echeance );
void verifie_ligne_selectionnee_echeance_visible ( void );
void modification_affichage_echeances ( gint *origine );
GDate *date_suivante_echeance ( struct operation_echeance *echeance,
			      GDate *date_courante );
gint classement_liste_echeances ( GtkWidget *liste,
				  GtkCListRow *ligne_1,
				  GtkCListRow *ligne_2 );



/************************/ 
/* fichier echeancier_formulaire.c */
/************************/ 

GtkWidget *creation_formulaire_echeancier ( void );
void echap_formulaire_echeancier ( void );
void entree_perd_focus_echeancier ( GtkWidget *entree,
				    GdkEventFocus *ev,
				    gint *no_origine );
void affiche_cache_le_formulaire_echeancier ( void );
void clique_champ_formulaire_echeancier ( GtkWidget *entree,
					  GdkEventButton *ev,
					  gint *no_origine );
gboolean pression_touche_formulaire_echeancier ( GtkWidget *widget,
						 GdkEventKey *ev,
						 gint no_widget );
void affiche_date_limite_echeancier ( void );
void cache_date_limite_echeancier ( void );
void affiche_personnalisation_echeancier ( void );
void cache_personnalisation_echeancier ( void );
void fin_edition_echeance ( void );
gint comparaison_date_echeance (  struct operation_echeance *echeance_1,
				  struct operation_echeance *echeance_2);
void formulaire_echeancier_a_zero ( void );
void incrementation_echeance ( struct operation_echeance *echeance );
void date_selectionnee_echeancier ( GtkCalendar *calendrier,
				    GtkWidget *popup );
void date_limite_selectionnee_echeancier ( GtkCalendar *calendrier,
					   GtkWidget *popup );
void completion_operation_par_tiers_echeancier ( void );
void degrise_formulaire_echeancier ( void );




/***********************************/ 
/* fichier tiers_onglet.c */
/***********************************/ 

GtkWidget *onglet_tiers ( void );
void remplit_arbre_tiers ( void );
gint classement_alphabetique_tree ( GtkWidget *tree,
				    GtkCListRow *ligne_1,
				    GtkCListRow *ligne_2 );
void ouverture_node_tiers ( GtkWidget *arbre,
			    GtkCTreeNode *node,
			    gpointer null );
void selection_ligne_tiers ( GtkCTree *arbre_tiers,
			     GtkCTreeNode *noeud,
			     gint colonne,
			     gpointer null );
void verifie_double_click ( GtkWidget *liste,
			    GdkEventButton *ev,
			    gpointer null );
void enleve_selection_ligne_tiers ( void );
void modification_du_texte_tiers ( GtkText *texte,
				   gpointer null );
void clique_sur_modifier_tiers ( GtkWidget *bouton_modifier,
				 gpointer null );
void clique_sur_annuler_tiers ( GtkWidget *bouton_annuler,
				gpointer null );
struct struct_tiers *ajoute_nouveau_tiers ( gchar *tiers );
void supprimer_tiers ( GtkWidget *bouton,
		       gpointer null );
void creation_liste_tiers_combofix ( void );
void changement_taille_liste_tiers ( GtkWidget *clist,
					   GtkAllocation *allocation,
					   gpointer null );
void mise_a_jour_tiers ( void );
gint recherche_tiers_par_no ( struct struct_tiers *tiers,
			      gint *no_tiers );
gint recherche_tiers_par_nom ( struct struct_tiers *tiers,
			       gchar *ancien_tiers );
gfloat *calcule_total_montant_tiers ( void );
gchar *calcule_total_montant_tiers_par_compte ( gint no_tiers,
					       gint no_compte );
void appui_sur_ajout_tiers ( void );



/***********************************/ 
/* fichier devises.c */
/***********************************/ 


void creation_devises_de_base ( void );
GtkWidget *creation_option_menu_devises ( gint devise_cachee,
					  GSList *liste_tmp );
void ajout_devise ( GtkWidget *bouton,
		    GtkWidget *widget );
gint bloque_echap_choix_devise ( GtkWidget *dialog,
				 GdkEventKey *key,
				 gpointer null );
void nom_nouvelle_devise_defini ( GtkWidget *entree,
				  GdkEventFocus *ev,
				  GtkWidget *label );
void retrait_devise ( GtkWidget *bouton,
		      GtkWidget *liste );
gint recherche_devise_par_nom ( struct struct_devise *devise,
				gchar *nom );
gint recherche_devise_par_no ( struct struct_devise *devise,
			       gint *no_devise );
gint selection_devise ( gchar *nom_du_compte );
void passe_a_l_euro ( GtkWidget *toggle_bouton,
		      GtkWidget *hbox );
void demande_taux_de_change ( struct struct_devise *devise_compte,
			      struct struct_devise *devise,
			      gint une_devise_compte_egale_x_devise_ope,
			      gdouble taux_change,
			      gdouble frais_change );
void devise_selectionnee ( GtkWidget *menu_shell,
			   gint origine );
GtkWidget *onglet_devises ( void );
void selection_ligne_devise ( GtkWidget *liste,
			      gint ligne,
			      gint colonne,
			      GdkEventButton *ev,
			      GtkWidget *frame );
void deselection_ligne_devise ( GtkWidget *liste,
				gint ligne,
				gint colonne,
				GdkEventButton *ev,
				GtkWidget *frame );
void change_passera_euro ( GtkWidget *bouton,
			   GtkWidget *liste );
void changement_devise_associee ( GtkWidget *menu_devises,
				  GtkWidget *liste );
void changement_nom_entree_devise ( void );
void changement_code_entree_devise ( void );
gdouble calcule_montant_devise_renvoi ( gdouble montant_init,
					gint no_devise_renvoi,
					gint no_devise_montant,
					gint une_devise_compte_egale_x_devise_ope,
					gdouble taux_change,
					gdouble frais_change );



/***********************************/ 
/* fichier barre_outils.c */
/***********************************/ 



GtkWidget *creation_barre_outils ( void );
void change_aspect_liste ( GtkWidget *bouton,
			   gint demande );
GtkWidget *creation_barre_outils_echeancier ( void );
GtkWidget *creation_barre_outils_tiers ( void );
GtkWidget *creation_barre_outils_categ ( void );
GtkWidget *creation_barre_outils_imputation ( void );
void demande_expand_arbre ( GtkWidget *bouton,
			    gint *liste );
void mise_a_jour_boutons_caract_liste ( gint no_compte );



/************************/ 
/* fichier banque.c */
/************************/ 

void ajout_banque ( GtkWidget *bouton,
		    GtkWidget *clist );
void applique_modif_banque ( GtkWidget *liste );
void annuler_modif_banque ( GtkWidget *bouton,
			    GtkWidget *liste );
void supprime_banque ( GtkWidget *bouton,
		       GtkWidget *liste );
GtkWidget *creation_menu_banques ( void );
void affiche_detail_banque ( GtkWidget *bouton,
			     gpointer null );
GtkWidget *onglet_banques ( void );
void selection_ligne_banque ( GtkWidget *liste,
			      gint ligne,
			      gint colonne,
			      GdkEventButton *ev,
			      GtkWidget *frame );
void deselection_ligne_banque ( GtkWidget *liste,
				gint ligne,
				gint colonne,
				GdkEventButton *ev,
				GtkWidget *frame );
void modif_detail_banque ( GtkWidget *entree,
			   gpointer null );


/************************/ 
/* fichier ventilation.c */
/************************/ 

GtkWidget *creation_fenetre_ventilation ( void );
GtkWidget *creation_verification_ventilation ( void );
GtkWidget *creation_formulaire_ventilation ( void );
void clique_champ_formulaire_ventilation ( void );
void entree_ventilation_perd_focus ( GtkWidget *entree,
				     GdkEventFocus *ev,
				     gint *no_origine );
void ventiler_operation ( gdouble montant );
void changement_taille_liste_ventilation ( GtkWidget *clist,
					   GtkAllocation *allocation,
					   gpointer null );
gboolean traitement_clavier_liste_ventilation ( GtkCList *liste,
						GdkEventKey *evenement,
						gpointer null );
void selectionne_ligne_souris_ventilation ( GtkCList *liste,
					    GdkEventButton *evenement,
					    gpointer null );
void appui_touche_ventilation ( GtkWidget *entree,
				GdkEventKey *evenement,
				gint *no_origine );
void echap_formulaire_ventilation ( void );
void fin_edition_ventilation ( void );
void edition_operation_ventilation ( void );
void supprime_operation_ventilation ( void );
void affiche_liste_ventilation ( void );
void ajoute_ope_sur_liste_ventilation ( struct struct_ope_ventil *operation );
void mise_a_jour_couleurs_liste_ventilation ( void );
void selectionne_ligne_ventilation ( void );
void calcule_montant_ventilation ( void );
void mise_a_jour_labels_ventilation ( void );
void valider_ventilation ( void );
void annuler_ventilation ( void );
GSList *creation_liste_ope_de_ventil ( struct structure_operation *operation );
void validation_ope_de_ventilation ( struct structure_operation *operation );




/***********************************/ 
/* fichier qif.c */
/***********************************/ 

void importer_fichier_qif ( void );
/* void iqc_fichier(void); */
void selection_fichier_qif ( void );
void fichier_choisi_importation_qif ( GtkWidget *fenetre );
void traitement_donnees_brutes ( void );

void exporter_fichier_qif ( void );
void click_compte_export_qif ( GtkWidget *bouton,
			       GtkWidget *entree );



/***********************************/ 
/* fichier operations_classement.c */
/***********************************/ 

gint classement_liste_par_date ( GtkWidget *liste,
				 GtkCListRow *ligne_1,
				 GtkCListRow *ligne_2 );
gint classement_liste_par_no_ope ( GtkWidget *liste,
				   GtkCListRow *ligne_1,
				   GtkCListRow *ligne_2 );
gint classement_liste_par_tri_courant ( GtkWidget *liste,
					GtkCListRow *ligne_1,
					GtkCListRow *ligne_2 );
gint classement_sliste ( struct structure_operation *operation_1,
			 struct structure_operation *operation_2 );
gint classement_sliste_par_date ( struct structure_operation *operation_1,
				  struct structure_operation *operation_2 );
gint classement_sliste_par_tri_courant ( struct structure_operation *operation_1,
					 struct structure_operation *operation_2 );
gint classe_liste_alphabetique ( gchar *string_1,
				 gchar *string_2 );


/***********************************/ 
/* fichier categories_onglet.c */
/***********************************/ 

GtkWidget *onglet_categories ( void );
void remplit_arbre_categ ( void );
void ouverture_node_categ ( GtkWidget *arbre,
			    GtkCTreeNode *node,
			    gpointer null );
void selection_ligne_categ ( GtkCTree *arbre_categ,
			     GtkCTreeNode *noeud,
			     gint colonne,
			     gpointer null );
void verifie_double_click_categ ( GtkWidget *liste,
				  GdkEventButton *ev,
				  gpointer null );
void enleve_selection_ligne_categ ( void );
void modification_du_texte_categ ( void );
void clique_sur_modifier_categ ( void );
void clique_sur_annuler_categ ( void );
void supprimer_categ ( void );
void supprimer_sous_categ ( void );

void creation_liste_categories ( void );
void merge_liste_categories ( void );
void creation_liste_categ_combofix ( void );
void mise_a_jour_categ ( void );
struct struct_categ *ajoute_nouvelle_categorie ( gchar *categorie );
struct struct_sous_categ *ajoute_nouvelle_sous_categorie ( gchar *sous_categorie,
							   struct struct_categ *categorie );
gint recherche_categorie_par_nom ( struct struct_categ *categorie,
			     gchar *categ );
gint recherche_sous_categorie_par_nom ( struct struct_sous_categ *sous_categorie,
				  gchar *sous_categ );
gint recherche_categorie_par_no ( struct struct_categ *categorie,
				  gint *no_categorie );
gint recherche_sous_categorie_par_no ( struct struct_sous_categ *sous_categorie,
				       gint *no_sous_categorie );
void calcule_total_montant_categ ( void );
gchar *calcule_total_montant_categ_par_compte ( gint categ,
						gint sous_categ,
						gint no_compte );
void appui_sur_ajout_categorie ( void );
void appui_sur_ajout_sous_categorie ( void );
void exporter_categ ( void );
void importer_categ ( void );



/***********************************/ 
/* fichier patienter.c */
/***********************************/ 

void mise_en_route_attente ( gchar *message );
void update_attente ( gchar *message );
void annulation_attente ();
gint animation_patienter ();


/***********************************/ 
/* fichier type_operations.c */
/***********************************/ 

GtkWidget *onglet_types_operations ( void );
void selection_ligne_arbre_types ( GtkWidget *arbre,
				   GtkCTreeNode *node,
				   gint col,
				   GtkWidget *vbox );
void deselection_ligne_arbre_types ( GtkWidget *arbre,
				     GtkCTreeNode *node,
				     gint col,
				     GtkWidget *vbox );
void modification_entree_nom_type ( void );
void modification_type_affichage_entree ( void );
void modification_type_numerotation_auto (void);
void modification_entree_type_dernier_no ( void );
void modification_type_signe ( gint *no_menu );
void modification_type_par_defaut ( void );
gint recherche_type_ope_par_no ( struct struct_type_ope *type_ope,
				 gint *no_type );
void ajouter_type_operation ( void );
void supprimer_type_operation ( void );
void modif_tri_date_ou_type ( void );
void inclut_exclut_les_neutres ( void );
void remplit_liste_tri_par_type ( gint no_compte );
void selection_type_liste_tri ( void );
void deselection_type_liste_tri ( void );
void deplacement_type_tri_haut ( void );
void deplacement_type_tri_bas ( void );
void save_ordre_liste_type_tri ( void );
GtkWidget *creation_menu_types ( gint demande,
				 gint compte,
				 gint origine );
gint cherche_no_menu_type ( gint demande );
gint cherche_no_menu_type_associe ( gint demande,
				    gint origine );
gint cherche_no_menu_type_echeancier ( gint demande );
void changement_choix_type_formulaire ( struct struct_type_ope *type );
void changement_choix_type_echeancier ( struct struct_type_ope *type );

/***********************************/ 
/* fichier exercice.c */
/***********************************/ 

GtkWidget *onglet_exercices ( void );
void ajout_exercice ( GtkWidget *bouton,
		      GtkWidget *clist );
void supprime_exercice ( GtkWidget *bouton,
			 GtkWidget *liste );
void selection_ligne_exercice ( GtkWidget *liste,
			      gint ligne,
			      gint colonne,
			      GdkEventButton *ev,
				GtkWidget *frame );
void deselection_ligne_exercice ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev,
				  GtkWidget *frame );
void modif_detail_exercice ( void );
void applique_modif_exercice ( GtkWidget *liste );
void annuler_modif_exercice ( GtkWidget *bouton,
			      GtkWidget *liste );
gint recherche_exercice_par_nom ( struct struct_exercice *exercice,
				  gchar *nom );
gint recherche_exercice_par_no ( struct struct_exercice *exercice,
				 gint *no_exercice );
GtkWidget *creation_menu_exercices ( gint origine );
gint cherche_no_menu_exercice ( gint no_demande,
				GtkWidget *option_menu );
void affiche_exercice_par_date ( GtkWidget *entree_date,
				 GtkWidget *option_menu_exercice );
void association_automatique ( void );
gint recherche_exo_correspondant ( GDate *date );



/***********************************/ 
/* fichier imputation_budgetaire.c */
/***********************************/ 

GtkWidget *onglet_imputations ( void );
void remplit_arbre_imputation ( void );
void ouverture_node_imputation ( GtkWidget *arbre,
				 GtkCTreeNode *node,
				 gpointer null );
void selection_ligne_imputation ( GtkCTree *arbre,
				  GtkCTreeNode *noeud,
				  gint colonne,
				  gpointer null );
void verifie_double_click_imputation ( GtkWidget *liste,
				       GdkEventButton *ev,
				       gpointer null );
void enleve_selection_ligne_imputation ( void );
void modification_du_texte_imputation ( void );
void clique_sur_modifier_imputation ( void );
void clique_sur_annuler_imputation ( void );
void supprimer_imputation ( void );
void supprimer_sous_imputation ( void );
void creation_liste_imputations ( void );
void creation_liste_imputation_combofix ( void );
void mise_a_jour_imputation ( void );
struct struct_imputation *ajoute_nouvelle_imputation ( gchar *imputation );
struct struct_sous_imputation *ajoute_nouvelle_sous_imputation ( gchar *sous_imputation,
								 struct struct_imputation *imputation );
gint recherche_imputation_par_nom ( struct struct_imputation *imputation,
				    gchar *nom_imputation );
gint recherche_sous_imputation_par_nom ( struct struct_sous_imputation *sous_imputation,
					 gchar *nom_sous_imputation );
gint recherche_imputation_par_no ( struct struct_imputation *imputation,
				   gint *no_imputation );
gint recherche_sous_imputation_par_no ( struct struct_sous_imputation *sous_imputation,
					gint *no_sous_imputation );
void fusion_categories_imputation ( void );
void calcule_total_montant_imputation ( void );
gchar *calcule_total_montant_imputation_par_compte ( gint imputation,
						     gint sous_imputation,
						     gint no_compte );
void appui_sur_ajout_imputation ( void );
void appui_sur_ajout_sous_imputation ( void );
void exporter_ib ( void );
void importer_ib ( void );


/***********************************/ 
/* fichier affichage.c */
/***********************************/ 

GtkWidget *onglet_affichage ( void );
void selection_choix_ordre_comptes ( GtkWidget *box );
void deselection_choix_ordre_comptes ( GtkWidget *box );
void deplacement_haut ( void );
void deplacement_bas ( void );
void choix_fonte ( GtkWidget *bouton,
		   gchar *fonte,
		   gpointer null );
void choix_fonte_general ( GtkWidget *bouton,
			   gchar *fonte,
			   gpointer null );
void modification_logo_accueil ( void );
void remise_a_zero_logo ( GtkWidget *bouton,
			  GtkWidget *pixmap_entry );


/***********************************/ 
/* fichier etats_onglet.c */
/***********************************/ 

GtkWidget *creation_onglet_etats ( void );
GtkWidget *creation_liste_etats ( void );
GtkWidget *creation_barre_boutons_etats ( void );
void remplissage_liste_etats ( void );
void ajout_etat ( void );
void change_choix_nouvel_etat ( GtkWidget *menu_item,
				GtkWidget *label_description );
void efface_etat ( void );
void changement_etat ( GtkWidget *bouton,
		       struct struct_etat *etat );
gint recherche_etat_par_no ( struct struct_etat *etat,
			     gint *no_etat );
void exporter_etat ( void );
void importer_etat ( void );
void dupliquer_etat ( void );

/***********************************/ 
/* fichier etats_config.c */
/***********************************/ 

void personnalisation_etat (void);
void annule_modif_config ( void );
void selectionne_liste_exo_etat_courant ( void );
void selectionne_liste_comptes_etat_courant ( void );
void selectionne_liste_virements_etat_courant ( void );
void selectionne_liste_categ_etat_courant ( void );
void selectionne_devise_categ_etat_courant ( void );
void selectionne_liste_ib_etat_courant ( void );
void selectionne_devise_ib_etat_courant ( void );
void selectionne_liste_tiers_etat_courant ( void );
void selectionne_devise_tiers_etat_courant ( void );
void recuperation_info_perso_etat ( void );
void sens_desensitive_pointeur ( GtkWidget *boutton,
				 GtkWidget *widget );
void stylise_tab_label_etat ( gint *no_page );
GtkWidget *onglet_etat_dates ( void );
void click_liste_etat ( GtkCList *liste,
			GdkEventButton *evenement,
			gint origine );
void clique_sur_entree_date_etat ( GtkWidget *entree,
				   GdkEventButton *ev );
void date_selectionnee_etat ( GtkCalendar *calendrier,
			      GtkWidget *popup );
void change_separation_result_periode ( void );
void modif_type_separation_dates ( gint *origine );
void remplissage_liste_exo_etats ( void );
GtkWidget *onglet_etat_comptes ( void );
void remplissage_liste_comptes_etats ( void );
void selectionne_partie_liste_compte_etat ( gint *type_compte );
GtkWidget *onglet_etat_virements ( void );
void remplissage_liste_comptes_virements ( void );
void selectionne_partie_liste_compte_vir_etat ( gint *type_compte );
GtkWidget *onglet_etat_categories ( void );
void click_type_categ_etat ( gint type );
void remplissage_liste_categ_etats ( void );
GtkWidget *onglet_etat_ib ( void );
void click_type_ib_etat ( gint type );
void remplissage_liste_ib_etats ( void );
GtkWidget *onglet_etat_tiers ( void );
void remplissage_liste_tiers_etats ( void );
GtkWidget *onglet_etat_texte ( void );
void remplit_liste_comparaisons_textes_etat ( void );
void ajoute_ligne_liste_comparaisons_textes_etat ( struct struct_comparaison_textes_etat *ancien_comp_textes );
GtkWidget *cree_ligne_comparaison_texte ( struct struct_comparaison_textes_etat *comp_textes );
void retire_ligne_liste_comparaisons_textes_etat ( struct struct_comparaison_textes_etat *ancien_comp_textes );
GtkWidget *cree_bouton_champ ( struct struct_comparaison_textes_etat *comp_textes );
void sensitive_hbox_fonction_bouton_txt ( struct struct_comparaison_textes_etat *comp_textes );
GtkWidget *cree_bouton_operateur_txt ( struct struct_comparaison_textes_etat *comp_textes );
GtkWidget *onglet_etat_montant ( void );
void remplit_liste_comparaisons_montants_etat ( void );
void ajoute_ligne_liste_comparaisons_montants_etat ( struct struct_comparaison_montants_etat *ancien_comp_montants );
GtkWidget *cree_ligne_comparaison_montant ( struct struct_comparaison_montants_etat *comp_montants );
GtkWidget *cree_bouton_lien_lignes_comparaison ( void );
GtkWidget *cree_bouton_comparateur_montant ( struct struct_comparaison_montants_etat *comp_montants );
void change_comparaison_montant ( GtkWidget *menu_item,
				  struct struct_comparaison_montants_etat *comp_montants );
GtkWidget *cree_bouton_comparateur_texte ( struct struct_comparaison_textes_etat *comp_textes );
void change_comparaison_texte ( GtkWidget *menu_item,
				struct struct_comparaison_textes_etat *comp_textes );
GtkWidget *cree_bouton_lien ( GtkWidget *hbox );
void sensitive_widget ( GtkWidget *widget );
void desensitive_widget ( GtkWidget *widget );
void retire_ligne_liste_comparaisons_montants_etat ( struct struct_comparaison_montants_etat *ancien_comp_montants );
GtkWidget *onglet_etat_divers ( void );
GtkWidget *page_organisation_donnees ( void );
void click_haut_classement_etat ( void );
void click_bas_classement_etat ( void );
GtkWidget *page_affichage_donnees ( void );
GtkWidget *onglet_affichage_etat_generalites ( void );
GtkWidget *onglet_affichage_etat_operations ( void );
GtkWidget *onglet_affichage_etat_devises ( void );
GtkWidget *onglet_affichage_etat_divers ( void );
GtkWidget *onglet_etat_mode_paiement ( void );
void remplissage_liste_modes_paiement_etats ( void );
gint recherche_nom_dans_liste ( gchar *nom_liste,
				gchar *nom_test );
void selectionne_liste_modes_paiement_etat_courant ( void );

/***********************************/ 
/* fichier etats_calculs.c */
/***********************************/ 

void affichage_etat ( struct struct_etat *etat, 
		      struct struct_etat_affichage *affichage );
GSList *recupere_opes_etat ( struct struct_etat *etat );
gchar *recupere_texte_test_etat ( struct structure_operation *operation,
				  gint champ );
gint verifie_texte_test_etat ( struct struct_comparaison_textes_etat *comp_textes,
			       gchar *texte_ope );
gint verifie_chq_test_etat ( struct struct_comparaison_textes_etat *comp_textes,
			     gchar *no_chq );
gint compare_cheques_etat ( gint chq_ope,
			   gint chq_test,
			   gint comparateur );
gint compare_montants_etat ( gdouble montant_ope,
			     gdouble montant_test,
			     gint comparateur );
void rafraichissement_etat ( struct struct_etat *etat );
gint classement_liste_opes_etat ( struct structure_operation *operation_1,
				  struct structure_operation *operation_2 );
gint classement_ope_perso_etat ( struct structure_operation *operation_1,
				 struct structure_operation *operation_2 );
void etape_finale_affichage_etat ( GSList *ope_selectionnees, 
				   struct struct_etat_affichage *affichage );
void denote_struct_sous_jaccentes ( gint origine );


/***********************************/ 
/* fichier etats_support.c */
/***********************************/ 

gchar * etats_titre ();


/***********************************/ 
/* fichier etats_gtktable.c */
/***********************************/ 

gint gtktable_initialise ( );
gint gtktable_affiche_titre ( gint ligne );
gint gtktable_affiche_separateur ( gint ligne );
gint gtktable_affiche_total_categories ( gint ligne );
gint gtktable_affiche_total_sous_categ ( gint ligne );
gint gtktable_affiche_total_ib ( gint ligne );
gint gtktable_affiche_total_sous_ib ( gint ligne );
gint gtktable_affiche_total_compte ( gint ligne );
gint gtktable_affiche_total_tiers ( gint ligne );
gint gtktable_affiche_total_periode ( struct structure_operation *operation,
				      gint ligne,
				      gint force );
gint gtktable_affichage_ligne_ope ( struct structure_operation *operation,
				    gint ligne );
gint gtktable_affiche_total_partiel ( gdouble total_partie,
				      gint ligne,
				      gint type );
gint gtktable_affiche_total_general ( gdouble total_general,
				      gint ligne );
gint gtktable_affiche_categ_etat ( struct structure_operation *operation,
				   gchar *decalage_categ,
				   gint ligne );
gint gtktable_affiche_sous_categ_etat ( struct structure_operation *operation,
					gchar *decalage_sous_categ,
					gint ligne );
gint gtktable_affiche_ib_etat ( struct structure_operation *operation,
				gchar *decalage_ib,
				gint ligne );
gint gtktable_affiche_sous_ib_etat ( struct structure_operation *operation,
				     gchar *decalage_sous_ib,
				     gint ligne );
gint gtktable_affiche_compte_etat ( struct structure_operation *operation,
				    gchar *decalage_compte,
				    gint ligne );
gint gtktable_affiche_tiers_etat ( struct structure_operation *operation,
				   gchar *decalage_tiers,
				   gint ligne );
gint gtktable_affiche_titre_revenus_etat ( gint ligne );
gint gtktable_affiche_titre_depenses_etat ( gint ligne );
gint gtktable_affiche_totaux_sous_jaccent ( gint origine,
					    gint ligne );
gint gtktable_affiche_titres_colonnes ( gint ligne );
gint gtktable_finish ( );
void gtktable_click_sur_ope_etat ( struct structure_operation *operation );


/************************/ 
/* fichier affichage_liste.c */
/************************/ 

GtkWidget *onglet_affichage_liste ( void );
GtkWidget *cree_menu_quatres_lignes ( void );
void allocation_clist_affichage_liste ( GtkWidget *clist,
					GtkAllocation *allocation );
gboolean pression_bouton_classement_liste ( GtkWidget *clist,
					    GdkEventButton *ev );
gboolean lache_bouton_classement_liste ( GtkWidget *clist,
					 GdkEventButton *ev );
void remplissage_tab_affichage_ope ( GtkWidget *clist );
void toggled_bouton_affichage_liste ( GtkWidget *bouton,
				      gint *no_bouton );
void changement_taille_liste_affichage ( GtkWidget *clist,
					 GtkAllocation *allocation );
void recuperation_noms_colonnes_et_tips ( void );
void raz_affichage_ope ( void );


/************************/ 
/* fichier etat_io.c */
/************************/ 

gboolean charge_etat ( gchar *nom_etat );
gboolean charge_etat_version_0_4_0 ( xmlDocPtr doc );
gboolean enregistre_etat ( gchar *nom_etat );
gint recupere_devise_par_nom_etat ( gchar *nom_devise );
gint recupere_exo_par_nom_etat ( gchar *nom_exo );
gint recupere_compte_par_nom_etat ( gchar *nom_compte );
gint recupere_categ_par_nom_etat ( gchar *nom_categ );
gint recupere_ib_par_nom_etat ( gchar *nom );
gint recupere_tiers_par_nom_etat ( gchar *nom );
