#ifndef _STRUCTURES_H
#define _STRUCTURES_H

#include <glib.h>
#include <gtk/gtk.h>

/* fichier d'en tête structures.h */
/* contient presque toutes les structures et enum du prog */


/* the VERSION_FICHIER is in fact the lowest version which can be used to open the */
/* file, so each time we make an incompatibility issue, VERSION_FICHIER must be set */
/* to VERSION_GRISBI */
#define VERSION_FICHIER         "2.0.0"
#define OLD_VERSION_FICHIER		"0.6.0"

#define VERSION_FICHIER_ETAT    "0.6.0"
#define VERSION_FICHIER_CATEG   "0.6.0"
#define VERSION_FICHIER_IB      "0.6.0"

#define CSV_MAX_TOP_LINES       10	                /** How many lines to show in CSV preview.  */
#define GSB_EPSILON             0.0000005           /* Sert à comparer des doubles */
#define GSB_MAX_SPIN_BUTTON     100000000000000.0   /* Dimensionne la largeur naturelle maxi des spin_button */
#ifdef __APPLE__
#define ETAT_WWW_BROWSER        "open"				/* définit le browser par défaut */
#else
#define ETAT_WWW_BROWSER        "xdg-open"          /* définit le browser par défaut */
#endif

#define ETATS_MAX_OPES			3000				/* Nombre d'opérations sélectionnées avant avertissement */

/* Nbre de messages de delete et de warnings */
#define NBRE_MSG_WARNINGS		9
#define NBRE_MSG_DELETE			7

/* Sorting option for the transactions. Used in gsb_transactions_list_display_sort_changed () */
#define PRIMARY_SORT            0
#define SECONDARY_SORT          1

/* definitions pour le basculement en basse résolution */
#define LOW_DEF_HEIGHT_HIGH		900
#define LOW_DEF_WIDTH_HIGH		1600

/* Dimensions min et position de la fenetre de l'application */
#define WIN_MIN_HEIGHT			700
#define WIN_MIN_WIDTH			1030
#define WIN_POS_X				10
#define WIN_POS_Y				10

/* Largeur min du panel de gauche de la fenetre principale */
#define PANEL_MIN_WIDTH			250

/* margin for widgets */
#define MARGIN_BOX				 5				/* spacing for gtk_box_new () */
#define MARGIN_END              10
#define MARGIN_START            10
#define MARGIN_TOP              10
#define MARGIN_BOTTOM           10
#define MARGIN_PADDING_BOX      15

/* width for widget */
#define BOX_BORDER_WIDTH		10
#define ENTRY_MIN_WIDTH_1		100
#define FORM_COURT_WIDTH		120				/* largeur minimale demandée pour les champs courts du formulaire */
#define FORM_LONG_WIDTH			200				/* largeur minimale demandée pour les champs longs du formulaire */

/* constants for Preferences */
#define PREFS_PANED_MIN_WIDTH	235				/* Largeur Min du panel gauche des preferences */
#define PREFS_WIN_MIN_HEIGHT	605				/* Hauteur Min de la fenetre des preferences */
#define PREFS_WIN_MIN_WIDTH		910				/* Largeur Min de la fenetre des preferences */
#define SW_MAX_CONTENT_WIDTH	600				/* Since Gtk-3.22 */
#define SW_MIN_HEIGHT			150

/* used for gtk_tree_view_column_set_alignment() */
#define COLUMN_LEFT				0.0
#define COLUMN_CENTER			0.5
#define COLUMN_RIGHT			1.0

/* used for labels alignment */
#define GSB_LEFT 				0
#define GSB_CENTER 				0.5
#define GSB_RIGHT 				1
#define GSB_TOP 				0
#define GSB_BOTTOM 				1

/* valeurs pour le logo de grisbi */
#define LOGO_WIDTH 				48
#define LOGO_HEIGHT 			48

 /* définitions pour les noms de fichier dans les boutons de la vue accueil */
#define GSB_NBRE_CHAR_TRUNC		15
#define GSB_NBRE_LIGNES_BOUTON	 6

/* troncature des titres de colonnes pour l'affichage de la composition */
#define	TRUNC_FORM_FIELD		20 		/* du formulaire */
#define TRUNC_LIST_COL_NAME		 8		/* de la liste des ope */

/* définitions pour les tableaux des transactions */
#define CUSTOM_MODEL_VISIBLE_COLUMNS	7		/* number of visible columns */
#define TRANSACTION_LIST_ROWS_NB 		4		/* definition of the number of max rows for a line, for now limit to 4 */

/*START_EXTERN*/
/* variables initialisées lors de l'exécution de grisbi PROVISOIRE */
extern struct _GrisbiWinRun run;				/* declared in grisbi_win.c */
extern struct _GrisbiWinEtat etat;				/* declared in grisbi_win.c */
/*END_EXTERN*/

/******************************************************************************/
/* Structures                                                                 */
/******************************************************************************/
typedef struct _GrisbiAppConf			GrisbiAppConf;
typedef struct _GrisbiWinEtat			GrisbiWinEtat;
typedef struct _GrisbiWinRun			GrisbiWinRun;

/* structure etat variables configurées par le fichier de comptes */
struct _GrisbiWinEtat
{
	/* variables generales */
    gchar *		accounting_entity;
	gchar *		adr_common;
	gchar *		adr_secondary;
	gchar *		date_format;								/* format local d'affichage des dates */
    gint		is_archive;									/** TRUE if the file is an archive, FALSE else */

	/* files and backup part */
	gint 		crypt_file;									/* TRUE if we want to crypt the file */
    gint 		fichier_deja_ouvert;						/* à un si lors de l'ouverture, le fichier semblait déjà ouvert */
	gboolean	use_icons_file_dir;							/* TRUE = icons in accounts file directory */

    /* devises pour les catégories, imputations budgétaires et les tiers */
    gint		no_devise_totaux_categ;
    gint		no_devise_totaux_ib;
    gint		no_devise_totaux_tiers;

    /* reconciliation */
    gint		reconcile_end_date;							/* Date initiale + 1 mois par défaut */
    gboolean	reconcile_sort;								/* TRUE = Sort by descending date the reconciliations */

    /* formulaire */
    gint		affiche_nb_ecritures_listes;
    gint		retient_affichage_par_compte;				/* à 1 si les caractéristiques de l'affichage (R, non R ...) diffèrent par compte */
	gboolean	form_date_force_prev_year;					/* FALSE = default année en cours TRUE = forcer l'année précédente pour les dates futures */

    /* Fonts & logo */
    gint		utilise_logo;
    gchar		*name_logo;

    gboolean	automatic_separator;   						/* TRUE if do automatic separator */

    /* import files */
    gint		extract_number_for_check;      				/* TRUE if Extracting a number and save it in the field No Cheque/Virement */
    gint		copy_payee_in_note;            				/* TRUE si recopie le tiers dans les notes FALSE par défaut */
    gint		fusion_import_transactions;    				/* TRUE if merge transactions imported with transactions found*/
    gint		associate_categorie_for_payee;				/* TRUE to automatically retrieve the category of the payee if it exists */
    gint		get_fyear_by_value_date;					/* TRUE to get the fyear by value date, FALSE by date */
	gint		import_files_nb_days;						/* Number of days for search transactions */

    gchar *		csv_separator;                              /* CSV separator to use while parsing a CSV file. */
    gboolean	csv_skipped_lines [ CSV_MAX_TOP_LINES ];	/* Contains a pointer to skipped lines in CSV preview. */
	gboolean	csv_force_date_valeur_with_date;			/* force la date de valeur si non présente dans le fichier */
    gboolean	qif_no_import_categories;					/* TRUE if no new item can be appended in the categories divisions */
    gint		qif_use_field_extract_method_payment;      	/* use the field 'N' to define the method of payment */

    /* export files */
    gint		export_file_format;							/* EXPORT_QIF or EXPORT_CSV */
    gboolean	export_files_traitement;					/* 0 = traitement individuel, 1 = traitement automatique */
	gboolean	export_force_US_dates;						/* 0 = format interne 1 = format US */
	gboolean	export_force_US_numbers;					/* 0 = format interne 1 = format US */
	gboolean	export_quote_dates;							/* "cite les dates" TRUE par défaut */

    /* combofix configuration */
    gint		combofix_mixed_sort;						/* TRUE for no separation between the categories */
    gint		combofix_case_sensitive;					/* TRUE if case sensitive */
    gint		combofix_force_payee;						/* TRUE if no new item can be appended in the payee combofix */
    gint		combofix_force_category;					/* TRUE if no new item can be appended in the category and budget combofix */

    /* width panned */
    gint		largeur_colonne_echeancier;
    gint		largeur_colonne_comptes_comptes;
    gint		largeur_colonne_etat;

    /* variables sur l'échéancier */
    gint		affichage_commentaire_echeancier;			/* à 1 si le commentaire est affiché */
    gint		affichage_echeances;						/* affichage de la période affichée dans la vue échéancier */
    gint		affichage_echeances_perso_nb_libre;			/* nombre de périodicité des échéances personnalisées */
    gint		affichage_echeances_perso_j_m_a;			/* type de périodicité des échéances personnalisées */
	gboolean	scheduler_set_default_account;				/* set default account for the scheduled form */
	gint		scheduler_default_account_number;			/* default account number for the scheduled form */
	gboolean	scheduler_set_fixed_date;					/* set fixed date for the scheduled transaction */
	gint		scheduler_set_fixed_date_day;				/* day of fixed date for the scheduled transaction (28, 29, 30 or 31) */

    /* variables pour les metatree */
    gint		metatree_add_archive_in_totals;				/* Add transactions archived in the totals */
	gboolean	metatree_assoc_mode;						/* Remplace Revenus/Dépenses par Produits/Charges */
    gint		metatree_sort_transactions;					/* 1 = sorting by increasing date 2 = Sort by date descending */
	gboolean	metatree_unarchived_payees;					/* TRUE = limit the list of payees for combofix */

    /* variables for the module estimate balance */
    gint		bet_debut_period;
    gint		bet_cash_account_option;				/* ajoute l'onglet prévision aux comptes de caisse */

    /* variables pour le simulateur de crédits */
    gdouble		bet_capital;
    gint		bet_currency;
    gdouble		bet_taux_annuel;
    gint		bet_index_duree;
    gdouble		bet_frais;
    gint		bet_type_taux;

    /* largeur des colonnes */
    gchar *		transaction_column_width;
    gchar *		scheduler_column_width;
};

/* structure conf variables configured by gsettings or grisbi.conf */
struct _GrisbiAppConf
{
/* root part*/
    gboolean	first_use;                                  /* TRUE = première utilisation de grisbi */

/* backup part */
    gboolean    compress_backup;                            /* TRUE if we want to compress the backup */
    gboolean    make_backup_every_minutes;                  /* TRUE to make backup every x mn */
    gint        make_backup_nb_minutes;                     /* the number of minutes we want to make a backup */
    gboolean    make_bakup_single_file;                     /* TRUE if a single backup file */
    gboolean    sauvegarde_demarrage;                       /* utilisé pour enregistrer le fichier s'il s'est bien ouvert */
	gboolean	sauvegarde_fermeture;						/* TRUE to create a backup when save file */
	gboolean	remove_backup_files;						/* removing automatically backup files */
	guint		remove_backup_months;						/* Number of months before removing backup files */

/* display part */
    gint        display_window_title;                       /* selection du titre principal de grisbi 1,2 ou 3*/
    gint		display_toolbar;                            /* Display mode of toolbar: GtkToolbarStyle */
    gboolean    formulaire_toujours_affiche;                /* TRUE formulaire toujours affiché */
    gboolean    group_partial_balance_under_accounts;       /* TRUE = in home page group the partial balance with accounts */
    gboolean    show_headings_bar;                          /* Show headings bar or not. */
    gboolean    show_closed_accounts;

/* files part */
    gboolean    compress_file;                              /* TRUE if we want to compress the Grisbi file */
    gboolean	dernier_fichier_auto;                       /* chargement du dernier fichier utilisé */
    gboolean    force_enregistrement;                       /* à un si on force l'enregistrement */
    gchar *     last_open_file;                             /* dernier fichier ouvert */
    gint        nb_max_derniers_fichiers_ouverts;           /* contient le nb max que peut contenir nb_derniers_fichiers_ouverts */
    gint        nb_derniers_fichiers_ouverts;               /* contient le nb de derniers fichiers ouverts */
    gboolean    sauvegarde_auto;                            /* utilisé pour enregistrer le fichier automatiquementà la fermeture */
	gboolean	force_import_directory;						/* force l'utilisation d'un répertoire pour l'importation des fichiers */
	gchar *     import_directory;                          	/* répertoire par défaut pour l'importation des fichiers */
	gboolean	import_remove_file;							/* remove file after import */

/* archive stuff */
    gboolean    archives_check_auto;                  		/* TRUE if we want to check the number of non archived transactions at the opening */
    gint        max_non_archived_transactions_for_check;    /* the max number of transaction before grisbi warm at the opening */

/* form part*/
    gboolean    affichage_exercice_automatique;             /* automatic fyear :FALSE = according to the date, TRUE = according to value date */
    gboolean    automatic_completion_payee;                 /* TRUE pour autoriser la completion automatique des opérations */
    gboolean    automatic_erase_credit_debit;               /* TRUE pour effacer les champs crédit et débit */
    gboolean    automatic_recover_splits;                   /* TRUE pour recréer automatiquement les sous opérations */
	gint		completion_minimum_key_length;				/* Minimum length of the search key in characters */
    gboolean    form_enter_key;								/* si TRUE, la touche entrée finit l'opération */
	gboolean	form_validate_split;						/* FALSE select a new transaction, TRUE select the mother transaction */
    gint        fyear_combobox_sort_order;                  /* fyear combobox type sort, GTK_SORT_ASCENDING by default */
    gboolean    limit_completion_to_current_account;        /* Limit payee completion to current account or do a full search. */

/* general part */
    gchar *     browser_command;
    gboolean    custom_fonte_listes;                        /* TRUE to use a custom font for the lists */
    gchar *     font_string;                                /* contain the description of the font, or NULL */
	gchar *		language_chosen;							/* choix de la langue : NULL par défaut = langue système */
	gchar *		current_theme;
	gint		force_type_theme;							/* 0 = auto, 1 = standard 2 = dark theme 3 = light theme */

    /* Home page */
    gboolean	pluriel_final;                              /* FALSE = finals TRUE = finaux */

    /* variables for the list of categories */
    gint        metatree_action_2button_press;              /* 0 default gtk, 1 edit_function, 2 manage division if possible */

	/* variables for the list of transactions */
    gboolean    show_transaction_gives_balance;             /* TRUE si on visualise l'opération qui donne le solde du jour */
    gboolean    show_transaction_selected_in_form;          /* TRUE will show the selected transaction in the form */
    gint		transactions_list_primary_sorting;          /* Primary sorting option for the transactions */
    gint    	transactions_list_secondary_sorting;        /* Secondary sorting option for the transactions */

/* geometry part */
    gint        x_position;                                 /* main_window x position */
    gint        y_position;                                 /* main_window y position */
    gint        main_width;                                 /* main_window width */
    gint        main_height;                                /* main_window height */
	gboolean	low_definition_screen;						/* TRUE use a low resolution screen */
    gboolean    full_screen;                                /* TRUE to full screen, 0 else */
    gboolean    maximize_screen;                            /* TRUE to maximize screen, 0 else */

/* panel part */
    gboolean 	active_scrolling_left_pane;                 /* TRUE active mouse scrolling in the left_pane. */
    gint        panel_width;                                /* navigation paned width */

/* prefs part */
	gint		prefs_archives_sort_order;					/* archive sort order, GTK_SORT_ASCENDING by default */
    gint		prefs_fyear_sort_order;						/* fyear type sort, GTK_SORT_ASCENDING by default */
	gint		prefs_height;								/* preferences height */
	gint		prefs_panel_width;							/* preferences paned width */
    gint        prefs_width;                                /* preferences width */

/* scheduled part */
    gboolean    balances_with_scheduled;                    /* TRUE = the balance incorporates the scheduled operations */
    gboolean    execute_scheduled_of_month;                 /* warn/execute scheduled at expiration (FALSE) or of the month (TRUE) */
	gboolean	last_selected_scheduler;					/* TRUE = last scheduler selected in list = default */
	gint		nb_days_before_scheduled;					/* nombre de jours avant relance des opérations planifiées */
	gboolean	scheduler_set_fixed_day;					/* Si TRUE le nombre est considéré comme un jour fixe */
	gint		scheduler_fixed_day;

/* Tips */
    gint        last_tip;
    gboolean    show_tip;

	/* NOT SAVED */
	/* general part */
	gint		use_type_theme;								/* 1 = standard 2 = dark theme 3 = light theme */

	/* CSS data */
	gint		prefs_change_css_data;						/* compteur de modification d'une règle CSS */
	gboolean	use_css_local_file;							/* TRUE si utilisation du fichier css local */
};

/* structure run
 * variables containing just 0 or 1
 * générées pendant l'exécution du programme
 *
 */
struct _GrisbiWinRun
{
	gboolean	definition_screen_toggled;					/* force l'utilisation des dimensions par defaut */

	/* old version of file */
    gboolean	old_version;								/* TRUE if file version < 0.6 */
	gboolean	account_number_is_0;						/* TRUE si an account has 0 for number */

    /* initialisation variables */
    gboolean	new_crypted_file;							/* 0 à l'exécution de init_variables () 1 si new crypted file*/
	gboolean	new_account_file;							/* 0 à l'exécution de init_variables () 1 si new account file */

	/* backup */
	gboolean	remove_backup_files;						/* on ne fait cette sauvegarde qu'une fois par session */

    /* file stuff */
    time_t		file_modification;
    gboolean	file_is_saving;
    gboolean	file_is_loading;
    gboolean	menu_save;

    /* reconciliation */
    gint		equilibrage;
    gint		reconcile_account_number;					/* Save the last reconciliation try: account */
    gchar *		reconcile_final_balance;					/* final balance amount */
    GDate *		reconcile_new_date;							/* new date */

    /* home page - accueil */
    /* ces 5 variables sont mises à 1 lorsqu'il est nécessaire de rafraichir cette */
    /* partie la prochaine fois qu'on va sur l'accueil */

    gboolean	mise_a_jour_liste_comptes_accueil;
    gboolean	mise_a_jour_liste_echeances_manuelles_accueil;
    gboolean	mise_a_jour_liste_echeances_auto_accueil;
    gboolean	mise_a_jour_soldes_minimaux;
    gboolean	mise_a_jour_fin_comptes_passifs;

	/* MAJ des liens entre devises */
	gboolean	block_update_links;							/* block la mise à jour des liens en cas d'ajout ou de suppression de devise */

	/* payees: variables pour les associations de tiers */
	gboolean 	import_asso_case_insensitive;				/* TRUE = ne tient pas compte de la casse */
	gboolean	import_asso_use_regex;						/* TRUE = la chaine de recherche est une expression régulière */
	gboolean	import_asso_replace_rule;					/* TRUE = remplacela règle existante */

	/* prefs */
	guint		prefs_css_rules_tab;						/* mémorise l'onglet css rules sélectionné */
	guint		prefs_divers_tab;							/* memorise l'onglet du notebook divers options */
	gboolean	prefs_expand_tree;							/* memorise le choix du bouton expand collapse du treeview des preferences */
	guint		prefs_import_tab;							/* mémorise l'onglet import sélectionné */
	gchar *		prefs_selected_row;							/* memorise l'onglet selectionné du treeview des preferences */

	/* Reports */
	gboolean	no_show_prefs;								/* si VRAI = ne pas montrer les préférences de l'état */
	gboolean	empty_report;								/* Si VRAI remplace la mise à jour de l'état sélectionné par un état vide */
															/* permet de gagner du temps exemple état vide et état recherche */

	/* Transactions */
	gint		display_one_line;			/* 1 fixes bug 1875 */
	gint		display_two_lines;			/* 1-2, 1-3, "1-4 */
	gint		display_three_lines;		/* 1-2-3, 1-2-4, 1-3-4 */

};

/** Contain pre-defined CSV separators */
struct CsvSeparators
{
    const gchar *name;		/** Visible name of CSV separator */
    const gchar *value; 	/** Real value */
};

/******************************************************************************/
/* Enum                                                                       */
/******************************************************************************/
typedef enum 	_ComboColumns			ComboColumns;
typedef enum	_CustomModelOpeColumns	CustomModelOpeColumns;
typedef enum 	_GsbTransactionType		GsbTransactionType;
typedef enum 	_GsbTitleType	 		GsbTitleType;
typedef enum 	_MetatreeContent		MetatreeContent;
typedef enum 	_SettingsSchema 		SettingsSchema;
typedef enum	_TransactionsField		TransactionsField;

/* définition du titre de grisbi */
enum _GsbTitleType
{
    GSB_ACCOUNT_ENTITY,
    GSB_ACCOUNT_HOLDER,
    GSB_ACCOUNT_FILENAME
};

/* definition of the columns of model for the left panel  */
enum LeftPanelTreeColumns
{
    LEFT_PANEL_TREE_TEXT_COLUMN,
    LEFT_PANEL_TREE_PAGE_COLUMN,
    LEFT_PANEL_TREE_BOLD_COLUMN,
    LEFT_PANEL_TREE_ITALIC_COLUMN,
    LEFT_PANEL_TREE_NUM_COLUMNS
};


/* définition du type d'origine pour les données du module budgétaire */
enum BetArrayOriginData
{
    SPP_ORIGIN_TRANSACTION,             /* Ligne issue d'une opération */
    SPP_ORIGIN_SCHEDULED,               /* Ligne issue d'une opération plannifiée */
    SPP_ORIGIN_FUTURE,                  /* Ligne issue d'une opération future */
    SPP_ORIGIN_HISTORICAL,              /* Ligne issue d'une donnée historique */
    SPP_ORIGIN_ACCOUNT,                 /* Ligne issue d'un compte */
    SPP_ORIGIN_ARRAY,                   /* Donnée provenant du tableau de prévision (duration_widget) */
    SPP_ORIGIN_CONFIG,                  /* Donnée provenant de la page des préférences (duration_widget) */
    SPP_ORIGIN_SIMULATOR,               /* Donnée provenant du tableau du simulateur de crédit */
    SPP_ORIGIN_FINANCE,                 /* Donnée provenant du tableau d'amortissement du simulateur */
	SPP_ORIGIN_INVERSE_FINANCE,			/* Donnée provenant du tableau d'amortissement du simulateur */
	 									/* AVEC INVERSION de colonnes */
    SPP_ORIGIN_SOLDE                    /* Ligne de solde au premier du mois  */
};


/* définition du type de mise à jour en fonction des données du module budgétaire */
enum BetTypeMaj
{
    BET_MAJ_FALSE = 0,
    BET_MAJ_ESTIMATE,
    BET_MAJ_HISTORICAL,
    BET_MAJ_FINANCIAL,
    BET_MAJ_ALL
};


/* recopie des types de transaction de la libofx en attendant une version propre */
enum _GsbTransactionType
{
    GSB_CREDIT,     /**< Generic credit */
    GSB_DEBIT,      /**< Generic debit */
    GSB_INT,        /**< Interest earned or paid (Note: Depends on signage of amount) */
    GSB_DIV,        /**< Dividend */
    GSB_FEE,        /**< FI fee */
    GSB_SRVCHG,     /**< Service charge */
    GSB_DEP,        /**< Deposit */
    GSB_ATM,        /**< ATM debit or credit (Note: Depends on signage of amount) */
    GSB_POS,        /**< Point of sale debit or credit (Note: Depends on signage of amount) */
    GSB_XFER,       /**< Transfer */
    GSB_CHECK,      /**< Check */
    GSB_PAYMENT,    /**< Electronic payment */
    GSB_CASH,       /**< Cash withdrawal */
    GSB_DIRECTDEP,  /**< Direct deposit */
    GSB_DIRECTDEBIT,/**< Merchant initiated debit */
    GSB_REPEATPMT,  /**< Repeating payment/standing order */
    GSB_OTHER       /**< Somer other type of transaction */
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

enum FormatDateOrder
{
    ORDER_DD_MM_YY = 0,
    ORDER_MM_DD_YY,
    ORDER_YY_MM_DD,
    ORDER_YY_DD_MM,
    ORDER_DD_YY_MM,
    ORDER_MM_YY_DD,
    ORDER_MAX
};

enum _ComboColumns
{
    COMBO_COL_VISIBLE_STRING = 0,    /* string : what we see in the combofix */
    COMBO_COL_REAL_STRING,           /* string : what we set in the entry when selecting something */
    COMBO_COL_VISIBLE,               /* boolean : if that line has to be showed */
    COMBO_COL_LIST_NUMBER,           /* int : the number of the list 0, 1 ou 2 (CREDIT DEBIT SPECIAL) */
    COMBO_COL_SEPARATOR,             /* TRUE : if this is a separator */
    COMBO_N_COLUMNS
};

enum _CustomModelOpeColumns			/* The data columns that we export via the tree model interface */
{
    /* for the 6 first col, this can be changed by user,
     * so juste name col_x, and comment the by default */
    CUSTOM_MODEL_COL_0 = 0,	/* by default, check */
    CUSTOM_MODEL_COL_1,		/* by default, date */
    CUSTOM_MODEL_COL_2,		/* by default, payee */
    CUSTOM_MODEL_COL_3,		/* by default, P/R */
    CUSTOM_MODEL_COL_4,		/* by default, debit */
    CUSTOM_MODEL_COL_5,		/* by default, credit */
    CUSTOM_MODEL_COL_6,		/* by default, balance */

    CUSTOM_MODEL_BACKGROUND,			/*< color of the background (a GdkRGBA) */
    CUSTOM_MODEL_SAVE_BACKGROUND, 		/*< when selection, save of the normal color of background (a GdkRGBA) */
    CUSTOM_MODEL_AMOUNT_COLOR,			/*< color of the amount (a string like "red" or NULL)*/
    CUSTOM_MODEL_TEXT_COLOR,			/*< color of the text */
    CUSTOM_MODEL_TRANSACTION_ADDRESS,		/* pointer to the transaction structure */
    CUSTOM_MODEL_WHAT_IS_LINE,			/*< on what the address point to ? IS_TRANSACTION, IS_ARCHIVE (see below) */
    CUSTOM_MODEL_FONT, 				/*< PangoFontDescription if used */
    CUSTOM_MODEL_TRANSACTION_LINE, 		/*< the line in the transaction (0, 1, 2 or 3) */
    CUSTOM_MODEL_VISIBLE, 			/*< whether that transaction is visible or not */
    CUSTOM_MODEL_CHECKBOX_VISIBLE,   		/*< whether the checkbox is visible or not */
    CUSTOM_MODEL_CHECKBOX_VISIBLE_RECONCILE,   	/*< whether the checkbox is visible or not during reconciliation */
    CUSTOM_MODEL_CHECKBOX_ACTIVE,   		/*< whether the checkbox is active or not */

    CUSTOM_MODEL_N_COLUMNS
};

enum _MetatreeContent				/* content of the metatree : 0 : payee, 1 : category, 2 : budget */
{
	METATREE_PAYEE,
	METATREE_CATEGORY,
	METATREE_BUDGET
};

enum _TransactionsField				/* the element number for each showable in the list */
{
    ELEMENT_DATE = 1,
    ELEMENT_VALUE_DATE,
    ELEMENT_PARTY,
    ELEMENT_BUDGET,
    ELEMENT_DEBIT,
    ELEMENT_CREDIT,
    ELEMENT_BALANCE,
    ELEMENT_AMOUNT,
    ELEMENT_PAYMENT_TYPE,
    ELEMENT_RECONCILE_NB,
    ELEMENT_EXERCICE,
    ELEMENT_CATEGORY,
    ELEMENT_MARK,
    ELEMENT_VOUCHER,
    ELEMENT_NOTES,
    ELEMENT_BANK,
    ELEMENT_NO,
    ELEMENT_CHQ
};


#endif
