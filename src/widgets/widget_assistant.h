#ifndef __WIDGET_ASSISTANT_H__
#define __WIDGET_ASSISTANT_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_ASSISTANT_TYPE    	(widget_assistant_get_type ())
#define WIDGET_ASSISTANT(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_ASSISTANT_TYPE, WidgetAssistant))
#define WIDGET_IS_ASSISTANT(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_ASSISTANT_TYPE))

typedef struct _WidgetAssistant				WidgetAssistant;
typedef struct _WidgetAssistantClass		WidgetAssistantClass;


struct _WidgetAssistant
{
    GtkDialog parent;
};

struct _WidgetAssistantClass
{
    GtkDialogClass parent_class;
};

/* START_DECLARATION */
GType				widget_assistant_get_type				(void) G_GNUC_CONST;

WidgetAssistant *	widget_assistant_new					(const gchar *title,
															 const gchar *explanation,
															 const gchar *image_filename,
															 GCallback enter_callback);

void 				widget_assistant_add_page 				(GtkWidget *dialog,
															 GtkWidget *widget,
															 gint position,
															 gint prev,
															 gint next,
															 GCallback enter_callback);
void 				widget_assistant_change_button_next 	(GtkWidget *dialog,
															 const gchar *title,
															 GtkResponseType response);
void 				widget_assistant_next_page 				(GtkWidget *dialog);
GtkResponseType 	widget_assistant_run 					(GtkWidget *dialog);
gboolean 			widget_assistant_sensitive_button_next 	(GtkWidget *dialog,
															 gboolean state);
gboolean 			widget_assistant_sensitive_button_prev 	(GtkWidget *dialog,
															 gboolean state);
void 				widget_assistant_set_next 				(GtkWidget *dialog,
															 gint page,
															 gint next);
void 				widget_assistant_set_prev 				(GtkWidget *dialog,
															 gint page,
															 gint prev);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_ASSISTANT_H__ */
