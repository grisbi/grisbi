#ifndef _IMPORT_H
#define _IMPORT_H (1)
/* START_INCLUDE_H */
#include "import.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean affichage_recapitulatif_importation ( GtkWidget * assistant );
GSList * import_selected_files ( GtkWidget * assistant );
void importer_fichier ( void );
GtkWidget *onglet_importation (void);
gchar * unique_imported_name ( gchar * account_name );
/* END_DECLARATION */


enum import_type {
  TYPE_UNKNOWN = 0,
  TYPE_QIF,
  TYPE_OFX,
  TYPE_GNUCASH,
  TYPE_CSV,
  TYPE_GBANKING,
  TYPE_MAX,
};


struct imported_file {
    gchar * name;
    gchar * coding_system;
    enum import_type type;
};


/* struture d'une importation : compte contient la liste des opés importées */
struct struct_compte_importation
{
    gchar *id_compte;

    enum import_type origine;    /* 0=qif, 1=ofx, 2=html, 3=gnucash, 4=csv */

    gchar *nom_de_compte;
    gchar *filename;
    gint type_de_compte;  /* 0=OFX_CHECKING,1=OFX_SAVINGS,2=OFX_MONEYMRKT,3=OFX_CREDITLINE,4=OFX_CMA,5=OFX_CREDITCARD,6=OFX_INVESTMENT, 7=cash */
    gchar *devise;

    GDate *date_depart;
    GDate *date_fin;

    GSList *operations_importees;          /* liste des struct des opés importées */

    gdouble solde;
    gchar *date_solde_qif;            /* utilisé temporairement pour un fichier qif */

    GtkWidget *bouton_devise;             /* adr du bouton de la devise dans le récapitulatif */
    gint action;             	/* action a faire */
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


/** Etat de rapprochement d'une opération */
enum operation_etat_rapprochement {
  OPERATION_NORMALE = 0,
  OPERATION_POINTEE,
  OPERATION_TELERAPPROCHEE,
  OPERATION_RAPPROCHEE,
};


/** Imported transaction.  */
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
    gpointer ope_correspondante; /* contient l'adrde l'opé qui correspond peut être à l'opé importée pour la présentation à l'utilisateur */
    GtkWidget *bouton;  /*  adr du bouton si cette opé est douteuse et vérifiée par l'utilisateur */

    gchar *tiers;
    gchar *notes;
    gulong cheque;

    gchar *categ;

    gdouble montant;

    enum operation_etat_rapprochement p_r;

    gint type_de_transaction;

    gint operation_ventilee;  /* à 1 si c'est une ventil, dans ce cas les opés de ventil suivent et ont ope_de_ventilation à 1 */
    gint ope_de_ventilation;

    /* Used by gnucash import */
    gchar * guid;
};


#endif
