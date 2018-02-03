#ifndef __CSV_TEMPLATE_RULE_H__
#define __CSV_TEMPLATE_RULE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define CSV_TEMPLATE_RULE_TYPE    	(csv_template_rule_get_type ())
#define CSV_TEMPLATE_RULE(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), CSV_TEMPLATE_RULE_TYPE, CsvTemplateRule))
#define CSV_IS_TEMPLATE_RULE(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CSV_TEMPLATE_RULE_TYPE))

#define GSB_RESPONSE_EDIT 5

typedef struct _CsvTemplateRule			CsvTemplateRule;
typedef struct _CsvTemplateRuleClass	CsvTemplateRuleClass;


typedef struct _CSVImportRule 			CSVImportRule;	/* structure pour les regles d'import des fichiers CSV */

typedef struct _SpecConfData			SpecConfData;	/* Données des regles d'import des fichiers CSV */

struct _CsvTemplateRule
{
    GtkDialog 		parent;
};

struct _CsvTemplateRuleClass
{
    GtkDialogClass 	parent_class;
};

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

struct _SpecConfData
{
	guint			csv_spec_conf_action;		/* action a effectuer */
	guint			csv_spec_conf_action_data;	/* colonne de la donnée à traiter par l'action */
	guint			csv_spec_conf_used_data;	/* colonne de la donnée générant l'action */
	gchar *			csv_spec_conf_used_text;	/* donnée générant l'action */
};

/*structure pour les regles d'import des fichiers CSV */
/* START_DECLARATION */
GType               csv_template_rule_get_type						(void) G_GNUC_CONST;

CsvTemplateRule * 	csv_template_rule_new							(GtkWidget *assistant);
CsvTemplateRule * 	csv_template_rule_edit							(GtkWindow *parent,
															 gint rule_number);
void				csv_template_rule_csv_import_rule_struct_free	(CSVImportRule *csv_rule);
void				csv_template_rule_spec_conf_data_struct_copy	(SpecConfData *spec_conf_data);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __CSV_TEMPLATE_RULE_H__ */
