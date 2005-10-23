#ifndef STRUCTURES_H
#define STRUCTURES_H (1)

/* fichier d'en tête structures.h */
/* contient toutes les structures du prog */

#include "print_config.h"
#include "utils_str.h"

#define VERSION_FICHIER "0.6.0"
#define VERSION_FICHIER_ETAT "0.6.0"
#define VERSION_FICHIER_CATEG "0.6.0"
#define VERSION_FICHIER_IB "0.6.0"

/* constante de debug originale, obsolete il faudrait utiliser maintenant */
/* la variable globale debugging_grisbi, voir erreur.c */
#define DEBUG utils_str_atoi (getenv ("DEBUG_GRISBI"))

/* constantes definissant le niveau de debug */
#define	DEBUG_LEVEL_ALERT			1		/* grave probleme */
#define DEBUG_LEVEL_IMPORTANT			2		/* probleme moins grave */
#define DEBUG_LEVEL_NOTICE			3		/* probleme encore moins grave :) */
#define DEBUG_LEVEL_INFO			4		/* autre information */
#define DEBUG_LEVEL_DEBUG			5		/* information de debug */
#define MAX_DEBUG_LEVEL				5		/* ignore higher debug levels */
/* constante indiquant dans quelle fichier on se trouve */
/* c'est un alias pratique pour la fonction debug */
#define DEBUG_WHERE_AM_I g_strdup_printf("%s line %05d",__FILE__,__LINE__)

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


/* variables contenant juste 0 ou 1 */

struct {
    guint modification_fichier;
    guint ctrl;
    guint equilibrage;
    guint r_modifiable;
    guint dernier_fichier_auto;
    guint sauvegarde_auto;             /* utilisé pour enregistrer le fichier automatiquementà la fermeture */
    guint sauvegarde_demarrage;        /* utilisé pour enregistrer le fichier s'il s'est bien ouvert */
    guint entree;   			 /* si etat.entree = 1, la touche entrée finit l'opération */ 
    guint alerte_mini;
    guint formulaire_toujours_affiche;
    guint alerte_permission;   /* à un si le message d'alerte s'affiche */
    guint fichier_deja_ouvert;   /* à un si lors de l'ouverture, le fichier semblait déjà ouvert */
    guint force_enregistrement;    /* à un si on force l'enregistrement */
    guint affichage_exercice_automatique;   /* à 1 si exercice automatique selon la date, 0 si affiche le dernier choisi */
    guint crypt_file;   /* TRUE if we want to crypt the file */
    guint compress_file;  /* TRUE if we want to compress the grisbi file */
    guint compress_backup;  /* TRUE if we want to compress the backup */

    guint formulaire_distinct_par_compte;  /* à 1 si le formulaire est différent pour chaque compte */
    guint affiche_boutons_valider_annuler;
    guint affiche_nb_ecritures_listes;
    guint classement_par_date;   /* à 1 si le classement de la liste d'opé se fait par date */
    guint largeur_auto_colonnes;
    guint retient_affichage_par_compte;   /* à 1 si les caractéristiques de l'affichage (R, non R ...) diffèrent par compte */
    guint en_train_de_sauvegarder;
    guint en_train_de_charger;
    guint utilise_logo;
    guint utilise_fonte_listes;
    guint affichage_grille;         /* à 1 si la grille est affichée */
    guint display_toolbar;
    guint show_closed_accounts;

    /* Various display message stuff  */
    guint display_message_lock_active;
    guint display_message_file_readable;
    guint display_message_minimum_alert;
    guint display_message_no_budgetary_line;
    guint display_message_qif_export_currency;
    guint display_message_ofx_security;

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

    /* variables sur l'échéancier */

    guint formulaire_echeancier_toujours_affiche;
    guint formulaire_echeance_dans_fenetre;        /* à 1 lorsque le formulaire de l'échéancier est dans une fenetre à part */
    guint affichage_commentaire_echeancier;     /* à 1 si le commentaire est affiché */

    /* Tips */
    gint last_tip; 
    gint show_tip; 
} etat;



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

    gint compte_virement;    /* à  -1 si c'est une échéance sans categ, et que ce n'est ni un virement ni une ventil */
    gint type_contre_ope;        /* variable suivant le compte */

    gint operation_ventilee;        /* à 1 si c'est une opé ventilée */
    guint no_operation_ventilee_associee;      /* si c'est une opé d'une ventilation, contient le no de l'opé ventilée */
};



#define COMPTE_ECHEANCE GTK_LABEL (GTK_BIN ( widget_formulaire_echeancier[5]  )  -> child ) -> label
#define ECHEANCE_COURANTE ((struct operation_echeance *)  ( pointeur_liste -> data ))



/* devises */

struct struct_devise
{
    gint no_devise;
    gchar *nom_devise;
    gchar *code_iso4217_devise;
    gchar *code_devise;

    /*     tous ces champs sont obsolètes, mais gardés pour les anciennes devises qui sont passées à l'euro */
    /* 	ils ne doivent plus être utilisés sauf dans ce cas */

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
    gint no_en_cours;
    gint no_compte;
};



/* la struct_ope_liee est utilisée pour le passage de la 0.3.1 à la 0.3.2 */
/* pour transformer les no d'opés */
/* à virer ensuite */

struct struct_ope_liee
{
    gint ancien_no_ope;
    gint compte_ope;
    gint nouveau_no_ope;
    gpointer ope_liee;
    gint ope_ventilee;
};

/* si en 2 parties, séparé d'un - */
/* si la 2ème est un numéro, elle est incrémentée à chaque équilibrage */

struct struct_no_rapprochement
{
    gint no_rapprochement;
    gchar *nom_rapprochement;
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
    void (* attach_label) (gchar *, gdouble, int, int, int, int, enum alignement, gpointer); /** Drraw a label with properties  */
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
    gint par_completion;    /*à 1 si cette opé a été ajoutée suite à une complétion de tiers*/
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
    gchar *flag_filename;
};


#define STANDARD_WIDTH 6
#define STANDARD_DIALOG_WIDTH 12

/* organisation du formulaire */

struct organisation_formulaire
{
    /*     nombre de colonnes : de 3 à 6 */

    gint nb_colonnes;

    /*     nombre de lignes : de 1 à 4 */

    gint nb_lignes;

    /*     remplissage du formulaire */
    /* 	1: date (obligatoire) */
    /* 	2: débit (obligatoire) */
    /* 	3: crédit (obligatoire) */
    /* 	4: date de val */
    /* 	5: exo */
    /* 	6: tiers */
    /* 	7: categ */
    /* 	8: bouton de ventil */
    /* 	9: ib */
    /* 	10: notes */
    /* 	11: mode de paiement */
    /* 	12: no de chq */
    /* 	13: devise */
    /* 	14: bouton change */
    /* 	15: pièce comptable */
    /* 	16: info b/g */
    /* 	17: mode paiement contre opé */
	
    gint tab_remplissage_formulaire[4][6];

    /*     rapport de taille entre les colonnes */

    gint taille_colonne_pourcent[6];

};
#endif
