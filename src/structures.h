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

#define IS_DEVELOPMENT_VERSION (1)

#define VERSION_FICHIER_ETAT "0.6.0"
#define VERSION_FICHIER_CATEG "0.6.0"
#define VERSION_FICHIER_IB "0.6.0"



#define COLON(s) (g_strconcat ( s, " : ", NULL ))
#define SPACIFY(s) (g_strconcat ( " ", s, " ", NULL ))
#define PRESPACIFY(s) (g_strconcat ( " ", s, NULL ))
#define POSTSPACIFY(s) (g_strconcat ( s, " ", NULL ))


#define CSV_MAX_TOP_LINES 10	/** How many lines to show in CSV preview.  */


/* variables contenant juste 0 ou 1 */
/* FIXME : scinder cette structure en 3 parties : */
/* la partie configurée par le fichier */
/* la partie configurée par la conf */
/* la partie configurée pendant le fonctionnement de grisbi */
struct {
    time_t modification_fichier;
    gint is_archive;		/** TRUE if the file is an archive, FALSE else */
    gint equilibrage;
    gint r_modifiable;

    /* files and backup part */
    gint dernier_fichier_auto;
    gint sauvegarde_auto;             /* utilisé pour enregistrer le fichier automatiquementà la fermeture */
    gint sauvegarde_demarrage;        /* utilisé pour enregistrer le fichier s'il s'est bien ouvert */
    gint make_backup;			/* TRUE for create a backup when save file */
    gint make_backup_every_minutes;	/* TRUE to make backup every x mn */
    gint make_backup_nb_minutes;	/* the number of minutes we want to make a backup */

    gboolean debug_mode;		/* TRUE in debug mode, FALSE for normale mode */
    gint entree;   			 /* si etat.entree = 1, la touche entrée finit l'opération */ 
    gint alerte_mini;
    gint formulaire_toujours_affiche;
    gint alerte_permission;   /* à un si le message d'alerte s'affiche */
    gint fichier_deja_ouvert;   /* à un si lors de l'ouverture, le fichier semblait déjà ouvert */
    gint force_enregistrement;    /* à un si on force l'enregistrement */
    gint show_transaction_selected_in_form;	/* TRUE will show the selected transaction in the form */
    gint affichage_exercice_automatique;   /* à 1 si exercice
					    * automatique selon la date, 0 si affiche le dernier choisi */
    gboolean limit_completion_to_current_account;   /** Limit payee *
						     completion to
						     current account,
						     or do a full
						     search. */
    gint crypt_file;   /* TRUE if we want to crypt the file */
    gint compress_file;  /* TRUE if we want to compress the grisbi file */
    gint compress_backup;  /* TRUE if we want to compress the backup */
    gint full_screen;		/* 1 to full screen, 0 else */

    /* archive stuff */
    gint check_for_archival; /* TRUE if we want to check the number of non archived transactions at the opening */
    gint max_non_archived_transactions_for_check; /* the max number of transaction before grisbi warm at the opening */

    gint formulaire_distinct_par_compte;  /* à 1 si le formulaire est différent pour chaque compte */
    gint affiche_boutons_valider_annuler;
    gint affiche_nb_ecritures_listes;
    gint largeur_auto_colonnes;
    gint retient_affichage_par_compte;   /* à 1 si les caractéristiques de l'affichage (R, non R ...) diffèrent par compte */
    gint en_train_de_sauvegarder;
    gint en_train_de_charger;
    gint utilise_logo;

    gint utilise_fonte_listes;		/* TRUE to use a custom font for the lists */
    gchar *font_string;			/* contain the description of the font, or NULL */
    
    gint display_toolbar;	/** Display mode of toolbar. */
    gboolean show_toolbar;		/** Show toolbar or not.  */
    gboolean show_headings_bar;	/** Show headings bar or not. */
    gboolean show_closed_accounts;

    gboolean automatic_separator; /* TRUE if do automatic separator */
    /* Various display message stuff  */
    gint display_message_lock_active;
    gint display_message_file_readable;
    gint display_message_minimum_alert;
    gint display_message_no_budgetary_line;
    gint display_message_qif_export_currency;
    gint display_message_ofx_security;

    /* combofix configuration */
    gint combofix_mixed_sort;  /* TRUE for no separation between the categories */
    gint combofix_max_item;    /* maximum number of items we want before showing the popup */
    gint combofix_case_sensitive;  /* TRUE if case sensitive */
    gint combofix_enter_select_completion; /* TRUE if enter close the popup and keep what is in the entry (else, select the current item in the list) */
    gint combofix_force_payee;   /* TRUE if no new item can be appended in the payee combofix */
    gint combofix_force_category;   /* TRUE if no new item can be appended in the category and budget combofix */
    
    /* Various commands */
    gchar * browser_command;
    gchar * latex_command;
    gchar * dvips_command;

    /* Print stuff */
    struct print_config print_config;
    
    /*     largeur des panned */
    gint largeur_colonne_comptes_operation;
    gint largeur_colonne_echeancier;
    gint largeur_colonne_comptes_comptes;
    gint largeur_colonne_etat;

    /* variables sur l'échéancier */
    gint affichage_commentaire_echeancier;     /* à 1 si le commentaire est affiché */

    /* Tips */
    gint last_tip; 
    gint show_tip; 

    gchar * csv_separator;	/** CSV separator to use while parsing
				 * a CSV file. */
    gboolean csv_skipped_lines [ CSV_MAX_TOP_LINES ]; /** Contains a
						       * pointer to
						       * skipped lines
						       * in CSV
						       * preview. */
} etat;



enum alignement {
    LEFT, CENTER, RIGHT,
};

#endif
