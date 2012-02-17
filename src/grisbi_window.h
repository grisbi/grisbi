#ifndef __GRISBI_WINDOW_H__
#define __GRISBI_WINDOW_H__

#include <gtk/gtk.h>

/*START_INCLUDE*/
/*END_INCLUDE*/



G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GRISBI_TYPE_WINDOW              (grisbi_window_get_type())
#define GRISBI_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GRISBI_TYPE_WINDOW, GrisbiWindow))
#define GRISBI_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GRISBI_TYPE_WINDOW, GrisbiWindowClass))
#define GRISBI_IS_WINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GRISBI_TYPE_WINDOW))
#define GRISBI_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GRISBI_TYPE_WINDOW))
#define GRISBI_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GRISBI_TYPE_WINDOW, GrisbitWindowClass))

/* Private structure type */
typedef struct _GrisbiWindowPrivate GrisbiWindowPrivate;


/* Main object structure */
typedef struct _GrisbiWindow GrisbiWindow;

struct _GrisbiWindow
{
    GtkWindow window;

    /*< private > */
    GrisbiWindowPrivate *priv;
};

/* Class definition */
typedef struct _GrisbiWindowClass GrisbiWindowClass;

struct _GrisbiWindowClass
{
    GtkWindowClass parent_class;
};


/* START_DECLARATION */
GType grisbi_window_get_type ( void ) G_GNUC_CONST;

GtkActionGroup *grisbi_window_get_action_group ( GrisbiWindow *window,
                        const gchar *action_group_name );
const gchar *grisbi_window_get_filename ( GrisbiWindow *window );
const gchar *grisbi_window_get_file_title ( GrisbiWindow *window );
GtkWidget *grisbi_window_get_headings_eb ( GrisbiWindow *window );
GtkWidget *grisbi_window_get_widget_by_name (  const gchar *name );
GtkUIManager *grisbi_window_get_ui_manager ( GrisbiWindow *window );
GtkWidget *grisbi_window_new_hpaned ( GrisbiWindow *window );
void grisbi_window_headings_update_label_markup ( gchar *label_name,
                        const gchar *text,
                        gboolean escape_text );
gboolean grisbi_window_set_filename ( GrisbiWindow *window,
                        const gchar *filename );
gboolean grisbi_window_set_file_title ( GrisbiWindow *window,
                        const gchar *file_title );
void grisbi_window_set_window_title ( GrisbiWindow *window,
                        const gchar *title );
void grisbi_window_statusbar_remove ( GrisbiWindow *window );
void grisbi_window_statusbar_push ( GrisbiWindow *window,
                        const gchar *msg );
/* END_DECLARATION */

G_END_DECLS

#endif  /* __GRISBI_APP_H__ */
