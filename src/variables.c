/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)		      */
/* 			http://www.grisbi.org				      */

/* Ce fichier contient toutes les variables globales définies */
/* ainsi que toutes les définitions diverses */


const GnomeUIInfo * menu_fichier;
const GnomeUIInfo * menu_comptes; 
const GnomeUIInfo * menu_reports; 
const GnomeUIInfo * help_menu;
const GnomeUIInfo * menu_principal; 
const GnomeUIInfo * menu_parametres;
const GnomeUIInfo * menu_importer;
const GnomeUIInfo * menu_exporter;
const GnomeUIInfo * menu_derniers_fichiers; 
const GnomeUIInfo * menu_cloture;


/*********************/ 
/* fichier main.c */
/*********************/ 

/*  Variables globales */

GtkWidget *window;
GtkWidget *statusbar;


/***********************************/ 
/* fichier comptes_onglet.c */
/***********************************/ 

GtkWidget *bouton_supprimer_compte;
GtkWidget *bouton_ouvert;
GtkWidget *label_compte_courant_onglet;
gint compte_courant_onglet;
GtkWidget *vbox_liste_comptes_onglet;


/***********************************/ 
/* fichier operations_comptes.c */
/***********************************/ 

GtkWidget *label_compte_courant;
int compte_courant;                      /*  n° de compte en cours de visualisation */
GtkWidget *vbox_liste_comptes;             /* adresse de la vbox contenant les icones de comptes */
GtkWidget *label_releve;                    /* adr du label du dernier relevé */


/***********************************/ 
/* fichier operations_liste.c */
/***********************************/ 

GtkWidget *notebook_listes_operations;          /*  adr du notebook qui contient les opés de chaque compte */
GtkWidget *bouton_affiche_cache_formulaire;
GtkWidget *fleche_haut;                        /* adr de l'icone de la flèche vers le haut  */
GtkWidget *fleche_bas;                        /* adr de l'icone de la flèche vers le bas  */
GtkWidget *frame_droite_bas;                  /* adr de la frame contenant le formulaire */
GtkWidget *barre_outils;                           /* adr de la barre d'outils */
gdouble solde_courant_affichage_liste;   /* valable uniquement pendant l'affichage de la liste */
gdouble solde_pointe_affichage_liste;   /* valable uniquement pendant l'affichage de la liste */
gchar *tips_col_liste_operations[7];
gchar *titres_colonnes_liste_operations[7];


/***********************************/ 
/* fichier erreur.c */
/***********************************/ 

GtkWidget *win_erreur;



/***********************************/ 
/* fichier fichier_configuration.c */
/***********************************/ 

gint largeur_window;
gint hauteur_window;
gchar *buffer_dernier_fichier;
gchar *nom_fichier_backup;
gchar *dernier_chemin_de_travail;
gint nb_derniers_fichiers_ouverts;          /* contient le nb de derniers fichiers ouverts */
gdouble nb_max_derniers_fichiers_ouverts;        /* contient le nb max que peut contenir nb_derniers_fichiers_ouverts ( réglé dans les paramètres ) */
gchar **tab_noms_derniers_fichiers_ouverts;
gdouble compression_fichier;
gdouble compression_backup;

/********************/ 
/* fichier disque.c */
/********************/ 


/*  Variables globales */

gchar *nom_fichier_comptes, *ancien_nom_fichier_comptes;
gchar *buffer_nom_fichier_comptes;
gchar buffer_en_tete[50];                      /* utilisé comme buffer lors de la lecture du fichier */
/* struct stat buffer_stat; */
gint ancien_mask;
FILE *pointeur_fichier_comptes;
FILE *pointeur_fichier_comptes_tmp;
int nb_comptes;
gpointer ** p_tab_nom_de_compte = NULL;  /* pointe vers un tableau de pointeurs vers les comptes en mémoire*/
gpointer ** p_tab_nom_de_compte_variable;  /* idem, mais utilisé pour se déplacer */
gpointer ** p_tab_nom_de_compte_courant;  /* pointe sur le tableau de pointeurs sur le compte courant*/
GtkWidget * solde_label;   /*  pointeur vers le label qui contient le solde sous la liste des opé */
GtkWidget * solde_label_pointe;   /*  pointeur vers le label qui contient le solde pointe sous la liste des opé */
gchar version[10];
GSList *ordre_comptes;
gchar *titre_fichier;
gchar *adresse_commune;
gchar *adresse_secondaire;

GSList *liste_struct_exercices;
gint no_derniere_exercice;
gint nb_exercices;

gint no_derniere_operation;     /* contient le dernier numéro d'opération de tous les comptes réunis */


/*********************/ 
/* fichier fenetre_principale.c */
/*********************/ 


GtkWidget *notebook_general;       /* adr du notebook de base */
GtkWidget *page_accueil;           /* adr de l'onglet accueil */
GtkWidget *page_operations;        /* adr de l'onglet opérations ( ancien gest_main ) */
GtkWidget *page_echeancier;        /* adr de l'onglet échéances */
GtkWidget *page_comptes;        /* adr de l'onglet comptes */
GtkWidget *page_tiers;             /* adr de l'onglet tiers */
GtkWidget *page_categories;             /* adr de l'onglet categories */
GtkWidget *page_imputations;             /* adr de l'onglet ib */
GtkWidget *page_etats;             /* adr de l'onglet etats */
GtkWidget *notebook_comptes_equilibrage;
GtkWidget *notebook_formulaire;
gint modif_tiers;
gint modif_categ;
gint modif_imputation;

/*********************/ 
/* fichier fenetre.c */
/*********************/ 

/* variables */

GtkStyle *style_derriere_selection;
gint ancienne_ligne_selection;
GtkStyle *style_couleur [2];
GtkStyle *style_rouge_couleur [2];
GtkStyle *style_gris;                     /* utilisé pour le grisé des échéances */
gchar * fonte_liste;
gchar * fonte_general;
gchar * list_font_name; /* FIXME FONTS: remove */
gchar * list_font_size; /* FIXME FONTS: remove */
gchar * general_font_name; /* FIXME FONTS: remove */
gchar * general_font_size; /* FIXME FONTS: remove */
GtkWidget *hbox_comptes_equilibrage;
GtkWidget *formulaire;
GtkWidget *pointeur_fenetre_nouveau_compte;

gint *no_operation_formulaire = NULL;

gint operations_affichees;       /* nb d'opé affichées ( variable en fonction qu'on affiche les R ou pas  */


/************************/ 
/* fichier operations_formulaire.c */
/************************/ 

GtkWidget *widget_formulaire_operations[19];
GtkWidget *separateur_formulaire_operations;
GtkWidget *hbox_valider_annuler_ope;
gchar date_courante[11];             /* contient soit la date du jour, soit la derniere date entree */
GtkStyle *style_entree_formulaire[2];


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

GtkWidget *entree_no_rapprochement;
GtkWidget *label_ancienne_date_equilibrage;
GtkWidget *entree_ancien_solde_equilibrage;
GtkWidget *entree_nouvelle_date_equilibrage;
GtkWidget *entree_nouveau_montant_equilibrage;
gdouble solde_initial;
gdouble solde_final;
gdouble operations_pointees;
GtkWidget *label_equilibrage_compte;
GtkWidget *label_equilibrage_initial;
GtkWidget *label_equilibrage_final;
GtkWidget *label_equilibrage_pointe;
GtkWidget *label_equilibrage_ecart;
GtkWidget *bouton_ok_equilibrage;  
GSList *liste_no_rapprochements;            /* contient la liste des structures de no_rapprochement */
gint ancien_nb_lignes_ope;              /* contient l'ancien nb_lignes_ope */


/************************/ 
/* fichier parametres.c */
/************************/ 

gchar *applet_dir;
gchar *applet_bin_dir;

GtkWidget *fenetre_preferences;
GtkWidget *bouton_entree_enregistre, *bouton_entree_enregistre_pas;
GtkWidget *bouton_solde_mini, *bouton_pas_solde_mini;
GtkWidget *bouton_affiche_permission, *bouton_affiche_pas_permission;
GtkWidget *entree_titre_fichier;
GtkWidget *entree_adresse_commune;
GtkWidget *entree_adresse_secondaire;

GtkWidget *bouton_avec_demarrage;
GtkWidget *bouton_sans_demarrage;
GtkWidget *bouton_save_auto;
GtkWidget *bouton_save_non_auto;
GtkWidget *bouton_force_enregistrement;
GtkWidget *bouton_force_pas_enregistrement;
GtkWidget *bouton_demande_backup;
GtkWidget *entree_chemin_backup;
GtkWidget *spin_button_derniers_fichiers_ouverts;
GtkWidget *spin_button_compression_fichier;
GtkWidget *spin_button_compression_backup;
GtkWidget *bouton_save_demarrage;

GtkWidget *entree_jours;
GtkWidget *bouton_2click_saisie, *bouton_2click_pas_saisie;

GSList *fichier_a_verifier;
GtkWidget *bouton_affichage_applet;
GtkWidget *frame_demarrage;
GtkWidget *liste_comptes_verifies;
gint fichier_verifier_selectionne, nb_fichier_verifier;
GtkWidget *bouton_enlever;
GSList *liste_suppression_fichier_a_verifier, *fichier_a_verifier_tmp;
gint pid_applet;


GSList *liste_struct_exercices_tmp;                 /*  utilisée à la place de la liste des exercices pour les paramètres */
gint no_derniere_exercice_tmp;                            /*  utilisé à la place de no_derniere_exercice pour les paramètres */
gint nb_exercices_tmp;                            /*  utilisé à la place de nb_exercices pour les paramètres */
GtkWidget *clist_exercices_parametres;
GtkWidget *bouton_supprimer_exercice;
GtkWidget *nom_exercice;
GtkWidget *debut_exercice;
GtkWidget *fin_exercice;
GtkWidget *affichage_exercice;
GtkWidget *hbox_boutons_modif_exercice;
gint ligne_selection_exercice;
GtkWidget *bouton_affichage_auto_exercice;


/***********************************/ 
/* fichier echeancier_liste.c */
/***********************************/ 

GtkWidget *frame_formulaire_echeancier;
GtkWidget *formulaire_echeancier;
GtkWidget *liste_echeances;
GtkWidget *entree_personnalisation_affichage_echeances;
GtkWidget *bouton_personnalisation_affichage_echeances;
GtkWidget *bouton_valider_echeance_perso;
GtkWidget *bouton_saisir_echeancier;
GtkWidget *calendrier_echeances;

struct operation_echeance *echeance_selectionnnee;

GSList *gsliste_echeances;                 /* contient la liste des struct échéances */
gint nb_echeances;
gint no_derniere_echeance;

gdouble decalage_echeance;      /* nb de jours avant l'échéance pour prévenir */

gint ancienne_largeur_echeances;

GSList *echeances_a_saisir;
GSList *echeances_saisies;
gint affichage_echeances; /* contient 0(mois), 1 (2 mois), 2(année), 3(toutes), 4(perso) */
gint affichage_echeances_perso_nb_libre;     /* contient le contenu de l'entrée */
gint affichage_echeances_perso_j_m_a;        /* contient 0 (jours), 1 (mois), 2 (années) */



/************************/ 
/* fichier echeancier_formulaire.c */
/************************/ 

GtkWidget *widget_formulaire_echeancier[19];
GtkWidget *label_saisie_modif;
GSList *liste_categories_echeances_combofix;        /*  liste des noms des categ et sous categ pour le combofix */
GtkWidget *separateur_formulaire_echeancier;
GtkWidget *hbox_valider_annuler_echeance;


/***********************************/ 
/* fichier accueil.c */
/***********************************/ 

gint id_temps;
gchar *chemin_logo;
GtkWidget *logo_accueil;
GtkWidget *label_temps;
GtkWidget *label_titre_fichier;
GtkWidget *frame_etat_comptes_accueil;
GtkWidget *frame_etat_fin_compte_passif;
GtkWidget *frame_etat_echeances_manuelles_accueil;
GtkWidget *frame_etat_echeances_auto_accueil;
GtkWidget *frame_etat_echeances_finies;
GtkWidget *frame_etat_soldes_minimaux_autorises;
GtkWidget *frame_etat_soldes_minimaux_voulus;
GtkStyle *style_label_nom_compte;
GtkStyle *style_label;
GtkWidget *dialogue_echeance;


/***********************************/ 
/* fichier tiers_onglet.c */
/***********************************/ 

GtkWidget *arbre_tiers;
GtkWidget *entree_nom_tiers;
GtkWidget *text_box;
GtkWidget *bouton_modif_tiers_modifier;
GtkWidget *bouton_modif_tiers_annuler;
GtkWidget *bouton_supprimer_tiers;
GdkPixmap *pixmap_ouvre;
GdkBitmap *masque_ouvre;
GdkPixmap *pixmap_ferme;
GdkBitmap *masque_ferme;
GtkWidget *bouton_ajouter_tiers;

gint nb_enregistrements_tiers;
gint no_dernier_tiers;
GList tiers_combo;                         /* liste des struct_chaine_completion pour la complétion  */
GSList *liste_struct_tiers;             /* contient la liste des struct liste_tiers de tous les tiers */
GSList *liste_tiers_combofix;            /* liste des tiers pour le combofix */
GSList *liste_tiers_combofix_echeancier;            /* liste des tiers pour le combofix de l'échéancier */
GDate **date_dernier_tiers;            /* contient la date du dernier tiers lors de l'affichage de chaque tiers */
gint no_devise_totaux_tiers;
gint *nb_ecritures_par_tiers;           /* variable tampon */
gint nb_ecritures_par_comptes;

/***********************************/ 
/* fichier categories_onglet.c */
/***********************************/ 


GtkWidget *arbre_categ;
GtkWidget *entree_nom_categ;
GtkWidget *bouton_categ_debit;
GtkWidget *bouton_categ_credit;
GtkWidget *bouton_modif_categ_modifier;
GtkWidget *bouton_modif_categ_annuler;
GtkWidget *bouton_supprimer_categ;
GtkWidget *bouton_ajouter_categorie;
GtkWidget *bouton_ajouter_sous_categorie;

GSList *liste_struct_categories;    /* liste des structures de catég */
GSList *liste_categories_combofix;        /*  liste des noms des categ et sous categ pour le combofix */
gint nb_enregistrements_categories;        /* nombre de catégories */
gint no_derniere_categorie;
gfloat *tab_montant_categ;             /* buffer */
gfloat **tab_montant_sous_categ;            /* buffer */
gint *nb_ecritures_par_categ;           /* buffer */
gint **nb_ecritures_par_sous_categ;           /* buffer */

/***********************************/ 
/* fichier devises.c */
/***********************************/ 

GSList *liste_struct_devises;
gint nb_devises;
gint no_derniere_devise;
struct struct_devise *devise_nulle;
gdouble taux_de_change[2];
GtkWidget *option_menu_devise_1;
GtkWidget *option_menu_devise_2;


GSList *liste_struct_devises_tmp;                 /*  utilisée à la place de la liste des devises pour les paramètres */
gint no_derniere_devise_tmp;                           /*  utilisé à la place de no_derniere_devise pour les paramètres */
gint nb_devises_tmp;                             /*  utilisé à la place de nb_devises pour les paramètres */
GtkWidget *clist_devises_parametres;
GtkWidget *bouton_supprimer_devise;
GtkWidget *entree_nom_devise_parametres;
GtkWidget *entree_iso_code_devise_parametres;
GtkWidget *entree_code_devise_parametres;
GtkWidget *check_button_euro;                    /*bouton passera à l'euro */
GtkWidget *hbox_devise_associee;                  /* ligne contenant l'option menu de la devise associée */
GtkWidget *label_date_dernier_change;       /* label qui affiche soit aucun change entré, soit la date du dernier change */
GtkWidget *hbox_ligne_change;                  /* hbox contenant le change entre les devises */
GtkWidget *option_menu_devises;                  /* contient la devise avec laquelle on fait le change */
GtkWidget *devise_1;                                      /* option menu qui contient la devise 1 */
GtkWidget *devise_2;                                     /* option menu qui contient la devise 2 */
GtkWidget *entree_conversion;                       /* entrée qui contient le change */
gint ligne_selection_devise;               /* contient la ligne sélectionnée dans la liste des devises */
struct struct_devise *devise_compte;
struct struct_devise *devise_operation;


/***********************************/ 
/* fichier gestion_comptes.c */
/***********************************/ 

GtkWidget *bouton_detail;
GtkWidget *detail_nom_compte;
GtkWidget *detail_type_compte;
GtkWidget *detail_titulaire_compte;
GtkWidget *detail_bouton_adresse_commune;
GtkWidget *detail_adresse_titulaire;
GtkWidget *detail_option_menu_banque;
GtkWidget *detail_no_compte;
GtkWidget *label_code_banque;
GtkWidget *detail_guichet;
GtkWidget *detail_cle_compte;
GtkWidget *detail_devise_compte;
GtkWidget *bouton_passage_euro;
GtkWidget *hbox_bouton_passage_euro;
GtkWidget *detail_compte_cloture;
GtkWidget *detail_solde_init;
GtkWidget *detail_solde_mini_autorise;
GtkWidget *detail_solde_mini_voulu;
GtkWidget *detail_commentaire;
GtkWidget *hbox_boutons_modif;

GSList *liste_struct_banques;
gint nb_banques;
gint no_derniere_banque;


/***********************************/ 
/* fichier ventilation.c */
/***********************************/ 


GtkWidget *liste_operations_ventilees;
gint ancienne_largeur_ventilation;          /* magouille utilisée pour bloquer un signal size-allocate qui s'emballe */
GtkWidget *widget_formulaire_ventilation[8];
GtkWidget *separateur_formulaire_ventilations;
GtkWidget *hbox_valider_annuler_ventil;
GtkWidget *label_somme_ventilee;                       /*  label correspondant */
GtkWidget *label_non_affecte;                       /*  label correspondant */
GtkWidget *label_montant_operation_ventilee;                       /*  label correspondant */
struct struct_ope_ventil *ligne_selectionnee_ventilation;              /* struct ope ligne sélectionnée */
gdouble montant_operation_ventilee;
gdouble somme_ventilee;
gint enregistre_ope_au_retour;            /* à 1 si au click du bouton valider on enregistre l'opé */


/***********************************/ 
/* fichier qif.c */
/***********************************/ 

GSList *liste_comptes_qif;
GSList *liste_entrees_exportation;



/***********************************/ 
/* fichier import.c */
/***********************************/ 

GSList *liste_comptes_importes;
GtkWidget *dialog_recapitulatif;
GtkWidget *table_recapitulatif;
gint virements_a_chercher;


/***********************************/ 
/* fichier barre_outils */
/***********************************/ 

GtkTooltips *tooltips;
GtkWidget *bouton_affiche_cache_formulaire_echeancier;
/* dOm : bouton d'affichage des commentaires dans echeancier */
GtkWidget *bouton_affiche_commentaire_echeancier;
GtkWidget *fleche_bas_echeancier;
GtkWidget *fleche_haut_echeancier;
GtkWidget *bouton_ope_4_lignes;
GtkWidget *bouton_ope_3_lignes;
GtkWidget *bouton_ope_2_lignes;
GtkWidget *bouton_ope_1_lignes;
GtkWidget *bouton_affiche_r;
GtkWidget *bouton_enleve_r;
GtkWidget *label_proprietes_operations_compte;


/***********************************/ 
/* fichier patienter.c */
/***********************************/ 

GtkWidget *fenetre_patience = NULL;
gint image_patience_en_cours;
GtkWidget *label_patience;
gint timeout_patience;
GtkWidget *image[15];
gint patience_en_cours = 0;


/***********************************/ 
/* fichier type_operations.c */
/***********************************/ 

GtkWidget *arbre_types_operations;
GSList **liste_tmp_types;
gint *type_defaut_debit;
gint *type_defaut_credit;
GtkWidget *bouton_ajouter_type;
GtkWidget *bouton_retirer_type;
GtkWidget *entree_type_nom;
GtkWidget *bouton_type_apparaitre_entree;
GtkWidget *bouton_type_numerotation_automatique;
GtkWidget *entree_type_dernier_no;
GtkWidget *bouton_signe_type;
GtkWidget *bouton_type_choix_defaut;
GtkWidget *bouton_type_choix_affichage_formulaire;
GtkWidget *entree_automatic_numbering;

GtkWidget *bouton_type_tri_date;
GtkWidget *bouton_type_tri_type;
GtkWidget *bouton_type_neutre_inclut;
GtkWidget *type_liste_tri;
GtkWidget *vbox_fleches_tri;
gint *tri_tmp;
GSList **liste_tri_tmp;
gint *neutres_inclus_tmp;



/***********************************/ 
/* fichier banque.c */
/***********************************/ 


GSList *liste_struct_banques_tmp;                 /*  utilisée à la place de la liste des banques pour les paramètres */
gint no_derniere_banque_tmp;                            /*  utilisé à la place de no_derniere_banque pour les paramètres */
gint nb_banques_tmp;                            /*  utilisé à la place de nb_banques pour les paramètres */
GtkWidget *clist_banques_parametres;
GtkWidget *bouton_supprimer_banque;
GtkWidget *nom_banque;
GtkWidget *code_banque;
GtkWidget *tel_banque;
GtkWidget *adr_banque;
GtkWidget *email_banque;
GtkWidget *web_banque;
GtkWidget *nom_correspondant;
GtkWidget *tel_correspondant;
GtkWidget *email_correspondant;
GtkWidget *fax_correspondant;
GtkWidget *remarque_banque;
GtkWidget *hbox_boutons_modif_banque;
gint ligne_selection_banque;




/***********************************/ 
/* fichier imputation_budgetaire.c */
/***********************************/ 


GtkWidget *arbre_imputation;
GtkWidget *entree_nom_imputation;
GtkWidget *bouton_imputation_debit;
GtkWidget *bouton_imputation_credit;
GtkWidget *bouton_modif_imputation_modifier;
GtkWidget *bouton_modif_imputation_annuler;
GtkWidget *bouton_supprimer_imputation;
GtkWidget *bouton_ajouter_imputation;
GtkWidget *bouton_ajouter_sous_imputation;

GSList *liste_struct_imputation;    /* liste des structures de catég */
GSList *liste_imputations_combofix;        /*  liste des noms des imputation et sous imputation pour le combofix */
gint nb_enregistrements_imputations;        /* nombre de catégories */
gint no_derniere_imputation;
gfloat *tab_montant_imputation;             /* buffer */
gfloat **tab_montant_sous_imputation;            /* buffer */
gint *nb_ecritures_par_imputation;           /* buffer */
gint **nb_ecritures_par_sous_imputation;           /* buffer */



/***********************************/ 
/* fichier affichage.c */
/***********************************/ 

GtkWidget *liste_choix_ordre_comptes;
GtkWidget *bouton_afficher_no_operation;
GtkWidget *bouton_afficher_date_bancaire;
GtkWidget *bouton_utiliser_exercices;
GtkWidget *bouton_utiliser_imputation_budgetaire;
GtkWidget *bouton_utiliser_piece_comptable;
GtkWidget *bouton_utiliser_info_banque_guichet;
GtkWidget *bouton_afficher_boutons_valider_annuler;
GtkWidget *bouton_choix_devise_totaux_tiers;
GtkWidget *bouton_afficher_nb_ecritures;
GtkWidget *bouton_classer_liste_par_date;
GtkWidget *bouton_classer_liste_par_date_bancaire;


/***********************************/ 
/* fichier etats_onglet.c */
/***********************************/ 

GSList *liste_struct_etats;
gint no_dernier_etat;
GtkWidget *frame_liste_etats;
GtkWidget *label_etat_courant;        /* label en haut de la liste des états */
GtkWidget *vbox_liste_etats;          /* vbox contenant la liste des états */
GtkWidget *bouton_etat_courant;          /* adr du bouton de l'état en cours, pour le refermer qd change */
struct struct_etat *etat_courant;
GtkWidget *bouton_effacer_etat;
GtkWidget *bouton_personnaliser_etat;
GtkWidget *bouton_raffraichir_etat;
GtkWidget *bouton_imprimer_etat;
GtkWidget *bouton_exporter_etat;
GtkWidget *bouton_importer_etat;
GtkWidget *bouton_dupliquer_etat;
GtkWidget *scrolled_window_etat;          /* contient l'état en cours */
gint nb_colonnes;
gint ligne_debut_partie;
GtkWidget *notebook_etats;
GtkWidget *onglet_affichage_etat;
GtkWidget *onglet_config_etat;
GtkWidget *notebook_config_etat;
GtkWidget *notebook_selection;
GtkWidget *notebook_aff_donnees;


/***********************************/ 
/* fichier etats_config.c */
/***********************************/ 

GtkWidget *entree_nom_etat;
GtkWidget *liste_type_classement_etat;
GtkWidget *bouton_opes_r_et_non_r_etat;
GtkWidget *bouton_opes_non_r_etat;
GtkWidget *bouton_opes_r_etat;
GtkWidget *bouton_afficher_opes;
GtkWidget *bouton_afficher_nb_opes;
GtkWidget *bouton_afficher_date_opes;
GtkWidget *bouton_afficher_tiers_opes;
GtkWidget *bouton_afficher_categ_opes;
GtkWidget *bouton_afficher_sous_categ_opes;
GtkWidget *bouton_afficher_ib_opes;
GtkWidget *bouton_afficher_sous_ib_opes;
GtkWidget *bouton_afficher_notes_opes;
GtkWidget *bouton_afficher_pc_opes;
GtkWidget *bouton_afficher_infobd_opes;
GtkWidget *bouton_afficher_no_ope;
GtkWidget *bouton_afficher_type_ope;
GtkWidget *bouton_afficher_no_rappr;
GtkWidget *bouton_afficher_no_cheque;
GtkWidget *bouton_afficher_exo_opes;
GtkWidget *bouton_pas_detailler_ventilation;
GtkWidget *bouton_separer_revenus_depenses;
GtkWidget *bouton_devise_general_etat;
GtkWidget *bouton_inclure_dans_tiers;
GtkWidget *bouton_afficher_titres_colonnes;
GtkWidget *bouton_titre_changement;
GtkWidget *bouton_titre_en_haut;
GtkWidget *bouton_choix_classement_ope_etat;
GtkWidget *bouton_rendre_ope_clickables;
GtkWidget *frame_onglet_generalites;

GtkWidget *radio_button_utilise_exo;
GtkWidget *radio_button_utilise_dates;
GtkWidget *liste_exo_etat;
GtkWidget *vbox_generale_exo_etat;
GtkWidget *bouton_exo_tous;
GtkWidget *bouton_exo_courant;
GtkWidget *bouton_exo_precedent;
GtkWidget *bouton_detaille_exo_etat;
GtkWidget *bouton_separe_exo_etat;
GtkWidget *liste_plages_dates_etat;
GtkWidget *entree_date_init_etat;
GtkWidget *entree_date_finale_etat;
GtkWidget *vbox_utilisation_exo;
GtkWidget *vbox_utilisation_date;
GtkWidget *bouton_separe_plages_etat;
GtkWidget *bouton_type_separe_plages_etat;
GtkWidget *bouton_debut_semaine;

GtkWidget *liste_comptes_etat;
GtkWidget *bouton_detaille_comptes_etat;
GtkWidget *vbox_generale_comptes_etat;
GtkWidget *bouton_regroupe_ope_compte_etat;
GtkWidget *bouton_affiche_sous_total_compte;
GtkWidget *bouton_afficher_noms_comptes;

GtkWidget *bouton_inclusion_virements_actifs_etat;
GtkWidget *bouton_inclusion_virements_hors_etat;
GtkWidget *bouton_non_inclusion_virements;
GtkWidget *bouton_inclusion_virements_perso;
GtkWidget *liste_comptes_virements;
GtkWidget *bouton_exclure_non_virements_etat;
GtkWidget *hbox_liste_comptes_virements;

GtkWidget *bouton_utilise_categ_etat;
GtkWidget *bouton_detaille_categ_etat;
GtkWidget *hbox_detaille_categ_etat;
GtkWidget *bouton_afficher_sous_categ;
GtkWidget *liste_categ_etat;
GtkWidget *bouton_affiche_sous_total_categ;
GtkWidget *bouton_affiche_sous_total_sous_categ;
GtkWidget *bouton_exclure_ope_sans_categ;
GtkWidget *bouton_devise_categ_etat;
GtkWidget *bouton_afficher_pas_de_sous_categ;
GtkWidget *bouton_afficher_noms_categ;

GtkWidget *bouton_utilise_ib_etat;
GtkWidget *bouton_detaille_ib_etat;
GtkWidget *hbox_detaille_ib_etat;
GtkWidget *liste_ib_etat;
GtkWidget *bouton_afficher_sous_ib;
GtkWidget *bouton_affiche_sous_total_ib;
GtkWidget *bouton_affiche_sous_total_sous_ib;
GtkWidget *bouton_exclure_ope_sans_ib;
GtkWidget *bouton_devise_ib_etat;
GtkWidget *bouton_afficher_pas_de_sous_ib;
GtkWidget *bouton_afficher_noms_ib;

GtkWidget *bouton_utilise_tiers_etat;
GtkWidget *bouton_detaille_tiers_etat;
GtkWidget *hbox_detaille_tiers_etat;
GtkWidget *liste_tiers_etat;
GtkWidget *bouton_affiche_sous_total_tiers;
GtkWidget *bouton_devise_tiers_etat;
GtkWidget *bouton_afficher_noms_tiers;

GtkWidget *bouton_utilise_texte;
GtkWidget *vbox_generale_textes_etat;
GtkWidget *liste_textes_etat;

GtkWidget *bouton_utilise_montant;
GtkWidget *vbox_generale_montants_etat;
GtkWidget *liste_montants_etat;
GtkWidget *bouton_devise_montant_etat;
GtkWidget *bouton_exclure_ope_nulles_etat;

gint ancienne_ib_etat;
gint ancienne_sous_ib_etat;
gint ancienne_categ_etat;
gint ancienne_categ_speciale_etat;
gint ancienne_sous_categ_etat;
gint ancien_compte_etat;
gint ancien_tiers_etat;
gdouble montant_categ_etat;
gdouble montant_sous_categ_etat;
gdouble montant_ib_etat;
gdouble montant_sous_ib_etat;
gdouble montant_compte_etat;
gdouble montant_tiers_etat;
gdouble montant_periode_etat;
gint nb_ope_categ_etat;
gint nb_ope_sous_categ_etat;
gint nb_ope_ib_etat;
gint nb_ope_sous_ib_etat;
gint nb_ope_compte_etat;
gint nb_ope_tiers_etat;
gint nb_ope_periode_etat;
gint nb_ope_general_etat;
gint nb_ope_partie_etat;
GDate *date_debut_periode;
gint changement_de_groupe_etat;
gint debut_affichage_etat;
struct struct_devise *devise_compte_en_cours_etat;
struct struct_devise *devise_categ_etat;
struct struct_devise *devise_ib_etat;
struct struct_devise *devise_tiers_etat;
struct struct_devise *devise_generale_etat;

gchar *nom_categ_en_cours;
gchar *nom_ss_categ_en_cours;
gchar *nom_ib_en_cours;
gchar *nom_ss_ib_en_cours;
gchar *nom_compte_en_cours;
gchar *nom_tiers_en_cours;
gint titres_affiches;

GtkWidget *bouton_detaille_mode_paiement_etat;
GtkWidget *vbox_mode_paiement_etat;
GtkWidget *liste_mode_paiement_etat;


/***********************************/ 
/* fichier etats_io.c */
/***********************************/ 

gchar *log_message;        /* utilisé comme log lors de l'import d'un état */


/************************/ 
/* fichier affichage_liste.c */
/************************/ 

gint col_depart_drag;
gint ligne_depart_drag;
gint tab_affichage_ope[4][7];
gint tab_affichage_ope_tmp[4][7];
GtkWidget *boutons_affichage_liste[17];
GtkWidget *clist_affichage_liste;
GtkWidget *bouton_choix_perso_colonnes;
GtkWidget *bouton_caracteristiques_lignes_par_compte;
GtkWidget *bouton_affichage_lignes_une_ligne;
GtkWidget *bouton_affichage_lignes_deux_lignes_1;
GtkWidget *bouton_affichage_lignes_deux_lignes_2;
GtkWidget *bouton_affichage_lignes_trois_lignes_1;
GtkWidget *bouton_affichage_lignes_trois_lignes_2;
GtkWidget *bouton_affichage_lignes_trois_lignes_3;
gint rapport_largeur_colonnes[7];    /* contient le % de chaque colonne */
gint taille_largeur_colonnes[7];   /* contient la taille de chaque colonne */
gint ligne_affichage_une_ligne;    /* contient le no de ligne à afficher lorsqu'on n'affiche qu'une ligne */
GSList *lignes_affichage_deux_lignes;    /* contient les no de lignes à afficher lorsqu'on affiche deux lignes */
GSList *lignes_affichage_trois_lignes;    /* contient les no de lignes à afficher lorsqu'on affiche trois lignes */


/************************/ 
/* fichier etats_calculs.c */
/************************/ 

gint dernier_chq;     /* quand on a choisi le plus grand, contient le dernier no de chq dans les comptes choisis */
gint dernier_pc;     /* quand on a choisi le plus grand, contient le dernier no de pc dans les comptes choisis */
gint dernier_no_rappr;     /* quand on a choisi le plus grand, contient le dernier no de rappr dans les comptes choisis */
struct struct_etat_affichage * etat_affichage_output;

