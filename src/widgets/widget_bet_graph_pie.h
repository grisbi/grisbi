#ifndef __WIDGET_BET_GRAPH_PIE_H__
#define __WIDGET_BET_GRAPH_PIE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "bet_graph.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_BET_GRAPH_PIE_TYPE		(widget_bet_graph_pie_get_type ())
#define WIDGET_BET_GRAPH_PIE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_BET_GRAPH_PIE_TYPE, WidgetBetGraphPie))
#define WIDGET_IS_BET_GRAPH_PIE(obj)	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_BET_GRAPH_PIE_TYPE))

typedef struct _WidgetBetGraphPie				WidgetBetGraphPie;
typedef struct _WidgetBetGraphPieClass			WidgetBetGraphPieClass;


struct _WidgetBetGraphPie
{
    GtkDialog parent;
};

struct _WidgetBetGraphPieClass
{
    GtkDialogClass parent_class;
};

/* START_DECLARATION */
GType					widget_bet_graph_pie_get_type				(void) G_GNUC_CONST;

WidgetBetGraphPie *		widget_bet_graph_pie_new					(BetGraphDataStruct *self,
																	 const gchar *title);

GtkWidget *				widget_bet_graph_pie_get_notebook			(GtkWidget *widget);
void					widget_bet_graph_pie_display_graph			(BetGraphDataStruct *self,
																	 gboolean display_sub_div,
																	 gint div_number);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_BET_GRAPH_PIE_H__ */
