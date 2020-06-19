#ifndef __WIDGET_CSS_RULES_H__
#define __WIDGET_CSS_RULES_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_CSS_RULES_TYPE    	(widget_css_rules_get_type ())
#define WIDGET_CSS_RULES(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_CSS_RULES_TYPE, WidgetCssRules))
#define WIDGET_IS_CSS_RULES(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_CSS_RULES_TYPE))

typedef struct _WidgetCssRules			WidgetCssRules;
typedef struct _WidgetCssRulesClass		WidgetCssRulesClass;


struct _WidgetCssRules
{
    GtkBox parent;
};

struct _WidgetCssRulesClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				widget_css_rules_get_type				(void) G_GNUC_CONST;

WidgetCssRules * 	widget_css_rules_new					(GtkWidget *page);
GtkWidget *			widget_css_rules_get_notebook			(GtkWidget *w_css_rules);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_CSS_RULES_H__ */
