/* fichier d'en tête structures.h */
/* contient toutes les structures du prog */


#define VERSION "0.3.3"

/* Chemin des fichiers */

#define APPLET_BIN_DIR "/usr/bin/grisbi_applet"
#define CHEMIN_LOGO "/usr/share/pixmaps/logo_grisbi.xpm"
#define FICHIER_CONF "Grisbi-dev"       /* à mettre à Grisbi-dev pdt le dvt et Grisbi à la sortie d'une version */

/* initialisation des couleurs */

#define COULEUR1_RED  55000
#define COULEUR1_GREEN  55000
#define COULEUR1_BLUE  65535
#define COULEUR2_RED  65535
#define COULEUR2_GREEN  65535
#define COULEUR2_BLUE  65535
#define COULEUR_SELECTION_RED  63000
#define COULEUR_SELECTION_GREEN  40000
#define COULEUR_SELECTION_BLUE  40000
#define COULEUR_ROUGE_RED  65535
#define COULEUR_ROUGE_GREEN  0
#define COULEUR_ROUGE_BLUE  0
#define COULEUR_NOIRE_RED 0
#define COULEUR_NOIRE_GREEN 0
#define COULEUR_NOIRE_BLUE 0
#define COULEUR_GRISE_RED 50000
#define COULEUR_GRISE_GREEN 50000
#define COULEUR_GRISE_BLUE 50000

/* variables contenant juste 0 ou 1 */

struct {
  guint ouvre_fichier : 1;
  guint modification_fichier : 1;
  guint formulaire_en_cours : 1;
  guint ancienne_date : 1;
  guint ctrl : 1;
  guint equilibrage : 1;
  guint r_affiches : 1;
  guint r_modifiable : 1;
  guint dernier_fichier_auto : 1;
  guint sauvegarde_auto : 1;
  guint entree : 1;
  guint alerte_mini : 1;
  guint formulaire_toujours_affiche : 1;
  guint formulaire_echeancier_toujours_affiche : 1;
  guint formulaire_echeance_dans_fenetre : 1;        /* à 1 lorsque le formulaire de l'échéancier est dans une fenetre à part */
  guint alerte_permission : 1;   /* à un si le message d'alerte s'affiche */
  guint fichier_deja_ouvert : 1;   /* à un si lors de l'ouverture, le fichier semblait déjà ouvert */
  guint force_enregistrement : 1;    /* à un si on force l'enregistrement */
  guint affichage_exercice_automatique : 1;   /* à 1 si exercice automatique selon la date, 0 si affiche le dernier choisi */
  guint affiche_tous_les_types : 1;      /* à 1 si tous les types sont dans le check button et non pas juste les débits ou crédits */
  guint affiche_no_operation :1;
  guint affiche_date_bancaire :1; /* GDC : à 1 si on veut afficher la colonne et le champ de la date réelle */
  guint utilise_exercice :1;
  guint utilise_imputation_budgetaire :1;
  guint utilise_piece_comptable :1;
  guint utilise_info_banque_guichet :1;
  guint affiche_boutons_valider_annuler :1;
  guint affiche_nb_ecritures_listes :1;
  guint classement_par_date :1;   /* à 1 si le classement de la liste d'opé se fait par date */
} etat;



struct structure_operation
{
  guint no_operation;

  GDate *date;
  guint jour;
  guint mois;
  guint annee;

  /* GDC La date a laquelle a reellement ete effectuee l'operation
     contrairement a date qui est celle de prise en compte en banque */
  GDate *date_bancaire;
  guint jour_bancaire;
  guint mois_bancaire;
  guint annee_bancaire;
  /* GDCFin */

  guint no_compte;

  gdouble montant;
  guint devise;
  guint une_devise_compte_egale_x_devise_ope;       /* si = 1 : 1 devise_compte = "taux_change * montant" devise_en_rapport */
  gdouble taux_change;
  gdouble frais_change;

  guint tiers;        /*  no du tiers */
  guint categorie;
  guint sous_categorie;
  gint operation_ventilee;        /* à 1 si c'est une opé ventilée */

  gchar *notes;

  guint type_ope;               /* variable suivant le type de compte */
  gchar *contenu_type;          /* ce peut être un no de chèque, de virement ou tout ce qu'on veut */

  gshort pointe;            /*  0=rien, 1=pointée, 2=rapprochée */
  gshort auto_man;           /* 0=manuel, 1=automatique */
  gint no_rapprochement;          /* contient le numéro du rapprochement si elle est rapprochée */

  guint no_exercice;             /* exercice de l'opé */
  guint imputation;
  guint sous_imputation;
  gchar *no_piece_comptable;
  gchar *info_banque_guichet;

  guint relation_no_operation;
  guint relation_no_compte;

  guint no_operation_ventilee_associee;      /* si c'est une opé d'une ventilation, contient le no de l'opé ventilée */
};




struct donnees_compte
{
  gint no_compte;
  gint type_de_compte;          /* 0 = bancaire, 1 = espèce, 2 = passif */
  gchar *nom_de_compte;
  gint nb_operations;
  gdouble solde_initial;
  gdouble solde_mini_voulu;
  gdouble solde_mini_autorise;
  gdouble solde_courant;
  gdouble solde_pointe;
  GSList *gsliste_operations;
  GtkWidget *clist_operations;        /* adr de la liste des opé */
  struct structure_operation *operation_selectionnee;       /* contient l'adr de la struct de la ligne sélectinnée */
  gint message_sous_mini;
  gint message_sous_mini_voulu;
  GDate *date_releve;
  gdouble solde_releve;
  gint dernier_no_rapprochement;
  GtkWidget *icone_ouverte;              /* adr de l'icone ouverte */
  GtkWidget *icone_fermee;              /* adr de l'icone fermée */
  gint mise_a_jour;                      /* à 1 lorsque la liste doit être rafraichie à l'affichage */
  gint devise;                         /* contient le no de la devise du compte */
  gint banque;                                      /* 0 = aucune, sinon, no de banque */
  gchar *no_guichet;
  gchar *no_compte_banque;
  gchar *cle_compte;
  gint compte_cloture;                          /* si = 1 => cloturé */
  gchar *commentaire;
  gint tri;                           /* si = 1 => tri en fonction des types, si 0 => des dates */
  GSList *liste_tri;                         /* contient la liste des types dans l'ordre du tri du compte */
  gint neutres_inclus;                           /* si = 1 => les neutres sont inclus dans les débits/crédits */
  gchar *titulaire;                  /* contient le nom du titulaire de ce compte */
  gchar *adresse_titulaire;          /* à NULL si c'est l'adresse commune, contient l'adresse sinon */
  GSList *liste_types_ope;         /* liste qui contient les struct de types d'opé du compte */
  gint type_defaut_debit;            /* no du type par défaut */
  gint type_defaut_credit;            /* no du type par défaut */
};



/* pointeurs vers les comptes en fonction de p...variable */

#define NO_COMPTE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> no_compte
#define TYPE_DE_COMPTE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> type_de_compte
#define NOM_DU_COMPTE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> nom_de_compte
#define NB_OPE_COMPTE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> nb_operations
#define SOLDE_INIT ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> solde_initial
#define SOLDE_MINI_VOULU ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> solde_mini_voulu 
#define SOLDE_MINI ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> solde_mini_autorise
#define SOLDE_COURANT ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> solde_courant
#define SOLDE_POINTE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> solde_pointe
#define LISTE_OPERATIONS ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> gsliste_operations
#define CLIST_OPERATIONS ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> clist_operations
#define OPERATION_SELECTIONNEE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> operation_selectionnee
#define MESSAGE_SOUS_MINI ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) ->  message_sous_mini
#define MESSAGE_SOUS_MINI_VOULU ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) ->  message_sous_mini_voulu
#define DATE_DERNIER_RELEVE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) ->  date_releve
#define SOLDE_DERNIER_RELEVE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) ->  solde_releve
#define DERNIER_NO_RAPPROCHEMENT ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> dernier_no_rapprochement
#define ICONE_OUVERTE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> icone_ouverte
#define ICONE_FERMEE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> icone_fermee
#define MISE_A_JOUR ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> mise_a_jour
#define DEVISE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> devise
#define BANQUE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> banque
#define NO_GUICHET ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> no_guichet
#define NO_COMPTE_BANQUE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> no_compte_banque
#define CLE_COMPTE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> cle_compte
#define COMPTE_CLOTURE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> compte_cloture
#define COMMENTAIRE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> commentaire
#define TRI ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> tri
#define LISTE_TRI ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> liste_tri
#define NEUTRES_INCLUS ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> neutres_inclus
#define TITULAIRE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> titulaire
#define ADRESSE_TITULAIRE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> adresse_titulaire
#define TYPES_OPES ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> liste_types_ope
#define TYPE_DEFAUT_DEBIT ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> type_defaut_debit
#define TYPE_DEFAUT_CREDIT ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> type_defaut_credit


struct operation_echeance
{
  guint no_operation;
  gint compte;

  GDate *date;
  guint jour;
  guint mois;
  guint annee;

  gdouble montant;
  gint devise;

  guint tiers;
  guint categorie;
  guint sous_categorie;
  gint compte_virement;
  gchar *notes;

  gint type_ope;        /* variable suivant le compte */
  gchar *contenu_type;          /* ce peut être un no de chèque, de virement ou tout ce qu'on veut */

  gshort auto_man;           /* 0=manuel, 1=automatique */

  guint no_exercice;             /* exercice de l'opé */
  guint imputation;
  guint sous_imputation;
  gchar *no_piece_comptable;
  gchar *info_banque_guichet;

  gshort periodicite;             /*  0=une fois, 1=hebdo, 2=mensuel, 3=annuel, 4=perso */
  gshort intervalle_periodicite_personnalisee;   /* 0=jours, 1=mois, 2=annees */
  guint periodicite_personnalisee;

  GDate *date_limite;
  gint jour_limite;
  gint mois_limite;
  gint annee_limite;
};



#define COMPTE_ECHEANCE GTK_LABEL (GTK_BIN ( widget_formulaire_echeancier[5]  )  -> child ) -> label
#define ECHEANCE_COURANTE ((struct operation_echeance *)  ( pointeur_liste -> data ))


struct struct_tiers
{
  guint no_tiers;
  gchar *nom_tiers;
  gchar *texte;                  /* contient le texte associé au tiers */
  gint liaison;                    /* à 1 si ce tiers est lié à un autre logiciel */
};


struct struct_categ
{
  gint no_categ;
  gchar *nom_categ;
  gint type_categ;                   /* 0=crédit ; 1 = débit ; 2 = spécial */
  gint no_derniere_sous_categ;
  GSList *liste_sous_categ;
};

struct struct_sous_categ
{
  gint no_sous_categ;
  gchar *nom_sous_categ;
};




/* devises */

struct struct_devise
{
  gint no_devise;
  gchar *nom_devise;
  gchar *code_devise;
  gint passage_euro;                                  /* à 1 si cette devise doit passer à l'euro */

  GDate *date_dernier_change;                     /*   dernière mise à jour du change, NULL si aucun change */
  gint une_devise_1_egale_x_devise_2;       /* si = 1 : 1 nom_devise = "change" devise_en_rapport */
  gint no_devise_en_rapport;
  gdouble change;                                          /* taux de change */
};

#define NOM_DEVISE_ASSOCIEE GTK_LABEL (GTK_BIN ( GTK_BUTTON ( &(GTK_OPTION_MENU (option_menu_devises) -> button ))) -> child ) -> label


/* banques */

struct struct_banque
{
  gint no_banque;
  gchar *code_banque;
  gchar *nom_banque;
  gchar *adr_banque;
  gchar *tel_banque;
  gchar *email_banque;
  gchar *web_banque;
  gchar *remarque_banque;
  gchar *nom_correspondant;
  gchar *tel_correspondant;
  gchar *email_correspondant;
  gchar *fax_correspondant;
};


struct struct_operation_qif
{
  gchar *date;

  gchar *tiers;
  gchar *categ;
  gchar *notes;

  gdouble montant;
  gulong cheque;
  gint operation_ventilee;

  gint p_r;

  gint ope_de_ventilation;
};



struct struct_exercice
{
  gint no_exercice;
  gchar *nom_exercice;
  GDate *date_debut;
  GDate *date_fin;
  gint affiche_dans_formulaire;
};

struct struct_type_ope
{
  gint no_type;
  gchar *nom_type;
  gint signe_type;   /* 0=neutre, 1=débit, 2=crédit */
  gint affiche_entree;    /* par ex pour virement, chèques ... */
  gint numerotation_auto;        /* pour les chèques */
  gulong no_en_cours;
  gint no_compte;
};

struct struct_imputation
{
  gint no_imputation;
  gchar *nom_imputation;
  gint type_imputation;                   /* 0=crédit ; 1 = débit */
  gint no_derniere_sous_imputation;
  GSList *liste_sous_imputation;
};

struct struct_sous_imputation
{
  gint no_sous_imputation;
  gchar *nom_sous_imputation;
};


/* la struct_ope_liee est utilisée pour le passage de la 0.3.1 à la 0.3.2 */
/* pour transformer les no d'opés */
/* à virer ensuite */

struct struct_ope_liee
{
  gint ancien_no_ope;
  gint compte_ope;
  gint nouveau_no_ope;
  struct structure_operation *ope_liee;
  gint ope_ventilee;
};

/* si en 2 parties, séparé d'un - */
/* si la 2ème est un numéro, elle est incrémentée à chaque équilibrage */

struct struct_no_rapprochement
{
  gint no_rapprochement;
  gchar *nom_rapprochement;
};


struct struct_etat
{
  gint no_etat;
  gchar *nom_etat;

  gint exo_date;         /* 0-> utilise l'exo / 1 -> utilise une plage de date */
  GSList *no_exercices;            /* liste des no dexos utilisés dans l'état */
  gint no_plage_date;       /* 0=perso, 1=toutes ... */
  GDate *date_perso_debut;
  GDate *date_perso_fin;

  GSList *no_comptes;
  GSList *no_categ;
  gint inclusion_virements_passif;    /* vers comptes passifs/actifs */
  gint inclusion_virements_hors_etat;
  GSList *no_ib;
  GSList *no_tiers;

  gchar texte;
  gdouble montant;
};
