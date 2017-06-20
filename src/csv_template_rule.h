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
	SpecConfData *	csv_spec_conf_data;			/* structure contenant les éléments de la ligne spéciale action */
};

struct _SpecConfData
{
	guint			combobox_action;			/* action a effectuer */
	guint			combobox_text_col;			/* colonne de la donnée générant l'action */
	guint			combobox_amount_col;		/* colonne de la donnée montant pour l'action inversion du montant */
	gchar *			entry_text_str;				/* donnée générant l'action */
};

/*structure pour les regles d'import des fichiers CSV */
/* START_DECLARATION */
GType               csv_template_rule_get_type				(void) G_GNUC_CONST;

CsvTemplateRule * 	csv_template_rule_new					(GtkWidget *assistant);
void				csv_template_rule_csv_rule_struct_free	(CSVImportRule *csv_rule);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __CSV_TEMPLATE_RULE_H__ */
