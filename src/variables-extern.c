/* Ce fichier contient toutes les variables globales définies */
/* ainsi que toutes les définitions diverses */


/* catégories présentes lors de la 1ère installation */

extern gchar *categories_de_base [];


enum menu_fichier_widget {
  NOUVEAU, OUVRIR, SEP1, 
  SAUVER, SAUVER_SOUS, SEP2,
  DERNIERS_FICHIERS, SEP3,
  IMPORTER, EXPORTER, SEP4,
  FERMER, QUITTER, MAX_MENU_FICHIER
};
extern GnomeUIInfo * menu_fichier;
extern GnomeUIInfo * menu_comptes; 
extern GnomeUIInfo * help_menu;
extern GnomeUIInfo * menu_principal; 
extern GnomeUIInfo * menu_parametres;
extern GnomeUIInfo * menu_importer;
extern GnomeUIInfo * menu_exporter;
extern GnomeUIInfo * menu_derniers_fichiers; 
extern GnomeUIInfo * menu_cloture;



/*********************/ 
/* fichier main.c */
/*********************/ 

/*  Variables globales */

extern GtkWidget *window;
extern GtkWidget *statusbar;


/***********************************/ 
/* fichier comptes_onglet.c */
/***********************************/ 

extern GtkWidget *bouton_supprimer_compte;
extern GtkWidget *bouton_ouvert;
extern GtkWidget *label_compte_courant_onglet;
extern gint compte_courant_onglet;
extern GtkWidget *vbox_liste_comptes_onglet;


/***********************************/ 
/* fichier operations_comptes.c */
/***********************************/ 

extern GtkWidget *label_compte_courant;
extern int compte_courant;                        /*  n° de compte en cours de visualisation */
extern GtkWidget *vbox_liste_comptes;            /* adresse de la vbox contenant les icones de comptes */
extern GtkWidget *label_releve;                    /* adr du label du dernier relevé */


/***********************************/ 
/* fichier operations_liste.c */
/***********************************/ 

extern GtkWidget *notebook_listes_operations;          /*  adr du notebook qui contient les opés de chaque compte */
extern GtkWidget *bouton_affiche_cache_formulaire;
extern GtkWidget *fleche_haut;                        /* adr de l'icone de la flèche vers le haut  */
extern GtkWidget *fleche_bas;                        /* adr de l'icone de la flèche vers le bas  */
extern GtkWidget *frame_droite_bas;                  /* adr de la frame contenant le formulaire */
extern GtkWidget *barre_outils;                           /* adr de la barre d'outils */
extern gdouble solde_courant_affichage_liste;   /* valable uniquement pendant l'affichage de la liste */
extern gdouble solde_pointe_affichage_liste;   /* valable uniquement pendant l'affichage de la liste */
extern gchar *tips_col_liste_operations[7];
extern gchar *titres_colonnes_liste_operations[7];


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
extern GtkWidget *page_comptes;        /* adr de l'onglet comptes */
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

extern GtkWidget *widget_formulaire_operations[19];
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
extern GtkWidget *bouton_valider_echeance_perso;
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
extern GtkStyle *style_label_nom_compte;
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
extern GSList *liste_tiers_combofix_echeancier;            /* liste des tiers pour le combofix de l'échéancier */
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
extern struct struct_devise *devise_compte;
extern struct struct_devise *devise_operation;



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
extern GtkWidget *widget_formulaire_ventilation[7];
extern GtkWidget *separateur_formulaire_ventilations;
extern GtkWidget *hbox_valider_annuler_ventil;
extern GtkWidget *label_somme_ventilee;                       /*  label correspondant */
extern GtkWidget *label_non_affecte;                       /*  label correspondant */
extern GtkWidget *label_montant_operation_ventilee;                       /*  label correspondant */
extern struct struct_ope_ventil *ligne_selectionnee_ventilation;              /* struct ope ligne sélectionnée */
extern gdouble montant_operation_ventilee;
extern gdouble somme_ventilee;
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
extern GtkWidget *bouton_ope_4_lignes;
extern GtkWidget *bouton_ope_3_lignes;
extern GtkWidget *bouton_ope_2_lignes;
extern GtkWidget *bouton_ope_1_lignes;
extern GtkWidget *bouton_affiche_r;
extern GtkWidget *bouton_enleve_r;
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
/* fichier etats_onglet.c */
/***********************************/ 

extern GSList *liste_struct_etats;
extern gint no_dernier_etat;
extern GtkWidget *frame_liste_etats;
extern GtkWidget *label_etat_courant;        /* label en haut de la liste des états */
extern GtkWidget *vbox_liste_etats;          /* vbox contenant la liste des états */
extern GtkWidget *bouton_etat_courant;          /* adr du bouton de l'état en cours, pour le refermer qd change */
extern struct struct_etat *etat_courant;
extern GtkWidget *bouton_effacer_etat;
extern GtkWidget *bouton_personnaliser_etat;
extern GtkWidget *bouton_imprimer_etat;
extern GtkWidget *bouton_exporter_etat;
extern GtkWidget *bouton_importer_etat;
extern GtkWidget *bouton_dupliquer_etat;
extern GtkWidget *scrolled_window_etat;          /* contient l'état en cours */
extern gint nb_colonnes;
extern gint ligne_debut_partie;
extern GtkWidget *notebook_etats;
extern GtkWidget *onglet_affichage_etat;
extern GtkWidget *onglet_config_etat;
extern GtkWidget *notebook_selection;


/***********************************/ 
/* fichier etats_config.c */
/***********************************/ 

extern GtkWidget *entree_nom_etat;
extern GtkWidget *liste_type_classement_etat;
extern GtkWidget *bouton_opes_r_et_non_r_etat;
extern GtkWidget *bouton_opes_non_r_etat;
extern GtkWidget *bouton_opes_r_etat;
extern GtkWidget *bouton_afficher_opes;
extern GtkWidget *bouton_afficher_nb_opes;
extern GtkWidget *bouton_afficher_date_opes;
extern GtkWidget *bouton_afficher_tiers_opes;
extern GtkWidget *bouton_afficher_categ_opes;
extern GtkWidget *bouton_afficher_sous_categ_opes;
extern GtkWidget *bouton_afficher_ib_opes;
extern GtkWidget *bouton_afficher_sous_ib_opes;
extern GtkWidget *bouton_afficher_notes_opes;
extern GtkWidget *bouton_afficher_pc_opes;
extern GtkWidget *bouton_afficher_infobd_opes;
extern GtkWidget *bouton_afficher_no_ope;
extern GtkWidget *bouton_afficher_type_ope;
extern GtkWidget *bouton_afficher_no_rappr;
extern GtkWidget *bouton_afficher_no_cheque;
extern GtkWidget *bouton_afficher_exo_opes;
extern GtkWidget *bouton_pas_detailler_ventilation;
extern GtkWidget *bouton_separer_revenus_depenses;
extern GtkWidget *bouton_devise_general_etat;
extern GtkWidget *bouton_inclure_dans_tiers;
extern GtkWidget *bouton_afficher_titres_colonnes;
extern GtkWidget *bouton_titre_changement;
extern GtkWidget *bouton_titre_en_haut;
extern GtkWidget *bouton_choix_classement_ope_etat;
extern GtkWidget *bouton_rendre_ope_clickables;
extern GtkWidget *frame_onglet_generalites;

extern GtkWidget *radio_button_utilise_exo;
extern GtkWidget *radio_button_utilise_dates;
extern GtkWidget *liste_exo_etat;
extern GtkWidget *vbox_generale_exo_etat;
extern GtkWidget *bouton_exo_tous;
extern GtkWidget *bouton_exo_courant;
extern GtkWidget *bouton_exo_precedent;
extern GtkWidget *bouton_detaille_exo_etat;
extern GtkWidget *bouton_separe_exo_etat;
extern GtkWidget *liste_plages_dates_etat;
extern GtkWidget *entree_date_init_etat;
extern GtkWidget *entree_date_finale_etat;
extern GtkWidget *vbox_utilisation_exo;
extern GtkWidget *vbox_utilisation_date;
extern GtkWidget *bouton_separe_plages_etat;
extern GtkWidget *bouton_type_separe_plages_etat;
extern GtkWidget *bouton_debut_semaine;

extern GtkWidget *liste_comptes_etat;
extern GtkWidget *bouton_detaille_comptes_etat;
extern GtkWidget *vbox_generale_comptes_etat;
extern GtkWidget *bouton_regroupe_ope_compte_etat;
extern GtkWidget *bouton_affiche_sous_total_compte;
extern GtkWidget *bouton_afficher_noms_comptes;

extern GtkWidget *bouton_inclusion_virements_actifs_etat;
extern GtkWidget *bouton_inclusion_virements_hors_etat;
extern GtkWidget *bouton_non_inclusion_virements;
extern GtkWidget *bouton_inclusion_virements_perso;
extern GtkWidget *liste_comptes_virements;
extern GtkWidget *bouton_exclure_non_virements_etat;
extern GtkWidget *hbox_liste_comptes_virements;

extern GtkWidget *bouton_utilise_categ_etat;
extern GtkWidget *bouton_detaille_categ_etat;
extern GtkWidget *hbox_detaille_categ_etat;
extern GtkWidget *bouton_afficher_sous_categ;
extern GtkWidget *liste_categ_etat;
extern GtkWidget *bouton_affiche_sous_total_categ;
extern GtkWidget *bouton_affiche_sous_total_sous_categ;
extern GtkWidget *bouton_exclure_ope_sans_categ;
extern GtkWidget *bouton_devise_categ_etat;
extern GtkWidget *bouton_afficher_pas_de_sous_categ;
extern GtkWidget *bouton_afficher_noms_categ;

extern GtkWidget *bouton_utilise_ib_etat;
extern GtkWidget *bouton_detaille_ib_etat;
extern GtkWidget *hbox_detaille_ib_etat;
extern GtkWidget *liste_ib_etat;
extern GtkWidget *bouton_afficher_sous_ib;
extern GtkWidget *bouton_affiche_sous_total_ib;
extern GtkWidget *bouton_affiche_sous_total_sous_ib;
extern GtkWidget *bouton_exclure_ope_sans_ib;
extern GtkWidget *bouton_devise_ib_etat;
extern GtkWidget *bouton_afficher_pas_de_sous_ib;
extern GtkWidget *bouton_afficher_noms_ib;

extern GtkWidget *bouton_utilise_tiers_etat;
extern GtkWidget *bouton_detaille_tiers_etat;
extern GtkWidget *hbox_detaille_tiers_etat;
extern GtkWidget *liste_tiers_etat;
extern GtkWidget *bouton_affiche_sous_total_tiers;
extern GtkWidget *bouton_devise_tiers_etat;
extern GtkWidget *bouton_afficher_noms_tiers;

extern GtkWidget *bouton_utilise_texte;
extern GtkWidget *vbox_generale_textes_etat;
extern GtkWidget *liste_textes_etat;

extern GtkWidget *bouton_utilise_montant;
extern GtkWidget *vbox_generale_montants_etat;
extern GtkWidget *liste_montants_etat;
extern GtkWidget *bouton_devise_montant_etat;
extern GtkWidget *bouton_exclure_ope_nulles_etat;

extern gint ancienne_ib_etat;
extern gint ancienne_sous_ib_etat;
extern gint ancienne_categ_etat;
extern gint ancienne_categ_speciale_etat;
extern gint ancienne_sous_categ_etat;
extern gint ancien_compte_etat;
extern gint ancien_tiers_etat;
extern gdouble montant_categ_etat;
extern gdouble montant_sous_categ_etat;
extern gdouble montant_ib_etat;
extern gdouble montant_sous_ib_etat;
extern gdouble montant_compte_etat;
extern gdouble montant_tiers_etat;
extern gdouble montant_periode_etat;
extern gint nb_ope_categ_etat;
extern gint nb_ope_sous_categ_etat;
extern gint nb_ope_ib_etat;
extern gint nb_ope_sous_ib_etat;
extern gint nb_ope_compte_etat;
extern gint nb_ope_tiers_etat;
extern gint nb_ope_periode_etat;
extern gint nb_ope_general_etat;
extern gint nb_ope_partie_etat;
extern GDate *date_debut_periode;
extern gint changement_de_groupe_etat;
extern gint debut_affichage_etat;
extern struct struct_devise *devise_compte_en_cours_etat;
extern struct struct_devise *devise_categ_etat;
extern struct struct_devise *devise_ib_etat;
extern struct struct_devise *devise_tiers_etat;
extern struct struct_devise *devise_generale_etat;

extern gchar *nom_categ_en_cours;
extern gchar *nom_ss_categ_en_cours;
extern gchar *nom_ib_en_cours;
extern gchar *nom_ss_ib_en_cours;
extern gchar *nom_compte_en_cours;
extern gchar *nom_tiers_en_cours;
extern gint titres_affiches;

extern GtkWidget *bouton_detaille_mode_paiement_etat;
extern GtkWidget *vbox_mode_paiement_etat;
extern GtkWidget *liste_mode_paiement_etat;


/***********************************/ 
/* fichier etats_io.c */
/***********************************/ 

extern gchar *log_message;        /* utilisé comme log lors de l'import d'un état */



/************************/ 
/* fichier affichage_liste.c */
/************************/ 

extern gint col_depart_drag;
extern gint ligne_depart_drag;
extern gint tab_affichage_ope[4][7];
extern gint tab_affichage_ope_tmp[4][7];
extern GtkWidget *boutons_affichage_liste[17];
extern GtkWidget *clist_affichage_liste;
extern GtkWidget *bouton_choix_perso_colonnes;
extern GtkWidget *bouton_caracteristiques_lignes_par_compte;
extern GtkWidget *bouton_affichage_lignes_une_ligne;
extern GtkWidget *bouton_affichage_lignes_deux_lignes_1;
extern GtkWidget *bouton_affichage_lignes_deux_lignes_2;
extern GtkWidget *bouton_affichage_lignes_trois_lignes_1;
extern GtkWidget *bouton_affichage_lignes_trois_lignes_2;
extern GtkWidget *bouton_affichage_lignes_trois_lignes_3;
extern gint rapport_largeur_colonnes[7];    /* contient le % de chaque colonne */
extern gint taille_largeur_colonnes[7];   /* contient la taille de chaque colonne */
extern gint ligne_affichage_une_ligne;    /* contient le no de ligne à afficher lorsqu'on n'affiche qu'une ligne */
extern GSList *lignes_affichage_deux_lignes;    /* contient les no de lignes à afficher lorsqu'on affiche deux lignes */
extern GSList *lignes_affichage_trois_lignes;    /* contient les no de lignes à afficher lorsqu'on affiche trois lignes */
