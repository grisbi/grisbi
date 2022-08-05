#ifndef __WIDGET_BET_GRAPH_OPTIONS_H__
#define __WIDGET_BET_GRAPH_OPTIONS_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "bet_graph.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_BET_GRAPH_OPTIONS_TYPE    	(widget_bet_graph_options_get_type ())
#define WIDGET_BET_GRAPH_OPTIONS(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_BET_GRAPH_OPTIONS_TYPE, WidgetBetGraphOptions))
#define WIDGET_IS_BET_GRAPH_OPTIONS(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_BET_GRAPH_OPTIONS_TYPE))

typedef struct _WidgetBetGraphOptions				WidgetBetGraphOptions;
typedef struct _WidgetBetGraphOptionsClass			WidgetBetGraphOptionsClass;

struct _WidgetBetGraphOptions
{
    GtkBox parent;
};

struct _WidgetBetGraphOptionsClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType						widget_bet_graph_options_get_type			(void) G_GNUC_CONST;

WidgetBetGraphOptions *		widget_bet_graph_options_new				(BetGraphDataStruct *self);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_BET_GRAPH_OPTIONS_H__ */
