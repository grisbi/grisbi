#ifndef __GRISBI_WINDOW_H__
#define __GRISBI_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GRISBI_TYPE_WINDOW              (grisbi_window_get_type())
#define GRISBI_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GRISBI_TYPE_WINDOW, GisbiWindow))
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


/* Public methods */

/* construction */
GType grisbi_window_get_type ( void ) G_GNUC_CONST;
GrisbiWindow *grisbi_window_create_window ( GrisbiWindow *window );


G_END_DECLS

#endif  /* __GRISBI_APP_H__ */
