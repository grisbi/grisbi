/* fichier d'en tête structures.h */
/* contient toutes les structures du prog */

#include "print_config.h"
#include "constants.h"

#define VERSION_FICHIER "0.5.0"
#define VERSION_FICHIER_ETAT "0.5.0"
#define VERSION_FICHIER_CATEG "0.5.0"
#define VERSION_FICHIER_IB "0.5.0"

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

#define COLON(s) (g_strconcat ( s, " : ", NULL ))
#define SPACIFY(s) (g_strconcat ( " ", s, " ", NULL ))
#define PRESPACIFY(s) (g_strconcat ( " ", s, NULL ))
#define POSTSPACIFY(s) (g_strconcat ( s, " ", NULL ))

#ifdef ENABLE_NLS
#  include <libintl.h>
#  define _(String) gettext(String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop(String)
#  else
#    define N_(String) (String)
#  endif /* gettext_noop */
#else
#  define _(String) (String)
#  define N_(String) (String)
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,String) (String)
#  define dcgettext(Domain,String,Type) (String)
#  define bindtextdomain(Domain,Directory) (Domain) 
#endif /* ENABLE_NLS */



/* variables contenant juste 0 ou 1 */

struct {
    guint modification_fichier;
    guint ancienne_date;
    guint ctrl;
    guint equilibrage;
    guint valeur_r_avant_rapprochement;
    guint r_modifiable;
    guint dernier_fichier_auto;
    guint sauvegarde_auto;             /* utilisé pour enregistrer le fichier automatiquementà la fermeture */
    guint sauvegarde_demarrage;        /* utilisé pour enregistrer le fichier s'il s'est bien ouvert */
    guint entree;
    guint alerte_mini;
    guint formulaire_toujours_affiche;
    guint formulaire_echeancier_toujours_affiche;
    guint formulaire_echeance_dans_fenetre;        /* à 1 lorsque le formulaire de l'échéancier est dans une fenetre à part */
    guint alerte_permission;   /* à un si le message d'alerte s'affiche */
    guint fichier_deja_ouvert;   /* à un si lors de l'ouverture, le fichier semblait déjà ouvert */
    guint force_enregistrement;    /* à un si on force l'enregistrement */
    guint affichage_exercice_automatique;   /* à 1 si exercice automatique selon la date, 0 si affiche le dernier choisi */
    guint affiche_tous_les_types;      /* à 1 si tous les types sont dans le check button et non pas juste les débits ou crédits */
    guint affiche_no_operation;
    guint affiche_date_bancaire; /* GDC : à 1 si on veut afficher la colonne et le champ de la date réelle */
    guint utilise_exercice;
    guint utilise_imputation_budgetaire;
    guint utilise_piece_comptable;
    guint utilise_info_banque_guichet;
    guint affiche_boutons_valider_annuler;
    guint affiche_nb_ecritures_listes;
    guint classement_par_date;   /* à 1 si le classement de la liste d'opé se fait par date */
    guint largeur_auto_colonnes;
    guint retient_affichage_par_compte;   /* à 1 si les caractéristiques de l'affichage (R, non R ...) diffèrent par compte */
    guint en_train_de_sauvegarder;
    guint en_train_de_charger;
    guint utilise_logo;
    guint utilise_fonte_listes;
    guint classement_rp;   /*à 1 si on veut un classement r/p avant les dates FIXME : à virer sur l'instable*/

    /* Various display message stuff  */
    guint display_message_lock_active;
    guint display_message_file_readable;
    guint display_message_minimum_alert;
    guint display_message_qif_export_currency;

    /* Various commands */
    gchar * browser_command;
    gchar * latex_command;
    gchar * dvips_command;

    /*     chemin de l'image affichée quand on patiente */

    gchar *fichier_animation_attente;

    /* Print stuff */
    struct print_config print_config;
    
    /*     largeur des panned */

    gint largeur_colonne_comptes_operation;
    gint largeur_colonne_echeancier;
    gint largeur_colonne_comptes_comptes;
    gint largeur_colonne_etat;

} etat;



struct structure_operation
{
    guint no_operation;

    gchar *id_operation;   /* utilisé lors d'import ofx pour éviter les doublons */

    GDate *date;
    guint jour;
    guint mois;
    guint annee;

    GDate *date_bancaire;
    guint jour_bancaire;
    guint mois_bancaire;
    guint annee_bancaire;

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

    gshort pointe;            /*  0=rien, 1=pointée, 2=rapprochée, 3=T */
    gshort auto_man;           /* 0=manuel, 1=automatique */
    gint no_rapprochement;          /* contient le numéro du rapprochement si elle est rapprochée */

    guint no_exercice;             /* exercice de l'opé */
    guint imputation;
    guint sous_imputation;
    gchar *no_piece_comptable;
    gchar *info_banque_guichet;

    guint relation_no_operation;
    gint relation_no_compte;             /* attention, doit être à gint et pas guint car à -1 si compte supprimé */

    guint no_operation_ventilee_associee;      /* si c'est une opé d'une ventilation, contient le no de l'opé ventilée */
};




struct donnees_compte
{
    gint no_compte;
    gchar *id_compte;            /*cet id est rempli lors d'un import ofx, il est invisible à l'utilisateur*/
    gint type_de_compte;          /* 0 = bancaire, 1 = espèce, 2 = passif, 3= actif */
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
    gint affichage_r;            /* à 1 si les R sont affichés pour ce compte */
    gint nb_lignes_ope;           /* contient le nb de lignes pour une opé (1, 2, 3, 4 ) */
};



/* pointeurs vers les comptes en fonction de p...variable */

#define NO_COMPTE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> no_compte
#define ID_COMPTE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> id_compte
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
#define AFFICHAGE_R ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> affichage_r
#define NB_LIGNES_OPE ((struct donnees_compte *) (*p_tab_nom_de_compte_variable)) -> nb_lignes_ope


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
    gchar *notes;

    gint type_ope;        /* variable suivant le compte */
    gchar *contenu_type;          /* ce peut être un no de chèque, de virement ou tout ce qu'on veut */

    gshort auto_man;           /* 0=manuel, 1=automatique */

    guint no_exercice;             /* exercice de l'opé */
    guint imputation;
    guint sous_imputation;

    gshort periodicite;             /*  0=une fois, 1=hebdo, 2=mensuel, 3=annuel, 4=perso */
    gshort intervalle_periodicite_personnalisee;   /* 0=jours, 1=mois, 2=annees */
    guint periodicite_personnalisee;

    GDate *date_limite;
    gint jour_limite;
    gint mois_limite;
    gint annee_limite;

    gint compte_virement;
    gint type_contre_ope;        /* variable suivant le compte */

    gint operation_ventilee;        /* à 1 si c'est une opé ventilée */
    guint no_operation_ventilee_associee;      /* si c'est une opé d'une ventilation, contient le no de l'opé ventilée */
};



#define COMPTE_ECHEANCE GTK_LABEL (GTK_BIN ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ) -> child ) -> label
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
    gchar *code_iso4217_devise;
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
    gdouble no_en_cours;
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

    gint afficher_r;         /* 0=ttes les opés, 1=que les opés non R, 2=que les opés R */
    gint afficher_opes;
    gint afficher_nb_opes;
    gint afficher_date_ope;
    gint afficher_tiers_ope;
    gint afficher_categ_ope;
    gint afficher_sous_categ_ope;
    gint afficher_ib_ope;
    gint afficher_sous_ib_ope;
    gint afficher_notes_ope;
    gint afficher_pc_ope;
    gint afficher_infobd_ope;
    gint afficher_no_ope;
    gint afficher_type_ope;
    gint afficher_cheque_ope;
    gint afficher_rappr_ope;
    gint afficher_exo_ope;
    gint type_classement_ope;     /* 0=date, 1=no opé, 2=tiers, 3=categ, 4=ib, 5=notes, 6=type ope, 7=no chq, 8=pc, 9=ibg, 10=no rappr */
    gint pas_detailler_ventilation;
    gint separer_revenus_depenses;
    gint devise_de_calcul_general;
    gint afficher_titre_colonnes;
    gint type_affichage_titres;        /* 0 = en haut, 1 = à chaque changement de section */
    gint inclure_dans_tiers;        /* à 1 si on veut que cet état apparaisse dans la liste des tiers du formulaire */
    gint ope_clickables;           /* à 1 si les opés peuvent être clickables */

    gint exo_date;         /* 1-> utilise l'exo / 0 -> utilise une plage de date */
    gint utilise_detail_exo;   /* 0=tous, 1=exercice courant, 2=exercice précédent, 3=perso */
    GSList *no_exercices;            /* liste des no d'exos utilisés dans l'état */
    gint separation_par_exo;       /* 1=oui, 0=non */
    gint no_plage_date;       /* 0=perso, 1=toutes ... */
    GDate *date_perso_debut;
    GDate *date_perso_fin;
    gint separation_par_plage;       /* 1=oui, 0=non */
    gint type_separation_plage;        /*  0=semaines, 1=mois, 2=année */
    gint jour_debut_semaine;           /* 0=lundi ... */

    GList *type_classement;  /* liste de no : 1=caté,2=ss categ,3=ib,4=ss ib,5=compte,6=tiers */

    gint utilise_detail_comptes;
    GSList *no_comptes;
    gint regroupe_ope_par_compte;
    gint affiche_sous_total_compte;
    gint afficher_nom_compte;

    gint type_virement;   /* 0: pas de virements / 1:seulement les virements vers comptes actifs-passifs/2:virements hors état/3:perso */
    GSList *no_comptes_virements;
    gint exclure_ope_non_virement;

    gint utilise_categ;
    gint utilise_detail_categ;
    GSList *no_categ;
    gint afficher_sous_categ;
    gint affiche_sous_total_categ;
    gint affiche_sous_total_sous_categ;
    gint exclure_ope_sans_categ;
    gint devise_de_calcul_categ;
    gint afficher_pas_de_sous_categ;
    gint afficher_nom_categ;

    gint utilise_ib;
    gint utilise_detail_ib;
    GSList *no_ib;
    gint afficher_sous_ib;
    gint affiche_sous_total_ib;
    gint affiche_sous_total_sous_ib;
    gint exclure_ope_sans_ib;
    gint devise_de_calcul_ib;
    gint afficher_pas_de_sous_ib;
    gint afficher_nom_ib;

    gint utilise_tiers;
    gint utilise_detail_tiers;
    GSList *no_tiers;
    gint affiche_sous_total_tiers;
    gint devise_de_calcul_tiers;
    gint afficher_nom_tiers;

    gint utilise_texte;
    GSList *liste_struct_comparaison_textes;

    gint utilise_montant;
    gint choix_devise_montant;
    GSList *liste_struct_comparaison_montants;
    gint exclure_montants_nuls;

    gint utilise_mode_paiement;
    GSList *noms_modes_paiement;
};


struct struct_comparaison_montants_etat
{
    gint lien_struct_precedente;    /* -1=1ère comparaison, 0=et, 1=ou, 2=sauf */
    gint comparateur_1;            /* 0= =, 1= <, 2= <=, 3= >, 4= >=, 5= !=, 6= =0, 7= !=0, 8= >0, 9= <0 */
    gdouble montant_1;
    gint lien_1_2;                   /* 0=et, 1=ou, 2=sauf, 3=aucun */
    gint comparateur_2;            /* 0= =, 1= <, 2= <=, 3= >, 4= >=, 5= != */
    gdouble montant_2;

    GtkWidget *hbox_ligne;
    GtkWidget *bouton_lien;
    GtkWidget *bouton_comparateur_1;
    GtkWidget *entree_montant_1;
    GtkWidget *bouton_lien_1_2;
    GtkWidget *hbox_partie_2;
    GtkWidget *bouton_comparateur_2;
    GtkWidget *entree_montant_2;
};

struct struct_comparaison_textes_etat
{
    gint lien_struct_precedente;    /* -1=1ère comparaison, 0=et, 1=ou, 2=sauf */
    gint champ;                      /* 0=tiers, 1= info tiers, 2= categ, 3= ss categ, 4=ib, 5= ss-ib */
    /*                         6=note, 7=ref banc, 8=pc, 9=chq, 10=rappr */

    /* pour les comparaisons de txt */

    gint operateur;                   /* 0=contient, 1=ne contient pas, 2=commence par, 3=se termine par, 4=vide, 5=non vide */
    gchar *texte;
    gint utilise_txt;                 /* pour les chaps à no (chq, pc), à 1 si on les utilise comme txt, 0 sinon */

    /* pour les comparaisons de chq */

    gint comparateur_1;            /* 0= =, 1= <, 2= <=, 3= >, 4= >=, 5= !=, 6= le plus grand */
    gint montant_1;
    gint lien_1_2;                   /* 0=et, 1=ou, 2=sauf, 3=aucun */
    gint comparateur_2;            /* 0= =, 1= <, 2= <=, 3= >, 4= >=, 5= !=, 6= le plus grand */
    gint montant_2;


    /* adr des widget utilisés */

    GtkWidget *vbox_ligne;
    GtkWidget *bouton_lien;
    GtkWidget *bouton_champ;

    /* pourles comparaisons de txt */

    GtkWidget *bouton_utilise_txt;    /* sensitif en cas de champ à no */
    GtkWidget *hbox_txt;
    GtkWidget *bouton_operateur;
    GtkWidget *entree_txt;

    /* pour les comparaisons de chq */

    GtkWidget *bouton_utilise_no;    /* sensitif en cas de champ à no */
    GtkWidget *hbox_chq;
    GtkWidget *bouton_comparateur_1;
    GtkWidget *entree_montant_1;
    GtkWidget *bouton_lien_1_2;
    GtkWidget *hbox_partie_2;
    GtkWidget *bouton_comparateur_2;
    GtkWidget *entree_montant_2;
};

enum alignement {
    LEFT, CENTER, RIGHT,
};

/** Device independant drawing functions for reports */
struct struct_etat_affichage
{
    gint (* initialise) ();	/** Initialisation of drawing session  */
    gint (* finish) ();		/** End of drawing session  */
    void (* attach_hsep) (int, int, int, int); /** Draw horizontal separator (aka "-")  */
    void (* attach_vsep) (int, int, int, int); /** Draw vertical separator (aka "|") */
    void (* attach_label) (gchar *, gdouble, int, int, int, int, enum alignement, struct structure_operation *); /** Drraw a label with properties  */
};


/* struture d'une opé de ventil */

struct struct_ope_ventil
{
    guint no_operation;

    gdouble montant;

    guint categorie;
    guint sous_categorie;

    gchar *notes;

    guint imputation;
    guint sous_imputation;
    gchar *no_piece_comptable;

    guint relation_no_operation;      /* contient le no de l'opé associée quand elle est déjà créée, -1 si c'est un virement non créé */
    guint relation_no_compte;
    gint no_type_associe;

    gint pointe;

    gint no_exercice;

    gint supprime;   /* à 1 quand cette opé doit être supprimée */
};


/* Handle an ISO 4217 currency.  Not specific to Grisbi. */
struct iso_4217_currency 
{
    gchar * continent;
    gchar * currency_name;
    gchar * country_name;
    gchar * currency_code;
    gchar * currency_nickname;
    gboolean active;
};


#define STANDARD_WIDTH 6
#define STANDARD_DIALOG_WIDTH 12


/* struture d'une importation : compte contient la liste des opés importées */

struct struct_compte_importation
{
    gchar *id_compte;

    gint origine;    /* 0=qif, 1=ofx, 2=html */

    gchar *nom_de_compte;
    gint type_de_compte;  /* 0=OFX_CHECKING,1=OFX_SAVINGS,2=OFX_MONEYMRKT,3=OFX_CREDITLINE,4=OFX_CMA,5=OFX_CREDITCARD,6=OFX_INVESTMENT, 7=cash */
    gchar *devise;

    GDate *date_depart;
    GDate *date_fin;

    GSList *operations_importees;          /* liste des struct des opés importées */

    gdouble solde;
    gchar *date_solde_qif;            /* utilisé temporairement pour un fichier qif */

    GtkWidget *bouton_devise;             /* adr du bouton de la devise dans le récapitulatif */
    GtkWidget *bouton_action;             /* adr du bouton de l'action dans le récapitulatif */
    GtkWidget *bouton_type_compte;             /* adr du bouton du type de compte dans le récapitulatif */
    GtkWidget *bouton_compte;             /* adr du bouton du compte dans le récapitulatif */
};


struct struct_ope_importation
{
    gchar *id_operation;

    gint no_compte;    /*  mis à jour si lors du marquage, si pas d'opé associée trouvée */
    gint devise;      /* mis à jour au moment de l'enregistrement de l'opé */
    GDate *date;
    GDate *date_de_valeur;
    gchar *date_tmp;      /* pour un fichier qif, utilisé en tmp avant de le transformer en gdate */

    gint action;       /* ce champ est à 0 si on enregisre l'opé, à 1 si on ne l'enregistre pas (demandé lors d'ajout des opés à un compte existant) */
    /*   et 2 si on ne veut pas l'enregistrer ni demander (une id qui a été retrouvée */
    struct structure_operation *ope_correspondante; /* contient l'adrde l'opé qui correspond peut être à l'opé importée pour la présentation à l'utilisateur */
    GtkWidget *bouton;  /*  adr du bouton si cette opé est douteuse et vérifiée par l'utilisateur */

    gchar *tiers;
    gchar *notes;
    gulong cheque;

    gchar *categ;

    gdouble montant;

    gint p_r;

    gint type_de_transaction;

    gint operation_ventilee;  /* à 1 si c'est une ventil, dans ce cas les opés de ventil suivent et ont ope_de_ventilation à 1 */
    gint ope_de_ventilation;
};



