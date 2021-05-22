#ifndef __WIDGET_CMP_AMOUNT_H__
#define __WIDGET_CMP_AMOUNT_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_CMP_AMOUNT_TYPE		(widget_cmp_amount_get_type ())
#define WIDGET_CMP_AMOUNT(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_CMP_AMOUNT_TYPE, WidgetCmpAmount))
#define WIDGET_IS_CMP_AMOUNT(obj)	+(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_CMP_AMOUNT_TYPE))

typedef struct _WidgetCmpAmount				WidgetCmpAmount;
typedef struct _WidgetCmpAmountClass		WidgetCmpAmountClass;


struct _WidgetCmpAmount
{
	GtkBox parent;
};

struct _WidgetCmpAmountClass
{
	GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				widget_cmp_amount_get_type			(void) G_GNUC_CONST;

WidgetCmpAmount *	widget_cmp_amount_new					(GtkWidget *page,
															 gint amount_cmp_number,
															 gboolean first_cmp_line);
gboolean			widget_cmp_amount_get_data				(GtkWidget *widget,
															 gint amount_cmp_number);
void 				widget_cmp_amount_init_data				(GtkWidget *widget,
															 gint amount_cmp_number);
void				widget_cmp_amount_show_button_add		(GtkWidget *widget,
															 gboolean show);
void				widget_cmp_amount_hide_button_link		(GtkWidget *widget);
void				widget_cmp_amount_show_button_remove	(GtkWidget *widget,
															 gboolean show);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_CMP_AMOUNT_H__ */
