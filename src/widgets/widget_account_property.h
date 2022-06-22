#ifndef __WIDGET_ACCOUNT_PROPERTY_H__
#define __WIDGET_ACCOUNT_PROPERTY_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_ACCOUNT_PROPERTY_TYPE		(widget_account_property_get_type ())
#define WIDGET_ACCOUNT_PROPERTY(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_ACCOUNT_PROPERTY_TYPE, WidgetAccountProperty))
#define WIDGET_IS_ACCOUNT_PROPERTY(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_ACCOUNT_PROPERTY_TYPE))

typedef struct _WidgetAccountProperty			WidgetAccountProperty;
typedef struct _WidgetAccountPropertyClass		WidgetAccountPropertyClass;


struct _WidgetAccountProperty
{
    GtkBox parent;
};

struct _WidgetAccountPropertyClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType						widget_account_property_get_type					(void) G_GNUC_CONST;

WidgetAccountProperty *		widget_account_property_new							(void);
void						widget_account_property_update_bank_data			(gint bank_number);
void						widget_account_property_fill						(void);
GtkWidget *					widget_account_property_get_combo_account_currency	(void);
GtkWidget *					widget_account_property_get_combo_bank_list			(void);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_ACCOUNT_PROPERTY_H__ */
