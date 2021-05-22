#ifndef __WIDGET_CMP_TEXT_H__
#define __WIDGET_CMP_TEXT_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_CMP_TEXT_TYPE		(widget_cmp_text_get_type ())
#define WIDGET_CMP_TEXT(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_CMP_TEXT_TYPE, WidgetCmpText))
#define WIDGET_IS_CMP_TEXT(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_CMP_TEXT_TYPE))

typedef struct _WidgetCmpText				WidgetCmpText;
typedef struct _WidgetCmpTextClass			WidgetCmpTextClass;


struct _WidgetCmpText
{
	GtkBox parent;
};

struct _WidgetCmpTextClass
{
	GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				widget_cmp_text_get_type		(void) G_GNUC_CONST;

WidgetCmpText *		widget_cmp_text_new						(GtkWidget *page,
															 gint text_cmp_number,
															 gboolean first_cmp_line);
gboolean			widget_cmp_text_get_data				(GtkWidget *widget,
															 gint text_cmp_number);
void				widget_cmp_text_init_data				(GtkWidget *widget,
															 gint text_cmp_number);
void				widget_cmp_text_show_button_add			(GtkWidget *widget,
															 gboolean show);
void				widget_cmp_text_hide_button_link		(GtkWidget *widget);
void				widget_cmp_text_show_button_remove		(GtkWidget *widget,
															 gboolean show);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_CMP_TEXT_H__ */
