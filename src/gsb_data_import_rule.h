#ifndef _GSB_DATA_IMPORT_RULE_H
#define _GSB_DATA_IMPORT_RULE_H (1)

#include <glib.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/** \struct
 * describe a rule
 * */
typedef struct _ImportRule ImportRule;

struct _ImportRule
{
	gint 	import_rule_number;

	/* general part */
	gchar *		charmap;					/* charmap du fichier importé   */
	gchar *		last_file_name;				/* last file imported with a rule */
	gchar *		rule_name;
	gchar *		type;						/* type de compte CSV QIF OFX */
	gint		account_number;
	gint		action;						/* action of the rule : IMPORT_ADD_TRANSACTIONS, IMPORT_MARK_TRANSACTIONS */
	gint		currency_number;			/* currency used to import the transactions */
	gboolean 	invert_transaction_amount;	/* if TRUE, all the transactions imported will have their amount inverted */

	/* CSV part */
	gint		csv_account_id_col;			/* numéro de colonne contenant Id compte */
	gint		csv_account_id_row;			/* numéro de ligne contenant Id compte */
	gchar *		csv_fields_str;				/* liste des libellés grisbi des colonnes du fichier CSV */
	gint		csv_first_line_data;		/* première ligne de données actives y compris ligne des noms de colonne */
	gboolean	csv_headers_present;		/* TRUE si les libellés des colonnes existent */
	gchar *		csv_separator;
	gint		csv_spec_nbre_lines;		/* nombre de lignes spéciales de conditions */
	gint		csv_spec_action;			/* action a effectuer 0 = skipped line 1 = invert the amount */
	gint		csv_spec_amount_col;		/* numéro de colonne contenant la donnée à traiter par l'action */
	gint		csv_spec_text_col;			/* numéro de colonne contenant le texte à rechercher */
	gchar *		csv_spec_text_str;			/* texte à rechercher */
	gchar *		csv_spec_cols_name;			/* nom des colonnes du fichier importé pour combo action et montant */
	GSList *	csv_spec_lines_list;		/* liste de structures contenant les éléments de la ligne spéciale action */
};

/* START_DECLARATION */
gboolean 		gsb_data_import_rule_account_has_rule 			(gint account_number);
gint 			gsb_data_import_rule_account_has_rule_name		(gint account_number,
																 const gchar *rule_name);
void			gsb_data_import_rule_free_csv_spec_lines_list	(gint import_rule_number);
void			gsb_data_import_rule_free_list					(void);
gint 			gsb_data_import_rule_get_account 				(gint import_rule_number);
gint 			gsb_data_import_rule_get_action 				(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_charmap 				(gint import_rule_number);
gint 			gsb_data_import_rule_get_csv_account_id_col		(gint import_rule_number);
gint 			gsb_data_import_rule_get_csv_account_id_row		(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_csv_fields_str			(gint import_rule_number);
gint 			gsb_data_import_rule_get_csv_first_line_data	(gint import_rule_number);
gint 			gsb_data_import_rule_get_csv_headers_present	(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_csv_separator			(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_csv_spec_cols_name		(gint import_rule_number);
GSList *		gsb_data_import_rule_get_csv_spec_lines_list	(gint import_rule_number);
gint 			gsb_data_import_rule_get_csv_spec_nbre_lines	(gint import_rule_number);
gint 			gsb_data_import_rule_get_currency 				(gint import_rule_number);
GSList *		gsb_data_import_rule_get_from_account 			(gint account_number);
gboolean 		gsb_data_import_rule_get_invert 				(gint import_rule_number);
const gchar *	gsb_data_import_rule_get_last_file_name 		(gint import_rule_number);
GSList *		gsb_data_import_rule_get_list 					(void);
const gchar *	gsb_data_import_rule_get_name 					(gint import_rule_number);
gint 			gsb_data_import_rule_get_number 				(gpointer rule_ptr);
const gchar *	gsb_data_import_rule_get_type					(gint import_rule_number);
gboolean 		gsb_data_import_rule_init_variables 			(void);
gint 			gsb_data_import_rule_new 						(const gchar *name);
gboolean 		gsb_data_import_rule_remove 					(gint import_rule_number);
gboolean 		gsb_data_import_rule_set_account 				(gint import_rule_number,
																 gint account_number);
gboolean 		gsb_data_import_rule_set_action 				(gint import_rule_number,
																 gint action);
gboolean 		gsb_data_import_rule_set_charmap 				(gint import_rule_number,
																 const gchar *charmap);
gboolean		gsb_data_import_rule_set_csv_account_id_col		(gint import_rule_number,
																 gint account_id_col);
gboolean		gsb_data_import_rule_set_csv_account_id_row		(gint import_rule_number,
																 gint csv_account_id_row);
gboolean		gsb_data_import_rule_set_csv_fields_str			(gint import_rule_number,
																 const gchar *csv_fields_str);
gboolean		gsb_data_import_rule_set_csv_first_line_data	(gint import_rule_number,
																 gint csv_first_line_data);
gboolean		gsb_data_import_rule_set_csv_headers_present	(gint import_rule_number,
																 gboolean csv_headers_present);
gboolean		gsb_data_import_rule_set_csv_separator			(gint import_rule_number,
																 const gchar *csv_separator);
gboolean		gsb_data_import_rule_set_csv_spec_cols_name		(gint import_rule_number,
																 const gchar *csv_spec_cols_name);
gboolean		gsb_data_import_rule_set_csv_spec_lines_list	(gint import_rule_number,
																 GSList *csv_spec_lines_list);
gboolean 		gsb_data_import_rule_set_csv_spec_nbre_lines	(gint import_rule_number,
																 gint csv_spec_nbre_lines);
gboolean 		gsb_data_import_rule_set_currency 				(gint import_rule_number,
																 gint currency_number);
gboolean 		gsb_data_import_rule_set_invert 				(gint import_rule_number,
																 gboolean invert);
gboolean 		gsb_data_import_rule_set_last_file_name 		(gint import_rule_number,
																 const gchar *last_file_name);
gboolean 		gsb_data_import_rule_set_name 					(gint import_rule_number,
																 const gchar *name);
gint 			gsb_data_import_rule_set_new_number 			(gint import_rule_number,
																 gint new_no_import_rule);
gboolean 		gsb_data_import_rule_set_type 					(gint import_rule_number,
																 const gchar *type);
/* END_DECLARATION */
#endif
