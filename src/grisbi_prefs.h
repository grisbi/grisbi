#ifndef __GRISBI_PREFS_H__
#define __GRISBI_PREFS_H__

#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "grisbi_window.h"
#include "structures.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GRISBI_TYPE_PREFS              (grisbi_prefs_get_type())
#define GRISBI_PREFS(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GRISBI_TYPE_PREFS, GrisbiPrefs))
#define GRISBI_PREFS_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GRISBI_TYPE_PREFS, GrisbiPrefsClass))
#define GRISBI_IS_PREFS(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GRISBI_TYPE_PREFS))
#define GRISBI_IS_PREFS_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GRISBI_TYPE_PREFS))
#define GRISBI_PREFS_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GRISBI_TYPE_PREFS, GrisbitPrefsClass))

/* Private structure type */
typedef struct _GrisbiPrefsPrivate GrisbiPrefsPrivate;


/* Main object structure */
typedef struct _GrisbiPrefs GrisbiPrefs;

struct _GrisbiPrefs
{
    GtkDialog dialog;

    /*< private > */
    GrisbiPrefsPrivate *priv;
};


/* Class definition */
typedef struct _GrisbiPrefsClass GrisbiPrefsClass;

struct _GrisbiPrefsClass
{
    GtkDialogClass parent_class;
};


/* START_DECLARATION */
GType grisbi_prefs_get_type ( void ) G_GNUC_CONST;
GtkWidget *grisbi_prefs_get_widget_by_name ( const gchar *name );
void grisbi_prefs_refresh_preferences ( gboolean new_file );
void grisb_prefs_show_dialog ( GrisbiWindow *parent );
/* END_DECLARATION */

G_END_DECLS

#endif  /* __GRISBI_PREFS_H__ */
