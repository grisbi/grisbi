#ifndef _STRUCTURES_H
#define _STRUCTURES_H (1)

/* fichier d'en tête structures.h */
/* contient presque toutes les structures du prog */


/* the VERSION_FICHIER is in fact the lowest version which can be used to open the
 * file, so each time we make an incompatibility issue, VERSION_FICHIER must be set
 * to VERSION_GRISBI */
#define VERSION_FICHIER "0.6.0"

/* Define this when in Git development branch. */
#define IS_DEVELOPMENT_VERSION 0

#define VERSION_FICHIER_ETAT "0.6.0"
#define VERSION_FICHIER_CATEG "0.6.0"
#define VERSION_FICHIER_IB "0.6.0"
#define VERSION_GTK_MAJOR 2
#define VERSION_GTK_MINOR 16
#define VERSION_GTK_MICRO 0

#define CSV_MAX_TOP_LINES 10	/** How many lines to show in CSV preview.  */

/* definition des variables PIXMAPS_DIR PLUGINS_DIR */
#define GRISBI_PIXMAPS_DIR (utils_get_pixmaps_dir ( ) )
#define GRISBI_PLUGINS_DIR (utils_get_plugins_dir ( ) ) 


/* structures pour la gestion de l'impression */
enum filetype
{
  POSTSCRIPT_FILE = 0,
  LATEX_FILE,
  NUM_FILETYPES,
}; 


struct paper_config 
{
  gchar *name;
  gfloat width;
  gfloat height;
};


enum orientation
{
  LANDSCAPE = 0,
  PORTRAIT,
  NUM_ORIENTATION,
}; 


struct print_config
{
  gboolean printer;
  gchar * printer_name;
  gchar * printer_filename;
  enum filetype filetype;
  struct paper_config paper_config;
  enum orientation orientation;
};


/** structure etat
 * variables contenant juste 0 ou 1
 * FIXME : scinder cette structure en 3 parties :
 * la partie configurée par le fichier
 * la partie configurée par la conf
 * la partie configurée pendant le fonctionnement de grisbi
 * */
struct gsb_etat_t
{
    time_t modification_fichier;
    gint is_archive;                /** TRUE if the file is an archive, FALSE else */

    /* Rapprochement */
    gint reconcile_end_date;        /* Date initiale + 1 mois par défaut */
    gint equilibrage;
    gint reconcile_account_number;  /* Save the last reconciliation try: account */
    gchar *reconcile_final_balance; /* final balance amount */
    GDate *reconcile_new_date;      /* new date */

    gboolean debug_mode;            /* TRUE in debug mode, FALSE for normale mode */

    /* files and backup part */
    gint crypt_file;                /* TRUE if we want to crypt the file */
    gint fichier_deja_ouvert;       /* à un si lors de l'ouverture, le fichier semblait déjà ouvert */

    /* formulaire */
    gint formulaire_toujours_affiche;
    gint affichage_exercice_automatique;    /* automatic fyear :0 to set according to the date, 2 according to value date */
    gint affiche_nb_ecritures_listes;
    gint largeur_auto_colonnes;
    gint retient_affichage_par_compte;      /* à 1 si les caractéristiques de l'affichage (R, non R ...) diffèrent par compte */
    gint en_train_de_sauvegarder;
    gint en_train_de_charger;

    /* Fonts & logo */
    gint utilise_logo;
    gboolean is_pixmaps_dir;        /* TRUE if path_icon == GRISBI_PIXMAPS_DIR */
    gchar *name_logo;
    
    gint display_toolbar;           /** Display mode of toolbar. */
    gboolean show_toolbar;          /** Show toolbar or not.  */
    gboolean show_headings_bar;     /** Show headings bar or not. */
    gboolean show_closed_accounts;
    gboolean automatic_separator;   /* TRUE if do automatic separator */

    /* Various display message stuff    */
    gint display_message_lock_active;
    gint display_message_file_readable;
    gint display_message_minimum_alert;
    gint display_message_no_budgetary_line;
    gint display_message_qif_export_currency;
    gint display_message_ofx_security;

    /* import files */
    gint get_extract_number_for_check;      /* TRUE if Extracting a number and save it in the field No Cheque/Virement */
    gint get_fusion_import_transactions;    /* TRUE if merge transactions imported with transactions found*/
    gint get_categorie_for_payee;           /* TRUE to automatically retrieve the category of the payee if it exists */
    gint get_fyear_by_value_date;           /* TRUE to get the fyear by value date, FALSE by date */

    gchar * csv_separator;                              /** CSV separator to use while parsing a CSV file. */
    gboolean csv_skipped_lines [ CSV_MAX_TOP_LINES ];   /* Contains a pointer to skipped lines in CSV preview. */

    /* export files */
    gint export_file_format;                /* EXPORT_QIF or EXPORT_CSV */
    gboolean export_files_traitement;       /* 0 = traitement individuel, 1 = traitement automatique */

    /* combofix configuration */
    gint combofix_mixed_sort;               /* TRUE for no separation between the categories */
    gint combofix_max_item;                 /* maximum number of items we want before showing the popup */
    gint combofix_case_sensitive;           /* TRUE if case sensitive */
    gint combofix_enter_select_completion;  /* TRUE if enter close the popup and keep what is in the entry (else, select the current item in the list) */
    gint combofix_force_payee;              /* TRUE if no new item can be appended in the payee combofix */
    gint combofix_force_category;           /* TRUE if no new item can be appended in the category and budget combofix */
    
    /* Print stuff */
    struct print_config print_config;
    
    /* width panned */
    gint largeur_colonne_echeancier;
    gint largeur_colonne_comptes_comptes;
    gint largeur_colonne_etat;

    /* variables sur l'échéancier */
    gint affichage_commentaire_echeancier;      /* à 1 si le commentaire est affiché */

    /* Tips */
    gint last_tip; 
    gint show_tip; 

    /* variables pour les metatree */
    gint metatree_sort_transactions;        /* 1 = sorting by increasing date 2 = Sort by date descending */
    gint add_archive_in_total_balance;      /* Add transactions archived in the totals */

    /* variables for the module estimate balance */
    gint bet_deb_period;
    gint bet_deb_cash_account_option;       /* ajoute l'onglet prévision aux comptes de caisse */

    /* variables pour le simulateur de crédits */
    gdouble bet_capital;
    gint bet_currency;
    gdouble bet_taux_annuel;
    gint bet_index_duree;
    gdouble bet_frais;
    gint bet_type_taux;
};

/* declared in parametres.c */
extern struct gsb_etat_t etat;


/** structure conf
 * variables containing just 0 or 1
 * configured by the file grisbi.conf
 *
 */
struct gsb_conf_t
{
    /* geometry */
    gint root_x;                                    /* main_window x position */
    gint root_y;                                    /* main_window y position */
    gint main_width;                                /* main_window width */
    gint main_height;                               /* main_window height */
    gint full_screen;                               /* 1 to full screen, 0 else */
    gint largeur_colonne_comptes_operation;
    gint prefs_width;                               /* preferences width */

    /* general part */
    gint r_modifiable;                              /* Changes in reconciled transactions */
    gint entree;                                    /* si etat.entree = 1, la touche entrée finit l'opération */ 
    gint alerte_mini;
    gint utilise_fonte_listes;                      /* TRUE to use a custom font for the lists */
    gchar *font_string;                             /* contain the description of the font, or NULL */
    gchar *browser_command;
    gchar *latex_command;
    gchar *dvips_command;
    gint pluriel_final;                             /* 0 = finals 1 = finaux */

    gint display_grisbi_title;                      /* selection du titre principal de grisbi */

    /* files part */
    gint sauvegarde_demarrage;                      /* utilisé pour enregistrer le fichier s'il s'est bien ouvert */
    gint sauvegarde_auto;                           /* utilisé pour enregistrer le fichier automatiquementà la fermeture */
    gint dernier_fichier_auto;
    gint compress_file;                             /* TRUE if we want to compress the grisbi file */
    gint alerte_permission;                         /* à un si le message d'alerte s'affiche */
    gint force_enregistrement;                      /* à un si on force l'enregistrement */

    /* backup part */
    gint make_backup;                               /* TRUE for create a backup when save file */
    gint make_backup_every_minutes;                 /* TRUE to make backup every x mn */
    gint make_backup_nb_minutes;                    /* the number of minutes we want to make a backup */
    gint make_bakup_single_file;                    /* TRUE if a single backup file */
    gint compress_backup;                           /* TRUE if we want to compress the backup */

    /* formulaire */
    gint automatic_completion_payee;                /* 1 pour autoriser la completion automatique des opérations */
    gboolean limit_completion_to_current_account;   /* Limit payee completion to current account or do a full search. */
    gboolean automatic_recover_splits;              /* 1 pour recréer automatiquement les sous opérations */

#if IS_DEVELOPMENT_VERSION == 1
    /* config file */
    gint stable_config_file_model;                  /* TRUE for use the stable config file as model */
#endif

    /* variables pour le calcul des soldes */
    gboolean balances_with_scheduled;               /* TRUE = the balance incorporates the scheduled operations */

    /* variables for the list of transactions */
    gint show_transaction_selected_in_form;         /* TRUE will show the selected transaction in the form */
    gint show_transaction_gives_balance;            /* TRUE si on visualise l'opération qui donne le solde du jour */
    gboolean transactions_list_primary_sorting;     /* Primary sorting option for the transactions */
    gboolean transactions_list_secondary_sorting;   /* Secondary sorting option for the transactions */

    /* variables for the list of transactions */
    gint metatree_action_2button_press;             /* 0 default gtk, 1 edit_function, 1 manage division if possible */

    /* archive stuff */
    gint check_for_archival;                        /* TRUE if we want to check the number of non archived transactions at the opening */
    gint max_non_archived_transactions_for_check;   /* the max number of transaction before grisbi warm at the opening */

};

/* declared in gsb_file_config.c */
extern struct gsb_conf_t conf;

/** structure run
 * variables containing just 0 or 1
 * générées pendant l'exécution du programme
 *
 */

struct gsb_run_t
{
    /* initialisation variables */
    gboolean new_crypted_file;      /* 0 à l'exécution de init_variables () 1 si new crypted file*/

    /* largeur des colonnes */
    gchar *transaction_column_width;
    gchar *scheduler_column_width;

};

/* declared in main.c */
extern struct gsb_run_t run;

/* structure définissant une association entre un tiers
 * et une chaine de recherche contenant un ou des jokers (%)
 */
struct struct_payee_asso
{
    gint    payee_number;
    gchar   *search_str;
};


/* définition du titre de grisbi */
typedef enum GSB_TITLE_NAME
{
    GSB_ACCOUNTS_TITLE,
    GSB_ACCOUNT_HOLDER,
    GSB_ACCOUNTS_FILE,
} GsbTitleType;


/* définition de l'alignement */
enum alignement
{
    ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT,
};


/* définition du type d'origine pour les données du module budgétaire */
enum bet_array_origin_data
{
    SPP_ORIGIN_TRANSACTION,             /* Ligne issue d'une opération */
    SPP_ORIGIN_SCHEDULED,               /* Ligne issue d'une opération plannifiée */
    SPP_ORIGIN_HISTORICAL,              /* Ligne issue d'une donnée historique */
    SPP_ORIGIN_FUTURE,                  /* Ligne issue d'une opération future */
    SPP_ORIGIN_ACCOUNT,                 /* Ligne issue d'un compte */
    SPP_ORIGIN_ARRAY,                   /* Donnée provenant du tableau de prévision */
    SPP_ORIGIN_CONFIG,                  /* Donnée provenant de la page des préférences */
    SPP_ORIGIN_SIMULATOR,               /* Donnée provenant du tableau du simulateur de crédit */
    SPP_ORIGIN_FINANCE,                 /* Donnée provenant du tableau d'amortissement du simulateur */
    SPP_ORIGIN_SOLDE                    /* Ligne de solde au premier du mois  */
};


/* définition du type de mise à jour en fonction des données du module budgétaire */
enum bet_type_maj 
{
    BET_MAJ_FALSE = 0,
    BET_MAJ_ESTIMATE,
    BET_MAJ_HISTORICAL,
    BET_MAJ_FINANCIAL,
    BET_MAJ_ALL,
};



#endif
