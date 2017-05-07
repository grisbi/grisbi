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

typedef struct _CsvTemplateRule          CsvTemplateRule;
typedef struct _CsvTemplateRuleClass     CsvTemplateRuleClass;


struct _CsvTemplateRule
{
    GtkDialog parent;
};

struct _CsvTemplateRuleClass
{
    GtkDialogClass parent_class;
};

/* START_DECLARATION */
GType               csv_template_rule_get_type			(void) G_GNUC_CONST;

CsvTemplateRule * 	csv_template_rule_new				(GtkWidget *assistant);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __CSV_TEMPLATE_RULE_H__ */
