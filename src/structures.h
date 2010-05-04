#ifndef _STRUCTURES_H
#define _STRUCTURES_H (1)

/* fichier d'en tête structures.h */
/* contient toutes les structures du prog */

#include "print_config.h"
#include "utils_str.h"

/* the VERSION_FICHIER is in fact the lowest version which can be used to open the
 * file, so each time we make an incompatibility issue, VERSION_FICHIER must be set
 * to VERSION_GRISBI */
#define VERSION_FICHIER "0.6.0"

/* Define this when in CVS development branch. */
#define IS_DEVELOPMENT_VERSION 1

#define VERSION_FICHIER_ETAT "0.6.0"
#define VERSION_FICHIER_CATEG "0.6.0"
#define VERSION_FICHIER_IB "0.6.0"
#define VERSION_GTK_MAJOR 2
#define VERSION_GTK_MINOR 16
#define VERSION_GTK_MICRO 0

#define COLON(s) (g_strconcat ( s, " : ", NULL ))
#define SPACIFY(s) (g_strconcat ( " ", s, " ", NULL ))
#define PRESPACIFY(s) (g_strconcat ( " ", s, NULL ))
#define POSTSPACIFY(s) (g_strconcat ( s, " ", NULL ))

#define CSV_MAX_TOP_LINES 10	/** How many lines to show in CSV preview.  */

/** structure etat
 * variables contenant juste 0 ou 1
 * FIXME : scinder cette structure en 3 parties :
 * la partie configurée par le fichier
 * la partie configurée par la conf
 * la partie configurée pendant le fonctionnement de grisbi
 * */
struct {
    time_t modification_fichier;
    gint is_archive;                /** TRUE if the file is an archive, FALSE else */
    gint equilibrage;

    gboolean debug_mode;            /* TRUE in debug mode, FALSE for normale mode */

    /* files and backup part */
    gint crypt_file;                /* TRUE if we want to crypt the file */
    gint fichier_deja_ouvert;       /* à un si lors de l'ouverture, le fichier semblait déjà ouvert */

    /* formulaire */
    gint formulaire_toujours_affiche;
    gint affichage_exercice_automatique;            /* automatic fyear :0 to set according to the date, 2 according to value date */
    gint automatic_completion_payee;                /* 1 pour autoriser la completion automatique des opérations */
    gboolean limit_completion_to_current_account;   /* Limit payee completion to current account or do a full search. */
    gint formulaire_distinct_par_compte;            /* à 1 si le formulaire est différent pour chaque compte */
    gint affiche_nb_ecritures_listes;
    gint largeur_auto_colonnes;
    gint retient_affichage_par_compte;      /* à 1 si les caractéristiques de l'affichage (R, non R ...) diffèrent par compte */
    gint en_train_de_sauvegarder;
    gint en_train_de_charger;
    gint utilise_logo;
    gint display_grisbi_title;      /* selection du titre principal de grisbi */
    
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

    /* import rules */
    gint get_extract_number_for_check;      /* TRUE if Extracting a number and save it in the field No Cheque/Virement */
    gint get_fusion_import_transactions;    /* TRUE if merge transactions imported with transactions found*/
    gint get_categorie_for_payee;           /* TRUE to automatically retrieve the category of the payee if it exists */
    gint get_fyear_by_value_date;           /* TRUE to get the fyear by value date, FALSE by date */

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

    gchar * csv_separator;                              /** CSV separator to use while parsing a CSV file. */
    gboolean csv_skipped_lines [ CSV_MAX_TOP_LINES ];   /* Contains a pointer to skipped lines in CSV preview. */

    /* variables pour les metatree */
    gint metatree_sort_transactions;        /* TRUE = sort transactions by date */
    gint add_archive_in_total_balance;      /* Add transactions archived in the totals */

    /* variables for the module estimate balance */
    gint bet_deb_period;
    
} etat;


/** structure conf
 * variables containing just 0 or 1
 * configured by the file grisbi.conf
 *
 */
struct {

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

    /* variables for the list of transactions */
    gint show_transaction_selected_in_form;         /* TRUE will show the selected transaction in the form */
    gint show_transaction_gives_balance;            /* TRUE si on visualise l'opération qui donne le solde du jour */
    gboolean transactions_list_sort_by_date;        /* Options for sorting by date */
    gboolean transactions_list_sort_by_value_date;  /* Options for sorting by value date */

    /* archive stuff */
    gint check_for_archival;                        /* TRUE if we want to check the number of non archived transactions at the opening */
    gint max_non_archived_transactions_for_check;   /* the max number of transaction before grisbi warm at the opening */

} conf;

/* structure définissant une association entre un tiers
 * et une chaine de recherche contenant un ou des jokers (%)
 */
struct struct_payee_asso
{
    gint    payee_number;
    gchar   *search_str;
};

enum alignement {
    LEFT, CENTER, RIGHT,
};


enum bet_array_origin_data {
    SPP_ORIGIN_TRANSACTION,
    SPP_ORIGIN_SCHEDULED,
    SPP_ORIGIN_HISTORICAL,
    SPP_ORIGIN_FUTURE,
    SPP_ORIGIN_ACCOUNT
};


#endif
