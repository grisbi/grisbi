#ifndef __CSV_TEMPLATE_RULE_H__
#define __CSV_TEMPLATE_RULE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define CSV_TEMPLATE_RULE_TYPE			(csv_template_rule_get_type ())

typedef struct _CSVImportRule 			CSVImportRule;		/* structure pour les regles d'import des fichiers CSV */
typedef struct _CsvSpecConfData			CsvSpecConfData;	/* Données des regles d'import des fichiers CSV */

struct _CSVImportRule
{
	gchar *			csv_rule_name;
	gint			csv_account_id_col;			/* numéro de colonne contenant Id compte */
	gint 			csv_account_id_row;			/* numéro de ligne contenant Id compte */
	gint 			csv_first_line_data;		/* première ligne utile du tableau des données */
	gboolean		csv_headers_present;		/* si la première ligne contient les en-têtes de colonnes */
	gchar *			csv_cols_name;				/* noms des colonnes du fichier importé */
	GSList *		csv_spec_lines_list;		/* liste de structures contenant les éléments de la ligne spéciale action */
};

struct _CsvSpecConfData
{
	guint			csv_spec_conf_action;		/* action a effectuer */
	guint			csv_spec_conf_action_data;	/* colonne de la donnée à traiter par l'action */
	guint			csv_spec_conf_used_data;	/* colonne de la donnée générant l'action */
	gchar *			csv_spec_conf_used_text;	/* donnée générant l'action */
};

G_DECLARE_FINAL_TYPE (CsvTemplateRule, csv_template_rule, CSV_TEMPLATE, RULE, GtkDialog)

/* START_DECLARATION */
CsvTemplateRule * 	csv_template_rule_new							(GtkWidget *assistant);
CsvTemplateRule * 	csv_template_rule_edit							(GtkWindow *parent,
															 gint rule_number);
void				csv_template_rule_csv_import_rule_struct_free	(CSVImportRule *rule);
CsvSpecConfData *	csv_template_rule_spec_conf_data_struct_copy	(CsvSpecConfData *spec_conf_data,
																	 gpointer data);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __CSV_TEMPLATE_RULE_H__ */
