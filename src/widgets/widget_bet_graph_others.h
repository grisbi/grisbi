#ifndef __WIDGET_BET_GRAPH_OTHERS_H__
#define __WIDGET_BET_GRAPH_OTHERS_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "bet_graph.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_BET_GRAPH_OTHERS_TYPE    	(widget_bet_graph_others_get_type ())
#define WIDGET_BET_GRAPH_OTHERS(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_BET_GRAPH_OTHERS_TYPE, WidgetBetGraphOthers))
#define WIDGET_IS_BET_GRAPH_OTHERS(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_BET_GRAPH_OTHERS_TYPE))

typedef struct _WidgetBetGraphOthers				WidgetBetGraphOthers;
typedef struct _WidgetBetGraphOthersClass			WidgetBetGraphOthersClass;


struct _WidgetBetGraphOthers
{
    GtkDialog parent;
};

struct _WidgetBetGraphOthersClass
{
    GtkDialogClass parent_class;
};

/* START_DECLARATION */
GType						widget_bet_graph_others_get_type		(void) G_GNUC_CONST;

WidgetBetGraphOthers *		widget_bet_graph_others_new								(BetGraphDataStruct *self,
																					 const gchar *title,
																					 gint origin_tab);
void						widget_bet_graph_others_show_grid_button_configure		(BetGraphDataStruct *self,
																					 gint active,
																					 gint hide);
void						widget_bet_graph_others_graph_update					(BetGraphDataStruct *self);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_BET_GRAPH_OTHERS_H__ */
