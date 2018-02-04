#ifndef _STRUCTURES_H
#define _STRUCTURES_H

#include <glib.h>
#include <gtk/gtk.h>

/* fichier d'en tête structures.h */
/* contient presque toutes les structures du prog */


/* the VERSION_FICHIER is in fact the lowest version which can be used to open the
 * file, so each time we make an incompatibility issue, VERSION_FICHIER must be set
 * to VERSION_GRISBI */
#define VERSION_FICHIER         "1.2.0"
#define OLD_VERSION_FICHIER		"0.6.0"

/* Define this when in Git development branch. */
#define IS_DEVELOPMENT_VERSION  1

#define VERSION_FICHIER_ETAT    "0.6.0"
#define VERSION_FICHIER_CATEG   "0.6.0"
#define VERSION_FICHIER_IB      "0.6.0"

#define CSV_MAX_TOP_LINES       10	                /** How many lines to show in CSV preview.  */
#define GSB_EPSILON             0.0000005           /* Sert à comparer des doubles */
#define GSB_MAX_SPIN_BUTTON     100000000000000.0   /* Dimensionne la largeur naturelle maxi des spin_button */
#ifdef __APPLE__
#define ETAT_WWW_BROWSER        "open"          /* définit le browser par défaut */
#else
#define ETAT_WWW_BROWSER        "xdg-open"          /* définit le browser par défaut */
#endif

/* Sorting option for the transactions. Used in gsb_transactions_list_display_sort_changed () */
#define PRIMARY_SORT            0
#define SECONDARY_SORT          1

/* margin for widgets */
#define MARGIN_BOX				 5					/* spacing for gtk_box_new () */
#define MARGIN_END              10
#define MARGIN_START            10
#define MARGIN_TOP              10
#define MARGIN_BOTTOM           10
#define MARGIN_PADDING_BOX      15

/* width for widget */
#define BOX_BORDER_WIDTH		10
#define ENTRY_MIN_WIDTH_1		100

/* constants for Preferences */
#define PREFS_MIN_WIN_HEIGHT	600				/* Hauteur Min de la fenetre des preferences */
#define PREFS_MIN_WIN_WIDTH		800				/* Largeur Min de la fenetre des preferences */
#define PREFS_MIN_PANED_WIDTH	120				/* Largeur Min du panel gauche des preferences */
#define SW_COEFF_UTIL_PG        90             	/* Scrolled Window in paddinggrid in prefs */
#define SW_COEFF_UTIL_SW        93				/* Scrolled Window in prefs */
#define SW_MAX_CONTENT_WIDTH	600				/* Since Gtk-3.22 */
#define SW_MIN_HEIGHT			150

/* used for gtk_tree_view_column_set_alignment() */
#define COLUMN_LEFT 0.0
#define COLUMN_CENTER 0.5
#define COLUMN_RIGHT 1.0

/* valeurs pour le logo de grisbi */
#define LOGO_WIDTH 48
#define LOGO_HEIGHT 48

/* variables initialisées lors de l'ouverture de grisbi par gsettings */
/* declared in grisbi_app.c */
extern struct GrisbiAppConf conf;

/* variables initialisées lors de l'exécution de grisbi PROVISOIRE */
/* declared in grisbi_win.c */
extern struct _GrisbiWinRun run;
extern struct _GrisbiWinEtat etat;

typedef enum 	_GsbOfxTransactionType	GsbOfxTransactionType;
typedef enum 	_GsbTitleType	 		GsbTitleType;
typedef enum 	_SettingsSchema 		SettingsSchema;

typedef struct _GrisbiWinEtat			GrisbiWinEtat;
typedef struct _GrisbiWinRun			GrisbiWinRun;

/** structure etat
 * variables contenant juste 0 ou 1
 * FIXME : scinder cette structure en 3 parties :
 * la partie configurée par le fichier
 * la partie configurée par la conf
 * la partie configurée pendant le fonctionnement de grisbi
 * */
struct _GrisbiWinEtat
{
    gint is_archive;                /** TRUE if the file is an archive, FALSE else */

    gboolean debug_mode;            /* TRUE in debug mode, FALSE for normale mode */

	/* variables generales */
    gchar *		accounting_entity;
	gchar *		adr_common;
	gchar *		adr_secondary;

	/* files and backup part */
	gint crypt_file;                /* TRUE if we want to crypt the file */
    gint fichier_deja_ouvert;       /* à un si lors de l'ouverture, le fichier semblait déjà ouvert */

    /* devises pour les catégories, imputations budgétaires et les tiers */
    gint no_devise_totaux_categ;
    gint no_devise_totaux_ib;
    gint no_devise_totaux_tiers;

    /* reconciliation */
    gint reconcile_end_date;        /* Date initiale + 1 mois par défaut */
    gboolean reconcile_sort;        /* TRUE = Sort by descending date the reconciliations */

    /* formulaire */
    gint affiche_nb_ecritures_listes;
    gint retient_affichage_par_compte;      /* à 1 si les caractéristiques de l'affichage (R, non R ...) diffèrent par compte */

    /* Fonts & logo */
    gint utilise_logo;
    gchar *name_logo;

    gboolean automatic_separator;   /* TRUE if do automatic separator */

    /* import files */
    gint extract_number_for_check;      	/* TRUE if Extracting a number and save it in the field No Cheque/Virement */
    gint copy_payee_in_note;            	/* TRUE si recopie le tiers dans les notes FALSE par défaut */
    gint fusion_import_transactions;    	/* TRUE if merge transactions imported with transactions found*/
    gint associate_categorie_for_payee;		/* TRUE to automatically retrieve the category of the payee if it exists */
    gint get_fyear_by_value_date;           /* TRUE to get the fyear by value date, FALSE by date */
	gint import_files_nb_days;				/* Number of days for search transactions */

    gchar * csv_separator;                              /* CSV separator to use while parsing a CSV file. */
    gboolean csv_skipped_lines [ CSV_MAX_TOP_LINES ];   /* Contains a pointer to skipped lines in CSV preview. */
	gboolean csv_force_date_valeur_with_date;			/* force la date de valeur si non présente dans le fichier */
    gint qif_use_field_extract_method_payment;      	/* use the field 'N' to define the method of payment */

    /* export files */
    gint export_file_format;                /* EXPORT_QIF or EXPORT_CSV */
    gboolean export_files_traitement;       /* 0 = traitement individuel, 1 = traitement automatique */
	gboolean	export_force_US_dates;		/* 0 = format interne 1 = format US */
	gboolean	export_force_US_numbers;	/* 0 = format interne 1 = format US */

    /* combofix configuration */
    gint combofix_mixed_sort;               /* TRUE for no separation between the categories */
    gint combofix_max_item;                 /* maximum number of items we want before showing the popup */
    gint combofix_case_sensitive;           /* TRUE if case sensitive */
    gint combofix_enter_select_completion;  /* TRUE if enter close the popup and keep what is in the entry (else, select the current item in the list) */
    gint combofix_force_payee;              /* TRUE if no new item can be appended in the payee combofix */
    gint combofix_force_category;           /* TRUE if no new item can be appended in the category and budget combofix */

    /* width panned */
    gint largeur_colonne_echeancier;
    gint largeur_colonne_comptes_comptes;
    gint largeur_colonne_etat;

    /* variables sur l'échéancier */
    gint affichage_commentaire_echeancier;      /* à 1 si le commentaire est affiché */
    gint affichage_echeances;                   /* affichage de la période affichée dans la vue échéancier */
    gint affichage_echeances_perso_nb_libre;    /* nombre de périodicité des échéances personnalisées */
    gint affichage_echeances_perso_j_m_a;       /* type de périodicité des échéances personnalisées */
	gboolean scheduler_set_default_account;		/* set default account for the scheduled form */
	gint scheduler_default_account_number;		/* default account number for the scheduled form */
	gboolean scheduler_set_fixed_date;			/* set fixed date for the scheduled form */
	gint scheduler_default_fixed_date;			/* default fixed date for the scheduled form */

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

    /* largeur des colonnes */
    gchar *transaction_column_width;
    gchar *scheduler_column_width;
};

/** structure conf
 * variables containing just 0 or 1
 * configured by the file grisbi.conf
 *
 */
struct GrisbiAppConf
{
/* root part*/
    gint        first_use;                                  /* première utilisation ou réinitialisation de grisbi */
    gboolean    force_classic_menu;                         /* TRUE force classic menu */

/* backup part */
    gboolean    compress_backup;                            /* TRUE if we want to compress the backup */
    gboolean    make_backup;                                /* TRUE for create a backup when save file */
    gboolean    make_backup_every_minutes;                  /* TRUE to make backup every x mn */
    gint        make_backup_nb_minutes;                     /* the number of minutes we want to make a backup */
    gboolean    make_bakup_single_file;                     /* TRUE if a single backup file */
    gboolean    sauvegarde_demarrage;                       /* utilisé pour enregistrer le fichier s'il s'est bien ouvert */

/* display part */
    gint        display_window_title;                       /* selection du titre principal de grisbi */
    gboolean    display_toolbar;                            /* Display mode of toolbar. */
    gboolean    formulaire_toujours_affiche;                /* TRUE formulaire toujours affiché */
    gboolean    group_partial_balance_under_accounts;       /* TRUE = in home page group the partial balance with accounts */
    gboolean    show_headings_bar;                          /* Show headings bar or not. */
    gboolean    show_closed_accounts;

/* files part */
    gboolean    compress_file;                              /* TRUE if we want to compress the Grisbi file */
    gint        dernier_fichier_auto;                       /* chargement du dernier fichier utilisé */
    gboolean    force_enregistrement;                       /* à un si on force l'enregistrement */
    gchar *     last_open_file;                             /* dernier fichier ouvert */
    gint        nb_max_derniers_fichiers_ouverts;           /* contient le nb max que peut contenir nb_derniers_fichiers_ouverts */
    gint        nb_derniers_fichiers_ouverts;               /* contient le nb de derniers fichiers ouverts */
    gboolean    sauvegarde_auto;                            /* utilisé pour enregistrer le fichier automatiquementà la fermeture */
	gboolean	force_import_directory;						/* force l'utilisation d'un répertoire pour l'importation des fichiers */
	gchar *     import_directory;                          	/* répertoire par défaut pour l'importation des fichiers */

/* archive stuff */
    gboolean    archives_check_auto;                  		/* TRUE if we want to check the number of non archived transactions at the opening */
    gint        max_non_archived_transactions_for_check;    /* the max number of transaction before grisbi warm at the opening */

/* form part*/
    gint        affichage_exercice_automatique;             /* automatic fyear :0 to set according to the date, 1 according to value date */
    gint        automatic_completion_payee;                 /* 1 pour autoriser la completion automatique des opérations */
    gboolean    automatic_erase_credit_debit;               /* 1 pour effacer les champs crédit et débit */
    gboolean    automatic_recover_splits;                   /* 1 pour recréer automatiquement les sous opérations */
    gboolean    entree;                                     /* si conf.entree = 1, la touche entrée finit l'opération */
	gint 		form_expander_label_width;					/* largeur du label de l'expander du formulaire */
    gint        fyear_combobox_sort_order;                  /* fyear combobox type sort, GTK_SORT_ASCENDING by default */
    gboolean    limit_completion_to_current_account;        /* Limit payee completion to current account or do a full search. */

/* general part */
    gchar *     browser_command;
    gint        custom_fonte_listes;                        /* TRUE to use a custom font for the lists */
    gchar *     font_string;                                /* contain the description of the font, or NULL */
	gchar *		language_chosen;							/* choix de la langue : NULL par défaut = langue système */

    /* Home page */
    gint        pluriel_final;                              /* 0 = finals 1 = finaux */

    /* variables for the list of categories */
    gint        metatree_action_2button_press;              /* 0 default gtk, 1 edit_function, 2 manage division if possible */

    /* variables for the list of transactions */
    gboolean    show_transaction_gives_balance;             /* TRUE si on visualise l'opération qui donne le solde du jour */
    gboolean    show_transaction_selected_in_form;          /* TRUE will show the selected transaction in the form */
    gboolean    transactions_list_primary_sorting;          /* Primary sorting option for the transactions */
    gboolean    transactions_list_secondary_sorting;        /* Secondary sorting option for the transactions */

/* geometry part */
    gint        x_position;                                 /* main_window x position */
    gint        y_position;                                 /* main_window y position */
    gint        main_width;                                 /* main_window width */
    gint        main_height;                                /* main_window height */
    gboolean    full_screen;                                /* TRUE to full screen, 0 else */
    gboolean    maximize_screen;                            /* TRUE to maximize screen, 0 else */

/* panel part */
    gboolean 	active_scrolling_left_pane;                 /* active mouse scrolling in the left_pane. */
    gint        panel_width;                                /* navigation paned width */

/* prefs part */
	gint		prefs_archives_sort_order;					/* archive sort order, GTK_SORT_ASCENDING by default */
    gint        prefs_fyear_sort_order;                     /* fyear type sort, GTK_SORT_ASCENDING by default */
	gint		prefs_height;								/* preferences height */
	gint		prefs_panel_width;							/* preferences paned width */
    gint        prefs_width;                                /* preferences width */

/* scheduled part */
    gboolean    balances_with_scheduled;                    /* TRUE = the balance incorporates the scheduled operations */
    gboolean    execute_scheduled_of_month;                 /* warn/execute scheduled at expiration (FALSE) or of the month (TRUE) */
	gint		nb_days_before_scheduled;					/* nombre de jours avant relance des opérations planifiées */
	gboolean	scheduler_set_fixed_day;					/* Si coché le nombre est considéré comme un jour fixe */
	gint		scheduler_fixed_day;

    /* Tips */
    gint        last_tip;
    gboolean    show_tip;
};

/** structure run
 * variables containing just 0 or 1
 * générées pendant l'exécution du programme
 *
 */

struct _GrisbiWinRun
{
    /* main window of grisbi */
    GtkWidget *window;

    /* old version of file */
    gboolean old_version;           /* TRUE if file version < 0.6 */

    /* initialisation variables */
    gboolean new_crypted_file;      /* 0 à l'exécution de init_variables () 1 si new crypted file*/

    /* file stuff */
    time_t file_modification;
    gboolean is_saving;
    gboolean is_loading;

    /* reconciliation */
    gint equilibrage;
    gint reconcile_account_number;  /* Save the last reconciliation try: account */
    gchar *reconcile_final_balance; /* final balance amount */
    GDate *reconcile_new_date;      /* new date */

    /* home page - accueil */
    /* ces 5 variables sont mises à 1 lorsqu'il est nécessaire de rafraichir cette */
    /* partie la prochaine fois qu'on va sur l'accueil */

    gboolean mise_a_jour_liste_comptes_accueil;
    gboolean mise_a_jour_liste_echeances_manuelles_accueil;
    gboolean mise_a_jour_liste_echeances_auto_accueil;
    gboolean mise_a_jour_soldes_minimaux;
    gboolean mise_a_jour_fin_comptes_passifs;
};

/* définition du titre de grisbi */
enum _GsbTitleType
{
    GSB_ACCOUNT_ENTITY,
    GSB_ACCOUNT_HOLDER,
    GSB_ACCOUNT_FILENAME,
};


/* définition de l'alignement */

/* definition of the columns of model for the left panel  */
enum LeftPanelTreeColumns
{
    LEFT_PANEL_TREE_TEXT_COLUMN,
    LEFT_PANEL_TREE_PAGE_COLUMN,
    LEFT_PANEL_TREE_BOLD_COLUMN,
    LEFT_PANEL_TREE_ITALIC_COLUMN,
    LEFT_PANEL_TREE_NUM_COLUMNS,
};


/* définition du type d'origine pour les données du module budgétaire */
enum BetArrayOriginData
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
enum BetTypeMaj
{
    BET_MAJ_FALSE = 0,
    BET_MAJ_ESTIMATE,
    BET_MAJ_HISTORICAL,
    BET_MAJ_FINANCIAL,
    BET_MAJ_ALL,
};


/* recopie des types de transaction de la libofx en attendant une version propre */
enum _GsbOfxTransactionType
{
    GSB_OFX_CREDIT,     /**< Generic credit */
    GSB_OFX_DEBIT,      /**< Generic debit */
    GSB_OFX_INT,        /**< Interest earned or paid (Note: Depends on signage of amount) */
    GSB_OFX_DIV,        /**< Dividend */
    GSB_OFX_FEE,        /**< FI fee */
    GSB_OFX_SRVCHG,     /**< Service charge */
    GSB_OFX_DEP,        /**< Deposit */
    GSB_OFX_ATM,        /**< ATM debit or credit (Note: Depends on signage of amount) */
    GSB_OFX_POS,        /**< Point of sale debit or credit (Note: Depends on signage of amount) */
    GSB_OFX_XFER,       /**< Transfer */
    GSB_OFX_CHECK,      /**< Check */
    GSB_OFX_PAYMENT,    /**< Electronic payment */
    GSB_OFX_CASH,       /**< Cash withdrawal */
    GSB_OFX_DIRECTDEP,  /**< Direct deposit */
    GSB_OFX_DIRECTDEBIT,/**< Merchant initiated debit */
    GSB_OFX_REPEATPMT,  /**< Repeating payment/standing order */
    GSB_OFX_OTHER       /**< Somer other type of transaction */
  };


enum _SettingsSchema
{
    SETTINGS_ROOT = 0,
    SETTINGS_FILES_BACKUP,
    SETTINGS_DISPLAY,
    SETTINGS_FILES_FILE,
    SETTINGS_FORM,
    SETTINGS_GENERAL,
    SETTINGS_GEOMETRY,
    SETTINGS_MESSAGES_DELETE,
    SETTINGS_MESSAGES_TIPS,
    SETTINGS_MESSAGES_WARNINGS,
    SETTINGS_PANEL,
    SETTINGS_PREFS,
    SETTINGS_SCHEDULED
};

#endif
