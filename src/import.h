#ifndef _IMPORT_H
#define _IMPORT_H (1)

/* START_INCLUDE_H */
#include "gsb_real.h"
#include "gsb_data_transaction.h"
#include "structures.h"
/* END_INCLUDE_H */

/* struture d'une importation : compte contient la liste des opés importées */
struct struct_compte_importation
{
    gchar *id_compte;

    gchar * origine;    /* QIF, OFX ... */

    gchar *nom_de_compte;
    gchar *filename;
    gchar *real_filename;	/* needed because filename is overwritten, need to fix that when work again with imports */
    gint type_de_compte;  /* 0=OFX_CHECKING,1=OFX_SAVINGS,2=OFX_MONEYMRKT,3=OFX_CREDITLINE,4=OFX_CMA,5=OFX_CREDITCARD,6=OFX_INVESTMENT, 7=cash */
    gchar *devise;

    GDate *date_depart;
    GDate *date_fin;

    GSList *operations_importees;          /* liste des struct des opés importées */

    gsb_real solde;
    gchar *date_solde_qif;            /* utilisé temporairement pour un fichier qif */

    GtkWidget *bouton_devise;             /* adr du bouton de la devise dans le récapitulatif */
    gint action;				/* IMPORT_CREATE_ACCOUNT, IMPORT_ADD_TRANSACTIONS, IMPORT_MARK_TRANSACTIONS */

    gboolean	invert_transaction_amount;	/* if TRUE, all the transactions imported will have their amount inverted */

    gboolean	create_rule;			/* if TRUE, we create a rule according the values of this structure */
    GtkWidget	*entry_name_rule;			/* entry containing the name of the rule */
    GtkWidget	*hbox_rule;

    GtkWidget *bouton_type_compte;             /* adr du bouton du type de compte dans le récapitulatif */
    GtkWidget *bouton_compte_add;             /* adr du bouton du compte
					   * dans le récapitulatif */
    GtkWidget *bouton_compte_mark;             /* adr du bouton du compte dans le récapitulatif */

    GtkWidget * hbox1;
    GtkWidget * hbox2;
    GtkWidget * hbox3;

    /* Used by gnucash import */
    gchar * guid;
};

/* possible actions to the import */
#define IMPORT_CREATE_ACCOUNT 0
#define IMPORT_ADD_TRANSACTIONS 1
#define IMPORT_MARK_TRANSACTIONS 2


/** Imported transaction.  */
struct struct_ope_importation
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
    
    gchar *tiers;
    gchar *notes;
    gchar *cheque;

    gchar *categ;
	gchar *budget;

    gsb_real montant;

    enum operation_etat_rapprochement p_r;

    gint type_de_transaction;

    gint operation_ventilee;  /* à 1 si c'est une ventil, dans ce cas les opés de ventil suivent et ont ope_de_ventilation à 1 */
    gint ope_de_ventilation;

    /* Used by gnucash import */
    gchar * guid;
};

/* possible actions to the transaction */
#define IMPORT_TRANSACTION_GET_TRANSACTION 0
#define IMPORT_TRANSACTION_ASK_FOR_TRANSACTION 1
#define IMPORT_TRANSACTION_LEAVE_TRANSACTION 2

struct imported_file
{
    gchar * name;
    const gchar * coding_system;
    const gchar * type;
};


typedef gboolean ( * import_function ) ( GtkWidget * assistant, struct imported_file * );

struct import_format
{
    gchar * name;
    gchar * complete_name;
    gchar * extension;
    gboolean ( * import ) ( GtkWidget * assistant, struct imported_file * );
};

/* START_DECLARATION */
gint gsb_import_associations_cmp_assoc (struct struct_payee_asso *assoc_1,
                                        struct struct_payee_asso *assoc_2);
GtkWidget *gsb_import_associations_gere_tiers ( void );
void gsb_import_associations_init_variables ( void );
gint gsb_import_associations_list_append_assoc ( gint payee_number,
                        const gchar *search_str );
gboolean gsb_import_by_rule ( gint rule );
gchar *gsb_import_formats_get_list_formats_to_string ( void );
G_MODULE_EXPORT void gsb_import_register_account ( struct struct_compte_importation * account );
G_MODULE_EXPORT void gsb_import_register_account_error ( struct struct_compte_importation * account );
GSList *import_selected_files ( GtkWidget * assistant );
void importer_fichier ( void );
GtkWidget *onglet_importation ( void );
G_MODULE_EXPORT void register_import_format ( struct import_format *format );
void register_import_formats ( void );
G_MODULE_EXPORT gchar * unique_imported_name ( gchar * account_name );
/* END_DECLARATION */



#endif
