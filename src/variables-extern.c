/* Ce fichier contient toutes les variables globales définies */
/* ainsi que toutes les définitions diverses */


/* catégories présentes lors de la 1ère installation */

extern gchar *categories_de_base [];


extern GnomeUIInfo menu_fichier [15];
extern GnomeUIInfo menu_comptes [7]; 
extern GnomeUIInfo help_menu [3];
extern GnomeUIInfo menu_principal [5]; 
extern GnomeUIInfo menu_parametres [10];
extern GnomeUIInfo menu_importer [2];
extern GnomeUIInfo menu_exporter [2];
extern GnomeUIInfo menu_derniers_fichiers [1]; 
extern GnomeUIInfo menu_cloture [1];



/*********************/ 
/* fichier main.c */
/*********************/ 

/*  Variables globales */

extern GtkWidget *window;
extern GtkWidget *statusbar;



/***********************************/ 
/* fichier liste_comptes.c */
/***********************************/ 

extern GtkWidget *label_compte_courant;
extern int compte_courant;                        /*  n° de compte en cours de visualisation */
extern GtkWidget *vbox_liste_comptes;            /* adresse de la vbox contenant les icones de comptes */
extern GtkWidget *label_releve;                    /* adr du label du dernier relevé */


/***********************************/ 
/* fichier liste_operations.c */
/***********************************/ 

extern GtkWidget *notebook_listes_operations;          /*  adr du notebook qui contient les opés de chaque compte */
extern GtkWidget *bouton_affiche_cache_formulaire;
extern GtkWidget *fleche_haut;                        /* adr de l'icone de la flèche vers le haut  */
extern GtkWidget *fleche_bas;                        /* adr de l'icone de la flèche vers le bas  */
extern GtkWidget *frame_droite_bas;                  /* adr de la frame contenant le formulaire */
extern GtkWidget *barre_outils;                           /* adr de la barre d'outils */



/***********************************/ 
/* fichier erreur.c */
/***********************************/ 

extern GtkWidget *win_erreur;



/***********************************/ 
/* fichier fichier_configuration.c */
/***********************************/ 

extern gint largeur_window;
extern gint hauteur_window;
extern gchar *buffer_dernier_fichier;
extern gchar *nom_fichier_backup;
extern gchar *dernier_chemin_de_travail;
extern gint nb_derniers_fichiers_ouverts;          /* contient le nb de derniers fichiers ouverts */
extern gint nb_max_derniers_fichiers_ouverts;        /* contient le nb max que peut contenir nb_derniers_fichiers_ouverts ( réglé dans les paramètres ) */
extern gchar **tab_noms_derniers_fichiers_ouverts;
extern gint compression_fichier;
extern gint compression_backup;



/********************/ 
/* fichier disque.c */
/********************/ 



/*  Variables globales */

extern gchar *nom_fichier_comptes, *ancien_nom_fichier_comptes;
extern gchar *buffer_nom_fichier_comptes;
extern gchar buffer_en_tete[50];                      /* utilisé comme buffer lors de la lecture du fichier */
/* extern struct stat buffer_stat; */
extern gint ancien_mask;
extern FILE *pointeur_fichier_comptes;
extern FILE *pointeur_fichier_comptes_tmp;
extern int nb_comptes;
extern gpointer ** p_tab_nom_de_compte;  /* pointe vers un tableau de pointeurs vers les comptes en mémoire*/
extern gpointer ** p_tab_nom_de_compte_variable;  /* idem, mais utilisé pour se déplacer */
extern gpointer ** p_tab_nom_de_compte_courant;  /* pointe sur le tableau de pointeurs sur le compte courant*/
extern GtkWidget * solde_label;   /*  pointeur vers le label qui contient le solde sous la liste des opé */
extern GtkWidget * solde_label_pointe;   /*  pointeur vers le label qui contient le solde pointe sous la liste des opé */
extern gchar version[10];
extern int nb_lignes_ope;        /* nbre de lignes par opération ( contient 4, 3 ou 1 ) */
extern GSList *ordre_comptes;
extern gchar *titre_fichier;
extern gchar *adresse_commune;

extern GSList *liste_struct_exercices;
extern gint no_derniere_exercice;
extern gint nb_exercices;

extern gint no_derniere_operation;     /* contient le dernier numéro d'opération de tous les comptes réunis */


/*********************/ 
/* fichier fenetre_principale.c */
/*********************/ 


extern GtkWidget *notebook_general;       /* adr du notebook de base */
extern GtkWidget *page_accueil;           /* adr de l'onglet accueil */
extern GtkWidget *page_operations;        /* adr de l'onglet opérations ( ancien gest_main ) */
extern GtkWidget *page_echeancier;        /* adr de l'onglet échéances */
extern GtkWidget *page_tiers;             /* adr de l'onglet tiers */
extern GtkWidget *page_categories;             /* adr de l'onglet categories */
extern GtkWidget *page_imputations;             /* adr de l'onglet ib */
extern GtkWidget *page_etats;             /* adr de l'onglet etats */
extern GtkWidget *notebook_comptes_equilibrage;
extern GtkWidget *notebook_formulaire;
extern gint modif_tiers;
extern gint modif_categ;
extern gint modif_imputation;



/*********************/ 
/* fichier fenetre.c */
/*********************/ 



/* variables */

extern GtkStyle *style_derriere_selection;
extern gint ancienne_ligne_selection;
extern GtkStyle *style_couleur [2];
extern GtkStyle *style_rouge_couleur [2];
extern GtkStyle *style_gris;                     /* utilisé pour le grisé des échéances */
extern gchar *fonte_liste;                                     /*  adr de la fonte en cours ou NULL si non choisie */
extern gchar *fonte_general;                                     /*  adr de la fonte en cours ou NULL si non choisie */
extern GtkWidget *hbox_comptes_equilibrage;
extern GtkWidget *formulaire;
extern GtkWidget *pointeur_fenetre_nouveau_compte;

extern gint *no_operation_formulaire;

extern gint operations_affichees;       /* nb d'opé affichées ( variable en fonction qu'on affiche les R ou pas  */


/************************/ 
/* fichier operations_formulaire.c */
/************************/ 

extern GtkWidget *widget_formulaire_operations[18];
extern GtkWidget *separateur_formulaire_operations;
extern GtkWidget *hbox_valider_annuler_ope;
extern gchar date_courante[11];             /* contient soit la date du jour, soit la derniere date entree */
extern GtkStyle *style_entree_formulaire[2];


/************************/ 
/* fichier new_compte.c */
/************************/ 




/************************/ 
/* fichier operations_fichier.c */
/************************/ 




/************************/ 
/* fichier completion.c */
/************************/ 



/************************/ 
/* fichier equilibrage.c */
/************************/ 

extern GtkWidget *entree_no_rapprochement;
extern GtkWidget *label_ancienne_date_equilibrage;
extern GtkWidget *entree_ancien_solde_equilibrage;
extern GtkWidget *entree_nouvelle_date_equilibrage;
extern GtkWidget *entree_nouveau_montant_equilibrage;
extern gdouble solde_initial;
extern gdouble solde_final;
extern gdouble operations_pointees;
extern GtkWidget *label_equilibrage_compte;
extern GtkWidget *label_equilibrage_initial;
extern GtkWidget *label_equilibrage_final;
extern GtkWidget *label_equilibrage_pointe;
extern GtkWidget *label_equilibrage_ecart;
extern GtkWidget *bouton_ok_equilibrage;  
extern GSList *liste_no_rapprochements;            /* contient la liste des structures de no_rapprochement */
extern gint ancien_nb_lignes_ope;              /* contient l'ancien nb_lignes_ope */


/************************/ 
/* fichier parametres.c */
/************************/ 


extern gchar *applet_dir;
extern gchar *applet_bin_dir;

extern GtkWidget *fenetre_preferences;
extern GtkWidget *bouton_entree_enregistre, *bouton_entree_enregistre_pas;
extern GtkWidget *bouton_solde_mini, *bouton_pas_solde_mini;
extern GtkWidget *bouton_affiche_permission, *bouton_affiche_pas_permission;
extern GtkWidget *entree_titre_fichier;
extern GtkWidget *entree_adresse_commune;


extern GtkWidget *bouton_avec_demarrage, *bouton_sans_demarrage;
extern GtkWidget *bouton_save_auto, *bouton_save_non_auto;
extern GtkWidget *bouton_force_enregistrement, *bouton_force_pas_enregistrement;
extern GtkWidget *bouton_demande_backup;
extern GtkWidget *entree_chemin_backup;
extern GtkWidget *spin_button_derniers_fichiers_ouverts;
extern GtkWidget *spin_button_compression_fichier;
extern GtkWidget *spin_button_compression_backup;

extern GtkWidget *entree_jours;
extern GtkWidget *bouton_2click_saisie, *bouton_2click_pas_saisie;

extern GSList *fichier_a_verifier;
extern GtkWidget *bouton_affichage_applet;
extern GtkWidget *frame_demarrage;
extern GtkWidget *liste_comptes_verifies;
extern gint fichier_verifier_selectionne, nb_fichier_verifier;
extern GtkWidget *bouton_enlever;
extern GSList *liste_suppression_fichier_a_verifier, *fichier_a_verifier_tmp;
extern gint pid_applet;


extern GSList *liste_struct_exercices_tmp;                 /*  utilisée à la place de la liste des exercices pour les paramètres */
extern gint no_derniere_exercice_tmp;                            /*  utilisé à la place de no_derniere_exercice pour les paramètres */
extern gint nb_exercices_tmp;                            /*  utilisé à la place de nb_exercices pour les paramètres */
extern GtkWidget *clist_exercices_parametres;
extern GtkWidget *bouton_supprimer_exercice;
extern GtkWidget *nom_exercice;
extern GtkWidget *debut_exercice;
extern GtkWidget *fin_exercice;
extern GtkWidget *affichage_exercice;
extern GtkWidget *hbox_boutons_modif_exercice;
extern gint ligne_selection_exercice;
extern GtkWidget *bouton_affichage_auto_exercice;



/***********************************/ 
/* fichier echeancier_liste.c */
/***********************************/ 



extern GtkWidget *frame_formulaire_echeancier;
extern GtkWidget *formulaire_echeancier;
extern GtkWidget *liste_echeances;
extern GtkWidget *entree_personnalisation_affichage_echeances;
extern GtkWidget *bouton_personnalisation_affichage_echeances;
extern GtkWidget *bouton_saisir_echeancier;
extern GtkWidget *calendrier_echeances;

extern struct operation_echeance *echeance_selectionnnee;

extern GSList *gsliste_echeances;                 /* contient la liste des struct échéances */
extern gint nb_echeances;
extern gint no_derniere_echeance;

extern gint decalage_echeance;

extern gint ancienne_largeur_echeances;

extern GSList *echeances_a_saisir;
extern GSList *echeances_saisies;
extern gint affichage_echeances; /* contient 0(mois), 1 (2 mois), 2(année), 3(toutes), 4(perso) */
extern gint affichage_echeances_perso_nb_libre;     /* contient le contenu de l'entrée */
extern gint affichage_echeances_perso_j_m_a;        /* contient 0 (jours), 1 (mois), 2 (années) */




/************************/ 
/* fichier echeances_formulaire.c */
/************************/ 

extern GtkWidget *widget_formulaire_echeancier[19];
extern GtkWidget *label_saisie_modif;
extern GSList *liste_categories_echeances_combofix;        /*  liste des noms des categ et sous categ pour le combofix */
extern GtkWidget *separateur_formulaire_echeancier;
extern GtkWidget *hbox_valider_annuler_echeance;



/***********************************/ 
/* fichier accueil.c */
/***********************************/ 

extern gint id_temps;
extern gchar *chemin_logo;
extern GtkWidget *logo_accueil;
extern GtkWidget *label_temps;
extern GtkWidget *label_titre_fichier;
extern GtkWidget *frame_etat_comptes_accueil;
extern GtkWidget *frame_etat_fin_compte_passif;
extern GtkWidget *frame_etat_echeances_manuelles_accueil;
extern GtkWidget *frame_etat_echeances_auto_accueil;
extern GtkWidget *frame_etat_echeances_finies;
extern GtkWidget *frame_etat_soldes_minimaux_autorises;
extern GtkWidget *frame_etat_soldes_minimaux_voulus;
extern GtkStyle *style_label;
extern GtkWidget *separateur_passif_manu;
extern GtkWidget *separateur_manu_auto;
extern GtkWidget *separateur_auto_mini;
extern GtkWidget *separateur_ech_finies_soldes_mini;
extern GtkWidget *separateur_des_soldes_mini;



/***********************************/ 
/* fichier tiers_onglet.c */
/***********************************/ 

extern GtkWidget *arbre_tiers;
extern GtkWidget *entree_nom_tiers;
extern GtkWidget *text_box;
extern GtkWidget *bouton_modif_tiers_modifier;
extern GtkWidget *bouton_modif_tiers_annuler;
extern GtkWidget *bouton_supprimer_tiers;
extern GdkPixmap *pixmap_ouvre;
extern GdkBitmap *masque_ouvre;
extern GdkPixmap *pixmap_ferme;
extern GdkBitmap *masque_ferme;
extern GtkWidget *bouton_ajouter_tiers;

extern gint nb_enregistrements_tiers;
extern gint no_dernier_tiers;
extern GList tiers_combo;                         /* liste des struct_chaine_completion pour la complétion  */
extern GSList *liste_struct_tiers;             /* contient la liste des struct liste_tiers de tous les tiers */
extern GSList *liste_tiers_combofix;            /* liste des tiers pour le combofix */
extern GDate **date_dernier_tiers;            /* contient la date du dernier tiers lors de l'affichage de chaque tiers */
extern gint no_devise_totaux_tiers;
extern gint *nb_ecritures_par_tiers;           /* variable tampon */
extern gint nb_ecritures_par_comptes;


/***********************************/ 
/* fichier categories_onglet.c */
/***********************************/

extern GtkWidget *arbre_categ;
extern GtkWidget *entree_nom_categ;
extern GtkWidget *bouton_categ_debit;
extern GtkWidget *bouton_categ_credit;
extern GtkWidget *bouton_modif_categ_modifier;
extern GtkWidget *bouton_modif_categ_annuler;
extern GtkWidget *bouton_supprimer_categ;
extern GtkWidget *bouton_ajouter_categorie;
extern GtkWidget *bouton_ajouter_sous_categorie;

extern GSList *liste_struct_categories;    /* liste des structures de catég */
extern GSList *liste_categories_combofix;        /*  liste des noms des categ et sous categ pour le combofix */
extern gint nb_enregistrements_categories;        /* nombre de catégories */
extern gint no_derniere_categorie;
extern gfloat *tab_montant_categ;            /* buffer */
extern gfloat **tab_montant_sous_categ;            /* buffer */
extern gint *nb_ecritures_par_categ;           /* buffer */
extern gint **nb_ecritures_par_sous_categ;           /* buffer */



/***********************************/ 
/* fichier devises.c */
/***********************************/ 

extern GSList *liste_struct_devises;
extern gint nb_devises;
extern gint no_derniere_devise;
extern struct struct_devise *devise_nulle;
extern gdouble taux_de_change[2];
extern GtkWidget *option_menu_devise_1;
extern GtkWidget *option_menu_devise_2;



extern GSList *liste_struct_devises_tmp;            /*  utilisée à la place de la liste des devises pour les paramètres */
extern gint no_derniere_devise_tmp;                    /*  utilisé à la place de no_derniere_devise pour les paramètres */
extern gint nb_devises_tmp;                             /*  utilisé à la place de nb_devises pour les paramètres */
extern GtkWidget *clist_devises_parametres;
extern GtkWidget *bouton_supprimer_devise;
extern GtkWidget *entree_nom_devise_parametres;
extern GtkWidget *entree_code_devise_parametres;
extern GtkWidget *check_button_euro;                    /*bouton passera à l'euro */
extern GtkWidget *hbox_devise_associee;                  /* ligne contenant l'option menu de la devise associée */
extern GtkWidget *label_date_dernier_change;       /* affiche soit aucun change entré, soit la date du dernier change */
extern GtkWidget *hbox_ligne_change;                  /* hbox contenant le change entre les devises */
extern GtkWidget *option_menu_devises;                  /* contient la devise avec laquelle on fait le change */
extern GtkWidget *devise_1;                                      /* option menu qui contient la devise 1 */
extern GtkWidget *devise_2;                                     /* option menu qui contient la devise 2 */
extern GtkWidget *entree_conversion;                       /* entrée qui contient le change */
extern gint ligne_selection_devise;               /* contient la ligne sélectionnée dans la liste des devises */



/***********************************/ 
/* fichier gestion_comptes.c */
/***********************************/ 

extern GtkWidget *detail_nom_compte;
extern GtkWidget *detail_type_compte;
extern GtkWidget *detail_titulaire_compte;
extern GtkWidget *detail_bouton_adresse_commune;
extern GtkWidget *detail_adresse_titulaire;
extern GtkWidget *detail_option_menu_banque;
extern GtkWidget *detail_no_compte;
extern GtkWidget *label_code_banque;
extern GtkWidget *detail_guichet;
extern GtkWidget *detail_cle_compte;
extern GtkWidget *detail_devise_compte;
extern GtkWidget *bouton_passage_euro;
extern GtkWidget *hbox_bouton_passage_euro;
extern GtkWidget *detail_compte_cloture;
extern GtkWidget *detail_solde_init;
extern GtkWidget *detail_solde_mini_autorise;
extern GtkWidget *detail_solde_mini_voulu;
extern GtkWidget *detail_commentaire;
extern GtkWidget *hbox_boutons_modif;

extern GSList *liste_struct_banques;
extern gint nb_banques;
extern gint no_derniere_banque;


/***********************************/ 
/* fichier ventilation.c */
/***********************************/ 


extern GtkWidget *liste_operations_ventilees;
extern gint ancienne_largeur_ventilation;       /* magouille utilisée pour bloquer un signal size-allocate qui s'emballe */
extern GtkWidget *widget_formulaire_ventilation[6];
extern GtkWidget *separateur_formulaire_ventilations;
extern GtkWidget *hbox_valider_annuler_ventil;
extern GtkWidget *label_somme_ventilee;                       /*  label correspondant */
extern GtkWidget *label_non_affecte;                       /*  label correspondant */
extern GtkWidget *label_montant_operation_ventilee;                       /*  label correspondant */
extern struct structure_operation *ligne_selectionnee_ventilation;              /* struct ope ligne sélectionnée */
extern gdouble montant_operation_ventilee;
extern gdouble somme_ventilee;
extern GtkWidget *bouton_credit;                          /* adr du radio_button crédit */
extern GtkWidget *bouton_debit;                          /* adr du radio_button crédit */
extern gint enregistre_ope_au_retour;            /* à 1 si au click du bouton valider on enregistre l'opé */


/***********************************/ 
/* fichier qif.c */
/***********************************/ 

extern GSList *liste_comptes_qif;
extern GSList *liste_entrees_exportation;


/***********************************/ 
/* fichier barre_outils */
/***********************************/ 

extern GtkTooltips *tooltips;
extern GtkWidget *bouton_affiche_cache_formulaire_echeancier;
extern GtkWidget *fleche_bas_echeancier;
extern GtkWidget *fleche_haut_echeancier;
extern GtkWidget *bouton_ope_completes;
extern GtkWidget *bouton_ope_semi_completes;
extern GtkWidget *bouton_ope_simples;
extern GtkWidget *bouton_affiche_r;
extern GtkWidget *bouton_enleve_r;
extern GtkWidget *bouton_affiche_liste;
extern GtkWidget *bouton_affiche_detail_compte;
extern GtkWidget *label_proprietes_operations_compte;


/***********************************/ 
/* fichier patienter.c */
/***********************************/ 

extern GtkWidget *fenetre_patience;
extern gint image_patience_en_cours;
extern GtkWidget *label_patience;
extern gint timeout_patience;
extern GtkWidget *image[15];
extern gint patience_en_cours;


/***********************************/ 
/* fichier type_operations.c */
/***********************************/ 

extern GtkWidget *arbre_types_operations;
extern GSList **liste_tmp_types;
extern gint *type_defaut_debit;
extern gint *type_defaut_credit;
extern GtkWidget *bouton_ajouter_type;
extern GtkWidget *bouton_retirer_type;
extern GtkWidget *entree_type_nom;
extern GtkWidget *bouton_type_apparaitre_entree;
extern GtkWidget *bouton_type_numerotation_automatique;
extern GtkWidget *entree_type_dernier_no;
extern GtkWidget *bouton_signe_type;
extern GtkWidget *bouton_type_choix_defaut;
extern GtkWidget *bouton_type_choix_affichage_formulaire;

extern GtkWidget *bouton_type_tri_date;
extern GtkWidget *bouton_type_tri_type;
extern GtkWidget *bouton_type_neutre_inclut;
extern GtkWidget *type_liste_tri;
extern GtkWidget *vbox_fleches_tri;
extern gint *tri_tmp;
extern GSList **liste_tri_tmp;
extern gint *neutres_inclus_tmp;


/***********************************/ 
/* fichier banque.c */
/***********************************/ 


extern GSList *liste_struct_banques_tmp;             /*  utilisée à la place de la liste des banques pour les paramètres */
extern gint no_derniere_banque_tmp;                     /*  utilisé à la place de no_derniere_banque pour les paramètres */
extern gint nb_banques_tmp;                            /*  utilisé à la place de nb_banques pour les paramètres */
extern GtkWidget *clist_banques_parametres;
extern GtkWidget *bouton_supprimer_banque;
extern GtkWidget *nom_banque;
extern GtkWidget *code_banque;
extern GtkWidget *tel_banque;
extern GtkWidget *adr_banque;
extern GtkWidget *email_banque;
extern GtkWidget *web_banque;
extern GtkWidget *nom_correspondant;
extern GtkWidget *tel_correspondant;
extern GtkWidget *email_correspondant;
extern GtkWidget *fax_correspondant;
extern GtkWidget *remarque_banque;
extern GtkWidget *hbox_boutons_modif_banque;
extern gint ligne_selection_banque;



/***********************************/ 
/* fichier imputation_budgetaire.c */
/***********************************/ 


extern GtkWidget *arbre_imputation;
extern GtkWidget *entree_nom_imputation;
extern GtkWidget *bouton_imputation_debit;
extern GtkWidget *bouton_imputation_credit;
extern GtkWidget *bouton_modif_imputation_modifier;
extern GtkWidget *bouton_modif_imputation_annuler;
extern GtkWidget *bouton_supprimer_imputation;
extern GtkWidget *bouton_ajouter_imputation;
extern GtkWidget *bouton_ajouter_sous_imputation;

extern GSList *liste_struct_imputation;    /* liste des structures de catég */
extern GSList *liste_imputations_combofix;        /*  liste des noms des imputation et sous imputation pour le combofix */
extern gint nb_enregistrements_imputations;        /* nombre de catégories */
extern gint no_derniere_imputation;
extern gfloat *tab_montant_imputation;             /* buffer */
extern gfloat **tab_montant_sous_imputation;            /* buffer */
extern gint *nb_ecritures_par_imputation;           /* buffer */
extern gint **nb_ecritures_par_sous_imputation;           /* buffer */



/***********************************/ 
/* fichier affichage.c */
/***********************************/ 


extern GtkWidget *liste_choix_ordre_comptes;
extern GtkWidget *bouton_afficher_no_operation;
extern GtkWidget *bouton_afficher_date_bancaire; /* GDC */
extern GtkWidget *bouton_utiliser_exercices;
extern GtkWidget *bouton_utiliser_imputation_budgetaire;
extern GtkWidget *bouton_utiliser_piece_comptable;
extern GtkWidget *bouton_utiliser_info_banque_guichet;
extern GtkWidget *bouton_afficher_boutons_valider_annuler;
extern GtkWidget *bouton_choix_devise_totaux_tiers;
extern GtkWidget *bouton_afficher_nb_ecritures;
extern GtkWidget *bouton_classer_liste_par_date;
extern GtkWidget *bouton_classer_liste_par_date_bancaire;


/***********************************/ 
/* fichier etats.c */
/***********************************/ 

extern GSList *liste_struct_etats;
extern gint no_dernier_etat;
extern GtkWidget *label_etat_courant;        /* label en haut de la liste des états */
extern GtkWidget *vbox_liste_etats;          /* vbox contenant la liste des états */
extern GtkWidget *bouton_etat_courant;          /* adr du bouton de l'état en cours, pour le refermer qd change */
extern gint no_etat_courant;
extern GtkWidget *bouton_effacer_etat;
extern GtkWidget *bouton_personnaliser_etat;

extern GtkWidget *entree_nom_etat;
extern GtkWidget *liste_type_classement_etat;

extern GtkWidget *radio_button_utilise_exo;
extern GtkWidget *radio_button_utilise_dates;
extern GtkWidget *liste_exo_etat;
extern GtkWidget *vbox_generale_exo_etat;
extern GtkWidget *bouton_detaille_exo_etat;
extern GtkWidget *liste_plages_dates_etat;
extern GtkWidget *entree_date_init_etat;
extern GtkWidget *entree_date_finale_etat;
extern GtkWidget *vbox_utilisation_exo;
extern GtkWidget *vbox_utilisation_date;

extern GtkWidget *liste_comptes_etat;
extern GtkWidget *bouton_detaille_comptes_etat;
extern GtkWidget *vbox_generale_comptes_etat;

extern GtkWidget *bouton_utilise_categ_etat;
extern GtkWidget *vbox_generale_categ_etat;
extern GtkWidget *bouton_detaille_categ_etat;
extern GtkWidget *hbox_detaille_categ_etat;
extern GtkWidget *bouton_inclusion_virements_actifs_etat;
extern GtkWidget *bouton_inclusion_virements_hors_etat;
extern GtkWidget *bouton_non_inclusion_virements;
extern GtkWidget *liste_categ_etat;

extern GtkWidget *bouton_utilise_ib_etat;
extern GtkWidget *vbox_generale_ib_etat;
extern GtkWidget *bouton_detaille_ib_etat;
extern GtkWidget *hbox_detaille_ib_etat;
extern GtkWidget *liste_ib_etat;

extern GtkWidget *bouton_utilise_tiers_etat;
extern GtkWidget *vbox_generale_tiers_etat;
extern GtkWidget *bouton_detaille_tiers_etat;
extern GtkWidget *hbox_detaille_tiers_etat;
extern GtkWidget *liste_tiers_etat;

extern GtkWidget *entree_texte_etat;
extern GtkWidget *entree_montant_etat;
