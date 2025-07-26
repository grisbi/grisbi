#ifndef _IMPORT_H
#define _IMPORT_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "csv_template_rule.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
/* END_INCLUDE_H */

/* struture d'une importation : compte contient la liste des opés importées */
struct ImportAccount
{
    gchar *id_compte;

    gchar * origine;    /* QIF, OFX ... */

    gchar *nom_de_compte;
    gchar *filename;
    gchar *real_filename;	/* needed because filename is overwritten, need to fix that when work again with imports */
    gint type_de_compte;	/* 0=OFX_CHECKING,1=OFX_SAVINGS,2=OFX_MONEYMRKT,3=OFX_CREDITLINE,4=OFX_CMA,5=OFX_CREDITCARD,6=OFX_INVESTMENT, 7=cash */
    gchar *devise;

    GDate *date_depart;
    GDate *date_fin;

    GSList *operations_importees;			/* liste des struct des opés importées */

    GsbReal solde;
    gchar *date_solde_qif;					/* utilisé temporairement pour un fichier qif */

    GtkWidget *bouton_devise;				/* adr du bouton de la devise dans le récapitulatif */
    gint action;							/* IMPORT_CREATE_ACCOUNT, IMPORT_ADD_TRANSACTIONS, IMPORT_MARK_TRANSACTIONS */

    gboolean	invert_transaction_amount;	/* if TRUE, all the transactions imported will have their amount inverted */

    gboolean	create_rule;				/* if TRUE, we create a rule according the values of this structure */
    GtkWidget	*entry_name_rule;			/* entry containing the name of the rule */
    GtkWidget	*hbox_rule;

    GtkWidget *bouton_type_compte;			/* adr du bouton du type de compte dans le récapitulatif */
    GtkWidget *bouton_compte_add;			/* adr du bouton du compte dans le récapitulatif */
    GtkWidget *bouton_compte_mark;			/* adr du bouton du compte dans le récapitulatif */

    GtkWidget * hbox1;
    GtkWidget * hbox2;
    GtkWidget * hbox3;

    /* Used by gnucash import */
	gchar * guid;

	/* for CSV type */
	gchar *		csv_rule_name;
	gint		csv_account_id_col;			/* numéro de colonne contenant Id compte */
	gint 		csv_account_id_row;			/* numéro de ligne contenant Id compte */
	gchar *		csv_fields_str;				/* liste des libellés grisbi des colonnes du fichier CSV */
	gint		csv_first_line_data;		/* première ligne de données actives */
	gboolean	csv_headers_present;		/* TRUE si les libellés des colonnes existent */
	gchar *		csv_spec_cols_name;			/* nom des colonnes du fichier importé pour les combo action data et used data */
	GSList * 	csv_spec_lines_list;		/* liste de structures contenant les éléments de la ligne spéciale action */
};

/* possible actions to the import */
#define IMPORT_CREATE_ACCOUNT 0
#define IMPORT_ADD_TRANSACTIONS 1
#define IMPORT_MARK_TRANSACTIONS 2


/** Imported transaction.  */
struct ImportTransaction
{
    gchar *id_operation;

    gint no_compte;             /* mis à jour si lors du marquage, si pas d'opé associée trouvée */
    gint devise;                /* mis à jour au moment de l'enregistrement de l'opé */
    GDate *date;
    GDate *date_de_valeur;
    gchar *date_tmp;            /* pour un fichier qif, utilisé en tmp avant de le transformer en gdate */

    gint action;		        /* IMPORT_TRANSACTION_GET_TRANSACTION, IMPORT_TRANSACTION_ASK_FOR_TRANSACTION, IMPORT_TRANSACTION_LEAVE_TRANSACTION*/
    gint ope_correspondante;    /* contient l'adr de l'opé qui correspond peut être à l'opé importée pour la présentation à l'utilisateur */
    GtkWidget *bouton;          /*  adr du bouton si cette opé est douteuse et vérifiée par l'utilisateur */
	GtkWidget *label_ope_find;	/* label de l'opération trouvée peut être remplacé par l'utilisateur */

    gchar *tiers;
    gchar *notes;
    gchar *cheque;
    gchar *payment_method;		/* Ajout pour l'importation des fichiers CSV */
    gchar *categ;
	gchar *budget;

    GsbReal montant;

    gint p_r;					/* OPERATION_NORMALE = 0, OPERATION_POINTEE, OPERATION_TELEPOINTEE, OPERATION_RAPPROCHEE */

    gint type_de_transaction;   /* Utilisé pour les fichiers OFX étendu à certains fichiers QIF (Champs N du fichier QIF) */
	gboolean ope_memorized;		/* memorized transaction voir à quoi ça sert */

    gboolean operation_ventilee;/* TRUE si c'est une ventil, dans ce cas les opés de ventil suivent et ont ope_de_ventilation à TRUE */
    gboolean ope_de_ventilation;

	gboolean transfert;			/* Pour fichier QIF */
	gchar *dest_account_name;	/* Nom du compte destinataire */

    /* Used by gnucash import */
    gchar * guid;
};

/* possible actions to the transaction */
#define IMPORT_TRANSACTION_GET_TRANSACTION 0
#define IMPORT_TRANSACTION_ASK_FOR_TRANSACTION 1
#define IMPORT_TRANSACTION_LEAVE_TRANSACTION 2

struct ImportFile
{
    gchar * 		name;
    const gchar * 	coding_system;
    const gchar * 	type;
	gboolean		import_categories;
};


struct ImportFormat
{
    const gchar * name;
    const gchar * complete_name;
    const gchar * extension;
    gboolean (* import) (GtkWidget * assistant, struct ImportFile *);
};

/* structure définissant une association entre un tiers
 * et une chaine de recherche contenant un ou des jokers (%)
 */
struct ImportPayeeAsso
{
    gint    payee_number;
    gchar   *search_str;
	gboolean	ignore_case;
	gboolean	use_regex;
};

/* START_DECLARATION */
void 		gsb_import_assistant_importer_fichier 			(void);

gboolean 	gsb_import_associations_add_assoc 				(gint payee_number,
															 const gchar *search_str,
															 gint ignore_case,
															 gint use_regex);
gint 		gsb_import_associations_cmp_assoc 				(struct ImportPayeeAsso *assoc_1,
															 struct ImportPayeeAsso *assoc_2);
gint		gsb_import_associations_get_last_payee_number	(void);
GSList *	gsb_import_associations_get_liste_associations	(void);
void 		gsb_import_associations_free_liste				(void);
void 		gsb_import_associations_init_variables 			(void);
gint 		gsb_import_associations_list_append_assoc 		(gint payee_number,
															 struct ImportPayeeAsso *assoc);
void 		gsb_import_associations_remove_assoc 			(gint payee_number);

gboolean 	gsb_import_by_rule 								(gint rule);
void		gsb_import_free_transaction						(struct ImportTransaction *transaction);
gchar *		gsb_ImportFormats_get_list_formats_to_string 	(void);
GSList *	gsb_import_import_selected_files 				(GtkWidget *assistant);
void 		gsb_import_register_account 					(struct ImportAccount *account);
void 		gsb_import_register_account_error 				(struct ImportAccount *account);
void 		gsb_import_register_import_formats 				(void);
gchar * 	gsb_import_unique_imported_name 				(const gchar *account_name);
/* END_DECLARATION */



#endif
